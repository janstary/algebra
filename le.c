#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <err.h>

#include "config.h"
#include "matrix.h"
#include "lineq.h"

extern const char* __progname;

int vflag = 0;

static void
usage(void)
{
	fprintf(stderr,
		"usage: %s [-g] matrix\n", __progname);
}

int
main(int argc, char** argv)
{
	struct matrix *mtx;
	struct linsol *sol;
	int c;

	while ((c = getopt(argc, argv, "v")) != -1) switch (c) {
		case 'v':
			vflag = 1;
			break;
	}
	argc -= optind;
	argv += optind;

	if (1 != argc) {
		usage();
		return 1;
	}

	if (NULL == (mtx = readmtx(*argv))) {
		warnx("Cannot read matrix from '%s'", *argv);
		return 1;
	}

	if (vflag)
		prmtx(mtx);

	if (NULL == (sol = linsolve(mtx))) {
		warnx("Cannot solve equations");
		return -1;
	}

	prsol(sol);
	return 0;
}
