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
#ifndef __KX_TIMEEVENTMANAGER
#define __KX_TIMEEVENTMANAGER

#include "SCA_EventManager.h"
#include "Value.h"
#include <vector>

using namespace std;

class SCA_TimeEventManager : public SCA_EventManager
{
	vector<CValue*>		m_timevalues; // values that need their time updated regularly
	
public:
	SCA_TimeEventManager(class SCA_LogicManager* logicmgr);
	virtual ~SCA_TimeEventManager();

	virtual void	NextFrame(double curtime,double deltatime);
	virtual void	RegisterSensor(class SCA_ISensor* sensor);
	void			AddTimeProperty(CValue* timeval);
	void			RemoveTimeProperty(CValue* timeval);
};
#endif //__KX_TIMEEVENTMANAGER
