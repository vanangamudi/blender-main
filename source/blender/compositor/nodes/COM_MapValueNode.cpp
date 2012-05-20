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

#include "COM_MapValueNode.h"

#include "COM_MapValueOperation.h"
#include "COM_ExecutionSystem.h"

MapValueNode::MapValueNode(bNode *editorNode): Node(editorNode)
{
}

void MapValueNode::convertToOperations(ExecutionSystem *graph, CompositorContext * context)
{
	InputSocket *colourSocket = this->getInputSocket(0);
	OutputSocket *valueSocket = this->getOutputSocket(0);
	TexMapping *storage =  (TexMapping*)this->getbNode()->storage;
	MapValueOperation *convertProg = new MapValueOperation();
	convertProg->setSettings(storage);
	colourSocket->relinkConnections(convertProg->getInputSocket(0), true, 0, graph);
	valueSocket->relinkConnections(convertProg->getOutputSocket(0));
	graph->addOperation(convertProg);
}