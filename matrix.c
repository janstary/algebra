#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdio.h>
#include <ctype.h>
#include <err.h>

#include "config.h"
#include "matrix.h"

/* Parse a string of numbers separated by whitespace and save it in an array.
 * Return the size of the array, or -1 for error. The returned array gets
 * allocated here; it is the caller's responsibility to free it later. */
long
mkrow(char *line, double **row)
{
	char *n, *e;
	long i = 0;
	*row = NULL;
	while ((n = strsep(&line, " \t\n"))) {
		if (*n == '\0')
			continue;
		if (NULL == (*row = reallocarray(*row, i+1, sizeof(double))))
			err(1, NULL);
		e = NULL;
		(*row)[i++] = strtod(n, &e);
		if (e && isprint(*e)) {
			warnx("Cannot read '%s' at '%s'", n, e);
			free(*row);
			return -1;
		}
	}
	return i;
}

/* Add a row of numbers to a given matrix.
 * The rows needs to have the same number of columns as the previous rows.
 * Return 0 for success, -1 for error. */
int
addrow(double *row, long cols, struct matrix *mtx)
{
	double **new = NULL;
	if (NULL == mtx || NULL == row)
		return -1;
	if (mtx->cols && mtx->cols != cols) {
		warnx("Row has %zu != %zu cols", cols, mtx->cols);
		return -1;
	}
	if (NULL == (new = reallocarray(mtx->m, mtx->rows+1, sizeof(double*))))
		err(1, NULL);
	mtx->m = new;
	mtx->m[mtx->rows++] = row;
	if (0 == mtx->cols)
		mtx->cols = cols;
	return 0;
}

void
freemtx(struct matrix *mtx)
{
	if (mtx) {
		free(mtx->m);
	}
}

void
prmtx(struct matrix *mtx)
{
	long i, j;
	if (NULL == mtx || 0 == mtx->rows || 0 == mtx->cols)
		return;
	for (i = 0; i < mtx->rows; i++) {
		for (j = 0; j < mtx->cols; j++)
			printf("% e ", mtx->m[i][j]);
		putchar('\n');
	}
}

struct matrix*
readmtx(const char* file)
{
	FILE *fp;
	double *row = NULL;
	struct matrix *mtx = NULL;
	char *line = NULL, *p, *q;
	long len, cols;
	size_t size = 0;

	if (NULL == file)
		return NULL;
	if (NULL == (fp = fopen(file, "r"))) {
		warnx("Cannot open '%s'", file);
		return NULL;
	}
	if (NULL == (mtx = calloc(1, sizeof(struct matrix))))
		err(1, NULL);
	while ((len = getline(&line, &size, fp)) != -1) {
		if (0 == --len)
			continue;
		line[len] = '\0';
		p = q = strdup(line);
		if (-1 == (cols = mkrow(q, &row))) {
			warnx("Cannot parse matrix row: '%s'", line);
			goto bad;
		}
		if (-1 == addrow(row, cols, mtx)) {
			warnx("Cannot add row %zu: '%s'", mtx->rows, line);
			goto bad;
		}
	}
	free(p);
	free(line);
	fclose(fp);
	return mtx;
bad:
	free(p);
	free(line);
	freemtx(mtx);
	fclose(fp);
	return NULL;
}

long
nulcols(double* row, long cols)
{
	long c, z = 0;
	if (NULL == row || 0 == cols)
		return 0;
	for (c = 0; c < cols; c++)
		if (0 == *row)
			z++;
	return z;

}

/* Perform the Gaussian Elimination on a given matrix.
 * Fill in the column index where linear dependency starts.
 * Return 0 on success, -1 on error.
 * NB: this _rewrites_ the matrix. */
int
gem(struct matrix* mtx)
{
	double minabs, *m, *A, *B, a, b;
	long c, r, j, z, maxcol, minrow, maxnul;
	if (NULL == mtx || 0 == mtx->rows || 0 == mtx->cols)
		return -1;
	if (1 == mtx->rows)
		return 0;
	if (NULL == (m = calloc(mtx->cols, sizeof(double))))
		err(1, NULL);
	/* go through all columns, see if they need geming. */
	for (c = 0, maxcol = MIN(mtx->cols, mtx->rows); c < maxcol; c++) {
		/* find the row with the smallest nonzero lead. */
		for (r = c, minabs = -1, maxnul = 0; r < mtx->rows; r++) {
			if (0 == mtx->m[r][c])
				continue;
			if ((minabs < 0)
			|| (ABS(mtx->m[r][c]) < minabs)) {
				minabs = mtx->m[r][c];
				minabs = ABS(minabs);
				minrow = r;
			} else if ((ABS(mtx->m[r][c]) == minabs)
			&& ((z = nulcols(mtx->m[r], mtx->cols)) > maxnul)) {
				minrow = r;
				maxnul = z;
			}
		}
		if (minabs < 0)
			goto elim;
		A = mtx->m[minrow];
		/* combine the other rows appropriately, zeroing their lead */
		for (r = c; r < mtx->rows; r++) {
			if (r == minrow)
				continue;
			B = mtx->m[r];
			a = A[c];
			b = B[c];
			B[c] = 0;
			for (j = c+1; j < mtx->cols; j++)
				B[j] = a * B[j] - b * A[j];
		}
		/* make the minimal row the first row */
		if (minrow != c) {
			for (j = 0; j < mtx->cols; j++) {
				m[j] = A[j];
				A[j] = mtx->m[c][j];
				mtx->m[c][j] = m[j];
			}
		}
	}
elim:
	/* delete the null rows */
	for (r = mtx->rows-1; r >= c; r--) {
		if (nulcols(mtx->m[r], mtx->cols) == mtx->cols) {
			free(mtx->m[r]);
			mtx->rows--;
		}
	}
	mtx->gcol = c;
	return 0;
}
