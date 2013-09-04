/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#include <string.h>

#include "cxform.h"

const ddCXform ddEmptyCXform = { 0x100, 0x100, 0x100, 0x100, 0, 0, 0, 0 };

ddCXform
dd_makeCXform(SInt16 rMult, SInt16 gMult, SInt16 bMult, SInt16 aMult,
			 SInt16 rAdd, SInt16 gAdd, SInt16 bAdd, SInt16 aAdd)
{
	ddCXform cXform;

	cXform.rMult = rMult;
	cXform.gMult = gMult;
	cXform.bMult = bMult;
	cXform.aMult = aMult;
	cXform.rAdd = rAdd;
	cXform.gAdd = gAdd;
	cXform.bAdd = bAdd;
	cXform.aAdd = aAdd;

	return cXform;
}


ddCXform
ddCXform_compose(ddCXform c1, ddCXform c2)
{
	ddCXform target;

	if ( CXFORM_ISEMPTY(c1) )
		return c2;

	if ( CXFORM_ISEMPTY(c2) )
		return c1;
	
	target.rMult = (SInt16)(((int)c1.rMult * c2.rMult) >> 8);
	target.gMult = (SInt16)(((int)c1.gMult * c2.gMult) >> 8);
	target.bMult = (SInt16)(((int)c1.bMult * c2.bMult) >> 8);
	target.aMult = (SInt16)(((int)c1.aMult * c2.aMult) >> 8);

	target.rAdd = c1.rAdd + c2.rAdd;
	target.gAdd = c1.gAdd + c2.gAdd;
	target.bAdd = c1.bAdd + c2.bAdd;
	target.aAdd = c1.aAdd + c2.aAdd;

	return target;
}


#define clamp(n) dd_max(dd_min((n),0xff),0)

ddColor
ddCXform_apply(ddCXform cXform, ddColor color)
{
	if ( CXFORM_ISEMPTY(cXform) )
		return color;
	
	return DDCOLOR(clamp((signed)((COLOR_ALPHA(color) * cXform.aMult) >> 8) + cXform.aAdd),
				clamp((signed)((COLOR_RED(color) * cXform.rMult) >> 8) + cXform.rAdd),
				clamp((signed)((COLOR_GREEN(color) * cXform.gMult) >> 8) + cXform.gAdd),
				clamp((signed)((COLOR_BLUE(color) * cXform.bMult) >> 8) + cXform.bAdd));
}


ddColor
ddCXform_applyPremultiplied(ddCXform cXform, ddColor color)
{
	int alpha;

	if ( CXFORM_ISEMPTY(cXform) )
		return color;
	
	alpha = clamp((signed)((COLOR_ALPHA(color) * cXform.aMult) >> 8) + cXform.aAdd);
	
	color = DDCOLOR(0xff,
				   clamp((signed)((COLOR_RED(color) * cXform.rMult) >> 8) + cXform.rAdd),
				   clamp((signed)((COLOR_GREEN(color) * cXform.gMult) >> 8) + cXform.gAdd),
				   clamp((signed)((COLOR_BLUE(color) * cXform.bMult) >> 8) + cXform.bAdd));
	
	if ( alpha == 0xff )
		return color;
	else if ( alpha == 0 )
		return (ddColor)0;
	else
		return ddColor_scale(color, alpha);
}


boolean
ddCXform_equals(ddCXform a, ddCXform b)
{
	return (a.rMult == b.rMult) && (a.gMult == b.gMult) &&
		  (a.bMult == b.bMult) && (a.aMult == b.aMult) &&
		  (a.rAdd == b.rAdd) && (a.gAdd == b.gAdd) &&
		  (a.bAdd == b.bAdd) && (a.aAdd == b.aAdd);
}

