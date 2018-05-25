# Copyright (c) 2018 Jan Stary <hans@stare.cz>
#
# Permission to use, copy, modify, and distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

VERSION = 1.0
TARBALL = algebra-$(VERSION).tar.gz

SRCS =			\
	lc.c		\
	le.c		\
	lincode.c	\
	lincode.h	\
	lineq.c		\
	lineq.h		\
	lsq.c		\
	matrix.c	\
	matrix.h

HAVE_SRCS =	have-err.c have-reallocarray.c have-strtonum.c
COMPAT_SRCS =	compat-err.c compat-reallocarray.c compat-strtonum.c
COMPAT_OBJS =	compat-err.o compat-reallocarray.o compat-strtonum.o

lc_OBJS =	lc.o lincode.o matrix.o
le_OBJS =	le.o lineq.o matrix.o
lsq_OBJS =	lsq.o lineq.o matrix.o
OBJS =		$(lc_OBJS) $(le_OBJS) $(lsq_OBJS) $(COMPAT_OBJS)

PROG =	lc le lsq
BINS =	$(PROG) lsqdiff
MAN1 =	lc.1 le.1 lsq.1

EXAMPLES = \
	example-data-exp	\
	example-data-log	\
	example-data-sin	\
	example-data-trivial	\
	example-function-square.c\
	example-matrix-3x4	\
	example-matrix-identity	\
	example-matrix-lincode	\
	example-matrix-manysol	\
	example-matrix-oneline	\
	example-matrix-onesol	\
	example-matrix-regular	\
	example-matrix-singular	\
	example-matrix-zeros

DISTFILES = \
	LICENSE			\
	Makefile		\
	Makefile.depend		\
	configure		\
	configure.local.example	\
	$(MAN1)			\
	$(SRCS)			\
	$(HAVE_SRCS)		\
	$(COMPAT_SRCS)		\
	$(EXAMPLES)		\
	TODO

include Makefile.local

all: $(BINS)

lint: $(MAN1)
	mandoc -Tlint -Wstyle $(MAN1)

install: $(BINS) $(MAN1)
	install -d $(BINDIR)      && install -m 0755 $(BINS) $(BINDIR)
	install -d $(MANDIR)/man1 && install -m 0444 $(MAN1) $(MANDIR)/man1

uninstall:
	cd $(BINDIR)      && rm $(BINS)
	cd $(MANDIR)/man1 && rm $(MAN1)

example: install
	lsqdiff diff-sin-1.png  -D1    example-data-sin
	lsqdiff diff-sin-2.png  -D2    example-data-sin
	lsqdiff diff-sin-3.png  -D3    example-data-sin
	lsqdiff diff-sin-4.png  -D4    example-data-sin
	lsqdiff diff-sin-1w.png -D1 -w example-data-sin
	lsqdiff diff-sin-2w.png -D2 -w example-data-sin
	lsqdiff diff-sin-3w.png -D3 -w example-data-sin
	lsqdiff diff-sin-4w.png -D4 -w example-data-sin
	lsqdiff diff-exp-1.png  -D1    example-data-exp
	lsqdiff diff-exp-2.png  -D2    example-data-exp
	lsqdiff diff-exp-3.png  -D3    example-data-exp
	lsqdiff diff-exp-4.png  -D4    example-data-exp
	lsqdiff diff-exp-1w.png -D1 -w example-data-exp
	lsqdiff diff-exp-2w.png -D2 -w example-data-exp
	lsqdiff diff-exp-3w.png -D3 -w example-data-exp
	lsqdiff diff-exp-4w.png -D4 -w example-data-exp
	lsqdiff diff-log-1.png  -D1    example-data-log
	lsqdiff diff-log-2.png  -D2    example-data-log
	lsqdiff diff-log-3.png  -D3    example-data-log
	lsqdiff diff-log-4.png  -D4    example-data-log
	lsqdiff diff-log-1w.png -D1 -w example-data-log
	lsqdiff diff-log-2w.png -D2 -w example-data-log
	lsqdiff diff-log-3w.png -D3 -w example-data-log
	lsqdiff diff-log-4w.png -D4 -w example-data-log

clean:
	rm -f $(PROG) $(OBJS)
	rm -rf $(TARBALL) algebra-$(VERSION)
	rm -rf diff*.png *.dSYM *.core *~ .*~

distclean: clean
	rm -f Makefile.local config.h config.h.old config.log config.log.old

lc: $(lc_OBJS) $(COMPAT_OBJS)
	$(CC) $(CFLAGS) -o $@ $(lc_OBJS) $(COMPAT_OBJS)

le: $(le_OBJS) $(COMPAT_OBJS)
	$(CC) $(CFLAGS) -o $@ $(le_OBJS) $(COMPAT_OBJS)

lsq: $(lsq_OBJS) $(COMPAT_OBJS)
	$(CC) $(CFLAGS) -o $@ $(lsq_OBJS) $(COMPAT_OBJS) -lm

dist: $(TARBALL)

$(TARBALL): $(DISTFILES)
	rm -rf .dist
	mkdir -p .dist/algebra-$(VERSION)/
	$(INSTALL) -m 0644 $(DISTFILES) .dist/algebra-$(VERSION)/
	( cd .dist && tar czf ../$@ algebra-$(VERSION) )
	rm -rf .dist/

Makefile.local config.h: configure $(HAVESRCS)
	@echo "$@ is out of date; please run ./configure"
	@exit 1

depend: config.h
	mkdep -f depend $(CFLAGS) $(SRCS)
	perl -e 'undef $$/; $$_ = <>; s|/usr/include/\S+||g; \
		s|\\\n||g; s|  +| |g; s| $$||mg; print;' \
		depend > _depend
	mv _depend depend

include Makefile.depend

.SUFFIXES: .c .o

.c.o:
	$(CC) $(CFLAGS) -c $<

