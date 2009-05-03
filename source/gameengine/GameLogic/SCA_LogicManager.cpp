/**
 * $Id$
 *
 * ***** BEGIN GPL LICENSE BLOCK *****
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
 * Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * The Original Code is Copyright (C) 2001-2002 by NaN Holding BV.
 * All rights reserved.
 *
 * The Original Code is: all of this file.
 *
 * Contributor(s): none yet.
 *
 * ***** END GPL LICENSE BLOCK *****
 * Regulates the top-level logic behaviour for one scene.
 */
#include "Value.h"
#include "SCA_LogicManager.h"
#include "SCA_ISensor.h"
#include "SCA_IController.h"
#include "SCA_IActuator.h"
#include "SCA_EventManager.h"
#include "SCA_PythonController.h"
#include <set>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


SCA_LogicManager::SCA_LogicManager()
{
}



SCA_LogicManager::~SCA_LogicManager()
{
	/* AddRef() is not used when the objects are added to m_mapStringToGameObjects
	   so Release() should not be used either. The memory leak big is fixed
	   in BL_ConvertBlenderObjects()

	int numgameobj = m_mapStringToGameObjects.size();
	for (int i = 0; i < numgameobj; i++)
	{
		CValue** gameobjptr = m_mapStringToGameObjects.at(i);
		assert(gameobjptr);
		if (gameobjptr)
			(*gameobjptr)->Release();
    
	}
	*/
	/*for (int i=0;i<m_sensorcontrollermap.size();i++)
	{
		vector<SCA_IController*>* controllerarray = *(m_sensorcontrollermap[i]);
		delete controllerarray;
	}
	*/
	for (vector<SCA_EventManager*>::iterator it = m_eventmanagers.begin();!(it==m_eventmanagers.end());it++)
	{
		delete (*it);
	}
	m_eventmanagers.clear();
	m_sensorcontrollermapje.clear();
	m_removedActuators.clear();
	m_activeActuators.clear();
}

// this function is a performance helper when the scene is destoyed
// without it, the map updated for each object... a massive slow down when there are
// large number of objects. By clearing the map upfront we avoid the waster of time.
void SCA_LogicManager::RemoveSensorMap()
{
	m_sensorcontrollermapje.clear();
}

/*
// this kind of fixes bug 398 but breakes games, so better leave it out for now.
// a removed object's gameobject (and logicbricks and stuff) didn't get released
// because it was still in the m_mapStringToGameObjects map.
void SCA_LogicManager::RemoveGameObject(const STR_String& gameobjname)
{
	int numgameobj = m_mapStringToGameObjects.size();
	for (int i = 0; i < numgameobj; i++)
	{
		CValue** gameobjptr = m_mapStringToGameObjects.at(i);
		assert(gameobjptr);

		if (gameobjptr)
		{
			if ((*gameobjptr)->GetName() == gameobjname)
				(*gameobjptr)->Release();
		}
	}

	m_mapStringToGameObjects.remove(gameobjname);
}
*/


void SCA_LogicManager::RegisterEventManager(SCA_EventManager* eventmgr)
{
	m_eventmanagers.push_back(eventmgr);
}



void SCA_LogicManager::RegisterGameObjectName(const STR_String& gameobjname,
											  CValue* gameobj)
{
	STR_HashedString mn = gameobjname;
	m_mapStringToGameObjects.insert(mn,gameobj);
}



void SCA_LogicManager::RegisterGameMeshName(const STR_String& gamemeshname, void* blendobj)
{
	STR_HashedString mn = gamemeshname;
	m_map_gamemeshname_to_blendobj.insert(mn, blendobj);
}



void SCA_LogicManager::RegisterGameObj(void* blendobj, CValue* gameobj) 
{
	m_map_blendobj_to_gameobj.insert(CHashedPtr(blendobj), gameobj);
}

