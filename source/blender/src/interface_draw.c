/**
 * $Id$
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
 */

/* 
     a full doc with API notes can be found in bf-blender/blender/doc/interface_API.txt

 */
 

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifndef WIN32
#include <unistd.h>
#else
#include <io.h>
#include "BLI_winstuff.h"
#endif   

#include "MEM_guardedalloc.h"

#include "PIL_time.h"

#include "BLI_blenlib.h"
#include "BLI_arithb.h"

#include "DNA_screen_types.h"
#include "DNA_space_types.h"
#include "DNA_userdef_types.h"
#include "DNA_vec_types.h"

#include "BKE_blender.h"
#include "BKE_utildefines.h"
#include "BKE_global.h"

#include "BIF_gl.h"
#include "BIF_graphics.h"
#include "BIF_keyval.h"
#include "BIF_mainqueue.h"

#include "BIF_screen.h"
#include "BIF_toolbox.h"
#include "BIF_mywindow.h"
#include "BIF_space.h"
#include "BIF_glutil.h"
#include "BIF_interface.h"
#include "BIF_butspace.h"
#include "BIF_language.h"

#include "BSE_view.h"

#include "mydevice.h"
#include "interface.h"
#include "blendef.h"

// globals
extern float UIwinmat[4][4];



/* ************** generic embossed rect, for window sliders etc ************* */

void uiEmboss(float x1, float y1, float x2, float y2, int sel)
{
	
	/* below */
	if(sel) glColor3ub(200,200,200);
	else glColor3ub(50,50,50);
	fdrawline(x1, y1, x2, y1);

	/* right */
	fdrawline(x2, y1, x2, y2);
	
	/* top */
	if(sel) glColor3ub(50,50,50);
	else glColor3ub(200,200,200);
	fdrawline(x1, y2, x2, y2);

	/* left */
	fdrawline(x1, y1, x1, y2);
	
}

/* ************** GENERIC ICON DRAW, NO THEME HERE ************* */

static void ui_draw_icon(uiBut *but, BIFIconID icon)
{
	int blend= 0;
	float xs=0, ys=0;

	if(but->flag & UI_ICON_LEFT) {
		if (but->type==BUTM) {
			xs= but->x1+1.0;
		}
		else if ((but->type==ICONROW) || (but->type==ICONTEXTROW)) {
			xs= but->x1+4.0;
		}
		else {
			xs= but->x1+6.0;
		}
		ys= (but->y1+but->y2- BIF_get_icon_height(icon))/2.0;
	}
	if(but->flag & UI_ICON_RIGHT) {
		xs= but->x2-17.0;
		ys= (but->y1+but->y2- BIF_get_icon_height(icon))/2.0;
	}
	if (!((but->flag & UI_ICON_RIGHT) || (but->flag & UI_ICON_LEFT))) {
		xs= (but->x1+but->x2- BIF_get_icon_width(icon))/2.0;
		ys= (but->y1+but->y2- BIF_get_icon_height(icon))/2.0;
	}

	glRasterPos2f(xs, ys);

	if(but->aspect>1.1) glPixelZoom(1.0/but->aspect, 1.0/but->aspect);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	/* calculate blend color */
	if ELEM4(but->type, ICONTOG, TOG, ROW, TOGN) {
		if(but->flag & UI_SELECT);
		else if(but->flag & UI_ACTIVE);
		else blend= -60;
	}
	BIF_draw_icon_blended(icon, but->themecol, blend);
	
/* old blending method... hrums */
/*	if(but->flag & UI_SELECT) {
		if(but->flag & UI_ACTIVE) {
			BIF_draw_icon_blended(icon, but->themecol, -80);
		} else {
			BIF_draw_icon_blended(icon, but->themecol, -45);
		}
	}
	else {
		if ((but->flag & UI_ACTIVE) && but->type==BUTM) {
			BIF_draw_icon_blended(icon, but->themecol, 0);
		} else if (but->flag & UI_ACTIVE) {
			BIF_draw_icon_blended(icon, but->themecol, 25);
		} else {
			BIF_draw_icon_blended(icon, but->themecol, 45);
		}
	}
*/
	glBlendFunc(GL_ONE, GL_ZERO);
	glDisable(GL_BLEND);

	glPixelZoom(1.0, 1.0);
}


/* ************** DEFAULT THEME, SHADED BUTTONS ************* */


#define M_WHITE		BIF_ThemeColorShade(colorid, 80)

#define M_ACT_LIGHT	BIF_ThemeColorShade(colorid, 55)
#define M_LIGHT		BIF_ThemeColorShade(colorid, 45)
#define M_HILITE	BIF_ThemeColorShade(colorid, 25)
#define M_LMEDIUM	BIF_ThemeColorShade(colorid, 10)
#define M_MEDIUM	BIF_ThemeColor(colorid)
#define M_LGREY		BIF_ThemeColorShade(colorid, -20)
#define M_GREY		BIF_ThemeColorShade(colorid, -45)
#define M_DARK		BIF_ThemeColorShade(colorid, -80)

#define M_NUMTEXT				BIF_ThemeColorShade(colorid, 25)
#define M_NUMTEXT_ACT_LIGHT		BIF_ThemeColorShade(colorid, 35)

#define MM_WHITE	BIF_ThemeColorShade(TH_BUT_NEUTRAL, 120)
#define MM_WHITE_OP	BIF_ThemeColorShadeAlpha(TH_BACK, 65, -100)
#define MM_WHITE_TR	BIF_ThemeColorShadeAlpha(TH_BACK, 65, -255)
#define MM_LIGHT	BIF_ThemeColorShade(TH_BUT_NEUTRAL, 45)
#define MM_MEDIUM	BIF_ThemeColor(TH_BUT_NEUTRAL)
#define MM_GREY		BIF_ThemeColorShade(TH_BUT_NEUTRAL, -45)
#define MM_DARK		BIF_ThemeColorShade(TH_BUT_NEUTRAL, -80)

