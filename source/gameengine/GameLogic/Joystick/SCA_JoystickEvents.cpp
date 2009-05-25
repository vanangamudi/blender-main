/**
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
 * Contributor(s): snailrose.
 *
 * ***** END GPL LICENSE BLOCK *****
 */
#ifndef DISABLE_SDL
#include <SDL.h>
#endif

#include "SCA_Joystick.h"
#include "SCA_JoystickPrivate.h"


#ifndef DISABLE_SDL
void SCA_Joystick::OnAxisMotion(SDL_Event* sdl_event)
{
	if(sdl_event->jaxis.axis >= JOYAXIS_MAX)
		return;
	
	m_axis_array[sdl_event->jaxis.axis]= sdl_event->jaxis.value;
	m_istrig_axis = 1;
}


void SCA_Joystick::OnHatMotion(SDL_Event* sdl_event)
{
	if(sdl_event->jhat.hat >= JOYAXIS_MAX)
		return;

	m_hat_array[sdl_event->jhat.hat]= sdl_event->jhat.value;
	m_istrig_hat = 1;
}

void SCA_Joystick::OnButtonUp(SDL_Event* sdl_event)
{
	m_istrig_button = 1;
	
	/* this is needed for the "all events" option
	 * so we know if there are no buttons pressed */
	int i;
	for (i=0; i<m_buttonmax; i++) {
		if (SDL_JoystickGetButton(m_private->m_joystick, i)) {
			m_buttonnum = i;
			return;
		}
	}
	m_buttonnum = -2;
}


void SCA_Joystick::OnButtonDown(SDL_Event* sdl_event)
{
	if(sdl_event->jbutton.button <= m_buttonmax) /* unsigned int so always above 0 */
	{
		m_istrig_button = 1;
		m_buttonnum = sdl_event->jbutton.button;
	}
}


void SCA_Joystick::OnNothing(SDL_Event* sdl_event)
{
	m_istrig_axis = m_istrig_button = m_istrig_hat = 0;
}

/* only handle events for 1 joystick */

void SCA_Joystick::HandleEvents(void)
{
	SDL_Event		sdl_event;
	
	int i;
	for (i=0; i<JOYINDEX_MAX; i++) {
		if(SCA_Joystick::m_instance[i])
			SCA_Joystick::m_instance[i]->OnNothing(&sdl_event);
	}
	
	if(SDL_PollEvent(&sdl_event))
	{
		/* Note! m_instance[sdl_event.jaxis.which]
		 * will segfault if over JOYINDEX_MAX, not too nice but what are the chances? */
		switch(sdl_event.type)
		{
		case SDL_JOYAXISMOTION:
			SCA_Joystick::m_instance[sdl_event.jaxis.which]->OnAxisMotion(&sdl_event);
			break;
		case SDL_JOYHATMOTION:
			SCA_Joystick::m_instance[sdl_event.jhat.which]->OnHatMotion(&sdl_event);
			break;
		case SDL_JOYBUTTONUP:
			SCA_Joystick::m_instance[sdl_event.jbutton.which]->OnButtonUp(&sdl_event);
			break;
		case SDL_JOYBUTTONDOWN:
			SCA_Joystick::m_instance[sdl_event.jbutton.which]->OnButtonDown(&sdl_event);
			break;
#if 0	/* Not used yet */
		case SDL_JOYBALLMOTION:
			SCA_Joystick::m_instance[sdl_event.jball.which]->OnBallMotion(&sdl_event);
			break;
#endif
		default:
			printf("SCA_Joystick::HandleEvents, Unknown SDL event, this should not happen\n");
			break;
		}
	}
}
#endif
