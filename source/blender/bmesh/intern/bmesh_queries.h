/*
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
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Contributor(s): Joseph Eagar.
 *
 * ***** END GPL LICENSE BLOCK *****
 */

#ifndef __BMESH_QUERIES_H__
#define __BMESH_QUERIES_H__

/** \file blender/bmesh/intern/bmesh_queries.h
 *  \ingroup bmesh
 */

int     BM_mesh_elem_count(BMesh *bm, const char htype);

int     BM_vert_in_face(BMFace *f, BMVert *v);
int     BM_verts_in_face(BMesh *bm, BMFace *f, BMVert **varr, int len);

int     BM_edge_in_face(BMFace *f, BMEdge *e);

int     BM_vert_in_edge(BMEdge *e, BMVert *v);
int     BM_verts_in_edge(BMVert *v1, BMVert *v2, BMEdge *e);

BMVert *BM_edge_other_vert(BMEdge *e, BMVert *v);
BMLoop *BM_face_other_loop(BMEdge *e, BMFace *f, BMVert *v);

int     BM_vert_edge_count(BMVert *v);
int     BM_edge_face_count(BMEdge *e);
int     BM_vert_face_count(BMVert *v);

int     BM_vert_is_wire(BMesh *bm, BMVert *v);
int     BM_edge_is_wire(BMesh *bm, BMEdge *e);

int     BM_vert_is_manifold(BMesh *bm, BMVert *v);
int     BM_edge_is_manifold(BMesh *bm, BMEdge *e);
int     BM_edge_is_boundary(BMEdge *e);

float   BM_loop_face_angle(BMesh *bm, BMLoop *l);

float   BM_edge_face_angle(BMesh *bm, BMEdge *e);
float   BM_vert_edge_angle(BMesh *bm, BMVert *v);

BMEdge *BM_edge_exists(BMVert *v1, BMVert *v2);

int     BM_face_exists_overlap(BMesh *bm, BMVert **varr, int len, BMFace **r_existface);

int     BM_face_exists(BMesh *bm, BMVert **varr, int len, BMFace **r_existface);

int     BM_face_exists_multi(BMesh *bm, BMVert **varr, BMEdge **earr, int len);
int     BM_face_exists_multi_edge(BMesh *bm, BMEdge **earr, int len);

int     BM_face_share_edge_count(BMFace *f1, BMFace *f2);
int     BM_edge_share_face_count(BMEdge *e1, BMEdge *e2);
int     BM_edge_share_vert_count(BMEdge *e1, BMEdge *e2);

BMVert *BM_edge_share_vert(BMEdge *e1, BMEdge *e2);

void    BM_edge_ordered_verts(BMEdge *edge, BMVert **r_v1, BMVert **r_v2);

#endif /* __BMESH_QUERIES_H__ */