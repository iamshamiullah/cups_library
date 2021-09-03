/**
 * Copyright ( C ) 2007-2013 by Seiko Instruments Inc.
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation under the terms of the GNU General Public License is hereby
 * granted. No representations are made about the suitability of this software
 * for any purpose. It is provided "as is" without express or implied warranty.
 * See the GNU General Public License for more details.
 */

#include "sii.h"
#include "sii_cmd.h"
#include "rastertosii.h"

#define CUPSPAGEHEADER	cups_page_header2_t
#define CUPSRASTER		cups_raster_t

#define	OUTPUTCMD()																\
for( tagCmd.nIndex = 0; tagCmd.nIndex < tagCmd.tagSize; tagCmd.nIndex++ )		\
{																						\
	putchar( ( unsigned char )tagCmd.pbyOut[ tagCmd.nIndex ] );					\
}

#define	CLEANUP(){															\
	if( pbyRasterOutData != NULL )											\
	{																			\
		free( pbyRasterOutData );											\
		pbyRasterOutData = NULL;												\
	}																			\
	if( pbyRasterOutLine != NULL )											\
	{																			\
		free( pbyRasterOutLine );											\
		pbyRasterOutLine = NULL;												\
	}																			\
	if( pbyRasterData != NULL )												\
	{																			\
		free( pbyRasterData );												\
		pbyRasterData = NULL;												\
	}																			\
	if( pbyRasterDataNext != NULL )											\
	{																			\
		free( pbyRasterDataNext );											\
		pbyRasterDataNext = NULL;											\
	}																			\
}


typedef struct cups_settings
{
	int				nModelNumber;
	int				nSpeed;
	int				nPageCutMode;
	int				nCutTiming;
	int				nPrintDensity;
	int				nPrintQuality;
	int				nDpi;
	int				nBlankImage;
	int				nDither;
	int				nReduced;
	int				nMaxBytesPerLine;
	int				nValidDataBytesPerLine;
	int				nCmdDataBytesPerLine;
	int				nOutputLine;
	int				nOutputDataSize;
} CUPSSETTINGS;


static int			stnSigFlag = 0;		// signal flag
void				Initialize( char *, CUPSSETTINGS * );
void				StartDoc( CUPSSETTINGS * );
void				EndPage( CUPSSETTINGS * );
void				EndDoc( CUPSSETTINGS * );
void				PrintLine( CUPSSETTINGS *, CUPSPAGEHEADER *, unsigned char *, unsigned char *, int);
int					nPrintData( CUPSSETTINGS *, CUPSRASTER * );
void				CancelPrint( int );
void				CheckTerm( void );
void				CutandFeedPaper( int, int, int );
void				FeedPaper( int );
int					nGetMarginLen( int, int, int );
void				CutPaper( int );
unsigned char		byDraftReducedPixel( unsigned int, unsigned char * );
void				DitherInit( int, int );
void				DitherTerm();
int					DitherOutput(int, int, unsigned char*, unsigned char*, int );






/**
 * Initialize
 * initialize settings, ppd, etc.
 *
 * @param[ in ] *pszCommandLineOptions
 * @param[ out ] *pSettings
 */