/* base shaded button */
static void shaded_button(float x1, float y1, float x2, float y2, float asp, int colorid, int flag, int mid)
{
	/* 'mid' arg determines whether the button is in the middle of
	 * an alignment group or not. 0 = not middle, 1 = is in the middle.
	 * Done to allow cleaner drawing
	 */
	 
	/* *** SHADED BUTTON BASE *** */
	glShadeModel(GL_SMOOTH);
	glBegin(GL_QUADS);
	
	if(flag & UI_SELECT) {
		if(flag & UI_ACTIVE) M_MEDIUM;
		else M_LGREY;
	} else {
		if(flag & UI_ACTIVE) M_LIGHT;
		else M_HILITE;
	}

	glVertex2f(x1,y1);
	glVertex2f(x2,y1);

	if(flag & UI_SELECT) {
		if(flag & UI_ACTIVE) M_LGREY;
		else M_GREY;
	} else {
		if(flag & UI_ACTIVE) M_ACT_LIGHT;
		else M_LIGHT;
	}

	glVertex2f(x2,(y2-(y2-y1)/3));
	glVertex2f(x1,(y2-(y2-y1)/3));
	glEnd();
	

	glShadeModel(GL_FLAT);
	glBegin(GL_QUADS);
	
	if(flag & UI_SELECT) {
		if(flag & UI_ACTIVE) M_LGREY;
		else M_GREY;
	} else {
		if(flag & UI_ACTIVE) M_ACT_LIGHT;
		else M_LIGHT;
	}
	
	glVertex2f(x1,(y2-(y2-y1)/3));
	glVertex2f(x2,(y2-(y2-y1)/3));
	glVertex2f(x2,y2);
	glVertex2f(x1,y2);

	glEnd();
	/* *** END SHADED BUTTON BASE *** */
	
	/* *** INNER OUTLINE *** */
	/* left */
	if(!(flag & UI_SELECT)) {
		glShadeModel(GL_SMOOTH);
		glBegin(GL_LINES);
		M_MEDIUM;
		glVertex2f(x1+1,y1+2);
		M_WHITE;
		glVertex2f(x1+1,y2);
		glEnd();
	}
	
	/* right */
		if(!(flag & UI_SELECT)) {
		glShadeModel(GL_SMOOTH);
		glBegin(GL_LINES);
		M_MEDIUM;
		glVertex2f(x2-1,y1+2);
		M_WHITE;
		glVertex2f(x2-1,y2);
		glEnd();
	}
	
	glShadeModel(GL_FLAT);
	
	/* top */
	if(flag & UI_SELECT) {
		if(flag & UI_ACTIVE) M_LGREY;
		else M_GREY;
	} else {
		if(flag & UI_ACTIVE) M_WHITE;
		else M_WHITE;
	}

	fdrawline(x1, (y2-1), x2, (y2-1));
	
	/* bottom */
	if(flag & UI_SELECT) {
		if(flag & UI_ACTIVE) M_MEDIUM;
		else M_LGREY;
	} else {
		if(flag & UI_ACTIVE) M_LMEDIUM;
		else M_MEDIUM;
	}
	fdrawline(x1, (y1+1), x2, (y1+1));
	/* *** END INNER OUTLINE *** */
	
	/* *** OUTER OUTLINE *** */
	if (mid) {
		// we draw full outline, its not AA, and it works better button mouse-over hilite
		MM_DARK;
		
		// left right
		fdrawline(x1, y1, x1, y2);
		fdrawline(x2, y1, x2, y2);
	
		// top down
		fdrawline(x1, y2, x2, y2);
		fdrawline(x1, y1, x2, y1); 
	} else {
		MM_DARK;
		glBegin(GL_LINE_LOOP);
		gl_round_box(x1, y1, x2, y2, 1.0);
		glEnd();
	}
	/* END OUTER OUTLINE */
}

/* base flat button */
static void flat_button(float x1, float y1, float x2, float y2, float asp, int colorid, int flag, int mid)
{
	/* 'mid' arg determines whether the button is in the middle of
	 * an alignment group or not. 0 = not middle, 1 = is in the middle.
	 * Done to allow cleaner drawing
	 */
	 
	/* *** FLAT TEXT/NUM FIELD *** */
	glShadeModel(GL_FLAT);
	if(flag & UI_SELECT) {
		if(flag & UI_ACTIVE) M_LGREY;
		else M_GREY;
	}
	else {
		if(flag & UI_ACTIVE) M_NUMTEXT_ACT_LIGHT;
		else M_NUMTEXT;
	}

	glRectf(x1, y1, x2, y2);
	/* *** END FLAT TEXT/NUM FIELD *** */
	
	/* *** OUTER OUTLINE *** */
	if (mid) {
		// we draw full outline, its not AA, and it works better button mouse-over hilite
		MM_DARK;
		
		// left right
		fdrawline(x1, y1, x1, y2);
		fdrawline(x2, y1, x2, y2);
	
		// top down
		fdrawline(x1, y2, x2, y2);
		fdrawline(x1, y1, x2, y1); 
	} else {
		MM_DARK;
		glBegin(GL_LINE_LOOP);
		gl_round_box(x1, y1, x2, y2, 1.0);
		glEnd();
	}
	/* END OUTER OUTLINE */
}

/* small side double arrow for iconrow */
static void ui_default_iconrow_arrows(float x1, float y1, float x2, float y2)
{
	glEnable( GL_POLYGON_SMOOTH );
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	
	glShadeModel(GL_FLAT);
	glBegin(GL_TRIANGLES);
	glVertex2f((short)x2-2,(short)(y2-(y2-y1)/2)+1);
	glVertex2f((short)x2-6,(short)(y2-(y2-y1)/2)+1);
	glVertex2f((short)x2-4,(short)(y2-(y2-y1)/2)+4);
	glEnd();
		
	glBegin(GL_TRIANGLES);
	glVertex2f((short)x2-2,(short)(y2-(y2-y1)/2) -1);
	glVertex2f((short)x2-6,(short)(y2-(y2-y1)/2) -1);
	glVertex2f((short)x2-4,(short)(y2-(y2-y1)/2) -4);
	glEnd();
	
	glDisable( GL_BLEND );
	glDisable( GL_POLYGON_SMOOTH );
}

