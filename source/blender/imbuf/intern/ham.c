/**
 *
 * ***** BEGIN GPL/BL DUAL LICENSE BLOCK *****
 *
 * The contents of this file may be used under the terms of either the GNU
 * General Public License Version 2 or later (the "GPL", see
 * http://www.gnu.org/licenses/gpl.html ), or the Blender License 1.0 or
 * later (the "BL", see http://www.blender.org/BL/ ) which has to be
 * bought from the Blender Foundation to become active, in which case the
 * above mentioned GPL option does not apply.
 *
 * The Original Code is Copyright (C) 2002 by NaN Holding BV.
 * All rights reserved.
 *
 * The Original Code is: all of this file.
 *
 * Contributor(s): none yet.
 *
 * ***** END GPL/BL DUAL LICENSE BLOCK *****
 * ham.c
 *
 * $Id$
 */

#include "BLI_blenlib.h"

#include "imbuf.h"
#include "imbuf_patch.h"
#include "IMB_imbuf_types.h"
#include "IMB_imbuf.h"

#include "IMB_cmap.h"
#include "IMB_hamx.h"
#include "IMB_ham.h"

extern short alpha_col0;

#define HAMB	0x0100
#define HAMG	0x0400
#define HAMR	0x0200
#define HAMC	0x1000
#define HAMFREE	0x2000

static void addhamdither(short x, unsigned char *dit,
				  short dmax, unsigned char *rgb,
				  unsigned short *ham,
				  short type, short round, short shift)
/*  short x, dmax, type, round, shift; */
/*  uchar *dit, *rgb; */
/*  unsigned short *ham; */
{
	short dx = 0;
	short c1, c2;

	for (;x>0;x--){
		if (ham[0] & (HAMFREE | type)){
			c2 = c1 = *rgb;
			
			/* wrap dither */
			while (dx >= dmax) dx -= dmax;
			
			c1 += dit[dx];
			if (c1 > 255) c1 = 255;
			c2 += round;
			if (c2 > 255) c2 = 255;
			
			if (c1 != c2){
				c1 >>= shift; c2 >>= shift;
				if (ham[1] & HAMFREE){
					ham[0] = type + c1;
					ham[1] = type + c2;
				} else if (ham[1] & type){
					ham[0] = type + c1;
				} else if ((ham[2] & (type | HAMFREE)) == type){
					ham[0] = type + c1;
				} else if ((ham[1] & HAMC) | (ham[2] & HAMC)){
					ham[0] = type + c1;
				}
			}
		}
		rgb += 4;
		ham ++;
		dx ++;
	}
}

static void convhamscanl(short x, short y,
				  unsigned char *rgbbase,
				  unsigned char coltab[][4],
				  short *deltab,
				  short bits)
