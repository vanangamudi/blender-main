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
 * support for animation modes - Reevan McKay
 */

#ifdef WIN32
#include "BLI_winstuff.h"
#endif

#include "BLI_arithb.h"
#include "BLI_blenlib.h"

#include "DNA_action_types.h"
#include "DNA_armature_types.h"
#include "DNA_object_types.h"
#include "DNA_scene_types.h"
#include "DNA_screen_types.h"
#include "DNA_view3d_types.h"

#include "BKE_action.h"
#include "BKE_armature.h"
#include "BKE_global.h"
#include "BKE_displist.h"

#include "BIF_gl.h"
#include "BIF_graphics.h"
#include "BIF_space.h"
#include "BIF_toolbox.h"
#include "BIF_screen.h"
#include "BIF_poseobject.h"

#include "BDR_editobject.h"

#include "BSE_edit.h"

#include "mydevice.h"
#include "blendef.h"

static void armature_filter_pose_keys (bPose *pose, bArmature* arm);
static void armature_bonechildren_filter_pose_keys (bPose *pose, Bone *bone);

void collect_pose_garbage(Object *ob)
{
	bPoseChannel *pchan, *next;
	Bone *bone;

	if (!ob)
		return;

	if (!ob->pose)
		return;

	switch (ob->type){
	case OB_ARMATURE:
			/* Remove unused pose channels */
			for (pchan = ob->pose->chanbase.first; pchan; pchan=next){
				next=pchan->next;
				bone = get_named_bone(ob->data, pchan->name);
				if (!bone)
					BLI_freelinkN(&ob->pose->chanbase, pchan);
			}
			break;
	default:
		break;
	}

}

void enter_posemode(void)
{
	Base *base;
	Object *ob;
	bArmature *arm;
	
	if(G.scene->id.lib) return;
	base= BASACT;
	if(base==0) return;
	if((base->lay & G.vd->lay)==0) return;
	
	ob= base->object;
	if(ob->data==0) return;
	
	if (ob->id.lib){
		error ("Can't pose libdata");
		return;
	}

	switch (ob->type){
	case OB_ARMATURE:
		arm= get_armature(ob);
		if( arm==0 ) return;
		G.obpose= ob;
		/*		make_poseMesh();	*/
		allqueue(REDRAWVIEW3D, 0);
		break;
	default:
		return;
	}

	if (G.obedit) exit_editmode(1);
	G.f &= ~(G_VERTEXPAINT | G_FACESELECT | G_TEXTUREPAINT | G_WEIGHTPAINT);


}

void filter_pose_keys (void)
{


	Object	*ob;
	bPoseChannel *chan;

	ob=G.obpose;
	if (!ob)
		return;

	switch (ob->type){
	case OB_ARMATURE:
		armature_filter_pose_keys (ob->pose, (bArmature*)ob->data);
		break;
	default:
		if (ob->pose){
			for (chan=ob->pose->chanbase.first; chan; chan=chan->next){
				chan->flag |= POSE_KEY;
			}
		}
		break;
	}
}

static void armature_filter_pose_keys (bPose *pose, bArmature *arm)
{
	Bone *bone;

	if (!pose)
		return;
	if (!arm)
		return;

	for (bone=arm->bonebase.first; bone; bone=bone->next){
		armature_bonechildren_filter_pose_keys (pose, bone);
	}
}

static void armature_bonechildren_filter_pose_keys (bPose *pose, Bone *bone)
{
	Bone *curbone;
	bPoseChannel *chan;

	if (!bone)
		return;

	for (chan=pose->chanbase.first; chan; chan=chan->next){
		if (BLI_streq(chan->name, bone->name))
			break;
	}

	if (chan){
		if (bone->flag & BONE_SELECTED){
			chan->flag |= POSE_KEY;		
		}
		else {
			chan->flag &= ~POSE_KEY;
		}
	}

	for (curbone=bone->childbase.first; curbone; curbone=curbone->next){
		armature_bonechildren_filter_pose_keys (pose, curbone);
	}
}

void exit_posemode (int freedata)
{
	Object *ob;

	if(G.obpose==0) return;

	ob= G.obpose;
	
	G.obpose= 0;
	makeDispList(ob);
	
	if(freedata) {
		setcursor_space(SPACE_VIEW3D, CURSOR_STD);
		
		countall();
		allqueue(REDRAWVIEW3D, 0);
	}
	else {
		G.obpose= ob;
	}

	scrarea_queue_headredraw(curarea);
}
