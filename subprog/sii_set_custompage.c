/**
 * Copyright ( C ) 2008 - 2012 by Seiko Instruments Inc.
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation under the terms of the GNU General Public License is hereby
 * granted. No representations are made about the suitability of this software
 * for any purpose. It is provided "as is" without express or implied warranty.
 * See the GNU General Public License for more details.
 */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>


// Setting for file
#define MAX_PARAM_NAME			256
#define MAX_FILE_NAME			256
#define PPD_DIR_NAME				"/etc/cups/ppd/"
#define MAX_PPD_DIR_NAME			14					//strlen(PPD_DIR_NAME)
#define PPD_EXT_NAME				".ppd"

// Setting for calculation
#define MM_TO_HEIGHT				2.83333
#define HEIGHT_BLANK				0.0
#define DOUBLE_TO_INT_ADVANCE	0.9999999

// sii mode
struct sii_model
{
	char		*pszName;				// Model Name
	double		dPaperWidthMm;		// Paper Width (Unit mm)
	double		dLeftBlank;			// Left Blank
	double		dBottomBlank;			// Bottom Blank
	double		dRightBlank;			// Rigth Blank
	double		dTopBlank;				// Top Blank
};

// setting string value
struct setting_string_t
{
	char		**ppszOldPpdRear;		// The back of searched string from PPD file
	char		**ppszOldPpdFront;	// The front of searched string from PPD file
	char		**ppNewPpd;			// The end of Create PPD file
	int			nWidth;				// Paper width (Unit pts)
	int			nHeight;				// Paper height (Unit pts)
	int			nWidthMm;				// Paper width (Unit mm)
	int			nHeightMm;				// Paper height (Unit mm)
	double		dLeft;					// Left Blank
	double		dBottom;				// Bottom Blank
	double		dRight;				// Rigth Blank
	double		dTop;					// Top Blank
	const char	*pszSerchString;		// Search string
	const char	*pszStartString;		// Search start string
	const char	*pszSetString;		// Setting string
	const char	*pszEndString;		// Search end string
};

// Support mode name
static const struct sii_model g_siimodel[] =
{
	{"RP-F10_G10 (80mm)" , 72.00, 0.00, 0.00, 0.00, 0.00},
	{"RP-F10_G10 (58mm)" , 54.00, 0.00, 0.00, 0.00, 0.00},
	{"CAP06-347" , 72.00, 0.00, 0.00, 0.00, 0.00},
	{"CAP06-247" , 54.00, 0.00, 0.00, 0.00, 0.00},
	{"CAP06-245" , 48.00, 0.00, 0.00, 0.00, 0.00},
	{"RP-D10 (80mm)" , 72.00, 0.00, 0.00, 0.00, 0.00},
	{"RP-D10 (58mm)" , 54.00, 0.00, 0.00, 0.00, 0.00},
	{"RP-E10 (80mm)" , 72.00, 0.00, 0.00, 0.00, 0.00},
	{"RP-E10 (58mm)" , 54.00, 0.00, 0.00, 0.00, 0.00},
	{NULL, 0, 0.00, 0.00, 0.00, 0.00}
};


static void length_error_print( int );
static void help_print( void );
static int mm_to_height(
			char *,
			char *,
			int  *,
			int  * );

// set & check string
static int setting_string(
			struct setting_string_t * );
static int setting_string_page_size_mm(
			struct setting_string_t * );
static int setting_string_page_size(
			struct setting_string_t * );
static int setting_string_page_region_mm(
			struct setting_string_t * );
static int setting_string_page_region(
			struct setting_string_t * );
static int setting_string_imageable_area(
			struct setting_string_t * );
static int setting_string_paper_dimension(
			struct setting_string_t * );


/**
 * length_error_print
 * Length Error Message Print
 *
 * @param[ in ] nMaxHeight		Max Height Length
 */
