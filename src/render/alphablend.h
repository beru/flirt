/*
 Flirt, an SWF rendering library
 Copyright (c) 2004-2006 Dave Hayden <dave@opaque.net>
 All rights reserved.
 
 http://www.opaque.net/flirt/

 This code is distributed under the two-clause BSD license.
 Read the LICENSE file or visit the URL above for details
*/

#ifndef DD_RENDER_ALPHABLEND_H_INCLUDED
#define DD_RENDER_ALPHABLEND_H_INCLUDED

#ifdef DD_COLOR_SAMPLE_ARGB
  #define RED     0x00ff0000
  #define GREEN   0x0000ff00
  #define BLUE    0x000000ff
  #define OPAQUE(c) (((c)>>24)&0xff)
#else
  #define RED     0xff000000
  #define GREEN   0x00ff0000
  #define BLUE    0x0000ff00
  #define OPAQUE(c) ((c)&0xff)
#endif

#ifdef DD_RENDER_TOPDOWN

/* c2 goes under c1 */

static ddColor inline
alpha_blend(ddColor c1, ddColor c2)
{
	UInt8 alpha1 = OPAQUE(c1);
	
	if ( alpha1 == 0xff || OPAQUE(c2) == 0 )
		return c1;

	if ( OPAQUE(c1) == 0 )
		return c2;

	return c1 + ddColor_scale(c2, 0xff - alpha1);
	/*
	 (((c1 & RED) + (((c2 & RED) >> 8) * a)) & RED) |
	 (((c1 & GREEN) + (((c2 & GREEN) * a) >> 8)) & GREEN) |
	 (((c1 & BLUE) + (((c2 & BLUE) * a) >> 8)) & BLUE) |
	 (((c1 & 0xff) + ((c2 & 0xff) * a) >> 8));
	 */
}

#else

/* c2 goes on top of c1 */

static ddColor inline
alpha_blend(ddColor c1, ddColor c2)
{
	UInt8 alpha1;
	UInt8 alpha2 = OPAQUE(c2);
	
	if ( alpha2 == 0x00 )
		return c1;

	if ( alpha2 == 0xff )
		return c2;

	alpha1 = ~alpha2; /* i.e., 0xff - alpha2 */

#ifdef DD_COLOR_SAMPLE_ARGB
	// XXX - not premultiplied
	return ((((c1 & RED) * alpha1 + (c2 & RED) * alpha2) >> 8) & RED) +
		((((c1 & GREEN) * alpha1 + (c2 & GREEN) * alpha2) >> 8) & GREEN) +
		((((c1 & BLUE) * alpha1 + (c2 & BLUE) * alpha2) >> 8) & BLUE);
#else
	// premultiplied:
	return (((((c1 & RED) >> 8) * alpha1) & RED) |
		((((c1 & GREEN) * alpha1) >> 8) & GREEN) |
		((((c1 & BLUE) * alpha1) >> 8) & BLUE) |
		(((OPAQUE(c1) * alpha1) >> 8) & 0xff)) + c2;
#endif
}

#endif /* DD_RENDER_TOPDOWN */

#endif /* DD_RENDER_ALPHABLEND_H_INCLUDED */
