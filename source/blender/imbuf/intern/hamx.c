/**
 *
 * ***** BEGIN GPL/BL DUAL LICENSE BLOCK *****
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version. The Blender
 * Foundation also sells licenses for use in proprietary software under
 * the Blender License.  See http://www.blender.org/BL/ for information
 * about this.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * The Original Code is Copyright (C) 2001-2002 by NaN Holding BV.
 * All rights reserved.
 *
 * The Original Code is: all of this file.
 *
 * Contributor(s): none yet.
 *
 * ***** END GPL/BL DUAL LICENSE BLOCK *****
 * hamx.c
 *
 * $Id$
 */

#include "BLI_blenlib.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef WIN32
#include "BLI_winstuff.h"
#include <io.h>
#endif


#include "imbuf.h"
#include "imbuf_patch.h"
#include "IMB_imbuf_types.h"
#include "IMB_imbuf.h"

#include "IMB_allocimbuf.h"
#include "IMB_filter.h"
#include "IMB_ham.h"
#include "IMB_hamx.h"

/* actually hard coded endianness */
#define GET_BIG_LONG(x) (((uchar *) (x))[0] << 24 | ((uchar *) (x))[1] << 16 | ((uchar *) (x))[2] << 8 | ((uchar *) (x))[3])
#define GET_LITTLE_LONG(x) (((uchar *) (x))[3] << 24 | ((uchar *) (x))[2] << 16 | ((uchar *) (x))[1] << 8 | ((uchar *) (x))[0])
#define SWAP_L(x) (((x << 24) & 0xff000000) | ((x << 8) & 0xff0000) | ((x >> 8) & 0xff00) | ((x >> 24) & 0xff))
#define SWAP_S(x) (((x << 8) & 0xff00) | ((x >> 8) & 0xff))

/* more endianness... should move to a separate file... */
#if defined(__sgi) || defined (__sparc) || defined (__sparc__) || defined (__PPC__) || defined (__ppc__) || defined (__BIG_ENDIAN__)
#define GET_ID GET_BIG_LONG
#define LITTLE_LONG SWAP_LONG
#else
#define GET_ID GET_LITTLE_LONG
#define LITTLE_LONG ENDIAN_NOP
#endif

#ifndef ABS
#define ABS(x)	((x) < 0 ? -(x) : (x))
#endif

