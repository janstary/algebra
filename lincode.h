#ifndef _ALGEBRA_LINCODE_H
#define _ALGEBRA_LINCODE_H

#include <stdlib.h>

struct lincode {
	long		size;
	struct word	*words;
	struct matrix	*genmtx;
	struct matrix	*ctlmtx;
};

#endif