void
Initialize(
	char			*pszCommandLineOptions,
	CUPSSETTINGS	*pSettings )
{
	int				nOptionsNum		= 0;
	int				nIndex				= 0;
	ppd_file_t		*pPpd				= NULL;
	ppd_choice_t	*pChoice			= NULL;
	cups_option_t	*pOptions			= NULL;

	// Open ppd
	pPpd = ppdOpenFile( getenv( "PPD" ) );
	if( pPpd == NULL )
	{
		pSettings->nMaxBytesPerLine = DEFAULTMAXWIDTH;
		return;
	}

	ppdMarkDefaults( pPpd );

	/* command line option*/
	nOptionsNum = cupsParseOptions( pszCommandLineOptions, 0, &pOptions );
	if( ( nOptionsNum != 0 ) && ( pOptions != NULL ) )
	{
		cupsMarkOptions( pPpd, nOptionsNum, pOptions );
		cupsFreeOptions( nOptionsNum, pOptions );
	}

	// Get model number
	pSettings->nModelNumber = pPpd->model_number;
	nIndex = 0;
	while( tblMaxBytesPerLine[ nIndex ].nModelNumber != 0 )
	{
		if( pSettings->nModelNumber == tblMaxBytesPerLine[ nIndex ].nModelNumber )
		{
			pSettings->nMaxBytesPerLine = tblMaxBytesPerLine[ nIndex ].nMaxBytesPerLine;
			break;
		}
		nIndex++;
	}

	// Get Speed and Print Density
	nIndex = 0;
	while( tblPrintConditions[ nIndex ].nModelNumber != 0 )
	{
		if( pSettings->nModelNumber == tblPrintConditions[ nIndex ].nModelNumber )
		{
			// Set Speed
			pSettings->nSpeed = CMDPRTSPEEDSPEC;
			if( ( pChoice = ppdFindMarkedChoice( pPpd, PRINTSPEED_TAG ) ) )
			{
				if( !strcmp( pChoice->choice, PRINTSPEED_TAG_HI ) )
				{
					pSettings->nSpeed = tblPrintConditions[ nIndex ].nSpeed[ PRTSPEEDHIGH ];
				}
				else if( !strcmp( pChoice->choice, PRINTSPEED_TAG_LOW ) )
				{
					pSettings->nSpeed = tblPrintConditions[ nIndex ].nSpeed[ PRTSPEEDLOW ];
				}
				else if( !strcmp( pChoice->choice, PRINTSPEED_TAG_MID_Q ) )
				{
					pSettings->nSpeed = tblPrintConditions[ nIndex ].nSpeed[ PRTSPEEDLOW ];
				}
				else if( !strcmp( pChoice->choice, PRINTSPEED_TAG_MID_S ) )
				{
					pSettings->nSpeed = tblPrintConditions[ nIndex ].nSpeed[ PRTSPEEDMID_S ];
				}
			}

			// Set Density
			pSettings->nPrintDensity = CMDPRTDENSITYSPEC;
			if( ( pChoice = ppdFindMarkedChoice( pPpd, DENSITY_TAG ) ) )
			{
				if( !strcmp( pChoice->choice, DENSITY_TAG_70 ) )
				{
					pSettings->nPrintDensity = tblPrintConditions[ nIndex ].nPrintDensity[ PRTDENSITY70 ];
				}
				else if( !strcmp( pChoice->choice, DENSITY_TAG_80 ) )
				{
					pSettings->nPrintDensity = tblPrintConditions[ nIndex ].nPrintDensity[ PRTDENSITY80 ];
				}
				else if( !strcmp( pChoice->choice, DENSITY_TAG_90 ) )
				{
					pSettings->nPrintDensity = tblPrintConditions[ nIndex ].nPrintDensity[ PRTDENSITY90 ];
				}
				else if( !strcmp( pChoice->choice, DENSITY_TAG_100 ) )
				{
					pSettings->nPrintDensity = tblPrintConditions[ nIndex ].nPrintDensity[ PRTDENSITY100 ];
				}
				else if( !strcmp( pChoice->choice, DENSITY_TAG_110 ) )
				{
					pSettings->nPrintDensity = tblPrintConditions[ nIndex ].nPrintDensity[ PRTDENSITY110 ];
				}
				else if( !strcmp( pChoice->choice, DENSITY_TAG_120 ) )
				{
					pSettings->nPrintDensity = tblPrintConditions[ nIndex ].nPrintDensity[ PRTDENSITY120 ];
				}
				else if( !strcmp( pChoice->choice, DENSITY_TAG_130 ) )
				{
					pSettings->nPrintDensity = tblPrintConditions[ nIndex ].nPrintDensity[ PRTDENSITY130 ];
				}
			}
			break;
		}
		nIndex++;
	}

	// Get page cut mode
	pSettings->nPageCutMode = FULLCUT;
	if( ( pChoice = ppdFindMarkedChoice( pPpd, CUTMODE_TAG ) ) )
	{
		if( !strcmp( pChoice->choice, CUTMODE_TAG_NON ) )
		{
			pSettings->nPageCutMode = NOCUT;
		}
		else if( !strcmp( pChoice->choice, CUTMODE_TAG_PAR ) )
		{
			pSettings->nPageCutMode = PARTIALCUT;
		}
	}

	// Get cut timing
	pSettings->nCutTiming = PAGECUT;
	if( ( pChoice = ppdFindMarkedChoice( pPpd, CUTTIME_TAG ) ) )
	{
		if( !strcmp( pChoice->choice, CUTTIME_TAG_DOC ) )
		{
			pSettings->nCutTiming = DOCCUT;
		}
	}

	// Get blank image feed mode
	pSettings->nBlankImage = BLANKFEED;
	if( ( pChoice = ppdFindMarkedChoice( pPpd, BLANKIMAGE_TAG ) ) )
	{
		if( !strcmp( pChoice->choice, BLANKIMAGE_TAG_NON ) )
		{
			pSettings->nBlankImage = BLANKNONFEED;
		}
	}

	// Get print quality
	pSettings->nPrintQuality = NORMAL;
	pSettings->nDpi = PRTPITCHNORMAL;
	if( ( pChoice = ppdFindMarkedChoice( pPpd, QUALITY_TAG ) ) )
	{
		if( !strcmp( pChoice->choice, QUALITY_TAG_DRAFT ) )
		{
			pSettings->nPrintQuality = DRAFT;
			pSettings->nDpi = PRTPITCHDRAFT;
		}
	}

	// Get dither
	pSettings->nDither = IMGDITHER_MODE_SCREEN;
	if( ( pChoice = ppdFindMarkedChoice( pPpd, DITHER_TAG ) ) )
	{
		if( !strcmp( pChoice->choice, DITHER_TAG_NONE ) )
		{
			pSettings->nDither = IMGDITHER_OFF;
		}
		else if( !strcmp( pChoice->choice, DITHER_TAG_ERRDIFF ) )
		{
			pSettings->nDither = IMGDITHER_MODE_ERRDIFF3;
		}
	}

	// Get reduced page
	pSettings->nReduced = REDUCEDOFF;
	if( ( pChoice = ppdFindMarkedChoice( pPpd, PAGE_SIZE_TAG ) ) )
	{
		nIndex = 0;
		while( pszReducePageSize[ nIndex ] != 0 )
		{
			if( !strcmp( pChoice->choice, pszReducePageSize[ nIndex++ ] ) )
			{
				if( pSettings->nPrintQuality == DRAFT )
				{
					pSettings->nReduced = REDUCEDON;
					break;
				}
			}
		}
	}

	ppdClose( pPpd );
}