static uchar hamx_array_char[] = {
	0x00,0x00,0xFF,0xFF, 0x00,0x00,0xFF,0xFF, 0x00,0x00,0xFF,0xFF, 0x00,0x00,0xFF,0xFF, 0x00,0x00,0xFF,0xFF, 0x00,0x00,0xFF,0xFF, 0x00,0x00,0xFF,0xFF, 0x00,0x00,0xFF,0xFF,
	0x00,0x00,0xFF,0xFF, 0x00,0x00,0xFF,0xFF, 0x00,0x00,0xFF,0xFF, 0x00,0x00,0xFF,0xFF, 0x00,0x00,0xFF,0xFF, 0x00,0x00,0xFF,0xFF, 0x00,0x00,0xFF,0xFF, 0x00,0x00,0xFF,0xFF,
	0x00,0xFF,0x00,0xFF, 0x00,0xFF,0x00,0xFF, 0x00,0xFF,0x00,0xFF, 0x00,0xFF,0x00,0xFF, 0x00,0xFF,0x00,0xFF, 0x00,0xFF,0x00,0xFF, 0x00,0xFF,0x00,0xFF, 0x00,0xFF,0x00,0xFF,
	0x00,0xFF,0x00,0xFF, 0x00,0xFF,0x00,0xFF, 0x00,0xFF,0x00,0xFF, 0x00,0xFF,0x00,0xFF, 0x00,0xFF,0x00,0xFF, 0x00,0xFF,0x00,0xFF, 0x00,0xFF,0x00,0xFF, 0x00,0xFF,0x00,0xFF,
	0x00,0xFF,0xFF,0x00, 0x00,0xFF,0xFF,0x00, 0x00,0xFF,0xFF,0x00, 0x00,0xFF,0xFF,0x00, 0x00,0xFF,0xFF,0x00, 0x00,0xFF,0xFF,0x00, 0x00,0xFF,0xFF,0x00, 0x00,0xFF,0xFF,0x00,
	0x00,0xFF,0xFF,0x00, 0x00,0xFF,0xFF,0x00, 0x00,0xFF,0xFF,0x00, 0x00,0xFF,0xFF,0x00, 0x00,0xFF,0xFF,0x00, 0x00,0xFF,0xFF,0x00, 0x00,0xFF,0xFF,0x00, 0x00,0xFF,0xFF,0x00,
	
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	
	0x00,0x00,0x00,0x00, 0x00,0x10,0x00,0x00, 0x00,0x20,0x00,0x00, 0x00,0x30,0x00,0x00, 0x00,0x40,0x00,0x00, 0x00,0x50,0x00,0x00, 0x00,0x60,0x00,0x00, 0x00,0x70,0x00,0x00,
	0x00,0x80,0x00,0x00, 0x00,0x90,0x00,0x00, 0x00,0xA0,0x00,0x00, 0x00,0xB0,0x00,0x00, 0x00,0xC0,0x00,0x00, 0x00,0xD0,0x00,0x00, 0x00,0xE0,0x00,0x00, 0x00,0xF0,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x10,0x00, 0x00,0x00,0x20,0x00, 0x00,0x00,0x30,0x00, 0x00,0x00,0x40,0x00, 0x00,0x00,0x50,0x00, 0x00,0x00,0x60,0x00, 0x00,0x00,0x70,0x00,
	0x00,0x00,0x80,0x00, 0x00,0x00,0x90,0x00, 0x00,0x00,0xA0,0x00, 0x00,0x00,0xB0,0x00, 0x00,0x00,0xC0,0x00, 0x00,0x00,0xD0,0x00, 0x00,0x00,0xE0,0x00, 0x00,0x00,0xF0,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x10, 0x00,0x00,0x00,0x20, 0x00,0x00,0x00,0x30, 0x00,0x00,0x00,0x40, 0x00,0x00,0x00,0x50, 0x00,0x00,0x00,0x60, 0x00,0x00,0x00,0x70,
	0x00,0x00,0x00,0x80, 0x00,0x00,0x00,0x90, 0x00,0x00,0x00,0xA0, 0x00,0x00,0x00,0xB0, 0x00,0x00,0x00,0xC0, 0x00,0x00,0x00,0xD0, 0x00,0x00,0x00,0xE0, 0x00,0x00,0x00,0xF0,
	
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x30, 0x00,0x00,0x00,0x60, 0x00,0x00,0x00,0x90, 0x00,0x00,0x00,0xC0, 0x00,0x00,0x00,0xF0,
	0x00,0x00,0x20,0x00, 0x00,0x00,0x20,0x30, 0x00,0x00,0x20,0x60, 0x00,0x00,0x20,0x90, 0x00,0x00,0x20,0xC0, 0x00,0x00,0x20,0xF0,
	0x00,0x00,0x40,0x00, 0x00,0x00,0x40,0x30, 0x00,0x00,0x40,0x60, 0x00,0x00,0x40,0x90, 0x00,0x00,0x40,0xC0, 0x00,0x00,0x40,0xF0,
	0x00,0x00,0x60,0x00, 0x00,0x00,0x60,0x30, 0x00,0x00,0x60,0x60, 0x00,0x00,0x60,0x90, 0x00,0x00,0x60,0xC0, 0x00,0x00,0x60,0xF0,
	0x00,0x00,0x90,0x00, 0x00,0x00,0x90,0x30, 0x00,0x00,0x90,0x60, 0x00,0x00,0x90,0x90, 0x00,0x00,0x90,0xC0, 0x00,0x00,0x90,0xF0,
	0x00,0x00,0xB0,0x00, 0x00,0x00,0xB0,0x30, 0x00,0x00,0xB0,0x60, 0x00,0x00,0xB0,0x90, 0x00,0x00,0xB0,0xC0, 0x00,0x00,0xB0,0xF0,
	0x00,0x00,0xD0,0x00, 0x00,0x00,0xD0,0x30, 0x00,0x00,0xD0,0x60, 0x00,0x00,0xD0,0x90, 0x00,0x00,0xD0,0xC0, 0x00,0x00,0xD0,0xF0,
	0x00,0x00,0xF0,0x00, 0x00,0x00,0xF0,0x30, 0x00,0x00,0xF0,0x60, 0x00,0x00,0xF0,0x90, 0x00,0x00,0xF0,0xC0, 0x00,0x00,0xF0,0xF0,
	0x00,0x50,0x00,0x00, 0x00,0x50,0x00,0x30, 0x00,0x50,0x00,0x60, 0x00,0x50,0x00,0x90, 0x00,0x50,0x00,0xC0, 0x00,0x50,0x00,0xF0,
	0x00,0x50,0x20,0x00, 0x00,0x50,0x20,0x30, 0x00,0x50,0x20,0x60, 0x00,0x50,0x20,0x90, 0x00,0x50,0x20,0xC0, 0x00,0x50,0x20,0xF0,
	0x00,0x50,0x40,0x00, 0x00,0x50,0x40,0x30, 0x00,0x50,0x40,0x60, 0x00,0x50,0x40,0x90, 0x00,0x50,0x40,0xC0, 0x00,0x50,0x40,0xF0,
	0x00,0x50,0x60,0x00, 0x00,0x50,0x60,0x30, 0x00,0x50,0x60,0x60, 0x00,0x50,0x60,0x90, 0x00,0x50,0x60,0xC0, 0x00,0x50,0x60,0xF0,
	0x00,0x50,0x90,0x00, 0x00,0x50,0x90,0x30, 0x00,0x50,0x90,0x60, 0x00,0x50,0x90,0x90, 0x00,0x50,0x90,0xC0, 0x00,0x50,0x90,0xF0,
	0x00,0x50,0xB0,0x00, 0x00,0x50,0xB0,0x30, 0x00,0x50,0xB0,0x60, 0x00,0x50,0xB0,0x90, 0x00,0x50,0xB0,0xC0, 0x00,0x50,0xB0,0xF0,
	0x00,0x50,0xD0,0x00, 0x00,0x50,0xD0,0x30, 0x00,0x50,0xD0,0x60, 0x00,0x50,0xD0,0x90, 0x00,0x50,0xD0,0xC0, 0x00,0x50,0xD0,0xF0,
	0x00,0x50,0xF0,0x00, 0x00,0x50,0xF0,0x30, 0x00,0x50,0xF0,0x60, 0x00,0x50,0xF0,0x90, 0x00,0x50,0xF0,0xC0, 0x00,0x50,0xF0,0xF0,
	0x00,0xA0,0x00,0x00, 0x00,0xA0,0x00,0x30, 0x00,0xA0,0x00,0x60, 0x00,0xA0,0x00,0x90, 0x00,0xA0,0x00,0xC0, 0x00,0xA0,0x00,0xF0,
	0x00,0xA0,0x20,0x00, 0x00,0xA0,0x20,0x30, 0x00,0xA0,0x20,0x60, 0x00,0xA0,0x20,0x90, 0x00,0xA0,0x20,0xC0, 0x00,0xA0,0x20,0xF0,
	0x00,0xA0,0x40,0x00, 0x00,0xA0,0x40,0x30, 0x00,0xA0,0x40,0x60, 0x00,0xA0,0x40,0x90, 0x00,0xA0,0x40,0xC0, 0x00,0xA0,0x40,0xF0,
	0x00,0xA0,0x60,0x00, 0x00,0xA0,0x60,0x30, 0x00,0xA0,0x60,0x60, 0x00,0xA0,0x60,0x90, 0x00,0xA0,0x60,0xC0, 0x00,0xA0,0x60,0xF0,
	0x00,0xA0,0x90,0x00, 0x00,0xA0,0x90,0x30, 0x00,0xA0,0x90,0x60, 0x00,0xA0,0x90,0x90, 0x00,0xA0,0x90,0xC0, 0x00,0xA0,0x90,0xF0,
	0x00,0xA0,0xB0,0x00, 0x00,0xA0,0xB0,0x30, 0x00,0xA0,0xB0,0x60, 0x00,0xA0,0xB0,0x90, 0x00,0xA0,0xB0,0xC0, 0x00,0xA0,0xB0,0xF0,
	0x00,0xA0,0xD0,0x00, 0x00,0xA0,0xD0,0x30, 0x00,0xA0,0xD0,0x60, 0x00,0xA0,0xD0,0x90, 0x00,0xA0,0xD0,0xC0, 0x00,0xA0,0xD0,0xF0,
	0x00,0xA0,0xF0,0x00, 0x00,0xA0,0xF0,0x30, 0x00,0xA0,0xF0,0x60, 0x00,0xA0,0xF0,0x90, 0x00,0xA0,0xF0,0xC0, 0x00,0xA0,0xF0,0xF0,
	0x00,0xF0,0x00,0x00, 0x00,0xF0,0x00,0x30, 0x00,0xF0,0x00,0x60, 0x00,0xF0,0x00,0x90, 0x00,0xF0,0x00,0xC0, 0x00,0xF0,0x00,0xF0,
	0x00,0xF0,0x20,0x00, 0x00,0xF0,0x20,0x30, 0x00,0xF0,0x20,0x60, 0x00,0xF0,0x20,0x90, 0x00,0xF0,0x20,0xC0, 0x00,0xF0,0x20,0xF0,
	0x00,0xF0,0x40,0x00, 0x00,0xF0,0x40,0x30, 0x00,0xF0,0x40,0x60, 0x00,0xF0,0x40,0x90, 0x00,0xF0,0x40,0xC0, 0x00,0xF0,0x40,0xF0,
	0x00,0xF0,0x60,0x00, 0x00,0xF0,0x60,0x30, 0x00,0xF0,0x60,0x60, 0x00,0xF0,0x60,0x90, 0x00,0xF0,0x60,0xC0, 0x00,0xF0,0x60,0xF0,
	0x00,0xF0,0x90,0x00, 0x00,0xF0,0x90,0x30, 0x00,0xF0,0x90,0x60, 0x00,0xF0,0x90,0x90, 0x00,0xF0,0x90,0xC0, 0x00,0xF0,0x90,0xF0,
	0x00,0xF0,0xB0,0x00, 0x00,0xF0,0xB0,0x30, 0x00,0xF0,0xB0,0x60, 0x00,0xF0,0xB0,0x90, 0x00,0xF0,0xB0,0xC0, 0x00,0xF0,0xB0,0xF0,
	0x00,0xF0,0xD0,0x00, 0x00,0xF0,0xD0,0x30, 0x00,0xF0,0xD0,0x60, 0x00,0xF0,0xD0,0x90, 0x00,0xF0,0xD0,0xC0, 0x00,0xF0,0xD0,0xF0,
	0x00,0xF0,0xF0,0x00, 0x00,0xF0,0xF0,0x30, 0x00,0xF0,0xF0,0x60, 0x00,0xF0,0xF0,0x90, 0x00,0xF0,0xF0,0xC0, 0x00,0xF0,0xF0,0xF0,
	
	0x00,0x10,0x10,0x10, 0x00,0x20,0x20,0x20, 0x00,0x30,0x30,0x30, 0x00,0x40,0x40,0x40,
	0x00,0x50,0x50,0x50, 0x00,0x60,0x60,0x60, 0x00,0x70,0x70,0x70, 0x00,0x80,0x80,0x80,
	0x00,0x90,0x90,0x90, 0x00,0xA0,0xA0,0xA0, 0x00,0xB0,0xB0,0xB0, 0x00,0xC0,0xC0,0xC0,
	0x00,0xD0,0xD0,0xD0, 0x00,0xE0,0xE0,0xE0
};

