/**
 * $Id$
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
 */

#ifndef __GPU_CANVAS_H
#define __GPU_CANVAS_H

#include <iostream>

#include "KXH_ketsji_hooks.h"
#include "GPC_Canvas.h"

class GPU_Canvas : public GPC_Canvas
{
private:
	
	/** Handle to the drawing resource. */
	KXH_plugin_handle m_plugin;
		
public:
	GPU_Canvas(KXH_plugin_handle display, int width, int height);
	virtual ~GPU_Canvas();
	
	virtual void Init(void);

	bool BeginDraw(void);
	void EndDraw(void);
	virtual void SwapBuffers(void);

};

#endif  // __GPU_CANVAS_H