/**
 * PrintLine
 * print line
 *
 * @param[ in ] *pSettings Settings
 * @param[ in ] *pHeader Header
 * @param[ in ] *pbyRasterData Raster data poninter
 * @param[ in ] *pbyRasterOutLine Raster data poninter(for Output Line)
 * @param[ in ] *bLastFlag Last Data Flag
 */
void
PrintLine(
	CUPSSETTINGS	*pSettings,
	CUPSPAGEHEADER	*pHeader,
	unsigned char	*pbyRasterData,
	unsigned char	*pbyRasterOutLine,
	int				bLastFlag )
{
	SIICMDOUT		tagCmd				= {0};
	unsigned char	*pbyTmpRasterData	= NULL;
	unsigned char	*pbyOutLine		= NULL;
	unsigned char	byOutData			= 0;
	int				nIndex				= 0;
	int				nMod				= 0;
	int				nLine				= 0;

	if( bLastFlag == FALSE )
	{
		if( pHeader->cupsBitsPerPixel == 1 )
		{
			// 1bit per pixcel
			nLine = pSettings->nValidDataBytesPerLine;
		}
		else
		{
			// 8bits per pixcel
			pbyTmpRasterData = pbyRasterData;

			for( nIndex = 0; nIndex < pSettings->nValidDataBytesPerLine; nIndex++ )
			{
				nMod = nIndex % 8;
				if( *pbyTmpRasterData++ > THRESHOLDVAL )
				{
					byOutData |= data_shift_tbl[ nMod ];
				}

				if( nMod == 7 )
				{
					pbyRasterData[nLine++] = byOutData;
					byOutData = 0;
				}
			}

			if( ( pSettings->nValidDataBytesPerLine % 8 ) > 0 )
			{
				pbyRasterData[nLine++] = byOutData;
			}
		}

		pbyOutLine = pbyRasterOutLine + pSettings->nOutputDataSize;
		memcpy(pbyOutLine, pbyRasterData, nLine);
		pSettings->nOutputDataSize += nLine;
		pSettings->nOutputLine++;
	}

	if( ( bLastFlag != FALSE ) || ( pSettings->nOutputLine >= OUTPUT_LINE_NUM ) )
	{
		memset( &tagCmd, 0, sizeof( SIICMDOUT ) );

		// output printing raster bit image command
		raster_bitimg(
				&tagCmd,
				pSettings->nPrintQuality,
				( unsigned char )( pSettings->nCmdDataBytesPerLine & 0xff ),
				( unsigned char )( ( pSettings->nCmdDataBytesPerLine >> 8 ) & 0xff ),
				( unsigned char )( pSettings->nOutputLine & 0xff ),
				( unsigned char )( ( pSettings->nOutputLine >> 8 ) & 0xff ) );

		OUTPUTCMD()

		nMod = fwrite(pbyRasterOutLine, 1, pSettings->nOutputDataSize, stdout);
		pSettings->nOutputDataSize = 0;
		pSettings->nOutputLine = 0;
	}
}

