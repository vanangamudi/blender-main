/**
 * $Id$
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

#include "GEN_Matrix4x4.h"



GEN_Matrix4x4::GEN_Matrix4x4()
{
	Identity();
}



GEN_Matrix4x4::GEN_Matrix4x4(const float value[4][4])
{
	for (int i=0;i<4;i++)
	{
		for (int j=0;j<4;j++)
			m_V[i][j] = value[i][j];
	}
}



GEN_Matrix4x4::GEN_Matrix4x4(const double value[16])
{
	for (int i=0;i<16;i++)
		m_Vflat[i] = value[i];
}



GEN_Matrix4x4::GEN_Matrix4x4(const GEN_Matrix4x4& other)
{
	SetMatrix(other);
}



GEN_Matrix4x4::GEN_Matrix4x4(const MT_Point3& orig,
							 const MT_Vector3& dir,
							 const MT_Vector3 up)
{
	MT_Vector3 z = -(dir.normalized());
	MT_Vector3 x = (up.cross(z)).normalized();
	MT_Vector3 y = (z.cross(x));
	
	m_V[0][0] = x.x();
	m_V[0][1] = y.x();
	m_V[0][2] = z.x();
	m_V[0][3] = 0.0f;
	
	m_V[1][0] = x.y();
	m_V[1][1] = y.y();
	m_V[1][2] = z.y();
	m_V[1][3] = 0.0f;
	
	m_V[2][0] = x.z();
	m_V[2][1] = y.z();
	m_V[2][2] = z.z();
	m_V[2][3] = 0.0f;
	
	m_V[3][0] = orig.x();//0.0f;
	m_V[3][1] = orig.y();//0.0f;
	m_V[3][2] = orig.z();//0.0f;
	m_V[3][3] = 1.0f;
	
	//Translate(-orig);
}



MT_Vector3 GEN_Matrix4x4::GetRight() const
{
	return MT_Vector3(m_V[0][0], m_V[0][1], m_V[0][2]);
}



MT_Vector3 GEN_Matrix4x4::GetUp() const
{
	return MT_Vector3(m_V[1][0], m_V[1][1], m_V[1][2]);
}



MT_Vector3 GEN_Matrix4x4::GetDir() const
{
	return MT_Vector3(m_V[2][0], m_V[2][1], m_V[2][2]);
}



MT_Point3 GEN_Matrix4x4::GetPos() const
{
	return MT_Point3(m_V[3][0], m_V[3][1], m_V[3][2]);
}



void GEN_Matrix4x4::Identity()
{
	for (int i=0; i<4; i++)
	{
		for (int j=0; j<4; j++)
			m_V[i][j] = (i==j?1.0f:0.0f);
	} 
}



void GEN_Matrix4x4::SetMatrix(const GEN_Matrix4x4& other)
{
	for (int i=0; i<16; i++)
		m_Vflat[i] = other.m_Vflat[i];
}



double*	GEN_Matrix4x4::getPointer()
{
	return &m_V[0][0];
}



const double* GEN_Matrix4x4::getPointer() const
{
	return &m_V[0][0];
}	



void GEN_Matrix4x4::setElem(int pos,double newvalue)
{
	m_Vflat[pos] = newvalue;
}	





GEN_Matrix4x4 GEN_Matrix4x4::Perspective(MT_Scalar inLeft,
MT_Scalar inRight,
MT_Scalar inBottom,
MT_Scalar inTop,
MT_Scalar inNear,
MT_Scalar inFar)
{

	GEN_Matrix4x4 mat;
	
	// Column 0
	mat(0, 0) = -(2.0*inNear)			/ (inRight-inLeft);
	mat(1, 0) = 0;
	mat(2, 0) = 0;
	mat(3, 0) = 0;

	// Column 1
	mat(0, 1) = 0;
	mat(1, 1) = (2.0*inNear)			/ (inTop-inBottom);
	mat(2, 1) = 0;
	mat(3, 1) = 0;

	// Column 2
	mat(0, 2) =  (inRight+inLeft)		/ (inRight-inLeft);
	mat(1, 2) =  (inTop+inBottom)		/ (inTop-inBottom);
	mat(2, 2) = -(inFar+inNear)			/ (inFar-inNear);
	mat(3, 2) = -1;

	// Column 3
	mat(0, 3) = 0;
	mat(1, 3) = 0;
	mat(2, 3) = -(2.0*inFar*inNear)		/ (inFar-inNear);
	mat(3, 3) = 0;

	return mat;
}