/* side double arrow for menu */
static void ui_default_menu_arrows(float x1, float y1, float x2, float y2)
{
	glEnable( GL_POLYGON_SMOOTH );
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	
	glShadeModel(GL_FLAT);
	glBegin(GL_TRIANGLES);
	glVertex2f((short)x2-4,(short)(y2-(y2-y1)/2)+1);
	glVertex2f((short)x2-12,(short)(y2-(y2-y1)/2)+1);
	glVertex2f((short)x2-8,(short)(y2-(y2-y1)/2)+4);
	glEnd();
		
	glBegin(GL_TRIANGLES);
	glVertex2f((short)x2-4,(short)(y2-(y2-y1)/2) -1);
	glVertex2f((short)x2-12,(short)(y2-(y2-y1)/2) -1);
	glVertex2f((short)x2-8,(short)(y2-(y2-y1)/2) -4);
	glEnd();
	
	glDisable( GL_BLEND );
	glDisable( GL_POLYGON_SMOOTH );
}

/* left/right arrows for number fields */
static void ui_default_num_arrows(float x1, float y1, float x2, float y2)
{
	glEnable( GL_POLYGON_SMOOTH );
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	
	glShadeModel(GL_FLAT);
	glBegin(GL_TRIANGLES);
	
	glVertex2f((short)x1+5,(short)(y2-(y2-y1)/2));
	glVertex2f((short)x1+10,(short)(y2-(y2-y1)/2)+4);
	glVertex2f((short)x1+10,(short)(y2-(y2-y1)/2)-4);
	glEnd();

	/* right */
	glShadeModel(GL_FLAT);
	glBegin(GL_TRIANGLES);

	glVertex2f((short)x2-5,(short)(y2-(y2-y1)/2));
	glVertex2f((short)x2-10,(short)(y2-(y2-y1)/2)-4);
	glVertex2f((short)x2-10,(short)(y2-(y2-y1)/2)+4);
	glEnd();
	
	glDisable( GL_BLEND );
	glDisable( GL_POLYGON_SMOOTH );

}

/* button/popup menu/iconrow drawing code */
static void ui_default_button(int type, int colorid, float asp, float x1, float y1, float x2, float y2, int flag)
{
	int align= (flag & UI_BUT_ALIGN);

	if(align) {
	
		/* *** BOTTOM OUTER SUNKEN EFFECT *** */
		if (align != UI_BUT_ALIGN_DOWN) {
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glEnable(GL_BLEND);
			MM_WHITE_OP;
			fdrawline(x1, y1-1, x2, y1-1);	
			glDisable(GL_BLEND);
		}
		/* *** END BOTTOM OUTER SUNKEN EFFECT *** */
		
		switch(align) {
		case UI_BUT_ALIGN_TOP:
			uiSetRoundBox(12);
			
			/* last arg in shaded_button() determines whether the button is in the middle of
			 * an alignment group or not. 0 = not middle, 1 = is in the middle.
			 * Done to allow cleaner drawing
			 */
			 
			shaded_button(x1, y1, x2, y2, asp, colorid, flag, 0);
			break;
		case UI_BUT_ALIGN_DOWN:
			uiSetRoundBox(3);
			shaded_button(x1, y1, x2, y2, asp, colorid, flag, 0);
			break;
		case UI_BUT_ALIGN_LEFT:
			
			/* RIGHT OUTER SUNKEN EFFECT */
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glEnable(GL_BLEND);
			glShadeModel(GL_SMOOTH);
			glBegin(GL_LINES);
			MM_WHITE_OP;
			glVertex2f(x2+1,y1);
			MM_WHITE_TR;
			glVertex2f(x2+1,y2);
			glEnd();
			glDisable(GL_BLEND);
			
			uiSetRoundBox(6);
			shaded_button(x1, y1, x2, y2, asp, colorid, flag, 0);
			break;
		case UI_BUT_ALIGN_RIGHT:
		
			/* LEFT OUTER SUNKEN EFFECT */
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glEnable(GL_BLEND);
			glShadeModel(GL_SMOOTH);
			glBegin(GL_LINES);
			MM_WHITE_OP;
			glVertex2f(x1-1,y1);
			MM_WHITE_TR;
			glVertex2f(x1-1,y2);
			glEnd();
			glDisable(GL_BLEND);
		
			uiSetRoundBox(9);
			shaded_button(x1, y1, x2, y2, asp, colorid, flag, 0);
			break;
			
		case UI_BUT_ALIGN_DOWN|UI_BUT_ALIGN_RIGHT:
			uiSetRoundBox(1);
			shaded_button(x1, y1, x2, y2, asp, colorid, flag, 0);
			break;
		case UI_BUT_ALIGN_DOWN|UI_BUT_ALIGN_LEFT:
			uiSetRoundBox(2);
			shaded_button(x1, y1, x2, y2, asp, colorid, flag, 0);
			break;
		case UI_BUT_ALIGN_TOP|UI_BUT_ALIGN_RIGHT:
		
			/* LEFT OUTER SUNKEN EFFECT */
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glEnable(GL_BLEND);
			glShadeModel(GL_SMOOTH);
			glBegin(GL_LINES);
			MM_WHITE_OP;
			glVertex2f(x1-1,y1);
			MM_WHITE_TR;
			glVertex2f(x1-1,y2);
			glEnd();
			glDisable(GL_BLEND);
		
			uiSetRoundBox(8);
			shaded_button(x1, y1, x2, y2, asp, colorid, flag, 0);
			break;
		case UI_BUT_ALIGN_TOP|UI_BUT_ALIGN_LEFT:
		
			/* RIGHT OUTER SUNKEN EFFECT */
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glEnable(GL_BLEND);
			glShadeModel(GL_SMOOTH);
			glBegin(GL_LINES);
			MM_WHITE_OP;
			glVertex2f(x2+1,y1);
			MM_WHITE_TR;
			glVertex2f(x2+1,y2);
			glEnd();
			glDisable(GL_BLEND);
			
			uiSetRoundBox(4);
			shaded_button(x1, y1, x2, y2, asp, colorid, flag, 0);
			break;
			
		default:
			shaded_button(x1, y1, x2, y2, asp, colorid, flag, 1);
			break;
		}
	} 
	else {
		uiSetRoundBox(15);
		shaded_button(x1, y1, x2, y2, asp, colorid, flag, 0);
	}
	
	/* *** EXTRA DRAWING FOR SPECIFIC CONTROL TYPES *** */
	switch(type) {
	case ICONROW:
	case ICONTEXTROW:
		/* DARKENED AREA */
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_BLEND);
		
		glColor4ub(0, 0, 0, 30);
		glRectf(x2-9, y1, x2, y2);
	
		glDisable(GL_BLEND);
		/* END DARKENED AREA */
	
		/* ICONROW DOUBLE-ARROW  */
		M_DARK;
		ui_default_iconrow_arrows(x1, y1, x2, y2);
		/* END ICONROW DOUBLE-ARROW */
		break;
	case MENU:
		/* DARKENED AREA */
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_BLEND);
		
		glColor4ub(0, 0, 0, 30);
		glRectf(x2-18, y1, x2, y2);
	
		glDisable(GL_BLEND);
		/* END DARKENED AREA */
	
		/* MENU DOUBLE-ARROW  */
		M_DARK;
		ui_default_menu_arrows(x1, y1, x2, y2);
		/* MENU DOUBLE-ARROW */
	}
	
	
}


