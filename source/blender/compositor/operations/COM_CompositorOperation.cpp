/*
 * Copyright 2011, Blender Foundation.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Contributor:
 *		Jeroen Bakker
 *		Monique Dewanchand
 */

#include "COM_CompositorOperation.h"
#include "COM_SocketConnection.h"
#include "BLI_listbase.h"
#include "DNA_scene_types.h"
#include "BKE_image.h"

extern "C" {
	#include "BLI_threads.h"
	#include "RE_pipeline.h"
	#include "RE_shader_ext.h"
	#include "RE_render_ext.h"
	#include "MEM_guardedalloc.h"
	#include "render_types.h"
}
#include "PIL_time.h"


CompositorOperation::CompositorOperation() : NodeOperation()
{
	this->addInputSocket(COM_DT_COLOR);
	this->addInputSocket(COM_DT_VALUE);

	this->setRenderData(NULL);
	this->m_outputBuffer = NULL;
	this->m_imageInput = NULL;
	this->m_alphaInput = NULL;
}

void CompositorOperation::initExecution()
{
	// When initializing the tree during initial load the width and height can be zero.
	this->m_imageInput = getInputSocketReader(0);
	this->m_alphaInput = getInputSocketReader(1);
	if (this->getWidth() * this->getHeight() != 0) {
		this->m_outputBuffer = (float *) MEM_callocN(this->getWidth() * this->getHeight() * 4 * sizeof(float), "CompositorOperation");
	}
}

void CompositorOperation::deinitExecution()
{
	if (!isBreaked()) {
		const RenderData *rd = this->m_rd;
		Render *re = RE_GetRender_FromData(rd);
		RenderResult *rr = RE_AcquireResultWrite(re);

		if (rr) {
			if (rr->rectf != NULL) {
				MEM_freeN(rr->rectf);
			}
			rr->rectf = this->m_outputBuffer;
		}
		else {
			if (this->m_outputBuffer) {
				MEM_freeN(this->m_outputBuffer);
			}
		}

		BLI_lock_thread(LOCK_DRAW_IMAGE);
		BKE_image_signal(BKE_image_verify_viewer(IMA_TYPE_R_RESULT, "Render Result"), NULL, IMA_SIGNAL_FREE);
		BLI_unlock_thread(LOCK_DRAW_IMAGE);

		if (re) {
			RE_ReleaseResult(re);
			re = NULL;
		}
	}
	else {
		if (this->m_outputBuffer) {
			MEM_freeN(this->m_outputBuffer);
		}
	}

	this->m_outputBuffer = NULL;
	this->m_imageInput = NULL;
	this->m_alphaInput = NULL;
}


void CompositorOperation::executeRegion(rcti *rect, unsigned int tileNumber)
{
	float color[8]; // 7 is enough
	float *buffer = this->m_outputBuffer;

	if (!buffer) return;
	int x1 = rect->xmin;
	int y1 = rect->ymin;
	int x2 = rect->xmax;
	int y2 = rect->ymax;
	int offset = (y1 * this->getWidth() + x1) * COM_NUMBER_OF_CHANNELS;
	int x;
	int y;
	bool breaked = false;

	for (y = y1; y < y2 && (!breaked); y++) {
		for (x = x1; x < x2 && (!breaked); x++) {
			this->m_imageInput->read(color, x, y, COM_PS_NEAREST);
			if (this->m_alphaInput != NULL) {
				this->m_alphaInput->read(&(color[3]), x, y, COM_PS_NEAREST);
			}
			copy_v4_v4(buffer + offset, color);
			offset += COM_NUMBER_OF_CHANNELS;
			if (isBreaked()) {
				breaked = true;
			}
		}
		offset += (this->getWidth() - (x2 - x1)) * COM_NUMBER_OF_CHANNELS;
	}
}

void CompositorOperation::determineResolution(unsigned int resolution[], unsigned int preferredResolution[])
{
	int width = this->m_rd->xsch * this->m_rd->size / 100;
	int height = this->m_rd->ysch * this->m_rd->size / 100;

	// check actual render resolution with cropping it may differ with cropped border.rendering
	// FIX for: [31777] Border Crop gives black (easy)
	Render *re = RE_GetRender_FromData(this->m_rd);
	if (re) {
		RenderResult *rr = RE_AcquireResultRead(re);
		if (rr) {
			width = rr->rectx;
			height = rr->recty;
		}
		RE_ReleaseResult(re);
	}

	preferredResolution[0] = width;
	preferredResolution[1] = height;

	NodeOperation::determineResolution(resolution, preferredResolution);

	resolution[0] = width;
	resolution[1] = height;
}
