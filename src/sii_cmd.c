/**
 * Copyright ( C ) 2007-2012 by Seiko Instruments Inc.
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation under the terms of the GNU General Public License is hereby
 * granted. No representations are made about the suitability of this software
 * for any purpose. It is provided "as is" without express or implied warranty.
 * See the GNU General Public License for more details.
 */


#include "sii.h"
#include "sii_cmd.h"

#define CHK_NULL()							\
	if( pSiiCmdOut == NULL )					\
	{											\
		return ( ERR_NULL );					\
	}											\

#define init_cmd_out( out,len )				\
	memset( out->pbyOut,0,MAX_CMD_LEN );	\
	out->tagSize = len;						\


/**
 * Print and Feed Forward ( ESC 'J' )
 */
int
send_paper(
	SIICMDOUT		*pSiiCmdOut,
	unsigned char	byLen )
{
	CHK_NULL();

	init_cmd_out( pSiiCmdOut, CMD_SEND_PAPER_LEN );

	snprintf(
		pSiiCmdOut->pbyOut,
		MAX_CMD_LEN,
		"%s%c",
		CMD_SEND_PAPER,
		byLen );

	return ( SUCC );
}

/**
 * Marked Paper Form Feed ( GS '<' )
 */
int
mark_form_feed(
	SIICMDOUT		*pSiiCmdOut )
{
	CHK_NULL();

	init_cmd_out( pSiiCmdOut, CMD_MARK_FEED_LEN );

	snprintf(
		pSiiCmdOut->pbyOut,
		MAX_CMD_LEN,
		"%s",
		CMD_MARK_FEED );

	return ( SUCC );
}

/**
 * print raster bit image ( GS 'v' )
 */
int
raster_bitimg(
	SIICMDOUT		*pSiiCmdOut,
	unsigned char	byM,
	unsigned char	byXL,
	unsigned char	byXH,
	unsigned char	byYL,
	unsigned char	byYH )
{
	CHK_NULL();

	init_cmd_out( pSiiCmdOut, CMD_RASTER_BITIMG_LEN );

	snprintf(
		pSiiCmdOut->pbyOut,
		MAX_CMD_LEN,
		"%s%c%c%c%c%c",
		CMD_RASTER_BITIMG,
		byM,
		byXL,
		byXH,
		byYL,
		byYH );

	return ( SUCC );
}


/**
 * cut paper ( GS 'V' )
 */
int
cut_paper(
	SIICMDOUT		*pSiiCmdOut,
	unsigned char	byM )
{
	CHK_NULL();

	init_cmd_out( pSiiCmdOut, CMD_CUT_PAPER_LEN );

	snprintf(
		pSiiCmdOut->pbyOut,
		MAX_CMD_LEN,
		"%s%c",
		CMD_CUT_PAPER,
		byM );

	return ( SUCC );
}

/**
 * initialize printer ( ESC '@' + GS 'a' n + DC2 '=' n )
 */
int
init_prn(
	SIICMDOUT		*pSiiCmdOut )
{
	CHK_NULL();

	init_cmd_out( pSiiCmdOut, CMD_INIT_PRN_LEN );

	snprintf(
		pSiiCmdOut->pbyOut,
		MAX_CMD_LEN,
		"%s",
		CMD_INIT_PRN );

	return ( SUCC );
}

/**
 * select print speed and hedder energizing time ( GS '~' n )
 */
int
sel_speed(
	SIICMDOUT		*pSiiCmdOut,
	unsigned char	byN )
{
	CHK_NULL();

	init_cmd_out( pSiiCmdOut, CMD_SEL_SPEED_LEN );

	snprintf(
		pSiiCmdOut->pbyOut,
		MAX_CMD_LEN,
		"%s%c",
		CMD_SEL_SPEED,
		byN );

	return ( SUCC );
}

/**
 * select print density
 */
int
sel_density(
	SIICMDOUT		*pSiiCmdOut,
	unsigned char	byN )
{
	CHK_NULL();

	init_cmd_out( pSiiCmdOut, CMD_SEL_DENSITY_LEN );

	snprintf(
		pSiiCmdOut->pbyOut,
		MAX_CMD_LEN,
		"%s%c",
		CMD_SEL_DENSITY,
		byN );

	return ( SUCC );
}

/**
 * set base caluculating pitch ( GS 'P' )
 */
int
set_base_pitch(
	SIICMDOUT		*pSiiCmdOut,
	unsigned char	byX,
	unsigned char	byY )
{
	CHK_NULL();

	init_cmd_out( pSiiCmdOut, CMD_SET_BASE_PITCH_LEN );

	snprintf(
		pSiiCmdOut->pbyOut,
		MAX_CMD_LEN,
		"%s%c%c",
		CMD_SET_BASE_PITCH,
		byX,
		byY );

	return ( SUCC );
}