static int * hamx_array = (int *) (hamx_array_char);

static uchar cmap_hamx[] = {
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x30, 0x00,0x00,0x00,0x60, 0x00,0x00,0x00,0x90, 0x00,0x00,0x00,0xC0, 0x00,0x00,0x00,0xF0,
	0x00,0x00,0x20,0x00, 0x00,0x00,0x20,0x30, 0x00,0x00,0x20,0x60, 0x00,0x00,0x20,0x90, 0x00,0x00,0x20,0xC0, 0x00,0x00,0x20,0xF0,
	0x00,0x00,0x40,0x00, 0x00,0x00,0x40,0x30, 0x00,0x00,0x40,0x60, 0x00,0x00,0x40,0x90, 0x00,0x00,0x40,0xC0, 0x00,0x00,0x40,0xF0,
	0x00,0x00,0x60,0x00, 0x00,0x00,0x60,0x30, 0x00,0x00,0x60,0x60, 0x00,0x00,0x60,0x90, 0x00,0x00,0x60,0xC0, 0x00,0x00,0x60,0xF0,
	0x00,0x00,0x90,0x00, 0x00,0x00,0x90,0x30, 0x00,0x00,0x90,0x60, 0x00,0x00,0x90,0x90, 0x00,0x00,0x90,0xC0, 0x00,0x00,0x90,0xF0,
	0x00,0x00,0xB0,0x00, 0x00,0x00,0xB0,0x30, 0x00,0x00,0xB0,0x60, 0x00,0x00,0xB0,0x90, 0x00,0x00,0xB0,0xC0, 0x00,0x00,0xB0,0xF0,
	0x00,0x00,0xD0,0x00, 0x00,0x00,0xD0,0x30, 0x00,0x00,0xD0,0x60, 0x00,0x00,0xD0,0x90, 0x00,0x00,0xD0,0xC0, 0x00,0x00,0xD0,0xF0,
	0x00,0x00,0xF0,0x00, 0x00,0x00,0xF0,0x30, 0x00,0x00,0xF0,0x60, 0x00,0x00,0xF0,0x90, 0x00,0x00,0xF0,0xC0, 0x00,0x00,0xF0,0xF0,
	0x00,0x50,0x00,0x00, 0x00,0x50,0x00,0x30, 0x00,0x50,0x00,0x60, 0x00,0x50,0x00,0x90, 0x00,0x50,0x00,0xC0, 0x00,0x50,0x00,0xF0,
	0x00,0x50,0x20,0x00, 0x00,0x50,0x20,0x30, 0x00,0x50,0x20,0x60, 0x00,0x50,0x20,0x90, 0x00,0x50,0x20,0xC0, 0x00,0x50,0x20,0xF0,
	0x00,0x50,0x40,0x00, 0x00,0x50,0x40,0x30, 0x00,0x50,0x40,0x60, 0x00,0x50,0x40,0x90, 0x00,0x50,0x40,0xC0, 0x00,0x50,0x40,0xF0,
	0x00,0x50,0x60,0x00, 0x00,0x50,0x60,0x30, 0x00,0x50,0x60,0x60, 0x00,0x50,0x60,0x90, 0x00,0x50,0x60,0xC0, 0x00,0x50,0x60,0xF0,
	0x00,0x50,0x90,0x00, 0x00,0x50,0x90,0x30, 0x00,0x50,0x90,0x60, 0x00,0x50,0x90,0x90, 0x00,0x50,0x90,0xC0, 0x00,0x50,0x90,0xF0,
	0x00,0x50,0xB0,0x00, 0x00,0x50,0xB0,0x30, 0x00,0x50,0xB0,0x60, 0x00,0x50,0xB0,0x90, 0x00,0x50,0xB0,0xC0, 0x00,0x50,0xB0,0xF0,
	0x00,0x50,0xD0,0x00, 0x00,0x50,0xD0,0x30, 0x00,0x50,0xD0,0x60, 0x00,0x50,0xD0,0x90, 0x00,0x50,0xD0,0xC0, 0x00,0x50,0xD0,0xF0,
	0x00,0x50,0xF0,0x00, 0x00,0x50,0xF0,0x30, 0x00,0x50,0xF0,0x60, 0x00,0x50,0xF0,0x90, 0x00,0x50,0xF0,0xC0, 0x00,0x50,0xF0,0xF0,
	0x00,0xA0,0x00,0x00, 0x00,0xA0,0x00,0x30, 0x00,0xA0,0x00,0x60, 0x00,0xA0,0x00,0x90, 0x00,0xA0,0x00,0xC0, 0x00,0xA0,0x00,0xF0,
	0x00,0xA0,0x20,0x00, 0x00,0xA0,0x20,0x30, 0x00,0xA0,0x20,0x60, 0x00,0xA0,0x20,0x90, 0x00,0xA0,0x20,0xC0, 0x00,0xA0,0x20,0xF0,
	0x00,0xA0,0x40,0x00, 0x00,0xA0,0x40,0x30, 0x00,0xA0,0x40,0x60, 0x00,0xA0,0x40,0x90, 0x00,0xA0,0x40,0xC0, 0x00,0xA0,0x40,0xF0,
	0x00,0xA0,0x60,0x00, 0x00,0xA0,0x60,0x30, 0x00,0xA0,0x60,0x60, 0x00,0xA0,0x60,0x90, 0x00,0xA0,0x60,0xC0, 0x00,0xA0,0x60,0xF0,
	0x00,0xA0,0x90,0x00, 0x00,0xA0,0x90,0x30, 0x00,0xA0,0x90,0x60, 0x00,0xA0,0x90,0x90, 0x00,0xA0,0x90,0xC0, 0x00,0xA0,0x90,0xF0,
	0x00,0xA0,0xB0,0x00, 0x00,0xA0,0xB0,0x30, 0x00,0xA0,0xB0,0x60, 0x00,0xA0,0xB0,0x90, 0x00,0xA0,0xB0,0xC0, 0x00,0xA0,0xB0,0xF0,
	0x00,0xA0,0xD0,0x00, 0x00,0xA0,0xD0,0x30, 0x00,0xA0,0xD0,0x60, 0x00,0xA0,0xD0,0x90, 0x00,0xA0,0xD0,0xC0, 0x00,0xA0,0xD0,0xF0,
	0x00,0xA0,0xF0,0x00, 0x00,0xA0,0xF0,0x30, 0x00,0xA0,0xF0,0x60, 0x00,0xA0,0xF0,0x90, 0x00,0xA0,0xF0,0xC0, 0x00,0xA0,0xF0,0xF0,
	0x00,0xF0,0x00,0x00, 0x00,0xF0,0x00,0x30, 0x00,0xF0,0x00,0x60, 0x00,0xF0,0x00,0x90, 0x00,0xF0,0x00,0xC0, 0x00,0xF0,0x00,0xF0,
	0x00,0xF0,0x20,0x00, 0x00,0xF0,0x20,0x30, 0x00,0xF0,0x20,0x60, 0x00,0xF0,0x20,0x90, 0x00,0xF0,0x20,0xC0, 0x00,0xF0,0x20,0xF0,
	0x00,0xF0,0x40,0x00, 0x00,0xF0,0x40,0x30, 0x00,0xF0,0x40,0x60, 0x00,0xF0,0x40,0x90, 0x00,0xF0,0x40,0xC0, 0x00,0xF0,0x40,0xF0,
	0x00,0xF0,0x60,0x00, 0x00,0xF0,0x60,0x30, 0x00,0xF0,0x60,0x60, 0x00,0xF0,0x60,0x90, 0x00,0xF0,0x60,0xC0, 0x00,0xF0,0x60,0xF0,
	0x00,0xF0,0x90,0x00, 0x00,0xF0,0x90,0x30, 0x00,0xF0,0x90,0x60, 0x00,0xF0,0x90,0x90, 0x00,0xF0,0x90,0xC0, 0x00,0xF0,0x90,0xF0,
	0x00,0xF0,0xB0,0x00, 0x00,0xF0,0xB0,0x30, 0x00,0xF0,0xB0,0x60, 0x00,0xF0,0xB0,0x90, 0x00,0xF0,0xB0,0xC0, 0x00,0xF0,0xB0,0xF0,
	0x00,0xF0,0xD0,0x00, 0x00,0xF0,0xD0,0x30, 0x00,0xF0,0xD0,0x60, 0x00,0xF0,0xD0,0x90, 0x00,0xF0,0xD0,0xC0, 0x00,0xF0,0xD0,0xF0,
	0x00,0xF0,0xF0,0x00, 0x00,0xF0,0xF0,0x30, 0x00,0xF0,0xF0,0x60, 0x00,0xF0,0xF0,0x90, 0x00,0xF0,0xF0,0xC0, 0x00,0xF0,0xF0,0xF0,
	0x00,0x10,0x10,0x10, 0x00,0x20,0x20,0x20, 0x00,0x30,0x30,0x30, 0x00,0x40,0x40,0x40, 0x00,0x50,0x50,0x50, 0x00,0x60,0x60,0x60,
	0x00,0x70,0x70,0x70, 0x00,0x80,0x80,0x80, 0x00,0x90,0x90,0x90, 0x00,0xA0,0xA0,0xA0, 0x00,0xB0,0xB0,0xB0, 0x00,0xC0,0xC0,0xC0,
	0x00,0xD0,0xD0,0xD0, 0x00,0xE0,0xE0,0xE0
};


