/*
 * Copyright 2012, Blender Foundation.
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
 *		Sergey Sharybin
 */

#include "COM_MaskNode.h"
#include "COM_ExecutionSystem.h"
#include "COM_MaskOperation.h"

extern "C" {
	#include "DNA_mask_types.h"
}

MaskNode::MaskNode(bNode *editorNode) : Node(editorNode)
{
	/* pass */
}

void MaskNode::convertToOperations(ExecutionSystem *graph, CompositorContext *context)
{
	const RenderData *data = context->getRenderData();

	OutputSocket *outputMask = this->getOutputSocket(0);

	bNode *editorNode = this->getbNode();
	Mask *mask = (Mask *)editorNode->id;

	// always connect the output image
	MaskOperation *operation = new MaskOperation();
	operation->setbNode(editorNode);

	if (editorNode->custom1 & CMP_NODEFLAG_MASK_FIXED) {
		operation->setMaskWidth(editorNode->custom3);
		operation->setMaskHeight(editorNode->custom4);
	}
	else if (editorNode->custom1 & CMP_NODEFLAG_MASK_FIXED_SCENE) {
		operation->setMaskWidth(editorNode->custom3 * (data->size / 100.0f));
		operation->setMaskHeight(editorNode->custom4 * (data->size / 100.0f));
	}
	else {
		operation->setMaskWidth(data->xsch * data->size / 100.0f);
		operation->setMaskHeight(data->ysch * data->size / 100.0f);
	}

	if (outputMask->isConnected()) {
		outputMask->relinkConnections(operation->getOutputSocket());
	}

	operation->setMask(mask);
	operation->setFramenumber(context->getFramenumber());
	operation->setSmooth((bool)(editorNode->custom1 & CMP_NODEFLAG_MASK_AA) != 0);
	operation->setFeather((bool)(editorNode->custom1 & CMP_NODEFLAG_MASK_NO_FEATHER) == 0);

	graph->addOperation(operation);
}
