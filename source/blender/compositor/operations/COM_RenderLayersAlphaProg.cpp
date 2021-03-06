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

#include "COM_RenderLayersAlphaProg.h"

RenderLayersAlphaProg::RenderLayersAlphaProg() : RenderLayersBaseProg(SCE_PASS_COMBINED, 4)
{
	this->addOutputSocket(COM_DT_VALUE);
}

void RenderLayersAlphaProg::executePixel(float *output, float x, float y, PixelSampler sampler)
{
	int ix = x;
	int iy = y;
	float *inputBuffer = this->getInputBuffer();

	if (inputBuffer == NULL || ix < 0 || iy < 0 || ix >= (int)this->getWidth() || iy >= (int)this->getHeight() ) {
		output[0] = 0.0f;
		output[1] = 0.0f;
		output[2] = 0.0f;
		output[3] = 0.0f;
	}
	else {
		unsigned int offset = (iy * this->getWidth() + ix) * 4;
		output[0] = inputBuffer[offset + 3];
		output[1] = 0.0f;
		output[2] = 0.0f;
		output[3] = 0.0f;
	}
}
