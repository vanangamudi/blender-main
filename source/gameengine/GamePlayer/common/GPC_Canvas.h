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

#ifndef _GPC_CANVAS_H_
#define _GPC_CANVAS_H_

#include "RAS_ICanvas.h"
#include "RAS_Rect.h"

#if defined(WIN32) || defined(__APPLE__)
	#ifdef WIN32
		#pragma warning (disable:4786) // suppress stl-MSVC debug info warning
		#include <windows.h>
		#include <GL/gl.h>
	#else // WIN32
		// __APPLE__ is defined
		#include <AGL/gl.h>
	#endif // WIN32
#else //defined(WIN32) || defined(__APPLE__)
	#include <GL/gl.h>
#endif //defined(WIN32) || defined(__APPLE__)

#include <map>


class GPC_Canvas : public RAS_ICanvas
{
public:
	/**
	 * Used to position banners in the canvas.
	 */
	typedef enum {
		alignTopLeft,
		alignBottomRight
	} TBannerAlignment;

	typedef int TBannerId;

protected:
	/** 
	 * Used to store info for banners drawn on top of the canvas.
	 */
	typedef struct {
		/** Where the banner will be displayed. */
		TBannerAlignment alignment;
		/** Banner display enabled. */
		bool enabled;
		/** Banner display width. */
		unsigned int displayWidth;
		/** Banner display height. */
		unsigned int displayHeight;
		/** Banner image width. */
		unsigned int imageWidth;
		/** Banner image height. */
		unsigned int imageHeight;
		/** Banner image data. */
		unsigned char* imageData;
		/** Banner OpenGL texture name. */
		unsigned int textureName;
	} TBannerData;
	typedef std::map<TBannerId, TBannerData> TBannerMap;

	/** Width of the context. */
	int m_width;
	/** Height of the context. */
	int m_height;
	/** Rect that defines the area used for rendering,
	    relative to the context */
	RAS_Rect m_displayarea;

	/** Storage for the banners to display. */
	TBannerMap m_banners;
	/** State of banner display. */
	bool m_bannersEnabled;

public:

	GPC_Canvas(int width, int height);

	virtual ~GPC_Canvas();

	void Resize(int width, int height);


	/**
	 * @section Methods inherited from abstract base class RAS_ICanvas.
	 */
	
		int 
	GetWidth(
	) const {
		return m_width;
	}
	
		int 
	GetHeight(
	) const {
		return m_height;
	}

	const 
		RAS_Rect &
	GetDisplayArea(
	) const {
		return m_displayarea;
	};

		RAS_Rect &
	GetDisplayArea(
	) {
		return m_displayarea;
	};

		void 
	BeginFrame(
	) {};

	/**
	 * Draws overlay banners and progress bars.
	 */
		void 
	EndFrame(
	);
	
	void SetViewPort(int x1, int y1, int x2, int y2);

	void ClearColor(float r, float g, float b, float a);

	/**
	 * @section Methods inherited from abstract base class RAS_ICanvas.
	 * Semantics are not yet honoured.
	 */
	
	void SetMouseState(RAS_MouseState mousestate)
	{
		// not yet		
	}

	void SetMousePosition(int x, int y)
	{
		// not yet
	}

	void MakeScreenShot(const char* filename)
	{
		// not yet	
	}

	void ClearBuffer(int type);

	/**
	 * @section Services provided by this class.
	 */

	/**
	 * Enables display of a banner.
	 * The image data is copied inside.
	 * @param bannerWidth		Display width of the banner.
	 * @param bannerHeight		Display height of the banner.
	 * @param imageWidth		Width of the banner image in pixels.
	 * @param imageHeight		Height of the banner image in pixels.
	 * @param imageData			Pointer to the pixels of the image to display.
	 * @param alignement		Where the banner will be positioned on the canvas.
	 * @param enabled			Whether the banner will be displayed intiallly.
	 * @return A banner id.
	 */
	TBannerId AddBanner(
		unsigned int bannerWidth, unsigned int bannerHeight,
		unsigned int imageWidth, unsigned int imageHeight,
		unsigned char* imageData, TBannerAlignment alignment = alignTopLeft, 
		bool enabled = true);

	/**
	 * Disposes a banner.
	 * @param id Bannner to be disposed.
	 */
	void DisposeBanner(TBannerId id);

	/**
	 * Disposes all the banners.
	 */
	void DisposeAllBanners();

	/**
	 * Enables or disables display of a banner.
	 * @param id		Banner id of the banner to be enabled/disabled.
	 * @param enabled	New state of the banner.
	 */
	void SetBannerEnabled(TBannerId id, bool enabled = true);

	/**
	 * Enables or disables display of all banners.
	 * @param enabled	New state of the banners.
	 */
	void SetBannerDisplayEnabled(bool enabled = true);

protected:
	/**
	 * Disposes a banner.
	 * @param it Bannner to be disposed.
	 */
	void DisposeBanner(TBannerData& banner);

	/**
	 * Draws all the banners enabled.
	 */
	void DrawAllBanners(void);

	/**
	 * Draws a banner.
	 */
	void DrawBanner(TBannerData& banner);

	struct CanvasRenderState {
		int oldLighting;
		int oldDepthTest;
		int oldFog;
		int oldTexture2D;
		int oldBlend;
		int oldBlendSrc;
		int oldBlendDst;
		float oldColor[4];
		int oldWriteMask;
	};

		void			
	PushRenderState(
		CanvasRenderState & render_state
	);
		void
	PopRenderState(
		const CanvasRenderState & render_state
	);

	/** 
	 * Set up an orthogonal viewing,model and texture matrix
	 * for banners and progress bars.
	 */
		void
	SetOrthoProjection(
	);
	
	static TBannerId s_bannerId;
};

#endif // _GPC_CANVAS_H_
