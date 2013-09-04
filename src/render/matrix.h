/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#ifndef DD_RENDER_MATRIX_H_INCLUDED
#define DD_RENDER_MATRIX_H_INCLUDED

#include "../dd.h"
#include "rect.h"

typedef struct _ddMatrix ddMatrix;

/* if the shape's scaled or moved, we just need to recalc the vertexlist.
if it's rotated, we have to rebuild it */

typedef enum
{
	MATRIX_UPTODATE  = 0,
	MATRIX_MOVED     = (1<<0), /* only have to add diff to vertlist */
	MATRIX_SCALED    = (1<<1), /* have to recalc coords, but not slopes */
	MATRIX_STRETCHED = (1<<2), /* have to recalc slopes */
	MATRIX_MUNGED    = (1<<3)  /* have to reorder (rebuild) the vertlist */
} ddMatrixState;


/*
 |a b||x|   |dx|
 |c d||y| + |dy|
 */

/* XXX - abcd should have more digits after the decimal! */

struct _ddMatrix
{
	fixed a;
	fixed b;
	fixed c;
	fixed d;
	fixed dx;
	fixed dy;
};

extern const ddMatrix ddEmptyMatrix;
extern const ddMatrix ddIdentityMatrix;

#define DDMATRIX_ISIDENTITY(m) \
  ((m).a == ddIdentityMatrix.a && (m).b == ddIdentityMatrix.b && \
   (m).c == ddIdentityMatrix.c && (m).d == ddIdentityMatrix.d && \
   (m).a == ddIdentityMatrix.dx && (m).dy == ddIdentityMatrix.dy )
   
ddMatrix
dd_makeMatrix(fixed a, fixed b, fixed c, fixed d, fixed dx, fixed dy);

ddMatrix
dd_makeTranslationMatrix(fixed dx, fixed dy);

ddMatrix
dd_makeScalingMatrix(fixed xScale, fixed yScale);

ddMatrix
dd_makeRotationMatrix(double radians);

ddMatrix
ddMatrix_inverse(ddMatrix m);

ddMatrix
ddMatrix_multiply(ddMatrix m1, ddMatrix m2);

ddMatrix
ddMatrix_scale(ddMatrix m, fixed xScale, fixed yScale);

ddMatrix
ddMatrix_rotate(ddMatrix m, double radians);

ddMatrix
ddMatrix_translate(ddMatrix m, fixed dx, fixed dy);

ddMatrix
ddMatrix_invert(ddMatrix m);

ddMatrix
ddMatrix_interpolate(ddMatrix m1, ddMatrix m2, UInt16 ratio);

void
ddMatrix_apply(ddMatrix m, fixed *x, fixed *y);

ddRect
ddMatrix_applyRect(ddMatrix m, ddRect rect);

ddMatrixState
ddMatrix_compare(ddMatrix* m1, ddMatrix* m2);

boolean
ddMatrix_isFlipped(ddMatrix m);

#endif /* DD_MATRIX_H_INCLUDED */
