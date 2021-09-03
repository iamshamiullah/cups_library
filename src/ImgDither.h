/**
 * Copyright (C) 2007-2012 by Seiko Instruments Inc.
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation under the terms of the GNU General Public License is hereby
 * granted. No representations are made about the suitability of this software
 * for any purpose. It is provided "as is" without express or implied warranty.
 * See the GNU General Public License for more details.
 */

#include "sii.h"

#define	IMGDITHER_MODE_MINVALUE	IMGDITHER_MODE_ERRDIFF3
#define	IMGDITHER_MODE_MAXVALUE	IMGDITHER_MODE_SCREEN

#define	IMGDITHER_ED_THRESHOLD	128

// Error Diffusion Buffer structure
typedef struct tagEDERRBUF
{
	long	nLastErr[ 4 ];	// Error Buffer (4 raster)
} EDERRBUF,* PEDERRBUF;

// Color Table structure for 8bpp
typedef struct tagCOLORTABLE
{
	unsigned char rgbBlue;
	unsigned char rgbGreen;
	unsigned char rgbRed;
	unsigned char rgbReserved;
} COLORTABLE,* PCOLORTABLE;


// 4x4 Dither Pattern Table
static unsigned char bDitherTbl44[ 4 ][ 4 ]=
{
	{ 176,  64,  96, 144 },
	{ 192,   4,  32, 224 },
	{ 112, 128, 160,  80 },
	{  48, 240, 208,  16 }
};