static void
length_error_print(
	int		nMaxHeight )
{
	int		nMaxMm;			// MAX paper length (mm)
	int		nMinMm;			// MIN paper length (mm)

	nMaxMm = ( int )( ( ( double )nMaxHeight / MM_TO_HEIGHT ) + HEIGHT_BLANK );
	nMinMm = ( int )( ( ( double )HEIGHT_BLANK ) + DOUBLE_TO_INT_ADVANCE );
	fprintf( stderr,"Paper length error (%d < height_value <= %d)\n", nMinMm, nMaxMm );
}


/**
 * help_print
 * Usage help
 */
static void
help_print(
	void )
{
	fprintf( stderr, "Usage: sii_set_custompage [CUPS PrinterName] [Height]\n" );
	fprintf( stderr, "    [CUPS PrinterName] : PPD FileName\n" );
	fprintf( stderr, "                         see /etc/cups/ppd directory\n" );
	fprintf( stderr, "    [Height] : Paper Length (Unit=mm)\n" );
}

/**
 * mm_to_height
 * Argument length(mm) to height
 *
 * @param[ in ] *pszOldPpdData		Old PPD buffer
 * @param[ in ] *pszHeightArgument	size srgument String
 * @param[ out ] *pnHeight			height
 * @param[ out ] *pnHeightMm		height(mm)
 * @retval -1							failure
 * @retval 0 or more					success(height value)
 */
static int
mm_to_height(
	char		*pszOldPpdData,
	char		*pszHeightArgument,
	int			*pnHeight,
	int			*pnHeightMm )
{
	int		nRetVal;									// Return value of function
	char	*pszOldPpdRear;							// The back of searched string
	char	*pszOldPpdFront;							// The front of searched string
	int		nHeightMm;									// Height(mm)
	int		nHeight;									// Height
	int		nMaxHeight;								// MAX height
	char	szMaxMediaHeight[MAX_PARAM_NAME]={0};	// MAX height of PPD file
	size_t	Size;										// Size

	pszOldPpdRear = strstr( pszOldPpdData, "*MaxMediaHeight:" );
	if( pszOldPpdRear == NULL )
	{
		fprintf( stderr, "*MaxMediaHeight cannot detected from the PPD file\n" );
		return -1;
	}

	pszOldPpdRear  = strstr( pszOldPpdRear, "\"" );
	pszOldPpdFront = strstr( pszOldPpdRear + 1, "\"" );
	if( ( ( long )pszOldPpdFront - ( long )pszOldPpdRear - 1 ) > ( sizeof( szMaxMediaHeight ) - 1 ) )
	{
		fprintf( stderr, "The format of *MaxMediaHeight is different\n" );
		return -1;
	}

	strncpy( szMaxMediaHeight, pszOldPpdRear + 1, ( ( long )pszOldPpdFront - ( long )pszOldPpdRear - 1 ) );
	nMaxHeight = atoi( szMaxMediaHeight );

	Size = strlen( pszHeightArgument );
	if( Size > 8 )
	{
		length_error_print( nMaxHeight );
		return -1;
	}

	nRetVal = sscanf( pszHeightArgument, "%d", &nHeightMm );
	if( nRetVal < 0 )
	{
		length_error_print( nMaxHeight );
		return -1;
	}

	*pnHeightMm = nHeightMm;
	nHeight = ( int )( ( ( ( double )nHeightMm - HEIGHT_BLANK ) * MM_TO_HEIGHT ) + DOUBLE_TO_INT_ADVANCE ); //advance
	if( ( nHeightMm <= ( int )HEIGHT_BLANK ) || ( nMaxHeight < nHeight ) )
	{
		length_error_print( nMaxHeight );
		return -1;
	}

	*pnHeight = nHeight;
	return nHeight;
}

/**
 * setting_string
 * New ppd file setting
 *
 * @param[ in out ] *pSetString		SetStringData
 * @retval -1							failure
 * @retval 0							success
 */
static int
setting_string(
	struct setting_string_t	*pSetString )
{
	*pSetString->ppszOldPpdRear = strstr( *pSetString->ppszOldPpdRear, pSetString->pszSerchString );
	if( *pSetString->ppszOldPpdRear == NULL )
	{
		return -1;
	}