/* button/popup menu/iconrow drawing code */
static void ui_default_flat(int type, int colorid, float asp, float x1, float y1, float x2, float y2, int flag)
{
	int align= (flag & UI_BUT_ALIGN);

	if(align) {
	
		/* *** BOTTOM OUTER SUNKEN EFFECT *** */
		if (align != UI_BUT_ALIGN_DOWN) {
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glEnable(GL_BLEND);
			MM_WHITE_OP;
			fdrawline(x1, y1-1, x2, y1-1);	
			glDisable(GL_BLEND);
		}
		/* *** END BOTTOM OUTER SUNKEN EFFECT *** */
		
		switch(align) {
		case UI_BUT_ALIGN_TOP:
			uiSetRoundBox(12);
			
			/* last arg in shaded_button() determines whether the button is in the middle of
			 * an alignment group or not. 0 = not middle, 1 = is in the middle.
			 * Done to allow cleaner drawing
			 */
			 
			flat_button(x1, y1, x2, y2, asp, colorid, flag, 0);
			break;
		case UI_BUT_ALIGN_DOWN:
			uiSetRoundBox(3);
			flat_button(x1, y1, x2, y2, asp, colorid, flag, 0);
			break;
		case UI_BUT_ALIGN_LEFT:
			
			/* RIGHT OUTER SUNKEN EFFECT */
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glEnable(GL_BLEND);
			glShadeModel(GL_SMOOTH);
			glBegin(GL_LINES);
			MM_WHITE_OP;
			glVertex2f(x2+1,y1);
			MM_WHITE_TR;
			glVertex2f(x2+1,y2);
			glEnd();
			glDisable(GL_BLEND);
			
			uiSetRoundBox(6);
			flat_button(x1, y1, x2, y2, asp, colorid, flag, 0);
			break;
		case UI_BUT_ALIGN_RIGHT:
		
			/* LEFT OUTER SUNKEN EFFECT */
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glEnable(GL_BLEND);
			glShadeModel(GL_SMOOTH);
			glBegin(GL_LINES);
			MM_WHITE_OP;
			glVertex2f(x1-1,y1);
			MM_WHITE_TR;
			glVertex2f(x1-1,y2);
			glEnd();
			glDisable(GL_BLEND);
		
			uiSetRoundBox(9);
			flat_button(x1, y1, x2, y2, asp, colorid, flag, 0);
			break;
			
		case UI_BUT_ALIGN_DOWN|UI_BUT_ALIGN_RIGHT:
			uiSetRoundBox(1);
			flat_button(x1, y1, x2, y2, asp, colorid, flag, 0);
			break;
		case UI_BUT_ALIGN_DOWN|UI_BUT_ALIGN_LEFT:
			uiSetRoundBox(2);
			flat_button(x1, y1, x2, y2, asp, colorid, flag, 0);
			break;
		case UI_BUT_ALIGN_TOP|UI_BUT_ALIGN_RIGHT:
		
			/* LEFT OUTER SUNKEN EFFECT */
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glEnable(GL_BLEND);
			glShadeModel(GL_SMOOTH);
			glBegin(GL_LINES);
			MM_WHITE_OP;
			glVertex2f(x1-1,y1);
			MM_WHITE_TR;
			glVertex2f(x1-1,y2);
			glEnd();
			glDisable(GL_BLEND);
		
			uiSetRoundBox(8);
			flat_button(x1, y1, x2, y2, asp, colorid, flag, 0);
			break;
		case UI_BUT_ALIGN_TOP|UI_BUT_ALIGN_LEFT:
		
			/* RIGHT OUTER SUNKEN EFFECT */
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glEnable(GL_BLEND);
			glShadeModel(GL_SMOOTH);
			glBegin(GL_LINES);
			MM_WHITE_OP;
			glVertex2f(x2+1,y1);
			MM_WHITE_TR;
			glVertex2f(x2+1,y2);
			glEnd();
			glDisable(GL_BLEND);
			
			uiSetRoundBox(4);
			flat_button(x1, y1, x2, y2, asp, colorid, flag, 0);
			break;
			
		default:
			flat_button(x1, y1, x2, y2, asp, colorid, flag, 1);
			break;
		}
	} 
	else {
		uiSetRoundBox(15);
		flat_button(x1, y1, x2, y2, asp, colorid, flag, 0);
	}
	
	/* *** EXTRA DRAWING FOR SPECIFIC CONTROL TYPES *** */
	switch(type) {
	case NUM:
		/* SIDE ARROWS */
		/* left */
		if(flag & UI_SELECT) {
			if(flag & UI_ACTIVE) M_DARK;
			else M_DARK;
		} else {
			if(flag & UI_ACTIVE) M_GREY;
			else M_LGREY;
		}
		
		ui_default_num_arrows(x1, y1, x2, y2);
		/* END SIDE ARROWS */
	}
}

