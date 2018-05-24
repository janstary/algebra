#ifndef _ALGEBRA_MATRIX_H_
#define _ALGEBRA_MATRIX_H_

#include <stdlib.h>

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