	*pSetString->ppszOldPpdRear += strlen( pSetString->pszSerchString );
	*pSetString->ppszOldPpdRear = strstr( *pSetString->ppszOldPpdRear, pSetString->pszStartString );
	if( *pSetString->ppszOldPpdRear == NULL )
	{
		return -1;
	}
	*pSetString->ppszOldPpdRear += strlen( pSetString->pszStartString );

	memcpy( *pSetString->ppNewPpd, *pSetString->ppszOldPpdFront , ( ( long )*pSetString->ppszOldPpdRear - ( long )*pSetString->ppszOldPpdFront ) );
	*pSetString->ppNewPpd += ( ( long )*pSetString->ppszOldPpdRear - ( long )*pSetString->ppszOldPpdFront );

	strcpy( *pSetString->ppNewPpd, pSetString->pszSetString );
	*pSetString->ppNewPpd += strlen( pSetString->pszSetString );

	*pSetString->ppszOldPpdFront = strstr( *pSetString->ppszOldPpdRear, pSetString->pszEndString );
	if( *pSetString->ppszOldPpdFront == NULL )
	{
		return -1;
	}

	return 0;
}

/**
 * setting_string_page_size_mm
 * New ppd file setting PageSize(mm)
 *
 * @param[ in out ] *pSetString		SetStringData
 * @retval -1							failure
 * @retval 0							success
 */
static int
setting_string_page_size_mm(
	struct setting_string_t	*pSetString )
{
	char	szSetString[MAX_PARAM_NAME]={0};		// Setting string

	pSetString->pszSerchString = "*PageSize SELECTPAPERXXMM/Custom Paper Size";
	pSetString->pszStartString = "(";
	pSetString->pszEndString = ")";
	sprintf( szSetString, "%dmm * %dmm", pSetString->nWidthMm, pSetString->nHeightMm );
	pSetString->pszSetString = szSetString;

	return 	setting_string( pSetString );
}

/**
 * setting_string_page_size
 * New ppd file setting PageSize
 *
 * @param[ in out ] *pSetString		SetStringData
 * @retval -1							failure
 * @retval 0							success
 */
static int
setting_string_page_size(
	struct setting_string_t	*pSetString )
{
	char	szSetString[MAX_PARAM_NAME];		// Setting string

	pSetString->pszSerchString = "<</PageSize";
	pSetString->pszStartString = "[";
	pSetString->pszEndString = "]";
	sprintf( szSetString, "%d %d", pSetString->nWidth, pSetString->nHeight );
	pSetString->pszSetString = szSetString;

	return 	setting_string( pSetString );
}

/**
 * setting_string_page_region_mm
 * New ppd file setting PageRegion(mm)
 *
 * @param[ in out ] *pSetString		SetStringData
 * @retval -1							failure
 * @retval 0							success
 */
static int
setting_string_page_region_mm(
	struct setting_string_t	*pSetString )
{
	char	szSetString[MAX_PARAM_NAME]={0};		// Setting string

	pSetString->pszSerchString = "*PageRegion SELECTPAPERXXMM/Custom Paper Size";
	pSetString->pszStartString = "(";
	pSetString->pszEndString = ")";
	sprintf( szSetString, "%dmm * %dmm", pSetString->nWidthMm, pSetString->nHeightMm );
	pSetString->pszSetString = szSetString;

	return 	setting_string( pSetString );
}

/**
 * setting_string_page_region
 * New ppd file setting PageRegion
 *
 * @param[ in out ] *pSetString		SetStringData
 * @retval -1							failure
 * @retval 0							success
 */
static int
setting_string_page_region(
	struct setting_string_t *pSetString )
{
	return 	setting_string_page_size( pSetString );
}

/**
 * setting_string_imageable_area
 * New ppd file setting ImageableArea
 *
 * @param[ in out ] *pSetString		SetStringData
 * @retval -1							failure
 * @retval 0							success
 */
static int
setting_string_imageable_area(
	struct setting_string_t *pSetString )
{
	char	szSetString[MAX_PARAM_NAME]={0};		// Setting string

