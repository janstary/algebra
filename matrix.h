#ifndef _ALGEBRA_MATRIX_H_
#define _ALGEBRA_MATRIX_H_

#include <stdlib.h>

#define ABS(x)   ((x) > 0 ? (x) : (-(x)))
#define MIN(x,y) (((x) < (y)) ? (x) : (y))
#define MAX(x,y) (((x) > (y)) ? (x) : (y))

struct matrix {
	long rows;
	long cols;
	long gcol;
	double **m;
};

struct matrix*	readmtx(const char*);
void		freemtx(struct matrix*);
void		prmtx(struct matrix*);
int		gem(struct matrix*);

#endif
