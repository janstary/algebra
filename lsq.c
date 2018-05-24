/* TODO use dlopen() to compare ourselves to a given function.c */

#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <err.h>

#include "config.h"
#include "matrix.h"
#include "lineq.h"

int dflag;
int nflag;
int vflag;
int wflag;
int degree = 1;

extern char* __progname;

struct data {
	long num;
	struct pt {
		double x, y;
	}	*points;
};

static void
usage(void)
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

/* Evaluate a given polynomial at a given point. */
/* FIXME: Horner? Square-and-Multiply? */
double
eval(double *coef, long len, double x)
{
	long p;
	double *c, val = 0;
	for (p = 0, c = coef; p < len; p++, c++)
		val += (*c) * pow(x, p);
	return val;
}

/* The weight function: 1-x, cut of to zero at x=1.
 * TODO: also have a weight function suited at point x,
 * according to the density of neighbouring data points. */
/* TODO: cut off to zero for > epsilon */
double
weight(double x)
{
	if (x < 0 || x > 1)
		return 0;
	return exp(-x);
}

/* Prepare the optimization matrix weighted at point x
 * whose solution is the degree-tuple of the wlsq coeficients.
 * It the weight function is NULL, make it a constant 1.
 * Return the composed matrix, or NULL on error. */
struct matrix*
mkmtx(struct data *data, int degree, double(*w)(double), double x)
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
				mtx->m[r][c] += pow(p->x, r+c)
					* (w ? w(fabs(x - p->x)) : 1);
			}
		}
	}
	/* the right hand side */
	for (r = 0; r < mtx->rows; r++)
		for (n = 0, p = data->points; n < data->num; n++, p++)
			mtx->m[r][c] += pow(p->x, r) * p->y
				* (w ? w(fabs(x - p->x)) : 1);
	return mtx;
}

/* Approximate the original data with a polynomial. */
int
approx(struct data *data, double *coef, long len)
{
	long n;
	struct pt *p;
	double val;
	if (NULL == data || NULL == coef || 0 == len)
		return -1;
	for (n = 0, p = data->points; n < data->num; n++, p++) {
		val = eval(coef, len, p->x);
		if (dflag) {
			printf("% e % e % e % e\n", p->x, val, p->y, val-p->y);
		} else {
			printf("% e % e\n", p->x, val);
		}
	}
	return 0;
}


/* Compose and solve the set of linear equations
 * leading to the best polynomial to use at the given point.
 * Return the solution, or NULL on error. */
struct linsol*
wsol(struct data *data, int degree, double(*w)(double), double x)
{
	struct matrix *mtx;
	struct linsol *sol;
	if (NULL == data || NULL == w || degree < 1)
		return NULL;
	if (NULL == (mtx = mkmtx(data, degree, weight, x))) {
		warnx("Cannot figure out matrix at %e", x);
		return NULL;
	}
	if (NULL == (sol = linsolve(mtx))) {
		warnx("Cannot solve equations for %e", x);
		freemtx(mtx);
		return NULL;
	}
	freemtx(mtx);
	return sol;
}

/* Approximate the original data with polynomials,
 * using a specific polynomial at each point. */
int
wapprox(struct data *data)
{
	long n;
	struct pt *p;
	struct linsol *sol;
	double val;
	if (NULL == data)
		return -1;
	for (n = 0, p = data->points; n < data->num; n++, p++) {
		if (NULL == (sol = wsol(data, degree, weight, p->x))) {
			warnx("Cannot solve equations at %e", p->x);
			return -1;
		}
		val = eval(sol->par, sol->len, p->x);
		if (dflag && vflag) {
			printf("% e % e % e % e\n", p->x, val, p->y, val-p->y);
		} else {
			printf("% e % e\n", p->x, val);
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

	while ((c = getopt(argc, argv, "D:dnvw")) != -1) switch (c) {
		case 'D':
			degree = atoi(optarg);
			/* FIXME strtonum */
			break;
		case 'd':
			dflag = 1;
			vflag = 1;
			break;
		case 'n':
			nflag = 1;
			vflag = 1;
			break;
		case 'v':
			vflag = 1;
			break;
		case 'w':
			wflag = 1;
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

	if (data->num <= degree) {
		/* This will result in a singular matrix
		 * TODO: show those non-unique polynomials? */
		warnx("%zu points for degree %d", data->num, degree);
	}

	if (wflag) {
		/* weighted least-square regression */
		if (vflag)
			wapprox(data);
		if (nflag)
			return 0;
		while (1 == (c = fscanf(stdin, "%le", &x))) {
			if (NULL == (sol = wsol(data, degree, weight, x))) {
				warnx("Cannot solve equations for %e", x);
				continue;
			}
			printf("% e % e\n", x, eval(sol->par, sol->len, x));
			freesol(sol);
		}
		return feof(stdin) ? 0 : 1;
	} else {
		/* simple least-square regression */
		if (NULL == (mtx = mkmtx(data, degree, NULL, 0))) {
			warnx("Cannot figure out matrix from data");
			return 1;
		}
		if (NULL == (sol = linsolve(mtx))) {
			warnx("Cannot solve linear equations");
			return 1;
		}
		/*prsol(sol);*/
		freemtx(mtx);
		if (vflag)
			approx(data, sol->par, sol->len);
		if (nflag)
			return 0;
		while (1 == (c = fscanf(stdin, "%le", &x)))
			printf("% e % e\n", x, eval(sol->par, sol->len, x));
		freesol(sol);
		return feof(stdin) ? 0 : 1;
	}
	return 0;
}