	pSetString->pszSerchString = "*ImageableArea SELECTPAPERXXMM:";
	pSetString->pszStartString = "\"";
	pSetString->pszEndString = "\"";
	sprintf( szSetString, "%.1f %.1f %.1f %.1f", pSetString->dLeft, pSetString->dBottom, pSetString->dRight, pSetString->dTop );
	pSetString->pszSetString = szSetString;

	return 	setting_string( pSetString );
}

/**
 * setting_string_paper_dimension
 * New ppd file setting PaperDimension
 *
 * @param[ in out ] *pSetString		SetStringData
 * @retval -1							failure
 * @retval 0							success
 */
static int
setting_string_paper_dimension(
	struct setting_string_t *pSetString )
{
	char	szSetString[MAX_PARAM_NAME]={0};		// Setting string

	pSetString->pszSerchString = "*PaperDimension SELECTPAPERXXMM:";
	pSetString->pszStartString = "\"";
	pSetString->pszEndString = "\"";
	sprintf( szSetString, "%d %d", pSetString->nWidth, pSetString->nHeight );
	pSetString->pszSetString = szSetString;

	return 	setting_string( pSetString );
}

/**
 * main
 * New ppd file setting PaperDimension
 *
 * @param[ in ] argc			ArgumentCount
 * @param[ in ] **argv		ArgumentValue
 * @retval -1					failure
 * @retval	0 					success
 */