/**
 * nPrintData
 * print data
 *
 * @param[ in ] *pSettings Settings
 * @param[ in ] *pHeader Header
 * @retval EXIT_FAILURE failure
 * @retval EXIT_SUCCESS success
 */
int
nPrintData(
	CUPSSETTINGS	*pSettings,
	CUPSRASTER		*pRaster )
{
	unsigned char	*pbyRasterData		= NULL;
	unsigned char	*pbyRasterOutData		= NULL;
	unsigned char	*pbyRasterDataNext	= NULL;
	unsigned char	*pbyRasterOutLine		= NULL;
	unsigned char	byThreshHold			= 0;
	unsigned char	pbyPix[ 16 ]			= {0};
	CUPSPAGEHEADER	tagHeader;
	int				nBlankLineLen			= 0;
	int				nIndex					= 0;
	int				nY						= 0;
	int				nDotY					= 0;
	int				nPage					= 0;
	int				nRet					= 0;
	int				nHeight				= 0;
	int				nDataBytesPerLine		= 0;
	int				nImgPrinted			= 0;

	memset( &tagHeader, 0, sizeof( CUPSPAGEHEADER ) );

	StartDoc( pSettings );

	// Output Printer Data.
	while( cupsRasterReadHeader2( pRaster, &tagHeader ) )
	{
		if( tagHeader.cupsBitsPerPixel != 1 )
		{
			byThreshHold = THRESHOLDVAL;
		}

		nHeight = tagHeader.cupsHeight;
		nDataBytesPerLine = tagHeader.cupsBytesPerLine;
		if( ( nHeight == 0 ) || ( nDataBytesPerLine == 0 ) )
		{
			break;
		}

		if( pSettings->nReduced == REDUCEDON )
		{
			// If page size = "reduced" and print quality = "Draft"
			pSettings->nValidDataBytesPerLine = nDataBytesPerLine / 2;
			if( ( nDataBytesPerLine % 2 ) != 0 )
			{
				pSettings->nValidDataBytesPerLine++;
			}
		}
		else
		{
			// Other mode.
			pSettings->nValidDataBytesPerLine = nDataBytesPerLine;
		}

		if( tagHeader.cupsBitsPerPixel == 1 )
		{
			// 1 bits per pixcel
			if( pSettings->nValidDataBytesPerLine > ( pSettings->nMaxBytesPerLine / 8 ) )
			{
				pSettings->nValidDataBytesPerLine = ( pSettings->nMaxBytesPerLine / 8 );
			}

			pSettings->nCmdDataBytesPerLine = pSettings->nValidDataBytesPerLine;
		}
		else
		{
			// 8 bits per pixcel
			if( pSettings->nValidDataBytesPerLine > pSettings->nMaxBytesPerLine )
			{
				pSettings->nValidDataBytesPerLine = pSettings->nMaxBytesPerLine;
			}

			pSettings->nCmdDataBytesPerLine = pSettings->nValidDataBytesPerLine / 8;
			if( ( pSettings->nValidDataBytesPerLine % 8 ) > 0 )
			{
				pSettings->nCmdDataBytesPerLine++;
			}
		}

		// create work area
		if( pbyRasterData == NULL )
		{
			if( ( pbyRasterData = malloc( ( size_t )nDataBytesPerLine ) ) == NULL )
			{
				CLEANUP()
				return EXIT_FAILURE;
			}
		}

		if( pbyRasterOutLine == NULL )
		{
			if( ( pbyRasterOutLine = malloc( ( size_t )(nDataBytesPerLine * OUTPUT_LINE_NUM) ) ) == NULL )
			{
				CLEANUP()
				return EXIT_FAILURE;
			}
		}

		if( pbyRasterOutData == NULL )
		{
			if( ( pbyRasterOutData = malloc( ( size_t )nDataBytesPerLine ) ) == NULL )
			{
				CLEANUP()
				return EXIT_FAILURE;
			}
		}

		if( pSettings->nReduced == REDUCEDON )
		{
			if( pbyRasterDataNext == NULL )
			{
				if( ( pbyRasterDataNext = malloc( ( size_t )nDataBytesPerLine ) ) == NULL )
				{
					CLEANUP()
					return EXIT_FAILURE;
				}
				memset( pbyRasterDataNext, 0, nDataBytesPerLine );
			}
		}

		DitherInit( pSettings->nDither,
					nDataBytesPerLine);

		nImgPrinted = FALSE;
		nBlankLineLen = 0;
 		nPage++;

		// create page data
		for( nY = 0; nY < nHeight; nY++ )
		{
			CheckTerm();
			memset( pbyRasterData, 0, nDataBytesPerLine );
			memset( pbyRasterOutData, 0, nDataBytesPerLine );

			// Read Raster data.
			if( cupsRasterReadPixels( pRaster, pbyRasterData, nDataBytesPerLine ) < 1 )
			{
				break;
			}

			nDotY = nY;
			if( pSettings->nReduced == REDUCEDON )
			{
				// If page size = "reduced" and print quality = "Draft"
				nDotY = nY++/2;
				if( nY != nHeight )
				{
					// Read next line.
					if( cupsRasterReadPixels( pRaster, pbyRasterDataNext, nDataBytesPerLine ) < 1 )
					{
						break;
					}
				}
				else
				{
					memset( pbyRasterDataNext, 0, nDataBytesPerLine );
				}

				// Reduced and Draft mode
				// Reduce to 1/4 pixcels
				for( nIndex = 0; nIndex < nDataBytesPerLine; nIndex++ )
				{
					pbyPix[ 0 ] = pbyRasterData[ nIndex ];
					pbyPix[ 2 ] = pbyRasterDataNext[ nIndex ];
					if( nIndex != ( nDataBytesPerLine - 1 ) )
					{
						pbyPix[ 1 ] = pbyRasterData[ nIndex+1 ];
						pbyPix[ 3 ] = pbyRasterDataNext[ nIndex+1 ];
					}
					else
					{
						pbyPix[ 1 ] = 0;
						pbyPix[ 3 ] = 0;
					}

					pbyRasterData[ nIndex/2 ] = byDraftReducedPixel(
													tagHeader.cupsBitsPerPixel,
													pbyPix );

					nIndex++;
				}
			}

			// Dither
			if( ( tagHeader.cupsBitsPerPixel != 1 )	&&
				( pSettings->nDither != IMGDITHER_OFF )	)
			{
				nRet = DitherOutput(
						nDotY,
						pSettings->nValidDataBytesPerLine,
						pbyRasterData,
						pbyRasterOutData,
						pSettings->nDither );
				if( nRet == FALSE )
				{
					break;
				}
			}
			else
			{
				memcpy( pbyRasterOutData, pbyRasterData, pSettings->nValidDataBytesPerLine );
			}

			nIndex = 0;
			for( ;; )
			{
				// Check blank line.
				if( pbyRasterOutData[ nIndex ] > byThreshHold )
				{
					if( nBlankLineLen != 0 )
					{
						if( ( pSettings->nBlankImage == BLANKFEED ) || ( nImgPrinted == TRUE ) )
						{
							// Feed blank Lines.
							FeedPaper( nBlankLineLen );
							CheckTerm();
						}
						nBlankLineLen = 0;
					}

					// Print one dot line.
					PrintLine( pSettings, &tagHeader, pbyRasterOutData, pbyRasterOutLine, 0 );
					nImgPrinted = TRUE;
					break;
				}
				else{
					if( pSettings->nOutputLine > 0 )
					{
						// Print one dot line.
						PrintLine( pSettings, &tagHeader, pbyRasterOutData, pbyRasterOutLine, 1 );
					}
				}

				if( ++nIndex >= pSettings->nValidDataBytesPerLine )
				{
					// This line is blank.
					nBlankLineLen++;
					break;
				}
			}
		}

		if( pSettings->nOutputLine > 0 )
		{
			// Print one dot line.
			PrintLine( pSettings, &tagHeader, pbyRasterOutData, pbyRasterOutLine, 1 );
		}

		DitherTerm();

		// Paper discharge
		if( ( pSettings->nBlankImage == BLANKFEED ) && ( nBlankLineLen != 0 ) )
		{
			// Output blank Lines.
			FeedPaper( nBlankLineLen );
			CheckTerm();
		}

		if( ( pSettings->nBlankImage == BLANKFEED ) || nImgPrinted )
		{
			// Paper cut & End page
			EndPage( pSettings );
		}
		CheckTerm();
	}

	CLEANUP()

	if( nPage == 0 )
	{
		fputs( "ERROR: Pages not found\n", stderr );
		return EXIT_FAILURE;
	}

	EndDoc( pSettings );

	return EXIT_SUCCESS;
}