float adat_gamma = 1.0;
float adat_distort = 1.0;

/*
 * 
 * Nieuwe versie:
 * 
 * 32 helderheden	Y			 15 direct te bereiken (zwart & wit zijn specials)
 * 16 kleuren		H ue		
 * 7 intensiteiten	S aturation
 * 
 * Totaal 3584 'verschillende' kleuren. Eerste 512 kleuren vrij.
 * 
 * 
 */

void imb_convhamx(struct ImBuf *ibuf, unsigned char coltab[][4], short *deltab)
/*  struct ImBuf *ibuf; */
/*  uchar coltab[][4]; */
/*  short *deltab; */
{
	short r,g,b,lr,lg,lb,dr,dg,db,col,fout,type,step;
	int i;
	uchar *rect;

	/*
		b 	= 0000 xxxx
		g	= 0001 xxxx
		r	= 0010 xxxx
		cmap  >=  48
	*/

	for (step = 0 ; step < 2 ; step ++){
		rect =  (uchar *) ibuf->rect;
		rect += 4*step;
		i = ((ibuf->x * ibuf->y) + 2 - step - 1) / 2;

		lb = coltab[0][1];
		lg = coltab[0][2];
		lr = coltab[0][3];
		type = col = 0;

		for ( ;i>0;i--){
			b = rect[2] >> 4;
			g = rect[1] >> 4;
			r = rect[0] >> 4;

			if ((b-lb) | (g-lg) | (r-lr)){
				col = ((b<<8) + (g<<4) + r) << 1;
				fout = deltab[col + 1];
				col = deltab[col];
				type = 0;
				dr = quadr[lr-r] ;
				dg = quadr[lg-g] ;
				db = quadr[lb-b];

				if ((dr+dg)<=fout) {
					fout = dr+dg ;
					type = 1;
				}
				if ((dg+db)<=fout) {
					fout = dg+db;
					type = 2;
				}
				if ((dr+db)<=fout) {
					fout = dr+db;
					type = 4;
				}

				switch(type){
				case 1:
					lb = b ;
					col = b;
					break;
				case 4:
					lg = g ;
					col = g+16;
					break;
				case 2:
					lr = r ;
					col = r + 32;
					break;
				default:
					/*printf("%04x %5d %5d  ", (b<<8) + (g<<4) + r, col, fout);*/
					
					lb = coltab[col][1];
					lg = coltab[col][2];
					lr = coltab[col][3];
					/*printf("%01x%01x%01x %01x%01x%01x\n", b, g, r, lb, lg, lr);*/
					col += 48;
				}
			}
			rect[3] = col;
			rect += 8;
		}
	}
}

