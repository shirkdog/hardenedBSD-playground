#	$OpenBSD: Makefile,v 1.16 2015/11/20 18:53:42 tedu Exp $

OPENNTPD_SRC=	${.CURDIR}/../../../contrib/openntpd/src

PROG=	ntpd
# compat
SRCS=	imsg.c imsg-buffer.c adjfreq_freebsd.c progname.c setproctitle.c
# Imported from OpenBSD src makefile
SRCS+=	ntpd.c log.c ntp.c ntp_msg.c parse.y config.c \
	server.c client.c fake-sensors.c util.c ntp_dns.c \
	control.c constraint.c
CFLAGS+= -Wall -I${.CURDIR}
CFLAGS+= -fstack-protector-all
CFLAGS+= -Wstrict-prototypes -Wmissing-prototypes
CFLAGS+= -Wmissing-declarations
CFLAGS+= -Wshadow -Wpointer-arith -Wcast-qual
CFLAGS+= -Wsign-compare
# Add includes and defines
CFLAGS+= -I${OPENNTPD_SRC}
CFLAGS+= -I${OPENNTPD_SRC}/../include
CFLAGS+= -DHAVE_CLOSEFROM=1
CFLAGS+= -DHAVE_LIBTLS=yes

LIBADD+=	m md util tls ssl crypto
LINKS=	${BINDIR}/ntpd ${BINDIR}/ntpctl
MAN=	ntpd.8 ntpd.conf.5 ntpctl.8

.PATH:	${OPENNTPD_SRC}/../compat ${OPENNTPD_SRC}

.include <bsd.prog.mk>