int
main(
	int		argc,
	char	**argv )
{
	int			nRetVal;													// Return value of function
	char		szFileName[MAX_FILE_NAME + MAX_PPD_DIR_NAME]={0};	// File name
	char		szModelName[MAX_PARAM_NAME]={0};						// Model name
	FILE		*fp;														// FILE pointer
	size_t		Size;														// Size
	char		*pszOldPpdData;											// PPD data
	char		*pszNewPpdData;											// Create PPD data
	char		*pszOldPpdRear;											// The back of searched string
	char		*pszOldPpdFront;											// The front of searched string
	char		*pszNewPpd;												// The end of Create PPD file
	int			i;															// Loop
	int			nHeight;													// Heigth
	int			nHeightMm;													// Heigth (mm)
	int			nWidth;													// Width
	int			nWidthMm;													// Width (mm)
	struct setting_string_t	tagSetString;								// Setting string structure

	// Argument Check
	if( argc != 3 )
	{
		//	argument error
		fprintf( stderr, "Argument error\n" );
		help_print();
		goto error_exit;
	}

	snprintf( szFileName, sizeof( szFileName ) - 1, PPD_DIR_NAME "%s" PPD_EXT_NAME, argv[1] );
	fp = fopen( szFileName, "r" );

	// File Check
	if( fp == NULL )
	{
		// open error
		fprintf( stderr, "Printer name error(%s)\n", szFileName );
		help_print();
		goto error_exit;
	}

	fseek( fp, 0L, SEEK_END );
	Size = ftell( fp );
	fseek( fp, 0L, SEEK_SET );

	// Memory Check
	pszOldPpdData = calloc( Size + 1, sizeof( char ) );
	if( pszOldPpdData == NULL )
	{
		fclose( fp );
		fprintf( stderr, "Memory allocate error\n" );
		goto error_exit;
	}

	pszNewPpdData = calloc( Size + 1024, sizeof( char ) );
	if( pszNewPpdData == NULL )
	{
		fclose( fp );
		fprintf( stderr, "Memory allocate error\n" );
		goto error_cp_exit;
	}

	Size = fread( pszOldPpdData, Size, sizeof( char ), fp );
	fclose( fp );

	// mm to height
	nRetVal = mm_to_height( pszOldPpdData, argv[2], &nHeight, &nHeightMm );
	if( nRetVal < 0 )
	{
		goto error_cp2_exit;
	}

	// parameter Check
	pszOldPpdRear = strstr( pszOldPpdData, "*ModelName:" );
	if( pszOldPpdRear == NULL )
	{
		fprintf( stderr, "*ModelName cannot be detected from the PPD file\n" );
		goto error_cp2_exit;
	}

	pszOldPpdRear = strstr( pszOldPpdRear, "\"" );
	pszOldPpdFront = strstr( pszOldPpdRear + 1, "\"" );
	if( ( ( long )pszOldPpdFront - ( long )pszOldPpdRear - 1 ) > ( sizeof( szModelName ) -1 ) )
	{
		fprintf( stderr, "The format of *ModelName is different\n" );
		goto error_cp2_exit;
	}

	strncpy( szModelName, pszOldPpdRear + 1, ( ( long )pszOldPpdFront - ( long )pszOldPpdRear - 1 ) );

	for( i = 0; g_siimodel[i].pszName != NULL; i ++ )
	{
		if( strcmp( g_siimodel[i].pszName, szModelName ) == 0 )
		{
			break;
		}
	}

	if( g_siimodel[i].pszName == NULL )
	{
		fprintf( stderr, "This ModelName(%s) is not supported\n", szModelName );
		goto error_cp2_exit;
	}

	nWidthMm = g_siimodel[i].dPaperWidthMm;
	nWidth = ( int )( ( g_siimodel[i].dPaperWidthMm * MM_TO_HEIGHT ) + DOUBLE_TO_INT_ADVANCE );

	tagSetString.dLeft  = g_siimodel[i].dLeftBlank;
	tagSetString.dBottom = g_siimodel[i].dBottomBlank;
	tagSetString.dRight = ( double )nWidth - g_siimodel[i].dRightBlank;
	tagSetString.dTop = ( double )nHeight - g_siimodel[i].dTopBlank;

	// Setting parameter
	pszOldPpdFront = pszOldPpdData;
	pszOldPpdRear  = pszOldPpdData;
	pszNewPpd       = pszNewPpdData;
	tagSetString.ppszOldPpdFront = &pszOldPpdFront;
	tagSetString.ppszOldPpdRear = &pszOldPpdRear;
	tagSetString.ppNewPpd = &pszNewPpd;
	tagSetString.nWidthMm = nWidthMm;
	tagSetString.nHeightMm = nHeightMm;
	tagSetString.nWidth = nWidth;
	tagSetString.nHeight = nHeight;

	nRetVal = setting_string_page_size_mm( &tagSetString );
	if( nRetVal < 0 )
	{
		fprintf( stderr, "The format of *PageSize is different\n" );
		goto error_cp2_exit;
	}

	nRetVal = setting_string_page_size( &tagSetString );
	if( nRetVal < 0 )
	{
		fprintf( stderr, "The format of *PageSize is different\n" );
		goto error_cp2_exit;
	}

	nRetVal = setting_string_page_region_mm( &tagSetString );
	if( nRetVal < 0 )
	{
		fprintf( stderr, "The format of *PageRegion is different\n" );
		goto error_cp2_exit;
	}

	nRetVal = setting_string_page_region( &tagSetString );
	if( nRetVal < 0 )
	{
		fprintf( stderr, "The format of *PageRegion is different\n" );
		goto error_cp2_exit;
	}

	nRetVal = setting_string_imageable_area( &tagSetString );
	if( nRetVal < 0 )
	{
		fprintf( stderr, "The format of *ImageableArea is different\n" );
		goto error_cp2_exit;
	}

	nRetVal = setting_string_paper_dimension( &tagSetString );
	if( nRetVal < 0 )
	{
		fprintf( stderr, "The format of *PaperDimension is different\n" );
		goto error_cp2_exit;
	}

	strcpy( *tagSetString.ppNewPpd, *tagSetString.ppszOldPpdFront );

	// PPD file check
	fp = fopen( szFileName, "w" );
	if( fp == NULL )
	{
		fprintf( stderr, "Failed to create a custom paper.\nThis setting needs superuser privileges.\n" );
		goto error_cp2_exit;
	}

	Size = fwrite( pszNewPpdData, sizeof( char ), strlen( pszNewPpdData ), fp );
	fclose( fp );

	free( ( void* )pszNewPpdData );
	free( ( void* )pszOldPpdData );

	printf( "Custom paper setting completion\n" );

	return 0;

error_cp2_exit:
	free( ( void* )pszNewPpdData );

error_cp_exit:
	free( ( void* )pszOldPpdData );

error_exit:
	exit( -1 );
}
