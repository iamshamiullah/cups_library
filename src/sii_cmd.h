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

// print and send paper (ESC 'J')
int send_paper(SIICMDOUT *, unsigned char);

//  head of the mark paper (GS '<') 
int mark_form_feed(SIICMDOUT *);

//  cut paper (GS 'V')
int cut_paper(SIICMDOUT *, unsigned char);

// print raster bit image (GS 'v')
int raster_bitimg(SIICMDOUT *,
				  unsigned char,
				  unsigned char,
				  unsigned char,
				  unsigned char,
				  unsigned char);

// initialize printer (ESC '@' + GS 'a' + DC2 '=')
int init_prn(SIICMDOUT *);

// select print speed and hedder energizing time (GS 's')
int sel_speed(SIICMDOUT *, unsigned char);

// select print density
int sel_density(SIICMDOUT *, unsigned char);

//  set base caluculating pitch (GS 'P')
int set_base_pitch(SIICMDOUT *, unsigned char, unsigned char);


