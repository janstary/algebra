#include <unistd.h>
#include <stdio.h>
#include <err.h>

#include "config.h"
#include "matrix.h"
#include "lincode.h"

extern const char* __progname;

int cflag = 0;
int Cflag = 0;
int gflag = 0;
int Gflag = 0;
int vflag = 0;

static void
usage(void)
{
	fprintf(stderr,
		"usage: %s [-c] [-g] [-v] code [file ...]\n", __progname);
}

int
main(int argc, char** argv)
{
	struct matrix *mtx;
	struct lincode *lc;
	int c;

	while ((c = getopt(argc, argv, "cCgGv")) != -1) switch (c) {
		case 'c':
			cflag = 1;
			break;
		case 'C':
			Cflag = 1;
			break;
		case 'g':
			gflag = 1;
			break;
		case 'G':
			Gflag = 1;
			break;
		case 'v':
			vflag = 1;
			break;
	}
	argc -= optind;
	argv += optind;

	if (argc == 0) {
		usage();
		return 1;
	}

	if (NULL == (mtx = readmtx(*argv))) {
		warnx("Cannot read matrix from '%s'", *argv);
		return 1;
	}

	if (vflag)
		prmtx(mtx);

	if (NULL == (lc = calloc(1, sizeof(struct lincode))))
		err(1, NULL);
	if (cflag)
		lc->ctlmtx = mtx;
	else
		lc->genmtx = mtx;

	if (-1 == gem(mtx)) {
		warnx("Cannot GEM the matrix");
		return -1;
	}

	if (vflag) {
		putchar('\n');
		prmtx(mtx);
	}

	return 0;
}