static void ui_default_slider(int colorid, float fac, float aspect, float x1, float y1, float x2, float y2, int flag)
{
	float ymid, yc;

	/* the slider background line */
	ymid= (y1+y2)/2.0;
	//yc= 2.5*aspect;	// height of center line
	yc = 2.3; // height of center line
	
	if(flag & UI_SELECT) 
			BIF_ThemeColorShade(TH_BUT_NUM, -5);
	else
		if(flag & UI_ACTIVE) 
			BIF_ThemeColorShade(TH_BUT_NUM, +35); 
		else
			BIF_ThemeColorShade(TH_BUT_NUM, +25); 

	glRectf(x1, ymid-yc, x2, ymid+yc);
	
	/* top inner bevel */
	if(flag & UI_SELECT) BIF_ThemeColorShade(TH_BUT_NUM, -40); 
	else BIF_ThemeColor(TH_BUT_NUM); 
	fdrawline(x1+1, ymid+yc, x2, ymid+yc);
	
	/* bottom inner bevel */
	if(flag & UI_SELECT) BIF_ThemeColorShade(TH_BUT_NUM, +10); 
	else BIF_ThemeColorShade(TH_BUT_NUM, +45); 
	fdrawline(x1+1, ymid-yc, x2, ymid-yc);
	
	
	/* the movable slider */
	if(flag & UI_SELECT) BIF_ThemeColorShade(TH_BUT_NUM, +80); 
	else BIF_ThemeColorShade(TH_BUT_NUM, -45); 

	glShadeModel(GL_SMOOTH);
	glBegin(GL_QUADS);

	BIF_ThemeColorShade(TH_BUT_NUM, -45); 

	glVertex2f(x1,     y1+2.5);
	glVertex2f(x1+fac, y1+2.5);

	BIF_ThemeColor(TH_BUT_NUM); 

	glVertex2f(x1+fac, y2-2.5);
	glVertex2f(x1,     y2-2.5);

	glEnd();
	

	/* slider handle center */
	glShadeModel(GL_SMOOTH);
	glBegin(GL_QUADS);

	BIF_ThemeColor(TH_BUT_NUM); 
	glVertex2f(x1+fac-3, y1+2);
	glVertex2f(x1+fac, y1+4);
	BIF_ThemeColorShade(TH_BUT_NUM, +80); 
	glVertex2f(x1+fac, y2-2);
	glVertex2f(x1+fac-3, y2-2);

	glEnd();
	
	/* slider handle left bevel */
	BIF_ThemeColorShade(TH_BUT_NUM, +80); 
	fdrawline(x1+fac-3, y2-2, x1+fac-3, y1+2);
	
	/* slider handle right bevel */
	BIF_ThemeColorShade(TH_BUT_NUM, -45); 
	fdrawline(x1+fac, y2-2, x1+fac, y1+2);

	glShadeModel(GL_FLAT);
}

/* default theme callback */
static void ui_draw_default(int type, int colorid, float aspect, float x1, float y1, float x2, float y2, int flag)
{

	switch(type) {
	case TEX:
	case NUM: 
		ui_default_flat(type, colorid, aspect, x1, y1, x2, y2, flag);
		break;
	case ICONROW: 
	case ICONTEXTROW: 
	case MENU: 
	default: 
		ui_default_button(type, colorid, aspect, x1, y1, x2, y2, flag);
	}

}


/* *************** OLDSKOOL THEME ***************** */

static void ui_draw_outlineX(float x1, float y1, float x2, float y2, float asp1)
{
	float vec[2];
	
	glBegin(GL_LINE_LOOP);
	vec[0]= x1+asp1; vec[1]= y1-asp1;
	glVertex2fv(vec);
	vec[0]= x2-asp1; 
	glVertex2fv(vec);
	vec[0]= x2+asp1; vec[1]= y1+asp1;
	glVertex2fv(vec);
	vec[1]= y2-asp1;
	glVertex2fv(vec);
	vec[0]= x2-asp1; vec[1]= y2+asp1;
	glVertex2fv(vec);
	vec[0]= x1+asp1;
	glVertex2fv(vec);
	vec[0]= x1-asp1; vec[1]= y2-asp1;
	glVertex2fv(vec);
	vec[1]= y1+asp1;
	glVertex2fv(vec);
	glEnd();                
        
}


static void ui_draw_oldskool(int type, int colorid, float asp, float x1, float y1, float x2, float y2, int flag)
{
 	/* paper */
	if(flag & UI_SELECT) {
		if(flag & UI_ACTIVE) BIF_ThemeColorShade(colorid, -40);
		else BIF_ThemeColorShade(colorid, -30);
	}
	else {
		if(flag & UI_ACTIVE) BIF_ThemeColorShade(colorid, +30);
		else BIF_ThemeColorShade(colorid, +20);
	}
	
	glRectf(x1+1, y1+1, x2-1, y2-1);

	x1+= asp;
	x2-= asp;
	y1+= asp;
	y2-= asp;

	/* below */
	if(flag & UI_SELECT) BIF_ThemeColorShade(colorid, 0);
	else BIF_ThemeColorShade(colorid, -30);
	fdrawline(x1, y1, x2, y1);

	/* right */
	fdrawline(x2, y1, x2, y2);
	
	/* top */
	if(flag & UI_SELECT) BIF_ThemeColorShade(colorid, -30);
	else BIF_ThemeColorShade(colorid, 0);
	fdrawline(x1, y2, x2, y2);

	/* left */
	fdrawline(x1, y1, x1, y2);
	
	/* outline */
	glColor3ub(0,0,0);
	ui_draw_outlineX(x1, y1, x2, y2, asp);
	
	
	/* special type decorations */
	switch(type) {
	case NUM:
		if(flag & UI_SELECT) BIF_ThemeColorShade(colorid, -60);
		else BIF_ThemeColorShade(colorid, -30);
		ui_default_num_arrows(x1, y1, x2, y2);
		break;

	case ICONROW: 
	case ICONTEXTROW: 
		if(flag & UI_ACTIVE) BIF_ThemeColorShade(colorid, 0);
		else BIF_ThemeColorShade(colorid, -10);
		glRectf(x2-9, y1+asp, x2-asp, y2-asp);

		BIF_ThemeColorShade(colorid, -50);
		ui_default_iconrow_arrows(x1, y1, x2, y2);
		break;
		
	case MENU: 
		if(flag & UI_ACTIVE) BIF_ThemeColorShade(colorid, 0);
		else BIF_ThemeColorShade(colorid, -10);
		glRectf(x2-17, y1+asp, x2-asp, y2-asp);

		BIF_ThemeColorShade(colorid, -50);
		ui_default_menu_arrows(x1, y1, x2, y2);
		break;
	}
	
}

/* *************** BASIC ROUNDED THEME ***************** */