void SCA_LogicManager::UnregisterGameObj(void* blendobj, CValue* gameobj) 
{
	void **obp = m_map_blendobj_to_gameobj[CHashedPtr(blendobj)];
	if (obp && (CValue*)(*obp) == gameobj)
		m_map_blendobj_to_gameobj.remove(CHashedPtr(blendobj));
}

CValue* SCA_LogicManager::GetGameObjectByName(const STR_String& gameobjname)
{
	STR_HashedString mn = "OB"+gameobjname;
	CValue** gameptr = m_mapStringToGameObjects[mn];
	
	if (gameptr)
		return *gameptr;

	return NULL;
}


CValue* SCA_LogicManager::FindGameObjByBlendObj(void* blendobj) 
{
	void **obp= m_map_blendobj_to_gameobj[CHashedPtr(blendobj)];
	return obp?(CValue*)(*obp):NULL;
}



void* SCA_LogicManager::FindBlendObjByGameMeshName(const STR_String& gamemeshname) 
{
	STR_HashedString mn = gamemeshname;
	void **obp= m_map_gamemeshname_to_blendobj[mn];
	return obp?*obp:NULL;
}



void SCA_LogicManager::RemoveSensor(SCA_ISensor* sensor)
{
	sensormap_t::const_iterator mit = m_sensorcontrollermapje.find(sensor);
	if (mit != m_sensorcontrollermapje.end())
	{
		const controllerlist& contlist = mit->second;
		for (controllerlist::const_iterator c= contlist.begin();!(c==contlist.end());c++)
		{
			(*c)->UnlinkSensor(sensor);
		}
		m_sensorcontrollermapje.erase(sensor);
	}
	sensor->UnregisterToManager();
}

void SCA_LogicManager::RemoveController(SCA_IController* controller)
{
	sensormap_t::iterator sit;
	sit = m_sensorcontrollermapje.begin();
	if (sit==m_sensorcontrollermapje.end())
	{
		//TRICK: either there is no sensor at all, or the scene is being deleted 
		//(see KX_Scene::~KX_Scene()). In the first case, this is harmless. 
		//In the second case, we cannot rely on the sensor being still available, 
		//make the controller inactive to avoid link count. 
		//Need a better solution, maybe something similar to m_removedActuators.
		controller->SetActive(false);
	}
	controller->UnlinkAllSensors();
	controller->UnlinkAllActuators();
	for (;!(sit==m_sensorcontrollermapje.end());++sit)
	{
		(*sit).second.remove(controller);
	}
}


void SCA_LogicManager::RemoveDestroyedActuator(SCA_IActuator* actuator)
{
	m_removedActuators.push_back(SmartActuatorPtr(actuator,0));
	// take care that no controller can use this actuator again !

	sensormap_t::const_iterator sit;
	for (sit = m_sensorcontrollermapje.begin();!(sit==m_sensorcontrollermapje.end());++sit)
	{
		const controllerlist& contlist = sit->second;
		for (list<SCA_IController*>::const_iterator c= contlist.begin();!(c==contlist.end());c++)
		{
			(*c)->UnlinkActuator(actuator);
		}
	}
}



void SCA_LogicManager::RegisterToSensor(SCA_IController* controller,SCA_ISensor* sensor)
{
    m_sensorcontrollermapje[sensor].push_back(controller);
	controller->LinkToSensor(sensor);
}



void SCA_LogicManager::RegisterToActuator(SCA_IController* controller,SCA_IActuator* actua)
{
	controller->LinkToActuator(actua);
}



