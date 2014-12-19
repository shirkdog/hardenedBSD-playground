/*-
 * Copyright 1996-1998 John D. Polstra.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "opt_pax.h"

#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/systm.h>
#include <sys/exec.h>
#include <sys/imgact.h>
#include <sys/linker.h>
#include <sys/sysent.h>
#include <sys/imgact_elf.h>
#include <sys/pax.h>
#include <sys/proc.h>
#include <sys/syscall.h>
#include <sys/signalvar.h>
#include <sys/vnode.h>

#include <vm/vm.h>
#include <vm/pmap.h>
#include <vm/vm_param.h>

#include <machine/elf.h>
#include <machine/md_var.h>

static boolean_t elf32_arm_abi_supported(struct image_params *);

const struct sysentvec elf32_freebsd_sysvec = {
	.sv_size	= SYS_MAXSYSCALL,
	.sv_table	= sysent,
	.sv_mask	= 0,
	.sv_sigsize	= 0,
	.sv_sigtbl	= NULL,
	.sv_errsize	= 0,
	.sv_errtbl	= NULL,
	.sv_transtrap	= NULL,
	.sv_fixup	= __elfN(freebsd_fixup),
	.sv_sendsig	= sendsig,
	.sv_sigcode	= sigcode,
	.sv_szsigcode	= &szsigcode,
	.sv_prepsyscall	= NULL,
	.sv_name	= "FreeBSD ELF32",
	.sv_coredump	= __elfN(coredump),
	.sv_imgact_try	= NULL,
	.sv_minsigstksz	= MINSIGSTKSZ,
	.sv_pagesize	= PAGE_SIZE,
	.sv_minuser	= VM_MIN_ADDRESS,
	.sv_maxuser	= VM_MAXUSER_ADDRESS,
	.sv_usrstack	= USRSTACK,
	.sv_psstrings	= PS_STRINGS,
	.sv_stackprot	= VM_PROT_ALL,
	.sv_copyout_strings = exec_copyout_strings,
	.sv_setregs	= exec_setregs,
	.sv_fixlimit	= NULL,
	.sv_maxssiz	= NULL,
	.sv_flags	= SV_ABI_FREEBSD | SV_ILP32,
	.sv_set_syscall_retval = cpu_set_syscall_retval,
	.sv_fetch_syscall_args = cpu_fetch_syscall_args,
	.sv_syscallnames = syscallnames,
	.sv_schedtail	= NULL,
#ifdef PAX_ASLR
	.sv_pax_aslr_init = pax_aslr_init_vmspace,
#endif
};

static Elf32_Brandinfo freebsd_brand_info = {
	.brand		= ELFOSABI_FREEBSD,
	.machine	= EM_ARM,
	.compat_3_brand	= "FreeBSD",
	.emul_path	= NULL,
	.interp_path	= "/libexec/ld-elf.so.1",
	.sysvec		= &elf32_freebsd_sysvec,
	.interp_newpath	= NULL,
	.brand_note	= &elf32_freebsd_brandnote,
	.flags		= BI_CAN_EXEC_DYN | BI_BRAND_NOTE,
	.header_supported= elf32_arm_abi_supported,
};

SYSINIT(elf32, SI_SUB_EXEC, SI_ORDER_FIRST,
	(sysinit_cfunc_t) elf32_insert_brand_entry,
	&freebsd_brand_info);

static boolean_t
elf32_arm_abi_supported(struct image_params *imgp)
{
	const Elf_Ehdr *hdr = (const Elf_Ehdr *)imgp->image_header;

#ifdef __ARM_EABI__
	/*
	 * When configured for EABI, FreeBSD supports EABI vesions 4 and 5.
	 */
	if (EF_ARM_EABI_VERSION(hdr->e_flags) < EF_ARM_EABI_FREEBSD_MIN) {
		if (bootverbose)
			uprintf("Attempting to execute non EABI binary (rev %d) image %s",
			    EF_ARM_EABI_VERSION(hdr->e_flags), imgp->args->fname);
		return (FALSE);
	}
#else
	/*
	 * When configured for OABI, that's all we do, so reject EABI binaries.
	 */
	if (EF_ARM_EABI_VERSION(hdr->e_flags) != EF_ARM_EABI_VERSION_UNKNOWN) {
		if (bootverbose)
			uprintf("Attempting to execute EABI binary (rev %d) image %s",
			    EF_ARM_EABI_VERSION(hdr->e_flags), imgp->args->fname);
		return (FALSE);
	}