static void round_button(float x1, float y1, float x2, float y2, float asp, int colorid)
{
	float rad;
	
	rad= (y2-y1)/2.0;
	if(rad>7.0) rad= 7.0;
	
	glBegin(GL_POLYGON);
	gl_round_box(x1, y1, x2, y2, rad);
	glEnd();
	
	BIF_ThemeColorBlendShade(colorid, TH_BACK, 0.5, -70);
	
	glBegin(GL_LINE_LOOP);
	gl_round_box(x1, y1, x2, y2, rad);
	glEnd();
}

/* button in midst of alignment row */
static void round_button_mid(float x1, float y1, float x2, float y2, float asp, int colorid, int align)
{
	glRectf(x1, y1, x2, y2);
	
	BIF_ThemeColorBlendShade(colorid, TH_BACK, 0.5, -70);
	// we draw full outline, its not AA, and it works better button mouse-over hilite
	
	// left right
	fdrawline(x1, y1, x1, y2);
	fdrawline(x2, y1, x2, y2);

	// top down
	fdrawline(x1, y2, x2, y2);
	fdrawline(x1, y1, x2, y1);   
}

static void ui_draw_round(int type, int colorid, float asp, float x1, float y1, float x2, float y2, int flag)
{
	int align= (flag & UI_BUT_ALIGN);
	
	/* paper */
	if(flag & UI_SELECT) {
		if(flag & UI_ACTIVE) BIF_ThemeColorShade(colorid, -40);
		else BIF_ThemeColorShade(colorid, -30);
	}
	else {
		if(flag & UI_ACTIVE) BIF_ThemeColorShade(colorid, +30);
		else BIF_ThemeColorShade(colorid, +20);
	}
	
	if(align) {
		switch(align) {
		case UI_BUT_ALIGN_TOP:
			uiSetRoundBox(12);
			round_button(x1, y1, x2, y2, asp, colorid);
			break;
		case UI_BUT_ALIGN_DOWN:
			uiSetRoundBox(3);
			round_button(x1, y1, x2, y2, asp, colorid);
			break;
		case UI_BUT_ALIGN_LEFT:
			uiSetRoundBox(6);
			round_button(x1, y1, x2, y2, asp, colorid);
			break;
		case UI_BUT_ALIGN_RIGHT:
			uiSetRoundBox(9);
			round_button(x1, y1, x2, y2, asp, colorid);
			break;
			
		case UI_BUT_ALIGN_DOWN|UI_BUT_ALIGN_RIGHT:
			uiSetRoundBox(1);
			round_button(x1, y1, x2, y2, asp, colorid);
			break;
		case UI_BUT_ALIGN_DOWN|UI_BUT_ALIGN_LEFT:
			uiSetRoundBox(2);
			round_button(x1, y1, x2, y2, asp, colorid);
			break;
		case UI_BUT_ALIGN_TOP|UI_BUT_ALIGN_RIGHT:
			uiSetRoundBox(8);
			round_button(x1, y1, x2, y2, asp, colorid);
			break;
		case UI_BUT_ALIGN_TOP|UI_BUT_ALIGN_LEFT:
			uiSetRoundBox(4);
			round_button(x1, y1, x2, y2, asp, colorid);
			break;
			
		default:
			round_button_mid(x1, y1, x2, y2, asp, colorid, align);
			break;
		}
	} 
	else {
		uiSetRoundBox(15);
		round_button(x1, y1, x2, y2, asp, colorid);
	}
	
	/* special type decorations */
	switch(type) {
	case NUM:
		if(flag & UI_SELECT) BIF_ThemeColorShade(colorid, -60);
		else BIF_ThemeColorShade(colorid, -30);
		ui_default_num_arrows(x1, y1, x2, y2);
		break;

	case ICONROW: 
	case ICONTEXTROW: 
		if(flag & UI_ACTIVE) BIF_ThemeColorShade(colorid, 0);
		else BIF_ThemeColorShade(colorid, -10);
		// assuming its not inside alignment...
		uiSetRoundBox(6);
		glBegin(GL_POLYGON);
		gl_round_box(x2-9, y1+asp, x2-asp, y2-asp, 7.0);
		glEnd();

		BIF_ThemeColorShade(colorid, -60);
		ui_default_iconrow_arrows(x1, y1, x2, y2);
		break;
		
	case MENU: 
		if(flag & UI_ACTIVE) BIF_ThemeColorShade(colorid, 0);
		else BIF_ThemeColorShade(colorid, -10);
		// assuming its not inside alignment...
		if(x2-x1 > 24) {
			uiSetRoundBox(6);
			glBegin(GL_POLYGON);
			gl_round_box(x2-16, y1+asp, x2-asp, y2-asp, 7.0);
			glEnd();
		}
		BIF_ThemeColorShade(colorid, -60);
		ui_default_menu_arrows(x1, y1, x2, y2);
		break;
	}
}

/* *************** MINIMAL THEME ***************** */

// theme can define an embosfunc and sliderfunc, text+icon drawing is standard, no theme.