/**
 * CancelPrint
 * Signal handler.
 *
 * @param[ in ] nSig signal
 *
 */
void
CancelPrint(
	int				nSig )
{
	stnSigFlag = SIGTERM;
}

/**
 * CheckTerm
 * Check signal( SIGTERM ) and exit program if it is received.
 *
 */
void
CheckTerm( void )
{
	if( stnSigFlag == SIGTERM )
	{
		exit( EXIT_SUCCESS );
	}
}




/**
 * StartDoc
 * Start document
 *
 * @param[ in ] *pSettings
 */
void
StartDoc(
	CUPSSETTINGS	*pSettings )
{
	SIICMDOUT		tagCmd		= {0};

	memset( &tagCmd, 0, sizeof( SIICMDOUT ) );

	// signal set ( Don't stop printing one line by SIGTERM )
	signal( SIGTERM,CancelPrint );

	init_prn( &tagCmd );
	OUTPUTCMD()

	// set base pitch
	set_base_pitch( &tagCmd, pSettings->nDpi, pSettings->nDpi );
	OUTPUTCMD()

	// set print speed
	if(pSettings->nSpeed != CMDPRTSPEEDSPEC){
		sel_speed( &tagCmd, pSettings->nSpeed );
		OUTPUTCMD()
	}

	// set print density
	if(pSettings->nPrintDensity != CMDPRTDENSITYSPEC){
		sel_density( &tagCmd, pSettings->nPrintDensity );
		OUTPUTCMD()
	}
}