static short dec_hamx(struct ImBuf * ibuf, unsigned char *body, int cmap[])
/*  struct ImBuf * ibuf; */
/*  uchar *body; */
/*  int cmap[]; */
{
	int todo,i;
	int j,step,col;
	unsigned int *rect;

	for (step = 0 ; step < 2 ; step ++){
		rect = ibuf->rect;
		rect += step;
		todo = (ibuf->x * ibuf->y + 2 - step - 1) / 2;
		col = cmap[0];
		while (todo>0){
			i = *body++;

			if (i & 128){			/* fill */

				i = 257-i;
				todo -= i;
				j = *(body++);

				col = ((col & hamx_array[j]) | hamx_array[j + 256]);

				do{
					*rect = col;
					rect += 2;
				}while (--i);
			} else{				/* copy */
				i++;
				todo-=i;

				do{
					j = *(body++);
					*rect = col = ((col & hamx_array[j]) | hamx_array[j + 256]);
					rect += 2;
				}while (--i);
			}
		}
		if (todo) return (0);
	}
	return(1);
}


struct ImBuf *imb_loadanim(int *iffmem, int flags)
{
	int chunk, totlen, len, *mem, cmaplen = 0;
	unsigned int *cmap;
	uchar *body = 0;
	struct Adat adat;
	struct ImBuf *ibuf=0;
	static int is_flipped = FALSE;
	
