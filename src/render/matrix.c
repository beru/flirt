/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#include <math.h>

#include "matrix.h"

const ddMatrix ddEmptyMatrix = { 0, 0, 0, 0, 0, 0 };
const ddMatrix ddIdentityMatrix = { FIXED_I(1), 0, 0, FIXED_I(1), 0, 0 };

ddMatrix inline
_dd_newMatrix(fixed a, fixed b, fixed c, fixed d, fixed dx, fixed dy)
{
	ddMatrix m;
	m.a = a;
	m.b = b;
	m.c = c;
	m.d = d;
	m.dx = dx;
	m.dy = dy;
	return m;
}


ddMatrix
dd_makeMatrix(fixed a, fixed b, fixed c, fixed d, fixed dx, fixed dy)
{
	return _dd_newMatrix(a, b, c, d, dx, dy);
}


ddMatrix
dd_makeTranslationMatrix(fixed dx, fixed dy)
{
	return _dd_newMatrix(FIXED_I(1), 0, 0, FIXED_I(1), dx, dy);
}


ddMatrix
dd_makeScalingMatrix(fixed xScale, fixed yScale)
{
	return _dd_newMatrix(xScale, 0, 0, yScale, 0, 0);
}


ddMatrix
dd_makeRotationMatrix(double radians)
{
	fixed rsin = FIXED_D(sin(radians));
	fixed rcos = FIXED_D(cos(radians));

	return _dd_newMatrix(rcos, -rsin, rsin, rcos, 0.0, 0.0);
}


ddMatrix
ddMatrix_multiply(ddMatrix m1, ddMatrix m2)
{
	ddMatrix m;

	if ( DDMATRIX_ISIDENTITY(m1) )
		return m2;

	if ( DDMATRIX_ISIDENTITY(m2) )
		return m1;

	m.a = fixed_mult(m1.a, m2.a) + fixed_mult(m1.b, m2.c);
	m.c = fixed_mult(m1.c, m2.a) + fixed_mult(m1.d, m2.c);

	m.b = fixed_mult(m1.a, m2.b) + fixed_mult(m1.b, m2.d);
	m.d = fixed_mult(m1.c, m2.b) + fixed_mult(m1.d, m2.d);

	m.dx = fixed_mult(m1.a, m2.dx) + fixed_mult(m1.b, m2.dy) + m1.dx;
	m.dy = fixed_mult(m1.c, m2.dx) + fixed_mult(m1.d, m2.dy) + m1.dy;

	return m;
}


/* transform point (x,y) by matrix m */
void
ddMatrix_apply(ddMatrix m, fixed *x, fixed *y)
{
	fixed tmpx = *x;

	*x = fixed_mult(m.a, tmpx) + fixed_mult(m.b, *y) + m.dx;
	*y = fixed_mult(m.c, tmpx) + fixed_mult(m.d, *y) + m.dy;
}


ddRect
ddMatrix_applyRect(ddMatrix m, ddRect rect)
{
	fixed minx;
	fixed miny;
	fixed maxx;
	fixed maxy;
	
	fixed x = rect.left;
	fixed y = rect.top;
	
	ddMatrix_apply(m, &x, &y);
	
	minx = maxx = x;
	miny = maxy = y;
	
	x = rect.right;
	y = rect.top;

	ddMatrix_apply(m, &x, &y);
	
	minx = dd_min(minx, x);
	maxx = dd_max(maxx, x);

	x = rect.left;
	y = rect.bottom;
	
	ddMatrix_apply(m, &x, &y);
	
	minx = dd_min(minx, x);
	maxx = dd_max(maxx, x);

	x = rect.right;
	y = rect.bottom;
	
	ddMatrix_apply(m, &x, &y);
	
	minx = dd_min(minx, x);
	maxx = dd_max(maxx, x);
	
	return dd_makeRect(minx, miny, maxx, maxy);
}


ddMatrix
ddMatrix_scale(ddMatrix m, fixed xScale, fixed yScale)
{
	m.a = fixed_mult(m.a, xScale);
	m.b = fixed_mult(m.b, xScale);
	m.c = fixed_mult(m.c, yScale);
	m.d = fixed_mult(m.d, yScale);
	/*
	 m.dx = fixed_mult(m.dx, xScale);
	 m.dy = fixed_mult(m.dy, yScale);
	 */

	return m;
}


