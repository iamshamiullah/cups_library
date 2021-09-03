/**
 * Copyright (C) 2007-2013 by Seiko Instruments Inc.
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation under the terms of the GNU General Public License is hereby
 * granted. No representations are made about the suitability of this software
 * for any purpose. It is provided "as is" without express or implied warranty.
 * See the GNU General Public License for more details.
 */

// Model
#define RPFG10_3					9
#define RPFG10_2					8
#define CAP06_245					7
#define CAP06_247					6
#define CAP06_347					5
#define RPD10_3					4
#define RPD10_2					3
#define RPE10_3					2
#define RPE10_2					1


// MaxWidth
#define DEFAULTMAXWIDTH			72*8
#define RPFG10_3MAXWIDTH			72*8
#define RPFG10_2MAXWIDTH			54*8
#define CAP06_347MAXWIDTH			72*8
#define CAP06_247MAXWIDTH			54*8
#define CAP06_245MAXWIDTH			48*8
#define RPD10_3MAXWIDTH			72*8
#define RPD10_2MAXWIDTH			54*8
#define RPE10_3MAXWIDTH			72*8
#define RPE10_2MAXWIDTH			54*8


// Setting Parameter
#define PAGECUT					0
#define DOCCUT					1
#define BLANKFEED					0
#define BLANKNONFEED				1
#define FULLCUT					0
#define PARTIALCUT				1
#define NOCUT						2
#define NORMAL					0
#define DRAFT						3
#define DITHERON					0
#define DITHEROFF					1
#define NORMAL_MODE_IMAGE		0
#define DRAFT_MODE_IMAGE			3
#define REDUCEDOFF				0
#define REDUCEDON					1
#define THRESHOLDVAL				0x7F
#define BINMAXVAL					0xFF
#define BINMINVAL					0x00

// Reduce Page Size
static const char *pszReducePageSize[] =
{
	"A4",
	"Letter",
	NULL
};

// Printer Base pitch
#define PRTPITCHNORMAL			203
#define PRTPITCHDRAFT			101

// Printer Speed
#define PRTSPEED_NUM				4
#define PRTSPEEDMID_S			3
#define PRTSPEEDLOW				2
#define PRTSPEEDHIGH				1
#define PRTSPEEDSPEC				0

#define CMDPRTSPEEDSPEC			-1

// Printer Density
#define PRTDENSITY_NUM			8
#define PRTDENSITYSPEC			0
#define PRTDENSITY70				1
#define PRTDENSITY80				2
#define PRTDENSITY90				3
#define PRTDENSITY100			4
#define PRTDENSITY110			5
#define PRTDENSITY120			6
#define PRTDENSITY130			7

#define CMDPRTDENSITYSPEC			-1


// Printer Cut mode
#define PRTCUTFULL				0
#define PRTCUTPARTIAL			1

#define SENDBEFOREFULLCUT 		1
#define SENDAFTERFULLCUT			2
#define SENDBEFOREPARTIALCUT	3
#define SENDAFTERPARTIALCUT		4
#define SENDBEFORENONCUT			5
#define SENDAFTERNONCUT			6


#define OUTPUT_LINE_NUM			20


typedef struct tagSendLength_t
{
	int nModelNumber;
	int nBeforeFullCut;
	int nAfterFullCut;
	int nBeforePartialCut;
	int nAfterPartialCut;
	int nBeforeNonCut;
	int nAfterNonCut;
} SENDLENGTH;

static const SENDLENGTH tblSendLength[] =
{
	{RPFG10_3, 76, 16, 76, 16, 0, 0},
	{RPFG10_2, 76, 16, 76, 16, 0, 0},
	{CAP06_347, 76, 16, 76, 16, 0, 0},
	{CAP06_247, 76, 16, 76, 16, 0, 0},
	{CAP06_245, 76, 16, 76, 16, 0, 0},
	{RPD10_3, 72, 16, 72, 16, 0, 0},
	{RPD10_2, 72, 16, 72, 16, 0, 0},
	{RPE10_3, 96, 0, 96, 0, 0, 0},
	{RPE10_2, 96, 0, 96, 0, 0, 0},
	{0, 0, 0, 0, 0, 0,0}
};


typedef struct tagMaxBytesPerLine_t
{
	int nModelNumber;
	int nMaxBytesPerLine;
} MAXBYTELENGTH;

static const MAXBYTELENGTH tblMaxBytesPerLine[] =
{
	{RPFG10_3, RPFG10_3MAXWIDTH},
	{RPFG10_2, RPFG10_2MAXWIDTH},
	{CAP06_347, CAP06_347MAXWIDTH},
	{CAP06_247, CAP06_247MAXWIDTH},
	{CAP06_245, CAP06_245MAXWIDTH},
	{RPD10_3, RPD10_3MAXWIDTH},
	{RPD10_2, RPD10_2MAXWIDTH},
	{RPE10_3, RPE10_3MAXWIDTH},
	{RPE10_2, RPE10_2MAXWIDTH},
	{0, 0}
};

static const unsigned char data_shift_tbl[] = {
	0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01
};

typedef struct tagPrintConditions_t
{
	int nModelNumber;
	int nSpeed[ PRTSPEED_NUM ];
	int nPrintDensity[ PRTDENSITY_NUM ];
} PRINTCONDITIONS;


static const PRINTCONDITIONS tblPrintConditions[] =
{
	// +-------------------------------------------------------------------------------------------- Model
	// |			+-------------------------------------------------------------------------- PRTSPEEDSPEC (0)
	// |			|	    +--------------------------------------------------------------- PRTSPEEDHIGH (1)
	// |			|	    |	+------------------------------------------------------------ PRTSPEEDLOW (2)
	// |			|	    |	|  +--------------------------------------------------------- PRTSPEEDMID(Silent) (3)
	// |			|	    |  |  |		+---------------------------------------------- PRTDENSITYSPEC (0)
	// |			|	    |  |  |		|		+-------------------------------- PRTDENSITY70~130 (1~7)
	{RPFG10_3, 	{CMDPRTSPEEDSPEC, 0, 1, 3},	{CMDPRTDENSITYSPEC, 70,80,90,100,110,120,130} },
	{RPFG10_2, 	{CMDPRTSPEEDSPEC, 0, 1, 3},	{CMDPRTDENSITYSPEC, 70,80,90,100,110,120,130} },
	{CAP06_347,	{CMDPRTSPEEDSPEC, 0, 1, 0},	{CMDPRTDENSITYSPEC, 70,80,90,100,110,120,130} },
	{CAP06_247,	{CMDPRTSPEEDSPEC, 0, 1, 0},	{CMDPRTDENSITYSPEC, 70,80,90,100,110,120,130} },
	{CAP06_245,	{CMDPRTSPEEDSPEC, 0, 1, 0},	{CMDPRTDENSITYSPEC, 70,80,90,100,110,120,130} },
	{RPD10_3, 	{0,0,0,0},			{CMDPRTDENSITYSPEC, 70,80,90,100,110,120,130} },
	{RPD10_2,	{0,0,0,0},			{CMDPRTDENSITYSPEC, 70,80,90,100,110,120,130} },
	{RPE10_3,	{0,0,0,0},			{CMDPRTDENSITYSPEC, 70,80,90,100,110,120,130} },
	{RPE10_2,	{0,0,0,0},			{CMDPRTDENSITYSPEC, 70,80,90,100,110,120,130} },
	{0, 		{0,0,0,0},			{0,0,0,0,0,0,0,0} }
};