	mem=iffmem;
	if (GET_ID(mem) != FORM) return (0);
	if (GET_ID(mem + 2) != ANIM) return (0);
	totlen= (GET_BIG_LONG(mem + 1) + 1) & ~1;
	mem += 3;
	totlen -= 4;
	adat.w = 0;
	adat.xorig = 0;
	adat.yorig = 0;
	adat.gamma = adat_gamma;
	adat.distort = adat_distort;
	
	while(totlen > 0){
		chunk = GET_ID(mem);
		len = (GET_BIG_LONG(mem + 1) + 1) & ~1;
		mem += 2;

		totlen -= len+8;
		switch (chunk){
		case ADAT:
			if (len > sizeof(struct Adat)){
				memcpy(&adat,mem,sizeof(struct Adat));
			} else{
				memcpy(&adat,mem,len);
			}
			adat.w = BIG_SHORT(adat.w);
			adat.h = BIG_SHORT(adat.h);
			adat.type = BIG_SHORT(adat.type);
			adat.xorig = BIG_SHORT(adat.xorig);
			adat.yorig = BIG_SHORT(adat.yorig);
			break;
		case CMAP:
			cmap = (unsigned int *) mem;
			cmaplen = len;
			break;
		case BODY:
			body = (uchar *) mem;
			break;
		}
		mem = (int *)((uchar *)mem +len);
	}

