/* 
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
 * This is a new part of Blender.
 *
 * Contributor(s): Willian P. Germano
 *
 * ***** END GPL/BL DUAL LICENSE BLOCK *****
*/

#ifndef EXPP_constant_H
#define EXPP_constant_H

#include <Python.h>
#include <stdio.h>

#include "gen_utils.h"

/* Objects of <type 'constant'> are instanced inside many other Blender Python
 * objects, so this header file must contain only 'public' declarations */

/*****************************************************************************/
/* Python API function prototypes for the constant module.                   */
/*****************************************************************************/
PyObject *constant_New (void);

/*****************************************************************************/
/* Python C_constant structure definition:                                   */
/*****************************************************************************/
typedef struct {
  PyObject_HEAD
  PyObject *dict;
} C_constant;

/*****************************************************************************/
/* Python C_constant methods declarations:                                   */
/*****************************************************************************/
void constant_insert(C_constant *self, char *name, PyObject *args);

#endif /* EXPP_constant_H */
