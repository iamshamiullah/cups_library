/**
 * Copyright (C) 2007-2012 by Seiko Instruments Inc.
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation under the terms of the GNU General Public License is hereby
 * granted. No representations are made about the suitability of this software
 * for any purpose. It is provided "as is" without express or implied warranty.
 * See the GNU General Public License for more details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <cups/cups.h>
#include <cups/ppd.h>
#include <cups/raster.h>
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>



/// Define Error
#define SUCC							0
#define ERR_NULL						-1
#define ERR_RANGE						-2

#define TRUE							1
#define FALSE							0

// Define PPD Items
#define QUALITY_TAG						"PrintQuality"
#define QUALITY_TAG_NORMAL				"Normal"
#define QUALITY_TAG_DRAFT				"Draft"
#define DENSITY_TAG						"PrintDensity"
#define DENSITY_TAG_SPEC					"SpecifiedValue"
#define DENSITY_TAG_70					"70Percent"
#define DENSITY_TAG_80					"80Percent"
#define DENSITY_TAG_90					"90Percent"
#define DENSITY_TAG_100					"100Percent"
#define DENSITY_TAG_110					"110Percent"
#define DENSITY_TAG_120					"120Percent"
#define DENSITY_TAG_130					"130Percent"
#define BLANKIMAGE_TAG					"BlankImage"
#define BLANKIMAGE_TAG_FEED				"feed"
#define BLANKIMAGE_TAG_NON				"nonfeed"
#define CUTTIME_TAG						"CutTiming"
#define CUTTIME_TAG_DOC					"Document"
#define CUTTIME_TAG_PAGE					"Page"
#define CUTMODE_TAG						"PageCutMode"
#define CUTMODE_TAG_FULL					"FullCutPage"
#define CUTMODE_TAG_PAR					"PartialCutPage"
#define CUTMODE_TAG_NON					"NoCutPage"
#define PRINTSPEED_TAG					"PrintSpeed"
#define PRINTSPEED_TAG_SPEC				"SpecifiedValue"
#define PRINTSPEED_TAG_HI				"high"
#define PRINTSPEED_TAG_LOW				"quality"
#define PRINTSPEED_TAG_MID_Q				"middle(quality)"
#define PRINTSPEED_TAG_MID_S				"middle(silent)"
#define PAGE_SIZE_TAG					"PageSize"
#define DITHER_TAG						"Dither"
#define DITHER_TAG_NONE 					"ditherNone"
#define DITHER_TAG_SCREEN 				"ditherScreen"
#define DITHER_TAG_ERRDIFF 				"ditherErrDiff"


/// Define Command List                Use Octal
#define CMD_INIT_PRN						"\033@\035a\037\022=\001"  // ESC '@', GS 'a' 0x1F, DC2 '=' 0x01 
#define CMD_SEL_SPEED					"\035s"                    // GS 's'
#define CMD_SET_BASE_PITCH				"\035P"                    // GS 'P'
#define CMD_CUT_PAPER					"\035V"                    // GS 'V'
#define CMD_SEND_PAPER					"\033J"                    // 1B 'J'
#define CMD_MARK_FEED					"\035<"                    // GS '<'
#define CMD_RASTER_BITIMG				"\035v0"                   // GS 'v'
#define CMD_SEL_DENSITY					"\022\176"                 // DC2 '~'


/// Define Command length
#define MAX_CMD_LEN						32
#define CMD_INIT_PRN_LEN					8
#define CMD_SEND_PAPER_LEN				3
#define CMD_MARK_FEED_LEN				2
#define CMD_RASTER_BITIMG_LEN			8
#define CMD_CUT_PAPER_LEN				3
#define CMD_SEL_SPEED_LEN				3
#define CMD_SEL_DENSITY_LEN				3
#define CMD_SET_BASE_PITCH_LEN			4

// Define Dether
#define IMGDITHER_MODE_ERRDIFF3			-3	// Error Diffusion(16) - Floyd&Steinberg
#define IMGDITHER_MODE_SCREEN			0	// Pattern Dither - Screen
#define IMGDITHER_OFF					100


#ifndef ___struct_sii_cups_filter___
#define ___struct_sii_cups_filter___


/// Command output structure
typedef struct sii_cmd_out
{
	size_t			tagSize;					// Command output size
	unsigned char	pbyOut[ MAX_CMD_LEN ];	// Command output
	int				nIndex;					// Temporary Index
} SIICMDOUT;

#endif	//	___struct_sii_cups_filter___