/**
 * End Page
 *
 * @param[ in ] *pSettings
 */
void
EndPage(
	CUPSSETTINGS	*pSettings )
{
	if( pSettings->nCutTiming == PAGECUT )
	{
		CutandFeedPaper(
			pSettings->nModelNumber,
			pSettings->nPageCutMode,
			pSettings->nPrintQuality );
	}
}


/**
 * EndDoc
 * End Document
 *
 * @param[ in ] *pSettings
 */
void
EndDoc(
	CUPSSETTINGS	*pSettings )
{
	// Document Trail Feed and Cutting and Feed after cutting
	if( pSettings->nCutTiming == DOCCUT )
	{
		CutandFeedPaper(
			pSettings->nModelNumber,
			pSettings->nPageCutMode,
			pSettings->nPrintQuality );
	}
}



/**
 * Feed Paper
 *
 * @param[ in ] nLength
 */
void
FeedPaper(
	int				nLength )
{
	SIICMDOUT		tagCmd	= {0};
	unsigned char	byLen	= 0;

	do
	{
		if( nLength > 255 )
		{
			byLen = 0xFF;
		}
		else
		{
			byLen = ( unsigned char )nLength;
		}

		send_paper( &tagCmd, byLen );
		OUTPUTCMD()

		nLength -= ( int )byLen;
	}while( nLength > 0 );
}


