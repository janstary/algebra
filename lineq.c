#include <stdlib.h>
#include <stdio.h>
#include <err.h>

#include "config.h"
#include "matrix.h"
#include "lineq.h"

/* Solve a system of linear equations given by a matrix.
 * The rightmost column is taken as the right hand vector.
 * Return a linsol structure (even if there is no solution),
 * or NULL on error. */
struct linsol*
linsolve(struct matrix *mtx)
{
	double R;
	long r, c, g;
	struct linsol* sol;
	if (NULL == mtx) {
		warnx("Will not solve a NULL equation");
		return NULL;
	}
	if (1 == mtx->cols) {
		warnx("One column is not enough");
		return NULL;
	}
	if (-1 == gem(mtx)) {
		warnx("Could not GEM");
		return NULL;
	}
	if (NULL == (sol = calloc(1, sizeof(struct linsol))))
		err(1, NULL);
	sol->len = mtx->cols-1;
	if (mtx->gcol >= mtx->cols)
		return sol;
	if (NULL == (sol->par = calloc(mtx->cols-1, sizeof(double))))
		err(1, NULL);
	/* fill the tail of the solution with zeros */
	for (c = mtx->cols-2; c >= mtx->gcol; c--)
		sol->par[c] = 0;
	for (r = mtx->rows-1; r >= 0; r--) {
		R = mtx->m[r][mtx->cols-1];
		for (c = r+1; c < mtx->cols-1; c++)
			R -= mtx->m[r][c] * sol->par[c];
		sol->par[r] = R / mtx->m[r][r];
		/* yes, the row is the column here */
	}
	if (mtx->cols - mtx->gcol <= 1)
		return sol;
	sol->dim = (mtx->cols - mtx->gcol) - 1;
	if (NULL == (sol->hom = calloc(sol->dim, sizeof(double*))))
		err(1, NULL);
	for (g = 0; g < sol->dim; g++) {
		if (NULL == (sol->hom[g] = calloc(sol->len, sizeof(double))))
			err(1, NULL);
		for (c = 0; c < sol->dim; c++) /* (...,0,1,0,...,0) */
			sol->hom[g][sol->len-c-1] = (c == g ? 1 : 0);
		/* same as before, with a zero right-hand side */
		/* FIXME: this should be a subroutine. */
		for (r = mtx->rows-1; r >= 0; r--) {
			R = 0;
			for (c = r+1; c < mtx->cols-1; c++)
				R -= mtx->m[r][c] * sol->hom[g][c];
			sol->hom[g][r] = R / mtx->m[r][r];
			/* yes, the row is the column here */
		}
	}
	return sol;
}

void
prvec(double* vec, long len)
{
	long c;
	if (NULL == vec)
		return;
	putchar('(');
	for (c = 0; c < len; c++) {
		if (c > 0)
			printf(", ");
		printf("%e", vec[c]);
	}
	putchar(')');
}

void
prsol(struct linsol* sol)
{
	long g;
	if (NULL == sol) {
		warnx("Will not print a NULL solution");
		return;
	}
	if (NULL == sol->par)
		return;
	prvec(sol->par, sol->len);
	if (0 == sol->dim) {
		putchar('\n');
		return;
	}
	printf(" + <");
	for (g = 0; g < sol->dim; g++) {
		if (g > 0)
			printf(", ");
		prvec(sol->hom[g], sol->len);
	}
	printf(">\n");
}