ddMatrix
ddMatrix_invert(ddMatrix m)
{
	/*
	ddMatrix n;
	fixed det = fixed_mult(m.a, m.d) - fixed_mult(m.b, m.c);

	n.a = fixed_divide(m.d, det);
	n.b = fixed_divide(-m.b, det);
	n.c = fixed_divide(-m.c, det);
	n.d = fixed_divide(m.a, det);

	n.dx = -(fixed_mult(n.a, m.dx) + fixed_mult(n.b, m.dy));
	n.dy = -(fixed_mult(n.c, m.dx) + fixed_mult(n.d, m.dy));
	*/

	float det = FLOAT_F(m.a) * FLOAT_F(m.d) - FLOAT_F(m.b) * FLOAT_F(m.c);

	float a = FLOAT_F(m.d) / det;
	float b = FLOAT_F(-m.b) / det;
	float c = FLOAT_F(-m.c) / det;
	float d = FLOAT_F(m.a) / det;

	float dx = -(a * FLOAT_F(m.dx) + b * FLOAT_F(m.dy));
	float dy = -(c * FLOAT_F(m.dx) + d * FLOAT_F(m.dy));
	
	return _dd_newMatrix(FIXED_D(a), FIXED_D(b), FIXED_D(c), FIXED_D(d),
					  FIXED_D(dx), FIXED_D(dy));
}


ddMatrix
ddMatrix_rotate(ddMatrix m, double radians)
{
	/* | cos(r) -sin(r) |   | a b |   | a*cos(r)-c*sin(r)  b*cos(r)-d*sin(r) |
	| sin(r)  cos(r) | * | c d | = | a*sin(r)+c*cos(r)  b*sin(r)+d*cos(r) | */

	double sinr = sin(radians);
	double cosr = cos(radians);

	double a = DOUBLE_F(m.a) * cosr - DOUBLE_F(m.c) * sinr;
	double b = DOUBLE_F(m.b) * cosr - DOUBLE_F(m.d) * sinr;

	m.c = FIXED_D(DOUBLE_F(m.a) * sinr + DOUBLE_F(m.c) * cosr);
	m.d = FIXED_D(DOUBLE_F(m.b) * sinr + DOUBLE_F(m.d) * cosr);
	m.a = FIXED_D(a);
	m.b = FIXED_D(b);

	return m;
}


ddMatrix
ddMatrix_translate(ddMatrix m, fixed dx, fixed dy)
{
	m.dx += dx;
	m.dy += dy;
	return m;
}


/* same as invert, but without determinant scaling or dx,dy calc */
ddMatrix
ddMatrix_sloppyInvert(ddMatrix m)
{
	fixed a = m.a;
	m.a = m.d;
	m.b = -m.b;
	m.c = -m.c;
	m.d = a;
	return m;
}


/* returns the state difference between the two matrices-
that is, what's going to happen to a shape's vlist after applying the
second matrix if the first is already applied */

ddMatrixState
ddMatrix_compare(ddMatrix* m1, ddMatrix* m2)
{
	ddMatrix test;

	if ( m1 == NULL || m2 == NULL )
		return MATRIX_MUNGED;

	if ( m1->a == 0 && m1->b == 0 && m1->c == 0 && m1->d == 0 )
		return MATRIX_MUNGED;

	if ( m1->a == m2->a && m1->b == m2->b && m1->c == m2->c && m1->d == m2->d )
	{
		if ( m1->dx == m2->dx && m1->dy == m2->dy )
			return MATRIX_UPTODATE;

		return MATRIX_MOVED;
	}

	test = *m1;
	test = ddMatrix_sloppyInvert(test); /* don't need to scale by det here */
	test = ddMatrix_multiply(test, *m2);

	if ( test.c == 0 ) /* XXX - prolly need tolerance here */
	{
		if ( test.b == 0 && test.a == test.d )
			return MATRIX_SCALED;
		else
			return MATRIX_STRETCHED;
	}
	else
		return MATRIX_MUNGED;
}


ddMatrix
ddMatrix_interpolate(ddMatrix m1, ddMatrix m2, UInt16 ratio)
{
	ddMatrix m;
	SInt64 r1 = 0x10000 - ratio, r2 = ratio;

	m.a = (fixed)((r1 * m1.a + r2 * m2.a) >> 16);
	m.b = (fixed)((r1 * m1.b + r2 * m2.b) >> 16);
	m.c = (fixed)((r1 * m1.c + r2 * m2.c) >> 16);
	m.d = (fixed)((r1 * m1.d + r2 * m2.d) >> 16);
	m.dx = (fixed)((r1 * m1.dx + r2 * m2.dx) >> 16);
	m.dy = (fixed)((r1 * m1.dy + r2 * m2.dy) >> 16);

	return m;
}


boolean
ddMatrix_isFlipped(ddMatrix m)
{
	return fixed_mult(m.a, m.d) < fixed_mult(m.b, m.c);
}
