/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#ifndef DD_RENDER_CXFORM_H_INCLUDED
#define DD_RENDER_CXFORM_H_INCLUDED

#include "../dd.h"

typedef struct _ddCXform ddCXform;

struct _ddCXform
{
	SInt16 rMult;
	SInt16 gMult;
	SInt16 bMult;
	SInt16 aMult;
	SInt16 rAdd;
	SInt16 gAdd;
	SInt16 bAdd;
	SInt16 aAdd;
};

extern const ddCXform ddEmptyCXform;

#define CXFORM_ISEMPTY(c) \
(((c).aMult == ddEmptyCXform.aMult) && ((c).rMult == ddEmptyCXform.rMult) && \
 ((c).gMult == ddEmptyCXform.gMult) && ((c).bMult == ddEmptyCXform.bMult) && \
 ((c).aAdd == ddEmptyCXform.aAdd) && ((c).rAdd == ddEmptyCXform.rAdd) && \
 ((c).gAdd == ddEmptyCXform.gAdd) && ((c).bAdd == ddEmptyCXform.bAdd))

boolean ddCXform_equals(ddCXform a, ddCXform b);

ddCXform dd_makeCXform(SInt16 rMult, SInt16 gMult, SInt16 bMult, SInt16 aMult,
		      SInt16 rAdd, SInt16 gAdd, SInt16 bAdd, SInt16 aAdd);

ddCXform ddCXform_compose(ddCXform c1, ddCXform c2);

ddColor ddCXform_apply(ddCXform cXform, ddColor color);
ddColor ddCXform_applyPremultiplied(ddCXform cXform, ddColor color);

#endif /* DD_RENDER_CXFORM_H_INCLUDED */