/* super minimal button as used in logic menu */
static void ui_draw_minimal(int type, int colorid, float asp, float x1, float y1, float x2, float y2, int flag)
{
	
	x1+= asp;
	x2-= asp;
	y1+= asp;
	y2-= asp;

	/* paper */
	if(flag & UI_SELECT) {
		if(flag & UI_ACTIVE) BIF_ThemeColorShade(colorid, -40);
		else BIF_ThemeColorShade(colorid, -30);
	}
	else {
		if(flag & UI_ACTIVE) BIF_ThemeColorShade(colorid, +20);
		else BIF_ThemeColorShade(colorid, +10);
	}
	
	glRectf(x1, y1, x2, y2);

	if(flag & UI_SELECT) {
		BIF_ThemeColorShade(colorid, -60);

		/* top */
		fdrawline(x1, y2, x2, y2);
		/* left */
		fdrawline(x1, y1, x1, y2);
		BIF_ThemeColorShade(colorid, +40);

		/* below */
		fdrawline(x1, y1, x2, y1);
		/* right */
		fdrawline(x2, y1, x2, y2);
	}
	else {
		BIF_ThemeColorShade(colorid, +40);

		/* top */
		fdrawline(x1, y2, x2, y2);
		/* left */
		fdrawline(x1, y1, x1, y2);
		
		BIF_ThemeColorShade(colorid, -60);
		/* below */
		fdrawline(x1, y1, x2, y1);
		/* right */
		fdrawline(x2, y1, x2, y2);
	}
	
	/* special type decorations */
	switch(type) {
	case NUM:
		if(flag & UI_SELECT) BIF_ThemeColorShade(colorid, -60);
		else BIF_ThemeColorShade(colorid, -30);
		ui_default_num_arrows(x1, y1, x2, y2);
		break;

	case ICONROW: 
	case ICONTEXTROW: 
		if(flag & UI_ACTIVE) BIF_ThemeColorShade(colorid, 0);
		else BIF_ThemeColorShade(colorid, -10);
		glRectf(x2-9, y1+asp, x2-asp, y2-asp);

		BIF_ThemeColorShade(colorid, -50);
		ui_default_iconrow_arrows(x1, y1, x2, y2);
		break;
		
	case MENU: 
		if(flag & UI_ACTIVE) BIF_ThemeColorShade(colorid, 0);
		else BIF_ThemeColorShade(colorid, -10);
		glRectf(x2-17, y1+asp, x2-asp, y2-asp);

		BIF_ThemeColorShade(colorid, -50);
		ui_default_menu_arrows(x1, y1, x2, y2);
		break;
	}
	
	
}


/* fac is the slider handle position between x1 and x2 */
static void ui_draw_slider(int colorid, float fac, float aspect, float x1, float y1, float x2, float y2, int flag)
{
	float ymid, yc;

	/* the slider background line */
	ymid= (y1+y2)/2.0;
	yc= 1.7*aspect;	

	if(flag & UI_ACTIVE) 
		BIF_ThemeColorShade(colorid, -50); 
	else 
		BIF_ThemeColorShade(colorid, -40); 

	/* left part */
	glRectf(x1, ymid-2.0*yc, x1+fac, ymid+2.0*yc);
	/* right part */
	glRectf(x1+fac, ymid-yc, x2, ymid+yc);

	/* the movable slider */
	
	BIF_ThemeColorShade(colorid, +70); 
	glRectf(x1+fac-aspect, ymid-2.0*yc, x1+fac+aspect, ymid+2.0*yc);

}

/* ************** STANDARD MENU DRAWING FUNCTION (no callback yet) ************* */


// background for pulldowns, pullups, and other frontbuffer drawing temporal menus....
// has to be made themable still (now only color)

void uiDrawMenuBox(float minx, float miny, float maxx, float maxy)
{
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	
	glColor4ub(0, 0, 0, 20);
	
	/* to prevent gaps being drawn between box and shadow (rounding errors?) */
	fdrawline(minx+3, miny+0.25, maxx+0.25, miny+0.25);
	fdrawline(maxx+0.25, miny+0.25, maxx+0.25, maxy-3);
	
	glColor4ub(0, 0, 0, 70);
	fdrawline(minx+3, miny, maxx+1, miny);
	fdrawline(maxx+1, miny, maxx+1, maxy-3);
	
	glColor4ub(0, 0, 0, 70);
	fdrawline(minx+3, miny-1, maxx+1, miny-1);
	fdrawline(maxx+1, miny-1, maxx+1, maxy-3);

	glColor4ub(0, 0, 0, 55);
	fdrawline(minx+3, miny-2, maxx+2, miny-2);
	fdrawline(maxx+2, miny-2, maxx+2, maxy-3);

	glColor4ub(0, 0, 0, 35);
	fdrawline(minx+3, miny-3, maxx+3, miny-3);
	fdrawline(maxx+3, miny-3, maxx+3, maxy-3);

	glColor4ub(0, 0, 0, 20);
	fdrawline(minx+3, miny-4, maxx+4, miny-4);
	fdrawline(maxx+4, miny-4, maxx+4, maxy-3);
	
	glDisable(GL_BLEND);
	
	BIF_ThemeColor(TH_MENU_BACK);

	glRectf(minx, miny, maxx, maxy);
}

/* pulldown menu */
static void ui_draw_pulldown(int type, int colorid, float asp, float x1, float y1, float x2, float y2, int flag)
{

	if(flag & UI_ACTIVE) {
		BIF_ThemeColor(TH_MENU_HILITE);
		glRectf(x1-1, y1, x2+2, y2);

	} else {
		BIF_ThemeColor(colorid);	// is set at TH_MENU_ITEM when pulldown opened.
		glRectf(x1-1, y1, x2+2, y2);
	}

}




/* ************** TEXT AND ICON DRAWING FUNCTIONS ************* */



