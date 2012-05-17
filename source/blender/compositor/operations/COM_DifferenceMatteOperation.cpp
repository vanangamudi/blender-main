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

#include "COM_DifferenceMatteOperation.h"
#include "BLI_math.h"

DifferenceMatteOperation::DifferenceMatteOperation(): NodeOperation() {
	addInputSocket(COM_DT_COLOR);
	addInputSocket(COM_DT_COLOR);
	addOutputSocket(COM_DT_VALUE);

	inputImage1Program = NULL;
	inputImage2Program = NULL;
}

void DifferenceMatteOperation::initExecution() {
	this->inputImage1Program = this->getInputSocketReader(0);
	this->inputImage2Program = this->getInputSocketReader(1);
}
void DifferenceMatteOperation::deinitExecution() {
	this->inputImage1Program= NULL;
	this->inputImage2Program= NULL;
}

void DifferenceMatteOperation::executePixel(float* outputValue, float x, float y, PixelSampler sampler, MemoryBuffer *inputBuffers[]) {
	float inColor1[4];
	float inColor2[4];

	const float tolerence=this->settings->t1;
	const float falloff=this->settings->t2;
	float difference;
	float alpha;

	this->inputImage1Program->read(inColor1, x, y, sampler, inputBuffers);
	this->inputImage2Program->read(inColor2, x, y, sampler, inputBuffers);

	difference= fabs(inColor2[0]-inColor1[0])+
			   fabs(inColor2[1]-inColor1[1])+
			   fabs(inColor2[2]-inColor1[2]);

	/*average together the distances*/
	difference=difference/3.0;

	/*make 100% transparent*/
	if(difference < tolerence) {
		outputValue[0]=0.0;
	}
	/*in the falloff region, make partially transparent */
	else if(difference < falloff+tolerence) {
		difference=difference-tolerence;
		alpha=difference/falloff;
		/*only change if more transparent than before */
		if(alpha < inColor1[3]) {
			outputValue[0]=alpha;
		}
		else { /* leave as before */
			outputValue[0]=inColor1[3];
		}
	}
	else {
		/*foreground object*/
		outputValue[0]= inColor1[3];
	}
}