#endif
	return (TRUE);
}

void
elf32_dump_thread(struct thread *td __unused, void *dst __unused,
    size_t *off __unused)
{
}

/*
 * It is possible for the compiler to emit relocations for unaligned data.
 * We handle this situation with these inlines.
 */
#define	RELOC_ALIGNED_P(x) \
	(((uintptr_t)(x) & (sizeof(void *) - 1)) == 0)

static __inline Elf_Addr
load_ptr(Elf_Addr *where)
{
	Elf_Addr res;

	if (RELOC_ALIGNED_P(where))
		return *where;
	memcpy(&res, where, sizeof(res));
	return (res);
}

static __inline void
store_ptr(Elf_Addr *where, Elf_Addr val)
{
	if (RELOC_ALIGNED_P(where))
		*where = val;
	else
		memcpy(where, &val, sizeof(val));
}
#undef RELOC_ALIGNED_P


/* Process one elf relocation with addend. */
static int
elf_reloc_internal(linker_file_t lf, Elf_Addr relocbase, const void *data,
    int type, int local, elf_lookup_fn lookup)
{
	Elf_Addr *where;
	Elf_Addr addr;
	Elf_Addr addend;
	Elf_Word rtype, symidx;
	const Elf_Rel *rel;
	const Elf_Rela *rela;

	switch (type) {
	case ELF_RELOC_REL:
		rel = (const Elf_Rel *)data;
		where = (Elf_Addr *) (relocbase + rel->r_offset);
		addend = load_ptr(where);
		rtype = ELF_R_TYPE(rel->r_info);
		symidx = ELF_R_SYM(rel->r_info);
		break;
	case ELF_RELOC_RELA:
		rela = (const Elf_Rela *)data;
		where = (Elf_Addr *) (relocbase + rela->r_offset);
		addend = rela->r_addend;
		rtype = ELF_R_TYPE(rela->r_info);
		symidx = ELF_R_SYM(rela->r_info);
		break;
	default:
		panic("unknown reloc type %d\n", type);
	}

	if (local) {
		if (rtype == R_ARM_RELATIVE) {	/* A + B */
			addr = elf_relocaddr(lf, relocbase + addend);
			if (load_ptr(where) != addr)
				store_ptr(where, addr);
		}
		return (0);
	}

	switch (rtype) {

		case R_ARM_NONE:	/* none */
			break;

		case R_ARM_ABS32:
			addr = lookup(lf, symidx, 1);
			if (addr == 0)
				return -1;
			store_ptr(where, addr + load_ptr(where));
			break;

		case R_ARM_COPY:	/* none */
			/*
			 * There shouldn't be copy relocations in kernel
			 * objects.
			 */
			printf("kldload: unexpected R_COPY relocation\n");
			return -1;
			break;

		case R_ARM_JUMP_SLOT:
			addr = lookup(lf, symidx, 1);
			if (addr) {
				store_ptr(where, addr);
				return (0);
			}
			return (-1);
		case R_ARM_RELATIVE:
			break;

		default:
			printf("kldload: unexpected relocation type %d\n",
			       rtype);
			return -1;
	}
	return(0);
}

int
elf_reloc(linker_file_t lf, Elf_Addr relocbase, const void *data, int type,
    elf_lookup_fn lookup)
{

	return (elf_reloc_internal(lf, relocbase, data, type, 0, lookup));
}

int
elf_reloc_local(linker_file_t lf, Elf_Addr relocbase, const void *data,
    int type, elf_lookup_fn lookup)
{

	return (elf_reloc_internal(lf, relocbase, data, type, 1, lookup));
}

int
elf_cpu_load_file(linker_file_t lf __unused)
{

	/*
	 * The pmap code does not do an icache sync upon establishing executable
	 * mappings in the kernel pmap.  It's an optimization based on the fact
	 * that kernel memory allocations always have EXECUTABLE protection even
	 * when the memory isn't going to hold executable code.  The only time
	 * kernel memory holding instructions does need a sync is after loading
	 * a kernel module, and that's when this function gets called.  Normal
	 * data cache maintenance has already been done by the IO code, and TLB
	 * maintenance has been done by the pmap code, so all we have to do here
	 * is invalidate the instruction cache (which also invalidates the
	 * branch predictor cache on platforms that have one).
	 */
	cpu_icache_sync_all();
	return (0);
}

int
elf_cpu_unload_file(linker_file_t lf __unused)
{

	return (0);
}