	if (body == 0) return (0);
	if (adat.w == 0) return (0);

	adat_gamma = adat.gamma;
	adat_distort = adat.distort;

	if (flags & IB_test) ibuf=IMB_allocImBuf(adat.w, adat.h, 24, 0, 0);
	else ibuf=IMB_allocImBuf(adat.w, adat.h, 24, IB_rect, 0);
	if (ibuf==0) return (0);

	ibuf->ftype = (Anim | adat.type);
	ibuf->xorig = adat.xorig;
	ibuf->yorig = adat.yorig;
	ibuf->flags = flags;
	
	if (cmaplen){
		ibuf->cmap = malloc(cmaplen);
		memcpy(ibuf->cmap, cmap, cmaplen);
		ibuf->maxcol = cmaplen >> 2;
	}

	if (flags & IB_test){
		if (flags & IB_freem) free(iffmem);
		return(ibuf);
	}

	switch (adat.type){
	case HAMX:
		if (flags & IB_rect){
			if (!is_flipped) {
				int i;
				unsigned int * t;
				t = (unsigned int *) hamx_array_char;
				for (i = 0; i < sizeof(hamx_array_char) / sizeof(int) ; i++) {
					t[i] = SWAP_LONG(t[i]);
				}
				
				t = (unsigned int *) cmap_hamx;
			
				for (i = 0; i < sizeof(cmap_hamx) / sizeof(int) ; i++) {
					t[i] = SWAP_LONG(t[i]);
				}
				
				is_flipped= TRUE;
			}

			if (dec_hamx(ibuf,body,(int*) cmap_hamx) == 0){
				IMB_freeImBuf(ibuf);
				ibuf = 0;
			}
			if (flags & IB_ttob) IMB_flipy(ibuf);
		}
		break;
	default:
		IMB_freeImBuf(ibuf);
		ibuf = 0;
	}
	
