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
#include "ImgDither.h"

#define BPP			8
#define COLORNUM		256


static	COLORTABLE	tagColorTable[ COLORNUM ];
static	PEDERRBUF	pErrBuf = NULL;

static int Exec( long, long, unsigned char *, unsigned char *, int, int, PCOLORTABLE );
static int ExecPattern( int, long, long );
static int ExecFloyd16ED( int, int, int, PEDERRBUF );


//
// ExternalFunction
//
/// Start Dither
extern void DitherInit(
	int				nDitherMode,
	int				nRasterWidth)
{
	int				nIndex;

	for( nIndex = 0; nIndex < COLORNUM; nIndex++ )
	{
		tagColorTable[ nIndex ].rgbBlue		= nIndex;
		tagColorTable[ nIndex ].rgbGreen	= nIndex;
		tagColorTable[ nIndex ].rgbRed		= nIndex;
		tagColorTable[ nIndex ].rgbReserved = 0;
	}

	// Allocate Square Error Diffusion Buffer
	pErrBuf = malloc( sizeof( EDERRBUF ) * 112 * 8 );
	memset( pErrBuf, 0, sizeof( EDERRBUF ) * 112 * 8 );
}

//
// Terminate Dither
//
extern void DitherTerm()
{
	if( pErrBuf ) 
	{ 
		free( pErrBuf );
		pErrBuf = NULL; 
	}
}

//
// Dither Output
//
extern int DitherOutput(
	int				nRasterIndex,
	int				nRasterWidth,
	unsigned char	*pbSrcRaster,
	unsigned char	*pbDstRaster,
	int				nDitherMode )
{
	return Exec(
				nRasterIndex,
				nRasterWidth,
				pbSrcRaster,
				pbDstRaster,
				nDitherMode,
				BPP,
				(PCOLORTABLE)tagColorTable );
}

//
// Dither - Process Wrapper
//
static int Exec(
	long			nRasterIndex,
	long			nRasterWidth,
	unsigned char	*pbSrcRaster,
	unsigned char	*pbDstRaster,
	int				nDitherMode,
	int				nSrcBPP,
	PCOLORTABLE		pColorTable )
{
	unsigned char	*pbSrcPel;
	unsigned char	*pbDstPel;
	long			nXIdx = 0;
	long			nYIdx = nRasterIndex & 3;		// Raster Index within Error Diffusion table
	int				nSrcPelPitch;
	int				nIndex;
	int				nOut;

	// Parameter Error Check
	if( ( nDitherMode < IMGDITHER_MODE_MINVALUE ) || ( nDitherMode > IMGDITHER_MODE_MAXVALUE ) )
		return 0;

	if( ( nSrcBPP != 24 ) && ( nSrcBPP != 8 ) )
		return 0;

	// Raster Loop
	nSrcPelPitch = nSrcBPP / 8;		// Source image pixel pitch
	pbSrcPel = pbSrcRaster;
	pbDstPel = pbDstRaster;

	for( ; nXIdx < nRasterWidth; nXIdx++ )
	{
		// Convert Process
		switch( nSrcBPP )
		{
			case 24:// 24bpp - convert to Grayscale ( Source byte order - B.G.R ( BMP format ) )
				nIndex = ( int )( ( ( long )pbSrcPel[ 0 ] * 114 +
							( long )pbSrcPel[ 1 ] * 587 +
							( long )pbSrcPel[ 2 ] * 299 ) / 1000 );
				break;
			case 8: // 8bpp
				nIndex = ( int )pbSrcPel[ 0 ];	// color table not present - Through ( Grayscale Source )
				if( pColorTable )		// color table not present - Convert to Indexed color, and to Grayscale
				{
					nIndex = ( int )( ( ( long )pColorTable[ nIndex ].rgbBlue * 114 +
								( long )pColorTable[ nIndex ].rgbGreen * 587 +
								( long )pColorTable[ nIndex ].rgbRed * 299 ) / 1000 );
				}
				break;
			default:// Error Case
				return FALSE;
		}

		// Dither Process
		switch( nDitherMode )
		{
			case IMGDITHER_MODE_ERRDIFF3:	// Square Error Diffusion - Floyd&Steinberg( 16 )
				nOut = ExecFloyd16ED( nIndex, nXIdx, nRasterIndex, pErrBuf );
				break;
			case IMGDITHER_MODE_SCREEN:		// Pattern Dither - Screen ( Diamond )
				nOut = ExecPattern( nIndex, nYIdx, nXIdx );
				break;
			default:
				break;
		}

		// Save Result
		*pbDstPel = (unsigned char)nOut;

		pbSrcPel += nSrcPelPitch;
		pbDstPel++;
	}

	// Normal End
	return TRUE;
}

//
// Floyd&Steinberg( 16 ) Error Diffusion Process
//
static int ExecFloyd16ED(
	int				nIndex,
	int				nPixelIndex,
	int				nRasterIndex,
	PEDERRBUF		pErrBuf )
{
	int	nOut;
	int	nTmp;
	int	nErr;

	nIndex = nIndex ^ 0xff;
	if( pErrBuf ) 
	{
		nTmp = nIndex + ( int )( pErrBuf[ nPixelIndex ].nLastErr[ ( nRasterIndex ) & 1 ] / 1600 );
		nOut = ( nTmp < IMGDITHER_ED_THRESHOLD )? 0:255;
		nErr = nTmp - nOut;
		pErrBuf[ nPixelIndex ].nLastErr[ ( nRasterIndex ) & 1 ] = 0;	// Clear own location value
		pErrBuf[ nPixelIndex ].nLastErr[ ( nRasterIndex+1 ) & 1 ] += ( ( long )nErr * 500 );
		pErrBuf[ nPixelIndex+1 ].nLastErr[ ( nRasterIndex ) & 1 ] += ( ( long )nErr * 700 );
		pErrBuf[ nPixelIndex+1 ].nLastErr[ ( nRasterIndex+1 ) & 1 ] += ( ( long )nErr * 100 );
		if( nPixelIndex >= 1 )
		{ 
			pErrBuf[ nPixelIndex-1 ].nLastErr[ ( nRasterIndex+1 ) & 1 ] += ( ( long )nErr * 300 );
		}
	}

	return ( nOut^0xff );
}

//
// Pattern Dither Process
//
static int ExecPattern(
	int				nIndex,
	long			nRasterIndex,
	long			nPixelIndex )
{
	nIndex = nIndex ^ 0xff;

	// Dither conversion
	return ( nIndex > bDitherTbl44[ nRasterIndex & 3 ][ nPixelIndex & 3 ] ) ? 0 : 255;
}