/**
 * CutandFeedPaper
 *
 * @param[ in ] nModelNumber
 * @param[ in ] nCutMode
 * @param[ in ] nQuality
 */
void
CutandFeedPaper(
	int				nModelNumber,
	int				nCutMode,
	int				nQuality )
{
	SIICMDOUT		tagCmd	= {0};

	memset( &tagCmd, 0, sizeof( SIICMDOUT ) );
	mark_form_feed( &tagCmd );
	OUTPUTCMD()

	switch( nCutMode )
	{
	case FULLCUT:
		FeedPaper( nGetMarginLen( nModelNumber, nQuality, SENDBEFOREFULLCUT ) );
		CutPaper( PRTCUTFULL );
		FeedPaper( nGetMarginLen( nModelNumber, nQuality, SENDAFTERFULLCUT ) );
		break;
	case PARTIALCUT:
		FeedPaper( nGetMarginLen( nModelNumber, nQuality, SENDBEFOREPARTIALCUT ) );
		CutPaper( PRTCUTPARTIAL );
		FeedPaper( nGetMarginLen( nModelNumber, nQuality, SENDAFTERPARTIALCUT ) );
		break;
	case NOCUT:
		FeedPaper(
			nGetMarginLen( nModelNumber, nQuality, SENDBEFORENONCUT )
			+ nGetMarginLen( nModelNumber, nQuality, SENDAFTERNONCUT ) );
		break;
	default:
		break;
	}
}


/**
 * nGetMarginLen
 *
 * @param[ in ] nModelNumber	model number
 * @param[ in ] nQuality		quality( NORMAL or DRAFT )
 * @param[ in ] nSendMode		send mode
 */
int
nGetMarginLen(
	int				nModelNumber,
	int				nQuality,
	int				nSendMode )
{
	int				nIndex	= 0;
	int				nLen	= 0;

	while( tblSendLength[ nIndex ].nModelNumber != 0 )
	{
		if( nModelNumber == tblSendLength[ nIndex ].nModelNumber )
		{
			switch( nSendMode )
			{
			case SENDBEFOREFULLCUT:
				nLen = tblSendLength[ nIndex ].nBeforeFullCut;
				break;
			case SENDAFTERFULLCUT:
				nLen = tblSendLength[ nIndex ].nAfterFullCut;
				break;
			case SENDBEFOREPARTIALCUT:
				nLen = tblSendLength[ nIndex ].nBeforePartialCut;
				break;
			case SENDAFTERPARTIALCUT:
				nLen = tblSendLength[ nIndex ].nAfterPartialCut;
				break;
			case SENDBEFORENONCUT:
				nLen = tblSendLength[ nIndex ].nBeforeNonCut;
				break;
			case SENDAFTERNONCUT:
				nLen = tblSendLength[ nIndex ].nAfterNonCut;
				break;
			}
		}
		nIndex++;
	}

	if( nQuality == DRAFT )
	{
		nLen = nLen / 2;
	}

	return nLen;
}




/**
 * Cut paper
 *
 * @param[ in ] mode	FULLCUT or PARTIALCUT
 */