/*  short x, y, bits; */
/*  uchar *rgbbase; */
/*  uchar coltab[][4]; */
/*  short *deltab; */
{
	int a, r, g, b, lr, lg, lb, dr, dg, db, col, fout, type, x2;
	int round, shift;
	uchar *rgb, dit[2];
	unsigned short *ham, *hambase;

	/* Opzet:
		eerst wordt het gehele plaatje afgelopen, waarbij kleurovergangen gecodeerd
		worden: FGRB XXXX XXXX
		F			- vrije kleurwaarde, mag door ieder veranderd worden
		G/R/B		- groen/rood/blauw ham overgang, alleen door die kleur te veranderen
		XXXX XXXX	- N bits waarde.
	
		0000 XXXX XXXX is palet kleur.
	
		daarna wordt eerst de groen dither toegevoegd, dan de rood dither en
		tenslotte wordt blauwdither toegevoegd
	*/

	if ((hambase = (unsigned short *) malloc((x+4) * sizeof(unsigned short)))==0) return;

	lb = coltab[0][1];
	lg = coltab[0][2];
	lr = coltab[0][3];
	type = col = 0;

	ham = hambase;
	rgb = rgbbase;
	
	shift = 8 - bits;
	round = 1 << (shift - 1);
	
	/* om te voorkomen dat er 'ruis' ontstaat aan het einde van de regel */
	for (x2 = 3; x2 >= 0; x2 --) hambase[x + x2] = HAMFREE;
	
	for (x2 = x ;x2 > 0; x2--){
		r = rgb[0] + round;
		g = rgb[1] + round;
		b = rgb[2] + round;
		a = rgb[3];
		
		if (a < 128 && alpha_col0) {
			a = 1;
		} else a = 0;
		
		if (b > 255) b = 255;
		if (g > 255) {
			g = 255;
		}
		if (r > 255) r = 255;

		r >>= shift;
		g >>= shift;
		b >>= shift;

		if ((b-lb) | (g-lg) | (r-lr) | a){
			if (a) {
				col = 0;
				type = HAMC;
			} else {
				col = ((b << (2 * bits)) + (g << bits) + r) << 1;
				fout = deltab[col + 1];
				col = deltab[col];
				type = HAMC;
				
				dr = quadr[lr-r];
				dg = quadr[lg-g];
				db = quadr[lb-b];
	
				if ((dr+dg) <= fout){
					fout = dr+dg;
					col = b;
					type = HAMB;
				}
				if ((dg+db) <= fout){
					fout = dg+db;
					col = r;
					type = HAMR;
				}
				if ((dr+db) <= fout){
					fout = dr+db;
					col = g;
					type = HAMG;
				}
			}
			
			switch(type){
			case HAMG:
				lg = g;
				break;
			case HAMR:
				lr = r;
				break;
			case HAMB:
				lb = b;
				break;
			default:
				lb = coltab[col][1];
				lg = coltab[col][2];
				lr = coltab[col][3];
			}
			*ham = type + col;
		} else *ham = HAMG + HAMFREE + g;

		rgb += 4;
		ham ++;
	}


	if (y & 1){
		dit[0] = 0 << (shift - 2);
		dit[1] = 3 << (shift - 2);
	} else {
		dit[0] = 2 << (shift - 2);
		dit[1] = 1 << (shift - 2);
	}

	addhamdither(x,dit,2,rgbbase+2,hambase,HAMG, round, shift);

	if ((y & 1)==0){
		dit[0] = 3 << (shift - 2);
		dit[1] = 0 << (shift - 2);
	} else {
		dit[0] = 1 << (shift - 2);
		dit[1] = 2 << (shift - 2);
	}
	
	addhamdither(x,dit,2,rgbbase+3,hambase,HAMR, round, shift);
	addhamdither(x,dit,2,rgbbase+1,hambase,HAMB, round, shift);


	ham = hambase;
	rgb = rgbbase;
	rgb += 3;

	for (x2=x;x2>0;x2--){
		type = *(ham++);
		if (type & HAMG) type |= HAMR | HAMB;

		*rgb = (type & 0xff) | ((type & (HAMR | HAMB)) >> shift);
		rgb += 4;
	}

	free (hambase);
}


short imb_converttoham(struct ImBuf *ibuf)
/*  struct ImBuf* ibuf; */
{
	unsigned int coltab[256],*rect;
	short x,y,* deltab;
	int mincol;
	
	memcpy(coltab,ibuf->cmap,4 * ibuf->maxcol);

	mincol = ibuf->mincol;	
	if (alpha_col0 && mincol == 0) mincol = 1;

	if (ibuf->ftype == AN_hamx) {
		deltab = imb_coldeltatab((uchar *) coltab, 0, ibuf->maxcol, 4);
	} else {
		ibuf->cbits = ibuf->depth - 2;
		imb_losecmapbits(ibuf, coltab);
		deltab = imb_coldeltatab((uchar *) coltab, mincol, ibuf->maxcol, ibuf->cbits);
	}
	
	rect = ibuf->rect;
	x=ibuf->x;
	y=ibuf->y;

	if (ibuf->ftype == AN_hamx){
		IMB_dit2(ibuf, 2, 4);
		IMB_dit2(ibuf, 1, 4);
		IMB_dit2(ibuf, 0, 4);
		imb_convhamx(ibuf, coltab, deltab);
	} else {
		for(;y > 0; y--){
			convhamscanl(x, y, rect, coltab, deltab, ibuf->cbits);
			rect += x;
		}
	}

	return (TRUE);
}
