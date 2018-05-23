/* TODO use dlopen() to compare ourselves to a given function.c */

#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <err.h>

#include "matrix.h"
#include "lineq.h"

int dflag;
int nflag;
int vflag;
int degree = 1;

extern char* __progname;

struct data {
	long num;
	struct pt {
		double x, y;
	}	*points;
};

static void
usage()
{
	fprintf(stderr,
		"%s [-D degree] [-d] [-n] [-v] data\n"
		"%s [-D degree] [-d] [-n] [-v] function.so args\n"
		"%s [-D degree] [-d] [-n] [-v] function.so hi lo step\n",
		__progname, __progname, __progname);
}

void
prdata(struct data *data)
{
	long n;
	struct pt *p;
	if (NULL == data || 0 == data->num || NULL == data->points)
		return;
	for (n = 0, p = data->points; n < data->num; n++, p++)
		printf("% e % e\n", p->x, p->y);
}

int
rdata(FILE *fp, struct data *data)
{
	struct pt p;
	if (NULL == data)
		return -1;
	while (fscanf(fp, "%le %le\n", &p.x, &p.y) == 2) {
		if (NULL == (data->points
		= reallocarray(data->points, data->num+1, sizeof(struct pt))))
			err(1, NULL);
		data->points[data->num++] = p;
	}
	if (ferror(fp))
		err(1, NULL);
	return 0;
}

struct matrix*
mkmtx(struct data *data, int degree)
{
	struct pt *p;
	long r, c, n;
	struct matrix *mtx;
	if (NULL == data || 0 == data->num || degree < 1)
		return NULL;
	if (NULL == (mtx = calloc(1, sizeof(struct matrix))))
		err(1, NULL);
	mtx->rows = degree + 1;
	mtx->cols = degree + 2;
	if (NULL == (mtx->m = calloc(mtx->rows, sizeof(double*))))
		err(1, NULL);
	/* the linear combinations */
	for (r = 0; r < mtx->rows; r++) {
		if (NULL == (mtx->m[r] = calloc(mtx->cols, sizeof(double))))
			err(1, NULL);
		for (c = 0; c < mtx->cols-1; c++) {
			for (n = 0, p = data->points; n < data->num; n++, p++) {
				mtx->m[r][c] += pow(p->x, r+c);
			}
		}
	}
	/* the right hand side */
	for (r = 0; r < mtx->rows; r++)
		for (n = 0, p = data->points; n < data->num; n++, p++)
			mtx->m[r][c] += pow(p->x, r) * p->y;
	return mtx;
}

double
eval(double x, double *coef, long len)
{
	long p;
	double *c, val = 0;
	/* FIXME: square and multiply */
	for (p = 0, c = coef; p < len; p++, c++)
		val += (*c) * pow(x, p);
	return val;
}

/* TODO: display the diffs too */
/* TODO: save the sum of squared diffs in a given double */
int
approx(struct data *data, double *coef, long len)
{
	long n;
	struct pt *p;
	double d, e = 0, val;
	if (NULL == data || NULL == coef || 0 == len)
		return -1;
	for (n = 0, p = data->points; n < data->num; n++, p++) {
		val = eval(p->x, coef, len);
		if (dflag && vflag) {
			d = val - p->y;
			e += d * d;
			printf("% e % e % e % e\n", p->x, val, p->y, d);
		} else {
			printf("% e %e\n", p->x, val);
		}
	}
	return 0;
}

int
main(int argc, char** argv)
{
	int c;
	FILE *fp;
	struct data *data;
	struct matrix *mtx;
	struct linsol *sol;
	double x;

	while ((c = getopt(argc, argv, "D:dnv")) != -1) switch (c) {
		case 'D':
			degree = atoi(optarg);
			/* FIXME strtonum */
			break;
		case 'd':
			dflag = 1;
			break;
		case 'n':
			nflag = 1;
			vflag = 1;
			break;
		case 'v':
			vflag = 1;
			break;
	}
	argc -= optind;
	argv += optind;

	if (argc != 1) {
		usage();
		return 1;
	}

	if (degree < 1) {
		usage();
		return 1;
	}

	if (NULL == (fp = fopen(*argv, "r"))) {
		warnx("Cannot open '%s'", *argv);
		return 1;
	}
	if (NULL == (data = calloc(1, sizeof(struct data))))
		err(1, NULL);
	if (-1 == rdata(fp, data)) {
		warnx("Cannot read data from '%s'", *argv);
		return 1;
	}
	/*prdata(data);*/

	if (data->num <= degree) {
		/* This will result in a singular matrix
		 * TODO: show those non-unique polynomials? */
		warnx("%zu points for degree %d", data->num, degree);
	}

	if (NULL == (mtx = mkmtx(data, degree))) {
		warnx("Cannot figure out matrix from data");
		return 1;
	}
	/*prmtx(mtx);*/

	if (-1 == gem(mtx)) {
		warnx("Cannot GEM the optimization matrix");
		return 1;
	}
	/*prmtx(mtx);*/

	if (NULL == (sol = linsolve(mtx))) {
		warnx("Cannot solve linear equations");
		return 1;
	}
	/*prsol(sol);*/

	if (vflag)
		approx(data, sol->par, sol->len);

	/* read further arguments from stdin */
	if (nflag)
		return 0;
	while (1 == (c = fscanf(stdin, "%le", &x)))
		printf("% e % e\n", x, eval(x, sol->par, sol->len));
	return feof(stdin) ? 0 : 1;
}
