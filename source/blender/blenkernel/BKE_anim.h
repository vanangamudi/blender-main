/**
 * blenlib/BKE_anim.h (mar-2001 nzc);
 *	
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
#ifndef BKE_ANIM_H
#define BKE_ANIM_H

struct Path;
struct Object;
struct PartEff;
struct Scene;

void free_path(struct Path *path);
void calc_curvepath(struct Object *ob);
int interval_test(int min, int max, int p1, int cycl);
int where_on_path(struct Object *ob, float ctime, float *vec, float *dir);
void frames_duplilist(struct Object *ob);
void vertex_duplilist(struct Scene *sce, struct Object *par);
void particle_duplilist(struct Scene *sce, struct Object *par, struct PartEff *paf);
void free_duplilist(void);
void make_duplilist(struct Scene *sce, struct Object *ob);
	
#endif