void SCA_LogicManager::BeginFrame(double curtime, double fixedtime)
{
	for (vector<SCA_EventManager*>::const_iterator ie=m_eventmanagers.begin(); !(ie==m_eventmanagers.end()); ie++)
		(*ie)->NextFrame(curtime, fixedtime);

	// for this frame, look up for activated sensors, and build the collection of triggered controllers
	// int numsensors = this->m_activatedsensors.size(); /*unused*/

	for (vector<SCA_ISensor*>::const_iterator is=m_activatedsensors.begin();
	!(is==m_activatedsensors.end());is++)
	{
		SCA_ISensor* sensor = *is;
        const controllerlist& contlist = m_sensorcontrollermapje[sensor];
        for (list<SCA_IController*>::const_iterator c= contlist.begin();
			!(c==contlist.end());c++)
		{
				SCA_IController* contr = *c;//controllerarray->at(c);
				if (contr->IsActive())
				{
					m_triggeredControllerSet.insert(SmartControllerPtr(contr,0));
					// So that the controller knows which sensor has activited it.
					// Only needed for the python controller though.
					if (contr->GetType() == &SCA_PythonController::Type)
					{
						SCA_PythonController* pythonController = (SCA_PythonController*)contr;
						pythonController->AddTriggeredSensor(sensor);
					}
				}
		}
		//sensor->SetActive(false);
	}

	
	// int numtriggered = triggeredControllerSet.size(); /*unused*/
	for (set<SmartControllerPtr>::iterator tit=m_triggeredControllerSet.begin();
	!(tit==m_triggeredControllerSet.end());tit++)
	{
		(*tit)->Trigger(this);
	}
	m_triggeredControllerSet.clear();
}



void SCA_LogicManager::UpdateFrame(double curtime, bool frame)
{
	vector<SmartActuatorPtr>::iterator ra;
	for (ra = m_removedActuators.begin(); !(ra == m_removedActuators.end()); ra++)
	{
		m_activeActuators.erase(*ra);
		(*ra)->SetActive(false);
	}
	m_removedActuators.clear();
	
	// About to run actuators, but before update the sensors for those which depends on actuators
	for (vector<SCA_EventManager*>::const_iterator ie=m_eventmanagers.begin(); !(ie==m_eventmanagers.end()); ie++)
		(*ie)->UpdateFrame();

	for (set<SmartActuatorPtr>::iterator ia = m_activeActuators.begin();!(ia==m_activeActuators.end());ia++)
	{
		//SCA_IActuator* actua = *ia;
		if (!(*ia)->Update(curtime, frame))
		{
			//*ia = m_activeactuators.back();
			m_removedActuators.push_back(*ia);
			
			(*ia)->SetActive(false);
			//m_activeactuators.pop_back();
		} else if ((*ia)->IsNoLink())
		{
			// This actuator has no more links but it still active
			// make sure it will get a negative event on next frame to stop it
			// Do this check after Update() rather than before to make sure
			// that all the actuators that are activated at same time than a state
			// actuator have a chance to execute. 
			CValue* event = new CBoolValue(false);
			(*ia)->RemoveAllEvents();
			(*ia)->AddEvent(event);
		}
	}
	
	for ( ra = m_removedActuators.begin(); !(ra == m_removedActuators.end()); ra++)
	{
		m_activeActuators.erase(*ra);
		(*ra)->SetActive(false);
	}
	m_removedActuators.clear();
}



void* SCA_LogicManager::GetActionByName (const STR_String& actname)
{
	STR_HashedString an = "AC"+actname;
	void** actptr = m_mapStringToActions[an];

	if (actptr)
		return *actptr;

	return NULL;
}



void* SCA_LogicManager::GetMeshByName(const STR_String& meshname)
{
	STR_HashedString mn = "ME"+meshname;
	void** meshptr = m_mapStringToMeshes[mn];

	if (meshptr)
		return *meshptr;

	return NULL;
}



void SCA_LogicManager::RegisterMeshName(const STR_String& meshname,void* mesh)
{
	STR_HashedString mn = meshname;
	m_mapStringToMeshes.insert(mn,mesh);
}



void SCA_LogicManager::RegisterActionName(const STR_String& actname,void* action)
{
	STR_HashedString an = actname;
	m_mapStringToActions.insert(an, action);
}