	if (flags & IB_freem) free(iffmem);

	return (ibuf);
}


static unsigned char *makebody_anim(int bytes,
							 unsigned char *buf,
							 unsigned char *rect)
/*  register uchar *buf; */
/*  register uchar *rect; */
/*  int bytes; */
{
	register uchar last,this;
	register int copy;
	register uchar *rectstart,*temp;

	bytes--;
	rectstart = rect;
	last = *rect++;
	this = *rect++;
	copy = last^this;
	while (bytes>0){
		if (copy){
			do{
				last = this;
				this = *rect++;
				if (last == this){
					if (this == rect[-3]){		/* drie dezelfde? */
						bytes --;					/* bytes goed zetten */
						break;
					}
				}
			}while (--bytes != 0);

			copy = rect-rectstart;
			copy --;
			if (bytes) copy -= 2;

			temp = rect;
			rect = rectstart;

			while (copy){
				last = copy;
				if (copy>MAXDAT) last = MAXDAT;
				copy -= last;
				*buf++ = last-1;
				do{
					*buf++ = *rect++;
				}while(--last != 0);
			}
			rectstart = rect;
			rect = temp;
			last = this;

			copy = FALSE;
		} else {
			while (*rect++ == this){		/* zoek naar eerste afwijkende byte */
				if (--bytes == 0) break;	/* of einde regel */
			}
			rect --;
			copy = rect-rectstart;
			rectstart = rect;
			bytes --;
			this = *rect++;

			while (copy){
				if (copy>MAXRUN){
					*buf++ = -(MAXRUN-1);
					*buf++ = last;
					copy -= MAXRUN;
				} else {
					*buf++ = -(copy-1);
					*buf++ = last;
					break;
				}
			}
			copy=TRUE;
		}
	}
	return (buf);
}


short imb_enc_anim(struct ImBuf *ibuf, int file)
/*  struct ImBuf *ibuf; */
/*  int file; */
{
	int step, steps, size, i, skip;
	uchar *buf1, *crect, *_buf1, *_buf2, *bufend;
	short ok = TRUE;

	if (ibuf == 0) return (0);
	if (file < 0 ) return (0);
	if (ibuf->rect == 0) return(0);

	/* dither toevoegen */

	switch(ibuf->ftype){
	case AN_hamx:
		ibuf->cmap = (unsigned int *) cmap_hamx;
		ibuf->mincol = 0;
		ibuf->maxcol = sizeof(cmap_hamx) / 4;
		imb_converttoham(ibuf);
		steps = 2;
		break;
	}

	size = ((ibuf->x + 1)* (ibuf->y + 1)) / steps + 1024;
	if ((_buf1  = malloc(size)) == 0) return(0);
	if ((_buf2  = malloc(size)) == 0){
		free(_buf1);
		return(0);
	}

	skip = 4 * steps;
	for (step = 0 ; step < steps ; step ++){
		crect = (uchar *) ibuf->rect;
		crect += 4 * step;
		size = (ibuf->x * ibuf->y + steps - step - 1) / steps;
		buf1 = _buf1;
		if ((ibuf->ftype == AN_hamx) || (ibuf->ftype == AN_yuvx)){
			crect += 3;
			for (i = size ; i>0 ; i--){
				*(buf1 ++) = *crect;
				crect += skip;
			}
		} else{
			for (i = size ; i>0 ; i--){
				*(buf1 ++) = crect[1] + (crect[2] >> 2) + (crect[3] >> 5);
				crect += skip;
			}
		}
		bufend = makebody_anim(size,_buf2,_buf1);
		if (bufend == 0){
			ok = FALSE;
			break;
		}
		size = bufend - _buf2;
		if (write(file, _buf2, size) != size){
			ok = FALSE;
			break;
		}
	}
	free(_buf1);
	free(_buf2);
	return (ok);
}
