#ifndef _ALGEBRA_LINEQ_H_
#define _ALGEBRA_LINEQ_H_

#include "matrix.h"

struct linsol {
	long		len; /* length of vectors: R^n */
	long		dim; /* dimension of the hom solution */
	double*		par; /* a particular solution */
	double**	hom; /* a list of generators */
};

struct linsol*	linsolve(struct matrix*);
void		prsol(struct linsol*);

#endif
