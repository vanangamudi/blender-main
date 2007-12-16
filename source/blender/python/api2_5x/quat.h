/* 
 * $Id: quat.h 11398 2007-07-28 06:28:33Z campbellbarton $
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
 * Contributor(s): Joseph Gilbert
 *
 * ***** END GPL/BL DUAL LICENSE BLOCK *****
 *
 */

#ifndef EXPP_quat_h
#define EXPP_quat_h

#include <Python.h>

extern PyTypeObject BPyQuat_Type;

#define BPyQuat_Check(v) PyObject_TypeCheck(v, &BPyQuat_Type)

typedef struct {
	PyObject_VAR_HEAD 
	float *quat;				//1D array of data (alias)
	PyObject *source;
} BPyQuatObject;
/*coerced_object is a pointer to the object that it was
coerced from when a dummy vector needs to be created from
the coerce() function for numeric protocol operations*/

/*struct data contains a pointer to the actual data that the
object uses. It can use either PyMem allocated data (which will
be stored in py_data) or be a wrapper for data allocated through
blender (stored in blend_data). This is an either/or struct not both*/

//prototypes
PyObject *Quat_Identity( BPyQuatObject * self );
PyObject *Quat_Negate( BPyQuatObject * self );
PyObject *Quat_Conjugate( BPyQuatObject * self );
PyObject *Quat_Inverse( BPyQuatObject * self );
PyObject *Quat_Normalize( BPyQuatObject * self );
PyObject *Quat_ToEuler( BPyQuatObject * self );
PyObject *Quat_ToMatrix( BPyQuatObject * self );
PyObject *Quat_copy( BPyQuatObject * self );
PyObject *Quat_CreatePyObject( float *quat, PyObject * source );
PyObject * QuatType_Init();
int Quat_CheckPyObject(BPyQuatObject * self);

#endif				/* EXPP_quat_h */