void
CutPaper(
	int				nMode )
{
	SIICMDOUT		tagCmd	= {0};

	switch( nMode )
	{
	case FULLCUT:
		cut_paper( &tagCmd, PRTCUTFULL );
		break;
	case PARTIALCUT:
		cut_paper( &tagCmd, PRTCUTPARTIAL );
		break;
	default:
		return;
	}

	OUTPUTCMD()
}


/**
 * DraftReducedPixel
 * Get draft and reduced pixel
 *
 * @param[ in ] wBitsPerPixel
 * @param[ in ] *pbyPix
 * @return reduced pixel
 */
unsigned char
byDraftReducedPixel( unsigned int wBitsPerPixel, unsigned char *pbyPix )
{
	unsigned char byOutPix	= 0;

	if( wBitsPerPixel == 1 )
	{
		if( ( ( pbyPix[ 0 ] & 0xc0 ) | ( pbyPix[ 2 ] & 0xc0 ) ) != 0 )
		{
			byOutPix = byOutPix | 0x80;
		}

		if( ( ( pbyPix[ 0 ] & 0x30 ) | ( pbyPix[ 2 ] & 0x30 ) ) != 0 )
		{
			byOutPix = byOutPix | 0x40;
		}

		if( ( ( pbyPix[ 0 ] & 0x0c ) | ( pbyPix[ 2 ] & 0x0c ) ) != 0 )
		{
			byOutPix = byOutPix | 0x20;
		}

		if( ( ( pbyPix[ 0 ] & 0x03 ) | ( pbyPix[ 2 ] & 0x03 ) ) != 0 )
		{
			byOutPix = byOutPix | 0x10;
		}

		if( ( ( pbyPix[ 1 ] & 0xc0 ) | ( pbyPix[ 3 ] & 0xc0 ) ) != 0 )
		{
			byOutPix = byOutPix | 0x08;
		}

		if( ( ( pbyPix[ 1 ] & 0x30 ) | ( pbyPix[ 3 ] & 0x30 ) ) != 0 )
		{
			byOutPix = byOutPix | 0x04;
		}

		if( ( ( pbyPix[ 1 ] & 0x0c ) | ( pbyPix[ 3 ] & 0x0c ) ) != 0 )
		{
			byOutPix = byOutPix | 0x02;
		}

		if( ( ( pbyPix[ 1 ] & 0x03 ) | ( pbyPix[ 3 ] & 0x03 ) ) != 0 )
		{
			byOutPix = byOutPix | 0x01;
		}
	}
	else
	{
		byOutPix = pbyPix[ 0 ] | pbyPix[ 1 ] | pbyPix[ 2 ] | pbyPix[ 3 ];
	}

	return byOutPix;
}




int
main(
	int				argc,
	char			*argv[ ] )
{
	CUPSRASTER		*pRaster		= NULL;
	CUPSSETTINGS	tagSettings	= {0};
	SIICMDOUT		tagCmd			= {0};
	int				nFd				= 0;
	int				nSuccess		= EXIT_FAILURE;

	memset( &tagSettings, 0, sizeof( CUPSSETTINGS ) );
	memset( &tagCmd, 0, sizeof( SIICMDOUT ) );

	if( argc < 6 || argc > 7 )
	{
		fputs( "ERROR: rastertosii id user title copies options [ file ]\n", stderr );
		return EXIT_FAILURE;
	}

	if( argc == 7 )
	{
		if( ( nFd = open( argv[ 6 ], O_RDONLY ) ) == -1 )
		{
			perror( "ERROR: Unable to open the raster data file" );
			sleep( 1 );
			return EXIT_FAILURE;
		}
	}

	// Initialize
	Initialize( argv[ 5 ], &tagSettings );


	// Open raster data
	pRaster = cupsRasterOpen( nFd, CUPS_RASTER_READ );

	if( pRaster != NULL )
	{
		// Output printer data
		nSuccess = nPrintData( &tagSettings, pRaster );

		// Close raster data
		cupsRasterClose( pRaster );
	}

	if( nFd != 0 )
	{
		close( nFd );
	}

	return nSuccess;
}