/* draws text and icons for buttons */
static void ui_draw_text_icon(uiBut *but)
{
	float x;
	int len;
	char *cpoin;
	
	/* check for button text label */
	if (but->type == ICONTEXTROW) {
		ui_draw_icon(but, (BIFIconID) (but->icon+but->iconadd));
	}
	else if(but->drawstr[0]!=0) {
		
		// text button cursor
		if(but->pos != -1) {
			short t, pos, ch;
			
			pos= but->pos+strlen(but->str);
			if(pos >= but->ofs) {
				ch= but->drawstr[pos];
				but->drawstr[pos]= 0;
	
				t= but->aspect*BIF_GetStringWidth(but->font, but->drawstr+but->ofs, (U.transopts & TR_BUTTONS)) + 3;
	
				but->drawstr[pos]= ch;
				glColor3ub(255,0,0);
	
				glRects(but->x1+t, but->y1+2, but->x1+t+3, but->y2-2);
			}	
		}
		// cut string in 2 parts
		cpoin= strchr(but->drawstr, '|');
		if(cpoin) *cpoin= 0;

		/* If there's an icon too (made with uiDefIconTextBut) then draw the icon
		and offset the text label to accomodate it */
		
		if ( (but->flag & UI_HAS_ICON) && (but->flag & UI_ICON_LEFT) ) {
			ui_draw_icon(but, but->icon);

			if(but->flag & UI_TEXT_LEFT) x= but->x1+24.0;
			else x= (but->x1+but->x2-but->strwidth+1)/2.0;
		}
		else {
			if(but->flag & UI_TEXT_LEFT) x= but->x1+4.0;
			else x= (but->x1+but->x2-but->strwidth+1)/2.0;
		}
		
		/* text color, with pulldown item exception */
		if(but->embossfunc==ui_draw_pulldown) {
			if(but->flag & (UI_SELECT|UI_ACTIVE)) {		
				BIF_ThemeColor(TH_MENU_TEXT_HI);
			} else {
				BIF_ThemeColor(TH_MENU_TEXT);
			}
		}
		else {
			if(but->flag & UI_SELECT) {		
				BIF_ThemeColor(TH_BUT_TEXT_HI);
			} else {
				BIF_ThemeColor(TH_BUT_TEXT);
			}
		}

		/* tog3 button exception */
		if(but->type==TOG3 && (but->flag & UI_SELECT)) {
			int ok= 0;
			
			if( but->pointype==CHA ) {
				if( BTST( *(but->poin+2), but->bitnr )) ok= 1;
			}
			else if( but->pointype ==SHO ) {
				short *sp= (short *)but->poin;
				if( BTST( sp[1], but->bitnr )) ok= 1;
			}
			
			if (ok) glColor3ub(255, 255, 0);
		}
		
		glRasterPos2f( x, (but->y1+but->y2- 9.0)/2.0);
		BIF_DrawString(but->font, but->drawstr+but->ofs, (U.transopts & TR_BUTTONS));

		/* part text right aligned */
		if(cpoin) {
			len= BIF_GetStringWidth(but->font, cpoin+1, (U.transopts & TR_BUTTONS));
			glRasterPos2f( but->x2 - len*but->aspect-3, (but->y1+but->y2- 9.0)/2.0);
			BIF_DrawString(but->font, cpoin+1, (U.transopts & TR_BUTTONS));
			*cpoin= '|';
		}
	}
	/* if there's no text label, then check to see if there's an icon only and draw it */
	else if( but->flag & UI_HAS_ICON ) {
		ui_draw_icon(but, (BIFIconID) (but->icon+but->iconadd));
	}

}

static void ui_draw_but_COL(uiBut *but)
{
	float *fp;
	char colr, colg, colb;
	
	if( but->pointype==FLO ) {
		fp= (float *)but->poin;
		colr= floor(255.0*fp[0]+0.5);
		colg= floor(255.0*fp[1]+0.5);
		colb= floor(255.0*fp[2]+0.5);
	}
	else {
		char *cp= (char *)but->poin;
		colr= cp[0];
		colg= cp[1];
		colb= cp[2];
	}
	
	/* exception... hrms, but can't simply use the emboss callback for this now. */
	/* this button type needs review, and nice integration with rest of API here */
	if(but->embossfunc == ui_draw_round) {
		char *cp= BIF_ThemeGetColorPtr(U.themes.first, 0, TH_CUSTOM);
		cp[0]= colr; cp[1]= colg; cp[2]= colb;
		but->embossfunc(but->type, TH_CUSTOM, but->aspect, but->x1, but->y1, but->x2, but->y2, but->flag);
	}
	else {
		
		glColor3ub(colr,  colg,  colb);
		glRectf((but->x1), (but->y1), (but->x2), (but->y2));
		glColor3ub(0,  0,  0);
		fdrawbox((but->x1), (but->y1), (but->x2), (but->y2));
	}
}



/* nothing! */
static void ui_draw_nothing(int type, int colorid, float asp, float x1, float y1, float x2, float y2, int flag)
{
}


/* ************** EXTERN, called from interface.c ************* */
/* ************** MAIN CALLBACK FUNCTION          ************* */

void ui_set_embossfunc(uiBut *but, int drawtype)
{

	// not really part of standard minimal themes, just make sure it is set
	but->sliderfunc= ui_draw_slider;

	// standard builtin first:
	if(but->type==LABEL) but->embossfunc= ui_draw_nothing;
	else if(drawtype==UI_EMBOSSM) but->embossfunc= ui_draw_minimal;
	else if(drawtype==UI_EMBOSSN) but->embossfunc= ui_draw_nothing;
	else if(drawtype==UI_EMBOSSP) but->embossfunc= ui_draw_pulldown;
	else {
		int theme= BIF_GetThemeValue(TH_BUT_DRAWTYPE);
		
		// and the themes
		if(theme==1) {
			but->embossfunc= ui_draw_default;
			but->sliderfunc= ui_default_slider;
		}
		else if(theme==2) {
			but->embossfunc= ui_draw_round;
		}
		else if(theme==3) {
			but->embossfunc= ui_draw_oldskool;
		}
		else {
			but->embossfunc= ui_draw_minimal;
		}
	}
	
	// note: if you want aligning, adapt the call uiBlockEndAlign in interface.c 
}


void ui_draw_but(uiBut *but)
{
	double value;
	float x1, x2, y1, y2, fac;
	
	if(but==0) return;

	if(but->block->frontbuf==UI_NEED_DRAW_FRONT) {
		but->block->frontbuf= UI_HAS_DRAW_FRONT;
	
		glDrawBuffer(GL_FRONT);
		if(but->win==curarea->headwin) curarea->head_swap= WIN_FRONT_OK;
		else curarea->win_swap= WIN_FRONT_OK;
	}
	
	switch (but->type) {

	case NUMSLI:
	case HSVSLI:
	
		but->embossfunc(but->type, but->themecol, but->aspect, but->x1, but->y1, but->x2, but->y2, but->flag);
		ui_draw_text_icon(but);

		x1= (but->x1+but->x2)/2;
		x2= but->x2 - 5.0*but->aspect;
		y1= but->y1 + 2.0*but->aspect;
		y2= but->y2 - 2.0*but->aspect;
		
		value= ui_get_but_val(but);
		fac= (value-but->min)*(x2-x1)/(but->max - but->min);
		
		but->sliderfunc(but->themecol, fac, but->aspect, x1, y1, x2, y2, but->flag);
		break;
		
	case SEPR:
		//  only background
		break;
		
	case COL:
		ui_draw_but_COL(but);  // black box with color
		break;

	case LINK:
	case INLINK:
		ui_draw_icon(but, but->icon);
		break;

	default:
		but->embossfunc(but->type, but->themecol, but->aspect, but->x1, but->y1, but->x2, but->y2, but->flag);
		ui_draw_text_icon(but);
	
	}
}