void SCA_LogicManager::EndFrame()
{
	for (vector<SCA_ISensor*>::const_iterator is=m_activatedsensors.begin();
	!(is==m_activatedsensors.end());is++)
	{
		SCA_ISensor* sensor = *is;
		sensor->SetActive(false);
	}
	m_activatedsensors.clear();

	for (vector<SCA_EventManager*>::const_iterator ie=m_eventmanagers.begin();
	!(ie==m_eventmanagers.end());ie++)
	{
		(*ie)->EndFrame();
	}


}



void SCA_LogicManager::AddActivatedSensor(SCA_ISensor* sensor)
{
	// each frame, only add sensor once, and to avoid a seek, or bloated container
	// hold a flag in each sensor, with the 'framenr'
	if (!sensor->IsActive())
	{
		sensor->SetActive(true);
		m_activatedsensors.push_back(sensor);
	}
}

void SCA_LogicManager::AddTriggeredController(SCA_IController* controller, SCA_ISensor* sensor)
{
	m_triggeredControllerSet.insert(SmartControllerPtr(controller,0));
	// so that the controller knows which sensor has activited it
	// only needed for python controller
	if (controller->GetType() == &SCA_PythonController::Type)
	{
		SCA_PythonController* pythonController = (SCA_PythonController*)controller;
		pythonController->AddTriggeredSensor(sensor);
	}
}


void SCA_LogicManager::AddActiveActuator(SCA_IActuator* actua,CValue* event)
{
	if (!actua->IsActive())
	{
		actua->SetActive(true);
		m_activeActuators.insert(SmartActuatorPtr(actua,0));
	}
	actua->AddEvent(event->AddRef());
}



SCA_EventManager* SCA_LogicManager::FindEventManager(int eventmgrtype)
{
	// find an eventmanager of a certain type
	SCA_EventManager* eventmgr = NULL;

	for (vector<SCA_EventManager*>::const_iterator i=
	m_eventmanagers.begin();!(i==m_eventmanagers.end());i++)
	{
		SCA_EventManager* emgr = *i;
		if (emgr->GetType() == eventmgrtype)
		{
			eventmgr = emgr;
			break;
		}
	}
	return eventmgr;
}



SmartActuatorPtr::SmartActuatorPtr(const SmartActuatorPtr& other)
{
	this->m_actuator = other.m_actuator;
	this->m_actuator->AddRef();
}



SmartActuatorPtr::SmartActuatorPtr(SCA_IActuator* actua,int dummy)
: m_actuator(actua)
{
	actua->AddRef();
}



SmartActuatorPtr::~SmartActuatorPtr()
{
	m_actuator->Release();
}



bool SmartActuatorPtr::operator <(const SmartActuatorPtr& other) const
{
	
	return m_actuator->LessComparedTo(*other);
}



bool SmartActuatorPtr::operator ==(const SmartActuatorPtr& other) const
{
	bool result2 = other->LessComparedTo(m_actuator);
	return (m_actuator->LessComparedTo(*other) && result2);
}



SCA_IActuator*	SmartActuatorPtr::operator->() const
{
	return m_actuator;
}



SCA_IActuator*	 SmartActuatorPtr::operator*() const
{
	return m_actuator;
}



SmartControllerPtr::SmartControllerPtr(const SmartControllerPtr& copy)
{
	this->m_controller = copy.m_controller;
	this->m_controller->AddRef();
}



SmartControllerPtr::SmartControllerPtr(SCA_IController* contr,int dummy)
: m_controller(contr)
{
	m_controller->AddRef();
}



SmartControllerPtr::~SmartControllerPtr()
{
	m_controller->Release();
}



bool	SmartControllerPtr::operator <(const SmartControllerPtr& other) const
{
	return m_controller->LessComparedTo(*other);
}



bool	SmartControllerPtr::operator ==(const SmartControllerPtr& other) const
{
	return (m_controller->LessComparedTo(*other) && other->LessComparedTo(m_controller));
}



SCA_IController*	SmartControllerPtr::operator->() const
{
	return m_controller;
}



SCA_IController* 	SmartControllerPtr::operator*() const
{
	return m_controller;
}


