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
 * The Original Code is Copyright (C) 2012 Blender Foundation.
 * All rights reserved.
 *
 * The Original Code is: all of this file.
 *
 * Contributor(s): Peter Larabell.
 *
 * ***** END GPL LICENSE BLOCK *****
 */
/** \file raskter.c
 *  \ingroup RASKTER
 */

#include <stdlib.h>
#include "raskter.h"
//#define __PLX__FAKE_AA__
//#define __PLX_KD_TREE__
#ifdef __PLX_KD_TREE__
#include "kdtree.h"
#endif



/*
 * Sort all the edges of the input polygon by Y, then by X, of the "first" vertex encountered.
 * This will ensure we can scan convert the entire poly in one pass.
 *
 * Really the poly should be clipped to the frame buffer's dimensions here for speed of drawing
 * just the poly. Since the DEM code could end up being coupled with this, we'll keep it separate
 * for now.
 */
void preprocess_all_edges(struct r_fill_context *ctx, struct poly_vert *verts, int num_verts, struct e_status *open_edge) {
    int i;
    int xbeg;
    int ybeg;
    int xend;
    int yend;
    int dx;
    int dy;
    int temp_pos;
    int xdist;
    struct e_status *e_new;
    struct e_status *next_edge;
    struct e_status **next_edge_ref;
    struct poly_vert *v;
    /* set up pointers */
    v = verts;
    ctx->all_edges = NULL;
    /* initialize some boundaries */
    ctx->rb.xmax = v[0].x;
    ctx->rb.xmin = v[0].x;
    ctx->rb.ymax = v[0].y;
    ctx->rb.ymin = v[0].y;
    /* loop all verts */
    for(i = 0; i < num_verts; i++) {
        /* determine beginnings and endings of edges, linking last vertex to first vertex */
        xbeg = v[i].x;
        ybeg = v[i].y;
        /* keep track of our x and y bounds */
        if(xbeg >= ctx->rb.xmax) {
            ctx->rb.xmax = xbeg;
        } else if(xbeg <= ctx->rb.xmin) {
            ctx->rb.xmin = xbeg;
        }
        if(ybeg >= ctx->rb.ymax) {
            ctx->rb.ymax= ybeg;
        } else if(ybeg <= ctx->rb.ymin) {
            ctx->rb.ymin=ybeg;
        }
        if(i) {
            /* we're not at the last vert, so end of the edge is the previous vertex */
            xend = v[i - 1].x;
            yend = v[i - 1].y;
        } else {
            /* we're at the first vertex, so the "end" of this edge is the last vertex */
            xend = v[num_verts - 1].x;
            yend = v[num_verts - 1].y;
        }
        /* make sure our edges are facing the correct direction */
        if(ybeg > yend) {
            /* flip the Xs */
            temp_pos = xbeg;
            xbeg = xend;
            xend = temp_pos;
            /* flip the Ys */
            temp_pos = ybeg;
            ybeg = yend;
            yend = temp_pos;
        }

        /* calculate y delta */
        dy = yend - ybeg;
        /* dont draw horizontal lines directly, they are scanned as part of the edges they connect, so skip em. :) */
        if(dy) {
            /* create the edge and determine it's slope (for incremental line drawing) */
            e_new = open_edge++;

            /* calculate x delta */
            dx = xend - xbeg;
            if(dx > 0) {
                e_new->xdir = 1;
                xdist = dx;
            } else {
                e_new->xdir = -1;
                xdist = -dx;
            }

            e_new->x = xbeg;
            e_new->ybeg = ybeg;
            e_new->num = dy;
            e_new->drift_dec = dy;

            /* calculate deltas for incremental drawing */
            if(dx >= 0) {
                e_new->drift = 0;
            } else {
                e_new->drift = -dy + 1;
            }
            if(dy >= xdist) {
                e_new->drift_inc = xdist;
                e_new->xshift = 0;
            } else {
                e_new->drift_inc = xdist % dy;
                e_new->xshift = (xdist / dy) * e_new->xdir;
            }
            next_edge_ref = &ctx->all_edges;
            /* link in all the edges, in sorted order */
            for(;;) {
                next_edge = *next_edge_ref;
                if(!next_edge || (next_edge->ybeg > ybeg) || ((next_edge->ybeg == ybeg) && (next_edge->x >= xbeg))) {
                    e_new->e_next = next_edge;
                    *next_edge_ref = e_new;
                    break;
                }
                next_edge_ref = &next_edge->e_next;
            }
        }
    }
}

/*
 * This function clips drawing to the frame buffer. That clipping will likely be moved into the preprocessor
 * for speed, but waiting on final design choices for curve-data before eliminating data the DEM code will need
 * if it ends up being coupled with this function.
 */
static int rast_scan_fill(struct r_fill_context *ctx, struct poly_vert *verts, int num_verts, float intensity) {
    int x_curr;                 /* current pixel position in X */
    int y_curr;                 /* current scan line being drawn */
    int yp;                     /* y-pixel's position in frame buffer */
    int swixd = 0;              /* whether or not edges switched position in X */
    float *cpxl;                /* pixel pointers... */
    float *mpxl;
    float *spxl;
    struct e_status *e_curr;    /* edge pointers... */
    struct e_status *e_temp;
    struct e_status *edgbuf;
    struct e_status **edgec;


    /*
     * If the number of verts specified to render as a polygon is less than 3,
     * return immediately. Obviously we cant render a poly with sides < 3. The
     * return for this we set to 1, simply so it can be distinguished from the
     * next place we could return, /home/guest/blender-svn/soc-2011-tomato/intern/raskter/raskter.
     * which is a failure to allocate memory.
     */
    if(num_verts < 3) {
        return(1);
    }

    /*
     * Try to allocate an edge buffer in memory. needs to be the size of the edge tracking data
     * multiplied by the number of edges, which is always equal to the number of verts in
     * a 2D polygon. Here we return 0 to indicate a memory allocation failure, as opposed to a 1 for
     * the preceeding error, which was a rasterization request on a 2D poly with less than
     * 3 sides.
     */
    if((edgbuf = (struct e_status *)(malloc(sizeof(struct e_status) * num_verts))) == NULL) {
        return(0);
    }

    /*
     * Do some preprocessing on all edges. This constructs a table structure in memory of all
     * the edge properties and can "flip" some edges so sorting works correctly.
     */
    preprocess_all_edges(ctx, verts, num_verts, edgbuf);

    /* can happen with a zero area mask */
    if (ctx->all_edges == NULL) {
        free(edgbuf);
        return(1);
    }
    /*
     * Set the pointer for tracking the edges currently in processing to NULL to make sure
     * we don't get some crazy value after initialization.
     */
    ctx->possible_edges = NULL;

    /*
     * Loop through all scan lines to be drawn. Since we sorted by Y values during
     * preprocess_all_edges(), we can already exact values for the lowest and
     * highest Y values we could possibly need by induction. The preprocessing sorted
     * out edges by Y position, we can cycle the current edge being processed once
     * it runs out of Y pixels. When we have no more edges, meaning the current edge
     * is NULL after setting the "current" edge to be the previous current edge's
     * "next" edge in the Y sorted edge connection chain, we can stop looping Y values,
     * since we can't possibly have more scan lines if we ran out of edges. :)
     *
     * TODO: This clips Y to the frame buffer, which should be done in the preprocessor, but for now is done here.
     *       Will get changed once DEM code gets in.
     */
    for(y_curr = ctx->all_edges->ybeg; (ctx->all_edges || ctx->possible_edges); y_curr++) {

        /*
         * Link any edges that start on the current scan line into the list of
         * edges currently needed to draw at least this, if not several, scan lines.
         */

        /*
         * Set the current edge to the beginning of the list of edges to be rasterized
         * into this scan line.
         *
         * We could have lots of edge here, so iterate over all the edges needed. The
         * preprocess_all_edges() function sorted edges by X within each chunk of Y sorting
         * so we safely cycle edges to thier own "next" edges in order.
         *
         * At each iteration, make sure we still have a non-NULL edge.
         */
        for(edgec = &ctx->possible_edges; ctx->all_edges && (ctx->all_edges->ybeg == y_curr);) {
            x_curr = ctx->all_edges->x;                  /* Set current X position. */
            for(;;) {                                    /* Start looping edges. Will break when edges run out. */
                e_curr = *edgec;                         /* Set up a current edge pointer. */
                if(!e_curr || (e_curr->x >= x_curr)) {   /* If we have an no edge, or we need to skip some X-span, */
                    e_temp = ctx->all_edges->e_next;     /* set a temp "next" edge to test. */
                    *edgec = ctx->all_edges;             /* Add this edge to the list to be scanned. */
                    ctx->all_edges->e_next = e_curr;     /* Set up the next edge. */
                    edgec = &ctx->all_edges->e_next;     /* Set our list to the next edge's location in memory. */
                    ctx->all_edges = e_temp;             /* Skip the NULL or bad X edge, set pointer to next edge. */
                    break;                               /* Stop looping edges (since we ran out or hit empty X span. */
                } else {
                    edgec = &e_curr->e_next;             /* Set the pointer to the edge list the "next" edge. */
                }
            }
        }

        /*
         * Determine the current scan line's offset in the pixel buffer based on its Y position.
         * Basically we just multiply the current scan line's Y value by the number of pixels in each line.
         */
        yp = y_curr * ctx->rb.sizex;
        /*
         * Set a "scan line pointer" in memory. The location of the buffer plus the row offset.
         */
        spxl = ctx->rb.buf + (yp);
        /*
         * Set up the current edge to the first (in X) edge. The edges which could possibly be in this
         * list were determined in the preceeding edge loop above. They were already sorted in X by the
         * initial processing function.
         *
         * At each iteration, test for a NULL edge. Since we'll keep cycling edge's to their own "next" edge
         * we will eventually hit a NULL when the list runs out.
         */
        for(e_curr = ctx->possible_edges; e_curr; e_curr = e_curr->e_next) {
            /*
             * Calculate a span of pixels to fill on the current scan line.
             *
             * Set the current pixel pointer by adding the X offset to the scan line's start offset.
             * Cycle the current edge the next edge.
             * Set the max X value to draw to be one less than the next edge's first pixel. This way we are
             * sure not to ever get into a situation where we have overdraw. (drawing the same pixel more than
             * one time because it's on a vertex connecting two edges)
             *
             * Then blast through all the pixels in the span, advancing the pointer and setting the color to white.
             *
             * TODO: Here we clip to the scan line, this is not efficient, and should be done in the preprocessor,
             *       but for now it is done here until the DEM code comes in.
             */

            /* set up xmin and xmax bounds on this scan line */
            cpxl = spxl + MAX2(e_curr->x, 0);
            e_curr = e_curr->e_next;
            mpxl = spxl + MIN2(e_curr->x, ctx->rb.sizex) - 1;

            if((y_curr >= 0) && (y_curr < ctx->rb.sizey)) {
                /* draw the pixels. */
                for(; cpxl <= mpxl; *cpxl++ += intensity);
            }
        }

        /*
         * Loop through all edges of polygon that could be hit by this scan line,
         * and figure out their x-intersections with the next scan line.
         *
         * Either A.) we wont have any more edges to test, or B.) we just add on the
         * slope delta computed in preprocessing step. Since this draws non-antialiased
         * polygons, we dont have fractional positions, so we only move in x-direction
         * when needed to get all the way to the next pixel over...
         */
        for(edgec = &ctx->possible_edges; (e_curr = *edgec);) {
            if(!(--(e_curr->num))) {
                *edgec = e_curr->e_next;
            } else {
                e_curr->x += e_curr->xshift;
                if((e_curr->drift += e_curr->drift_inc) > 0) {
                    e_curr->x += e_curr->xdir;
                    e_curr->drift -= e_curr->drift_dec;
                }
                edgec = &e_curr->e_next;
            }
        }
        /*
         * It's possible that some edges may have crossed during the last step, so we'll be sure
         * that we ALWAYS intersect scan lines in order by shuffling if needed to make all edges
         * sorted by x-intersection coordinate. We'll always scan through at least once to see if
         * edges crossed, and if so, we set the 'swixd' flag. If 'swixd' gets set on the initial
         * pass, then we know we need to sort by x, so then cycle through edges again and perform
         * the sort.-
         */
        if(ctx->possible_edges) {
            for(edgec = &ctx->possible_edges; (e_curr = *edgec)->e_next; edgec = &(*edgec)->e_next) {
                /* if the current edge hits scan line at greater X than the next edge, we need to exchange the edges */
                if(e_curr->x > e_curr->e_next->x) {
                    *edgec = e_curr->e_next;
                    /* exchange the pointers */
                    e_temp = e_curr->e_next->e_next;
                    e_curr->e_next->e_next = e_curr;
                    e_curr->e_next = e_temp;
                    /* set flag that we had at least one switch */
                    swixd = 1;
                }
            }
            /* if we did have a switch, look for more (there will more if there was one) */
            for(;;) {
                /* reset exchange flag so it's only set if we encounter another one */
                swixd = 0;
                for(edgec = &ctx->possible_edges; (e_curr = *edgec)->e_next; edgec = &(*edgec)->e_next) {
                    /* again, if current edge hits scan line at higher X than next edge, exchange the edges and set flag */
                    if(e_curr->x > e_curr->e_next->x) {
                        *edgec = e_curr->e_next;
                        /* exchange the pointers */
                        e_temp = e_curr->e_next->e_next;
                        e_curr->e_next->e_next = e_curr;
                        e_curr->e_next = e_temp;
                        /* flip the exchanged flag */
                        swixd = 1;
                    }
                }
                /* if we had no exchanges, we're done reshuffling the pointers */
                if(!swixd) {
                    break;
                }
            }
        }
    }

    free(edgbuf);
    return 1;
}

int PLX_raskterize(float(*base_verts)[2], int num_base_verts,
                   float *buf, int buf_x, int buf_y, int do_mask_AA) {
    int subdiv_AA = (do_mask_AA != 0)? 0:0;
    int i;                                   /* i: Loop counter. */
    int sAx;
    int sAy;
    struct poly_vert *ply;                   /* ply: Pointer to a list of integer buffer-space vertex coordinates. */
    struct r_fill_context ctx = {0};
    const float buf_x_f = (float)(buf_x);
    const float buf_y_f = (float)(buf_y);
    float div_offset=(1.0f / (float)(subdiv_AA));
    float div_offset_static = 0.5f * (float)(subdiv_AA) * div_offset;
    /*
     * Allocate enough memory for our poly_vert list. It'll be the size of the poly_vert
     * data structure multiplied by the number of base_verts.
     *
     * In the event of a failure to allocate the memory, return 0, so this error can
     * be distinguished as a memory allocation error.
     */
    if((ply = (struct poly_vert *)(malloc(sizeof(struct poly_vert) * num_base_verts))) == NULL) {
        return(0);
    }

    ctx.rb.buf = buf;                            /* Set the output buffer pointer. */
    ctx.rb.sizex = buf_x;                        /* Set the output buffer size in X. (width) */
    ctx.rb.sizey = buf_y;                        /* Set the output buffer size in Y. (height) */
    /*
     * Loop over all verts passed in to be rasterized. Each vertex's X and Y coordinates are
     * then converted from normalized screen space (0.0 <= POS <= 1.0) to integer coordinates
     * in the buffer-space coordinates passed in inside buf_x and buf_y.
     *
     * It's worth noting that this function ONLY outputs fully white pixels in a mask. Every pixel
     * drawn will be 1.0f in value, there is no anti-aliasing.
     */

    if(!subdiv_AA) {
        for(i = 0; i < num_base_verts; i++) {                           /* Loop over all base_verts. */
            ply[i].x = (int)((base_verts[i][0] * buf_x_f) + 0.5f);       /* Range expand normalized X to integer buffer-space X. */
            ply[i].y = (int)((base_verts[i][1] * buf_y_f) + 0.5f); /* Range expand normalized Y to integer buffer-space Y. */
        }

        i = rast_scan_fill(&ctx, ply, num_base_verts,1.0f);  /* Call our rasterizer, passing in the integer coords for each vert. */
    } else {
        for(sAx=0; sAx < subdiv_AA; sAx++) {
            for(sAy=0; sAy < subdiv_AA; sAy++) {
                for(i=0; i < num_base_verts; i++) {
                    ply[i].x = (int)((base_verts[i][0]*buf_x_f)+0.5f - div_offset_static + (div_offset*(float)(sAx)));
                    ply[i].y = (int)((base_verts[i][1]*buf_y_f)+0.5f - div_offset_static + (div_offset*(float)(sAy)));
                }
                i = rast_scan_fill(&ctx, ply, num_base_verts,(1.0f / (float)(subdiv_AA*subdiv_AA)));
            }
        }
    }
    free(ply);                                      /* Free the memory allocated for the integer coordinate table. */
    return(i);                                      /* Return the value returned by the rasterizer. */
}

/*
 * This function clips drawing to the frame buffer. That clipping will likely be moved into the preprocessor
 * for speed, but waiting on final design choices for curve-data before eliminating data the DEM code will need
 * if it ends up being coupled with this function.
 */
static int rast_scan_feather(struct r_fill_context *ctx,
                             float(*base_verts_f)[2], int num_base_verts,
                             struct poly_vert *feather_verts, float(*feather_verts_f)[2], int num_feather_verts) {
    int x_curr;                 /* current pixel position in X */
    int y_curr;                 /* current scan line being drawn */
    int yp;                     /* y-pixel's position in frame buffer */
    int swixd = 0;              /* whether or not edges switched position in X */
    float *cpxl;                /* pixel pointers... */
    float *mpxl;
    float *spxl;
    struct e_status *e_curr;    /* edge pointers... */
    struct e_status *e_temp;
    struct e_status *edgbuf;
    struct e_status **edgec;

    /* from dem */
    int a;                          // a = temporary pixel index buffer loop counter
    float fsz;                        // size of the frame
    unsigned int rsl;               // long used for finding fast 1.0/sqrt
    float rsf;                      // float used for finding fast 1.0/sqrt
    const float rsopf = 1.5f;       // constant float used for finding fast 1.0/sqrt

    //unsigned int gradientFillOffset;
    float t;
    float ud;                // ud = unscaled edge distance
    float dmin;              // dmin = minimum edge distance
    float odist;                    // odist = current outer edge distance
    float idist;                    // idist = current inner edge distance
    float dx;                         // dx = X-delta (used for distance proportion calculation)
    float dy;                         // dy = Y-delta (used for distance proportion calculation)
    float xpxw = (1.0f / (float)(ctx->rb.sizex));  // xpxw = normalized pixel width
    float ypxh = (1.0f / (float)(ctx->rb.sizey));  // ypxh = normalized pixel height
#ifdef __PLX_KD_TREE__
    void *res_kdi;
    void *res_kdo;
    float clup[2];
#endif

    /*
     * If the number of verts specified to render as a polygon is less than 3,
     * return immediately. Obviously we cant render a poly with sides < 3. The
     * return for this we set to 1, simply so it can be distinguished from the
     * next place we could return,
     * which is a failure to allocate memory.
     */
    if(num_feather_verts < 3) {
        return(1);
    }

    /*
     * Try to allocate an edge buffer in memory. needs to be the size of the edge tracking data
     * multiplied by the number of edges, which is always equal to the number of verts in
     * a 2D polygon. Here we return 0 to indicate a memory allocation failure, as opposed to a 1 for
     * the preceeding error, which was a rasterization request on a 2D poly with less than
     * 3 sides.
     */
    if((edgbuf = (struct e_status *)(malloc(sizeof(struct e_status) * num_feather_verts))) == NULL) {
        return(0);
    }

    /*
     * Do some preprocessing on all edges. This constructs a table structure in memory of all
     * the edge properties and can "flip" some edges so sorting works correctly.
     */
    preprocess_all_edges(ctx, feather_verts, num_feather_verts, edgbuf);

    /* can happen with a zero area mask */
    if (ctx->all_edges == NULL) {
        free(edgbuf);
        return(1);
    }

    /*
     * Set the pointer for tracking the edges currently in processing to NULL to make sure
     * we don't get some crazy value after initialization.
     */
    ctx->possible_edges = NULL;

    /*
     * Loop through all scan lines to be drawn. Since we sorted by Y values during
     * preprocess_all_edges(), we can already exact values for the lowest and
     * highest Y values we could possibly need by induction. The preprocessing sorted
     * out edges by Y position, we can cycle the current edge being processed once
     * it runs out of Y pixels. When we have no more edges, meaning the current edge
     * is NULL after setting the "current" edge to be the previous current edge's
     * "next" edge in the Y sorted edge connection chain, we can stop looping Y values,
     * since we can't possibly have more scan lines if we ran out of edges. :)
     *
     * TODO: This clips Y to the frame buffer, which should be done in the preprocessor, but for now is done here.
     *       Will get changed once DEM code gets in.
     */
    for(y_curr = ctx->all_edges->ybeg; (ctx->all_edges || ctx->possible_edges); y_curr++) {

        /*
         * Link any edges that start on the current scan line into the list of
         * edges currently needed to draw at least this, if not several, scan lines.
         */

        /*
         * Set the current edge to the beginning of the list of edges to be rasterized
         * into this scan line.
         *
         * We could have lots of edge here, so iterate over all the edges needed. The
         * preprocess_all_edges() function sorted edges by X within each chunk of Y sorting
         * so we safely cycle edges to thier own "next" edges in order.
         *
         * At each iteration, make sure we still have a non-NULL edge.
         */
        for(edgec = &ctx->possible_edges; ctx->all_edges && (ctx->all_edges->ybeg == y_curr);) {
            x_curr = ctx->all_edges->x;                  /* Set current X position. */
            for(;;) {                                    /* Start looping edges. Will break when edges run out. */
                e_curr = *edgec;                         /* Set up a current edge pointer. */
                if(!e_curr || (e_curr->x >= x_curr)) {   /* If we have an no edge, or we need to skip some X-span, */
                    e_temp = ctx->all_edges->e_next;     /* set a temp "next" edge to test. */
                    *edgec = ctx->all_edges;             /* Add this edge to the list to be scanned. */
                    ctx->all_edges->e_next = e_curr;     /* Set up the next edge. */
                    edgec = &ctx->all_edges->e_next;     /* Set our list to the next edge's location in memory. */
                    ctx->all_edges = e_temp;             /* Skip the NULL or bad X edge, set pointer to next edge. */
                    break;                               /* Stop looping edges (since we ran out or hit empty X span. */
                } else {
                    edgec = &e_curr->e_next;             /* Set the pointer to the edge list the "next" edge. */
                }
            }
        }

        /*
         * Determine the current scan line's offset in the pixel buffer based on its Y position.
         * Basically we just multiply the current scan line's Y value by the number of pixels in each line.
         */
        yp = y_curr * ctx->rb.sizex;
        /*
         * Set a "scan line pointer" in memory. The location of the buffer plus the row offset.
         */
        spxl = ctx->rb.buf + (yp);
        /*
         * Set up the current edge to the first (in X) edge. The edges which could possibly be in this
         * list were determined in the preceeding edge loop above. They were already sorted in X by the
         * initial processing function.
         *
         * At each iteration, test for a NULL edge. Since we'll keep cycling edge's to their own "next" edge
         * we will eventually hit a NULL when the list runs out.
         */
        for(e_curr = ctx->possible_edges; e_curr; e_curr = e_curr->e_next) {
            /*
             * Calculate a span of pixels to fill on the current scan line.
             *
             * Set the current pixel pointer by adding the X offset to the scan line's start offset.
             * Cycle the current edge the next edge.
             * Set the max X value to draw to be one less than the next edge's first pixel. This way we are
             * sure not to ever get into a situation where we have overdraw. (drawing the same pixel more than
             * one time because it's on a vertex connecting two edges)
             *
             * Then blast through all the pixels in the span, advancing the pointer and setting the color to white.
             *
             * TODO: Here we clip to the scan line, this is not efficient, and should be done in the preprocessor,
             *       but for now it is done here until the DEM code comes in.
             */

            /* set up xmin and xmax bounds on this scan line */
            cpxl = spxl + MAX2(e_curr->x, 0);
            e_curr = e_curr->e_next;
            mpxl = spxl + MIN2(e_curr->x, ctx->rb.sizex) - 1;

            if((y_curr >= 0) && (y_curr < ctx->rb.sizey)) {
                t = ((float)((cpxl - spxl) % ctx->rb.sizex) + 0.5f) * xpxw;
                fsz = ((float)(y_curr) + 0.5f) * ypxh;
                /* draw the pixels. */
                for(; cpxl <= mpxl; cpxl++, t += xpxw) {
                    //do feather check
                    // first check that pixel isn't already full, and only operate if it is not
                    if(*cpxl < 0.9999f) {
#ifndef __PLX_KD_TREE__
                        dmin = 2.0f;                        // reset min distance to edge pixel
                        for(a = 0; a < num_feather_verts; a++) {  // loop through all outer edge buffer pixels
                            dx = t - feather_verts_f[a][0];          // set dx to gradient pixel column - outer edge pixel row
                            dy = fsz - feather_verts_f[a][1];        // set dy to gradient pixel row - outer edge pixel column
                            ud = dx * dx + dy * dy;               // compute sum of squares
                            if(ud < dmin) {                       // if our new sum of squares is less than the current minimum
                                dmin = ud;                        // set a new minimum equal to the new lower value
                            }
                        }
                        odist = dmin;                    // cast outer min to a float
                        rsf = odist * 0.5f;                       //
                        rsl = *(unsigned int *)&odist;            // use some peculiar properties of the way bits are stored
                        rsl = 0x5f3759df - (rsl >> 1);            // in floats vs. unsigned ints to compute an approximate
                        odist = *(float *)&rsl;                   // reciprocal square root
                        odist = odist * (rsopf - (rsf * odist * odist));  // -- ** this line can be iterated for more accuracy ** --
                        odist = odist * (rsopf - (rsf * odist * odist));
                        dmin = 2.0f;                        // reset min distance to edge pixel
                        for(a = 0; a < num_base_verts; a++) {     // loop through all inside edge pixels
                            dx = t - base_verts_f[a][0];             // compute delta in Y from gradient pixel to inside edge pixel
                            dy = fsz - base_verts_f[a][1];           // compute delta in X from gradient pixel to inside edge pixel
                            ud = dx * dx + dy * dy;   // compute sum of squares
                            if(ud < dmin) {           // if our new sum of squares is less than the current minimum we've found
                                dmin = ud;            // set a new minimum equal to the new lower value
                            }
                        }
                        idist = dmin;                    // cast inner min to a float
                        rsf = idist * 0.5f;                       //
                        rsl = *(unsigned int *)&idist;            //
                        rsl = 0x5f3759df - (rsl >> 1);            // see notes above
                        idist = *(float *)&rsl;                   //
                        idist = idist * (rsopf - (rsf * idist * idist));  //
                        idist = idist * (rsopf - (rsf * idist * idist));
                        /*
                         * Note once again that since we are using reciprocals of distance values our
                         * proportion is already the correct intensity, and does not need to be
                         * subtracted from 1.0 like it would have if we used real distances.
                         */
#else
                        clup[0]=t;
                        clup[1]=fsz;
                        res_kdi=kd_nearestf(ctx->kdi,clup);
                        res_kdo=kd_nearestf(ctx->kdo,clup);
                        kd_res_itemf(res_kdi,clup);
                        dx=t-clup[0];
                        dy=fsz-clup[1];
                        idist=dx*dx+dy*dy;
                        rsf = idist * 0.5f;                       //
                        rsl = *(unsigned int *)&idist;            //
                        rsl = 0x5f3759df - (rsl >> 1);            // see notes above
                        idist = *(float *)&rsl;                   //
                        idist = idist * (rsopf - (rsf * idist * idist));  //
                        idist = idist * (rsopf - (rsf * idist * idist));
                        kd_res_itemf(res_kdo,clup);
                        dx=t-clup[0];
                        dy=fsz-clup[1];
                        odist=dx*dx+dy*dy;
                        rsf = odist * 0.5f;                       //
                        rsl = *(unsigned int *)&odist;            // use some peculiar properties of the way bits are stored
                        rsl = 0x5f3759df - (rsl >> 1);            // in floats vs. unsigned ints to compute an approximate
                        odist = *(float *)&rsl;                   // reciprocal square root
                        odist = odist * (rsopf - (rsf * odist * odist));  // -- ** this line can be iterated for more accuracy ** --
                        odist = odist * (rsopf - (rsf * odist * odist));

#endif
                        /* set intensity, do the += so overlapping gradients are additive */
                        *cpxl = (idist / (idist+odist));
                    }
                }
            }
        }

        /*
         * Loop through all edges of polygon that could be hit by this scan line,
         * and figure out their x-intersections with the next scan line.
         *
         * Either A.) we wont have any more edges to test, or B.) we just add on the
         * slope delta computed in preprocessing step. Since this draws non-antialiased
         * polygons, we dont have fractional positions, so we only move in x-direction
         * when needed to get all the way to the next pixel over...
         */
        for(edgec = &ctx->possible_edges; (e_curr = *edgec);) {
            if(!(--(e_curr->num))) {
                *edgec = e_curr->e_next;
            } else {
                e_curr->x += e_curr->xshift;
                if((e_curr->drift += e_curr->drift_inc) > 0) {
                    e_curr->x += e_curr->xdir;
                    e_curr->drift -= e_curr->drift_dec;
                }
                edgec = &e_curr->e_next;
            }
        }
        /*
         * It's possible that some edges may have crossed during the last step, so we'll be sure
         * that we ALWAYS intersect scan lines in order by shuffling if needed to make all edges
         * sorted by x-intersection coordinate. We'll always scan through at least once to see if
         * edges crossed, and if so, we set the 'swixd' flag. If 'swixd' gets set on the initial
         * pass, then we know we need to sort by x, so then cycle through edges again and perform
         * the sort.-
         */
        if(ctx->possible_edges) {
            for(edgec = &ctx->possible_edges; (e_curr = *edgec)->e_next; edgec = &(*edgec)->e_next) {
                /* if the current edge hits scan line at greater X than the next edge, we need to exchange the edges */
                if(e_curr->x > e_curr->e_next->x) {
                    *edgec = e_curr->e_next;
                    /* exchange the pointers */
                    e_temp = e_curr->e_next->e_next;
                    e_curr->e_next->e_next = e_curr;
                    e_curr->e_next = e_temp;
                    /* set flag that we had at least one switch */
                    swixd = 1;
                }
            }
            /* if we did have a switch, look for more (there will more if there was one) */
            for(;;) {
                /* reset exchange flag so it's only set if we encounter another one */
                swixd = 0;
                for(edgec = &ctx->possible_edges; (e_curr = *edgec)->e_next; edgec = &(*edgec)->e_next) {
                    /* again, if current edge hits scan line at higher X than next edge,
                     * exchange the edges and set flag */
                    if(e_curr->x > e_curr->e_next->x) {
                        *edgec = e_curr->e_next;
                        /* exchange the pointers */
                        e_temp = e_curr->e_next->e_next;
                        e_curr->e_next->e_next = e_curr;
                        e_curr->e_next = e_temp;
                        /* flip the exchanged flag */
                        swixd = 1;
                    }
                }
                /* if we had no exchanges, we're done reshuffling the pointers */
                if(!swixd) {
                    break;
                }
            }
        }
    }

    free(edgbuf);
    return 1;
}

int PLX_raskterize_feather(float(*base_verts)[2], int num_base_verts, float(*feather_verts)[2], int num_feather_verts,
                           float *buf, int buf_x, int buf_y) {
    //void plx_floatsort(float(*f)[2], unsigned int n, int sortby);
    int i;                            /* i: Loop counter. */
    struct poly_vert *fe;             /* fe: Pointer to a list of integer buffer-space feather vertex coords. */
    struct r_fill_context ctx = {0};

    /* for faster multiply */
    const float buf_x_f = (float)buf_x;
    const float buf_y_f = (float)buf_y;
#ifdef __PLX_KD_TREE__
    ctx.kdi = kd_create(2);
    ctx.kdo = kd_create(2);
#endif
    /*
     * Allocate enough memory for our poly_vert list. It'll be the size of the poly_vert
     * data structure multiplied by the number of verts.
     *
     * In the event of a failure to allocate the memory, return 0, so this error can
     * be distinguished as a memory allocation error.
     */
    if((fe = (struct poly_vert *)(malloc(sizeof(struct poly_vert) * num_feather_verts))) == NULL) {
        return(0);
    }

    /*
     * Loop over all verts passed in to be rasterized. Each vertex's X and Y coordinates are
     * then converted from normalized screen space (0.0 <= POS <= 1.0) to integer coordinates
     * in the buffer-space coordinates passed in inside buf_x and buf_y.
     *
     * It's worth noting that this function ONLY outputs fully white pixels in a mask. Every pixel
     * drawn will be 1.0f in value, there is no anti-aliasing.
     */
    for(i = 0; i < num_feather_verts; i++) {             /* Loop over all verts. */
        fe[i].x = (int)((feather_verts[i][0] * buf_x_f) + 0.5f);  /* Range expand normalized X to integer buffer-space X. */
        fe[i].y = (int)((feather_verts[i][1] * buf_y_f) + 0.5f);  /* Range expand normalized Y to integer buffer-space Y. */
#ifdef __PLX_KD_TREE__
        kd_insertf(ctx.kdo,feather_verts[i],NULL);
    }
    for(i=0;i<num_base_verts;i++){
        kd_insertf(ctx.kdi,base_verts[i],NULL);
#endif
    }

    ctx.rb.buf = buf;                            /* Set the output buffer pointer. */
    ctx.rb.sizex = buf_x;                        /* Set the output buffer size in X. (width) */
    ctx.rb.sizey = buf_y;                        /* Set the output buffer size in Y. (height) */
    /* pre-sort the sets of edge verts on y */
    //plx_floatsort(base_verts,num_base_verts,0);
    //plx_floatsort(base_verts,num_base_verts,1);
    //plx_floatsort(feather_verts,num_feather_verts,0);
    //plx_floatsort(feather_verts,num_feather_verts,1);
    /* Call our rasterizer, passing in the integer coords for each vert. */
    i = rast_scan_feather(&ctx, base_verts, num_base_verts, fe, feather_verts, num_feather_verts);
    free(fe);
    return i;                                   /* Return the value returned by the rasterizer. */
}

#ifndef __PLX__FAKE_AA__

int get_range_expanded_pixel_coord(float normalized_value, int max_value) {
    return (int)((normalized_value * (float)(max_value)) + 0.5f);
}

__inline float get_pixel_intensity(float *buf, int buf_x, int buf_y, int pos_x, int pos_y) {
    if(pos_x < 0 || pos_x >= buf_x || pos_y < 0 || pos_y >= buf_y) {
        return 0.0f;
    }
    return buf[(pos_y * buf_x) + pos_x];
}

__inline float get_pixel_intensity_bilinear(float *buf, int buf_x, int buf_y, float u, float v) {
    int a;
    int b;
    int a_plus_1;
    int b_plus_1;
    float prop_u;
    float prop_v;
    float inv_prop_u;
    float inv_prop_v;
    if(u<0.0f || u>1.0f || v<0.0f || v>1.0f) {
        return 0.0f;
    }
    u = u * (float)(buf_x) - 0.5f;
    v = v * (float)(buf_y) - 0.5f;
    a = (int)(u);
    b = (int)(v);
    prop_u = u - (float)(a);
    prop_v = v - (float)(b);
    inv_prop_u = 1.0f - prop_u;
    inv_prop_v = 1.0f - prop_v;
    a_plus_1 = MIN2((buf_x-1),a+1);
    b_plus_1 = MIN2((buf_y-1),b+1);
    return (buf[(b * buf_x) + a] * inv_prop_u + buf[(b*buf_x)+(a_plus_1)] * prop_u)*inv_prop_v+(buf[((b_plus_1) * buf_x)+a] * inv_prop_u + buf[((b_plus_1)*buf_x)+(a_plus_1)] * prop_u) * prop_v;

}

__inline void set_pixel_intensity(float *buf, int buf_x, int buf_y, int pos_x, int pos_y, float intensity) {
    if(pos_x < 0 || pos_x >= buf_x || pos_y < 0 || pos_y >= buf_y) {
        return;
    }
    buf[(pos_y * buf_x) + pos_x] = intensity;
}
#endif

int PLX_antialias_buffer(float *buf, int buf_x, int buf_y) {
#ifdef __PLX__FAKE_AA__
#ifdef __PLX_GREY_AA__
    int i=0;
    int sz = buf_x * buf_y;
    for(i=0; i<sz; i++) {
        buf[i] *= 0.5f;
    }
#endif
    (void)buf_x;
    (void)buf_y;
    (void)buf;
    return 1;
#else
    const float jump01 = 1.0f;
    const float jump02 = 1.0f;
    const float jump03 = 1.0f;
    const float jump04 = 1.0f;
    const float jump05 = 1.0f;
    const float jump06 = 1.5f;
    const float jump07 = 2.0f;
    const float jump08 = 2.0f;
    const float jump09 = 2.0f;
    const float jump10 = 2.0f;
    const float jump11 = 4.0f;
    const float jump12 = 8.0f;

    const float edge_threshold = 0.063f;
    const float edge_threshold_min = 0.0312f;
    const float quality_subpix = 1.0f;

    float fpcx,fpcy;
    float fpsqx,fpsqy;
    float fprevx,fprevy;
    float fpfowx,fpfowy;
    float offset_dgx,offset_dgy;
    float pci;
    float pdi;
    float pri;
    float pui;
    float pli;
    float uli;
    float dri;
    float uri;
    float dli;
    float udi;
    float lri;
    float fsi;
    float ti;
    float cdi;
    float bi;
    float uui;
    float ddi;
    float eri;
    float efi;
    float cci;
    float ltz;
    float spX;
    float inv_r;
    float spP;
    float gdc;
    float sdc;
    float gedc;
    float sedc;
    float glu;
    float slu;
    float gr;
    float sr;
    float grexp;
    float r;
    float grc;
    float lre;
    float ude;
    float lre0;
    float ude0;
    float lre1;
    float ude1;
    float lre2;
    float ude2;
    float lre3;
    float ude3;
    float sdst;
    float tg0;
    float tg1;
    float tg2;
    float tg3;
    float tg4;
    float tg5;
    float tg6;
    float tg7;
    float ugrad;
    float dgrad;
    float grad;
    float gradexp;
    float revdst;
    float fowdst;
    float dst;
    float dsts;
    float inv_dsts;
    float pxOff;
    float gpxOff;
    float tgpxOff;
    float opx;
    float opy;
    int uls;
    int sph;
    int revsph;
    int fowsph;
    int lrsp;
    int done;
    int revpp;
    int revdone;
    int fowdone;
    int tug_of_war;
    int curr_x=0;
    int curr_y=0;
    opx = (1.0f / (float)(buf_x));
    opy = (1.0f / (float)(buf_y));
    for(curr_y=0; curr_y < buf_y; curr_y++) {
        for(curr_x=0; curr_x < buf_x; curr_x++) {
            fpcx = ((float)(curr_x) + 0.5f) * opx;
            fpcy = ((float)(curr_y) + 0.5f) * opy;
//#define __PLX_BILINEAR_INITIAL_SAMPLES__ 1
#ifdef __PLX_BILINEAR_INITIAL_SAMPLES__
            lumaM = get_pixel_intensity_bilinear(buf, buf_x, buf_y, posM_x, posM_y);
            lumaS = get_pixel_intensity_bilinear(buf, buf_x, buf_y, posM_x, posM_y + opy);
            lumaE = get_pixel_intensity_bilinear(buf, buf_x, buf_y, posM_x + opx, posM_y);
            lumaN = get_pixel_intensity_bilinear(buf, buf_x, buf_y, posM_x, posM_y - opy);
            lumaW = get_pixel_intensity_bilinear(buf, buf_x, buf_y, posM_x - opx, posM_y);
#else
    pci = get_pixel_intensity(buf, buf_x, buf_y, curr_x, curr_y);
    pdi = get_pixel_intensity(buf, buf_x, buf_y, curr_x, curr_y + 1);
    pri = get_pixel_intensity(buf, buf_x, buf_y, curr_x + 1, curr_y);
    pui = get_pixel_intensity(buf, buf_x, buf_y, curr_x, curr_y - 1);
    pli = get_pixel_intensity(buf, buf_x, buf_y, curr_x - 1, curr_y);
#endif
            gdc = MAX2(pdi, pci);
            sdc = MIN2(pdi, pci);
            gedc = MAX2(pri, gdc);
            sedc = MIN2(pri, sdc);
            glu = MAX2(pui, pli);
            slu = MIN2(pui, pli);
            gr = MAX2(glu, gedc);
            sr = MIN2(slu, sedc);
            grexp = gr * edge_threshold;
            r = gr - sr;
            grc = MAX2(edge_threshold_min, grexp);

            done = r < grc ? 1:0;
            if(done) {
                set_pixel_intensity(buf, buf_x, buf_y, curr_x, curr_y, pci);
            } else {

                uli = get_pixel_intensity_bilinear(buf, buf_x, buf_y, fpcx - opx, fpcy - opy);
                dri = get_pixel_intensity_bilinear(buf, buf_x, buf_y, fpcx + opx, fpcy + opy);
                uri = get_pixel_intensity_bilinear(buf, buf_x, buf_y, fpcx + opx, fpcy - opy);
                dli = get_pixel_intensity_bilinear(buf, buf_x, buf_y, fpcx - opx, fpcy + opy);

                udi = pui + pdi;
                lri = pli + pri;
                inv_r = 1.0f/r;
                spP = udi + lri;
                lre0 = (-2.0f * pci) + udi;
                ude0 = (-2.0f * pci) + lri;

                fsi = uri + dri;
                ti = uli + uri;
                lre1 = (-2.0f * pri) + fsi;
                ude1 = (-2.0f * pui) + ti;

                cdi = uli + dli;
                bi = dli + dri;
                lre3 = (ABS(lre0) * 2.0f) + ABS(lre1);
                ude3 = (ABS(ude0) * 2.0f) + ABS(ude1);
                lre2 = (-2.0f * pli) + cdi;
                ude2 = (-2.0f * pdi) + bi;
                lre = ABS(lre2) + lre3;
                ude = ABS(ude2) + ude3;

                spX = cdi + fsi;
                sdst = 1.0f / (float)(buf_x);
                lrsp = lre >= ude ? 1:0;
                tg0 = spP * 2.0f + spX;

                if(!lrsp) {
                    pui = pli;
                    pdi = pri;
                } else {
                    sdst = 1.0f / (float)(buf_y);
                }
                tg1 = (tg0 * (1.0f/12.0f)) - pci;

                ugrad = pui - pci;
                dgrad = pdi - pci;
                uui = pui + pci;
                ddi = pdi + pci;
                revpp = (ABS(ugrad)) >= (ABS(dgrad)) ? 1:0;
                grad = MAX2(ABS(ugrad), ABS(dgrad));
                if(revpp) {
                    sdst = -sdst;
                }
                tg2 = MAX2(MIN2(ABS(tg1) * inv_r,1.0f),0.0f);

                fpsqx = fpcx;
                fpsqy = fpcy;
                offset_dgx = (!lrsp) ? 0.0f:(1.0f / (float)(buf_x));
                offset_dgy = (lrsp) ? 0.0f:(1.0f / (float)(buf_y));
                if(!lrsp) {
                    fpsqx += sdst * 0.5f;
                } else {
                    fpsqy += sdst * 0.5f;
                }

                fprevx = fpsqx - offset_dgx * jump01;
                fprevy = fpsqy - offset_dgy * jump01;
                fpfowx = fpsqx + offset_dgx * jump01;
                fpfowy = fpsqy + offset_dgy * jump01;
                tg3 = ((-2.0f)*tg2) + 3.0f;
                eri = get_pixel_intensity_bilinear(buf, buf_x, buf_y,fprevx,fprevy);
                tg4 = tg2 * tg2;
                efi = get_pixel_intensity_bilinear(buf, buf_x, buf_y, fpfowx,fpfowy);

                if(!revpp) {
                    uui = ddi;
                }
                gradexp = grad * 1.0f/4.0f;
                cci =pci - uui * 0.5f;
                tg5 = tg3 * tg4;
                ltz = cci < 0.0f ? 1:0;

                eri -= uui * 0.5f;
                efi -= uui * 0.5f;
                revdone = (ABS(eri)) >= gradexp ? 1:0;
                fowdone = (ABS(efi)) >= gradexp ? 1:0;
                if(!revdone) {
                    fprevx -= offset_dgx * jump02;
                    fprevy -= offset_dgy * jump02;
                }
                tug_of_war = (!revdone) || (!fowdone) ? 1:0;
                if(!fowdone) {
                    fpfowx += offset_dgx * jump02;
                    fpfowy += offset_dgy * jump02;
                }

                if(tug_of_war) {
                    if(!revdone) {
                        eri = get_pixel_intensity_bilinear(buf, buf_x, buf_y, fprevx,fprevy);
                    }
                    if(!fowdone) {
                        efi = get_pixel_intensity_bilinear(buf, buf_x, buf_y, fpfowx, fpfowy);
                    }
                    if(!revdone) {
                        eri = eri - uui * 0.5;
                    }
                    if(!fowdone) {
                        efi = efi - uui * 0.5;
                    }
                    revdone = (ABS(eri)) >= gradexp ? 1:0;
                    fowdone = (ABS(efi)) >= gradexp ? 1:0;
                    if(!revdone) {
                        fprevx -= offset_dgx * jump03;
                        fprevy -= offset_dgy * jump03;
                    }
                    tug_of_war = (!revdone) || (!fowdone) ? 1:0;
                    if(!fowdone) {
                        fpfowx += offset_dgx * jump03;
                        fpfowy += offset_dgy * jump03;
                    }
                    if(tug_of_war) {
                        if(!revdone) {
                            eri = get_pixel_intensity_bilinear(buf, buf_x, buf_y,fprevx,fprevy);
                        }
                        if(!fowdone) {
                            efi = get_pixel_intensity_bilinear(buf, buf_x, buf_y, fpfowx,fpfowy);
                        }
                        if(!revdone) {
                            eri = eri - uui * 0.5;
                        }
                        if(!fowdone) {
                            efi = efi - uui * 0.5;
                        }
                        revdone = (ABS(eri)) >= gradexp ? 1:0;
                        fowdone = (ABS(efi)) >= gradexp ? 1:0;
                        if(!revdone) {
                            fprevx -= offset_dgx * jump04;
                            fprevy -= offset_dgy * jump04;
                        }
                        tug_of_war = (!revdone) || (!fowdone) ? 1:0;
                        if(!fowdone) {
                            fpfowx += offset_dgx * jump04;
                            fpfowy += offset_dgy * jump04;
                        }
                        if(tug_of_war) {
                            if(!revdone) {
                                eri = get_pixel_intensity_bilinear(buf, buf_x, buf_y,fprevx,fprevy);
                            }
                            if(!fowdone) {
                                efi = get_pixel_intensity_bilinear(buf, buf_x, buf_y, fpfowx,fpfowy);
                            }
                            if(!revdone) {
                                eri = eri - uui * 0.5;
                            }
                            if(!fowdone) {
                                efi = efi - uui * 0.5;
                            }
                            revdone = (ABS(eri)) >= gradexp ? 1:0;
                            fowdone = (ABS(efi)) >= gradexp ? 1:0;
                            if(!revdone) {
                                fprevx -= offset_dgx * jump05;
                                fprevy -= offset_dgy * jump05;
                            }
                            tug_of_war = (!revdone) || (!fowdone) ? 1:0;
                            if(!fowdone) {
                                fpfowx += offset_dgx * jump05;
                                fpfowy += offset_dgy * jump05;
                            }
                            if(tug_of_war) {
                                if(!revdone) {
                                    eri = get_pixel_intensity_bilinear(buf, buf_x, buf_y,fprevx,fprevy);
                                }
                                if(!fowdone) {
                                    efi = get_pixel_intensity_bilinear(buf, buf_x, buf_y, fpfowx,fpfowy);
                                }
                                if(!revdone) {
                                    eri = eri - uui * 0.5;
                                }
                                if(!fowdone) {
                                    efi = efi - uui * 0.5;
                                }
                                revdone = (ABS(eri)) >= gradexp ? 1:0;
                                fowdone = (ABS(efi)) >= gradexp ? 1:0;
                                if(!revdone) {
                                    fprevx -= offset_dgx * jump06;
                                    fprevy -= offset_dgy * jump06;
                                }
                                tug_of_war = (!revdone) || (!fowdone) ? 1:0;
                                if(!fowdone) {
                                    fpfowx += offset_dgx * jump06;
                                    fpfowy += offset_dgy * jump06;
                                }
                                if(tug_of_war) {
                                    if(!revdone) {
                                        eri = get_pixel_intensity_bilinear(buf, buf_x, buf_y,fprevx,fprevy);
                                    }
                                    if(!fowdone) {
                                        efi = get_pixel_intensity_bilinear(buf, buf_x, buf_y, fpfowx,fpfowy);
                                    }
                                    if(!revdone) {
                                        eri = eri - uui * 0.5;
                                    }
                                    if(!fowdone) {
                                        efi = efi - uui * 0.5;
                                    }
                                    revdone = (ABS(eri)) >= gradexp ? 1:0;
                                    fowdone = (ABS(efi)) >= gradexp ? 1:0;
                                    if(!revdone) {
                                        fprevx -= offset_dgx * jump07;
                                        fprevy -= offset_dgy * jump07;
                                    }
                                    tug_of_war = (!revdone) || (!fowdone) ? 1:0;
                                    if(!fowdone) {
                                        fpfowx += offset_dgx * jump07;
                                        fpfowy += offset_dgy * jump07;
                                    }
                                    if(tug_of_war) {
                                        if(!revdone) {
                                            eri = get_pixel_intensity_bilinear(buf, buf_x, buf_y,fprevx,fprevy);
                                        }
                                        if(!fowdone) {
                                            efi = get_pixel_intensity_bilinear(buf, buf_x, buf_y, fpfowx,fpfowy);
                                        }
                                        if(!revdone) {
                                            eri = eri - uui * 0.5;
                                        }
                                        if(!fowdone) {
                                            efi = efi - uui * 0.5;
                                        }
                                        revdone = (ABS(eri)) >= gradexp ? 1:0;
                                        fowdone = (ABS(efi)) >= gradexp ? 1:0;
                                        if(!revdone) {
                                            fprevx -= offset_dgx * jump08;
                                            fprevy -= offset_dgy * jump08;
                                        }
                                        tug_of_war = (!revdone) || (!fowdone) ? 1:0;
                                        if(!fowdone) {
                                            fpfowx += offset_dgx * jump08;
                                            fpfowy += offset_dgy * jump08;
                                        }
                                        if(tug_of_war) {
                                            if(!revdone) {
                                                eri = get_pixel_intensity_bilinear(buf, buf_x, buf_y,fprevx,fprevy);
                                            }
                                            if(!fowdone) {
                                                efi = get_pixel_intensity_bilinear(buf, buf_x, buf_y, fpfowx,fpfowy);
                                            }
                                            if(!revdone) {
                                                eri = eri - uui * 0.5;
                                            }
                                            if(!fowdone) {
                                                efi = efi - uui * 0.5;
                                            }
                                            revdone = (ABS(eri)) >= gradexp ? 1:0;
                                            fowdone = (ABS(efi)) >= gradexp ? 1:0;
                                            if(!revdone) {
                                                fprevx -= offset_dgx * jump09;
                                                fprevy -= offset_dgy * jump09;
                                            }
                                            tug_of_war = (!revdone) || (!fowdone) ? 1:0;
                                            if(!fowdone) {
                                                fpfowx += offset_dgx * jump09;
                                                fpfowy += offset_dgy * jump09;
                                            }
                                            if(tug_of_war) {
                                                if(!revdone) {
                                                    eri = get_pixel_intensity_bilinear(buf, buf_x, buf_y,fprevx,fprevy);
                                                }
                                                if(!fowdone) {
                                                    efi = get_pixel_intensity_bilinear(buf, buf_x, buf_y, fpfowx,fpfowy);
                                                }
                                                if(!revdone) {
                                                    eri = eri - uui * 0.5;
                                                }
                                                if(!fowdone) {
                                                    efi = efi - uui * 0.5;
                                                }
                                                revdone = (ABS(eri)) >= gradexp ? 1:0;
                                                fowdone = (ABS(efi)) >= gradexp ? 1:0;
                                                if(!revdone) {
                                                    fprevx -= offset_dgx * jump10;
                                                    fprevy -= offset_dgy * jump10;
                                                }
                                                tug_of_war = (!revdone) || (!fowdone) ? 1:0;
                                                if(!fowdone) {
                                                    fpfowx += offset_dgx * jump10;
                                                    fpfowy += offset_dgy * jump10;
                                                }
                                                if(tug_of_war) {
                                                    if(!revdone) {
                                                        eri = get_pixel_intensity_bilinear(buf, buf_x, buf_y,fprevx,fprevy);
                                                    }
                                                    if(!fowdone) {
                                                        efi = get_pixel_intensity_bilinear(buf, buf_x, buf_y, fpfowx,fpfowy);
                                                    }
                                                    if(!revdone) {
                                                        eri = eri - uui * 0.5;
                                                    }
                                                    if(!fowdone) {
                                                        efi = efi - uui * 0.5;
                                                    }
                                                    revdone = (ABS(eri)) >= gradexp ? 1:0;
                                                    fowdone = (ABS(efi)) >= gradexp ? 1:0;
                                                    if(!revdone) {
                                                        fprevx -= offset_dgx * jump11;
                                                        fprevy -= offset_dgy * jump11;
                                                    }
                                                    tug_of_war = (!revdone) || (!fowdone) ? 1:0;
                                                    if(!fowdone) {
                                                        fpfowx += offset_dgx * jump11;
                                                        fpfowy += offset_dgy * jump11;
                                                    }
                                                    if(tug_of_war) {
                                                        if(!revdone) {
                                                            eri = get_pixel_intensity_bilinear(buf, buf_x, buf_y,fprevx,fprevy);
                                                        }
                                                        if(!fowdone) {
                                                            efi = get_pixel_intensity_bilinear(buf, buf_x, buf_y, fpfowx,fpfowy);
                                                        }
                                                        if(!revdone) {
                                                            eri = eri - uui * 0.5;
                                                        }
                                                        if(!fowdone) {
                                                            efi = efi - uui * 0.5;
                                                        }
                                                        revdone = (ABS(eri)) >= gradexp ? 1:0;
                                                        fowdone = (ABS(efi)) >= gradexp ? 1:0;
                                                        if(!revdone) {
                                                            fprevx -= offset_dgx * jump12;
                                                            fprevy -= offset_dgy * jump12;
                                                        }
                                                        tug_of_war = (!revdone) || (!fowdone) ? 1:0;
                                                        if(!fowdone) {
                                                            fpfowx += offset_dgx * jump12;
                                                            fpfowy += offset_dgy * jump12;
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                revdst = fpcx - fprevx;
                fowdst = fpfowx - fpcx;
                if(!lrsp) {
                    revdst = fpcy - fprevy;
                    fowdst = fpfowy - fpcy;
                }

                revsph = ((eri < 0.0f) ? 1:0) != ltz ? 1:0;
                dsts = (fowdst + revdst);
                fowsph = ((efi < 0.0f) ? 1:0) != ltz ? 1:0;
                inv_dsts = 1.0f/dsts;

                uls = revdst < fowdst ? 1:0;
                dst = MIN2(revdst, fowdst);
                sph = (uls==1) ? revsph:fowsph;
                tg6 = tg5 * tg5;
                pxOff = (dst * (-inv_dsts)) + 0.5f;
                tg7 = tg6 * quality_subpix;

                gpxOff = (sph==1) ? pxOff : 0.0f;
                tgpxOff = MAX2(gpxOff, tg7);
                if(!lrsp) {
                    fpcx += tgpxOff * sdst;
                } else {
                    fpcy += tgpxOff * sdst;
                }
                set_pixel_intensity(buf,buf_x,buf_y,curr_x,curr_y,get_pixel_intensity_bilinear(buf, buf_x, buf_y, fpcx,fpcy));
            }
        }
    }
    return 1;

#endif
}

#define SWAP_POLYVERT(a,b)	point_temp[0]=(a)[0]; point_temp[1]=(a)[1]; (a)[0]=(b)[0]; (a)[1]=(b)[1]; (b)[0]=point_temp[0]; (b)[1]=point_temp[1];
#define __PLX_SMALL_COUNT__ 13
void plx_floatsort(float(*f)[2], unsigned int n, int sortby) {
    unsigned int a;
    unsigned int b;
    unsigned int c;
    unsigned int d=1;
    unsigned int hold;
    unsigned int index_list[50];
    int index_offset=0;
    float t[2];
    float point_temp[2];

    hold=n;
    for(;;) {
        if(hold-d < __PLX_SMALL_COUNT__) {
            for(b=d+1; b<=hold; b++) {
                t[1]=f[b][1];
                t[0]=f[b][0];
                for(a=b-1; a>=d; a--) {
                    if(f[a][sortby] <= t[sortby]) {
                        break;
                    }
                    f[a+1][1]=f[a][1];
                    f[a+1][0]=f[a][0];
                }
                f[a+1][1]=t[1];
                f[a+1][0]=t[0];
            }
            if(index_offset < 0) {
                break;
            }
            hold=index_list[index_offset--];
            d=index_list[index_offset--];
        } else {
            c=(d+hold) >> 1;
            SWAP_POLYVERT(f[c],f[d+1])
            if(f[d][sortby] > f[hold][sortby]) {
                SWAP_POLYVERT(f[d],f[hold])
            }
            if(f[d+1][sortby] > f[hold][sortby]) {
                SWAP_POLYVERT(f[d+1],f[hold])
            }
            if(f[d][sortby] > f[d+1][sortby]) {
                SWAP_POLYVERT(f[d],f[d+1])
            }
            a=d+1;
            b=hold;
            t[0]=f[d+1][0];
            t[1]=f[d+1][1];
            for(;;) {
                do a++;
                while(f[a][sortby] < t[sortby]);
                do b--;
                while(f[b][sortby] > t[sortby]);
                if(b < a) {
                    break;
                }
                SWAP_POLYVERT(f[a],f[b])
            }
            f[d+1][0]=f[b][0];
            f[d+1][1]=f[b][1];
            f[b][0]=t[0];
            f[b][1]=t[1];
            index_offset+=2;
            if(index_offset > __PLX_SMALL_COUNT__) {
                return;
            }
            if(hold-a+1 >= b-d) {
                index_list[index_offset]=hold;
                index_list[index_offset-1]=a;
                hold=b-1;
            } else {
                index_list[index_offset]=b-1;
                index_list[index_offset-1]=d;
                d=a;
            }
        }
    }
}

int plx_find_lower_bound(float v, float(*a)[2], int num_feather_verts) {
    int x;
    int l;
    int r;
    l=1;
    r=num_feather_verts;
    for(;;) {
        // interpolation style search
        //x=l+(v-a[l][1])*(r-l) / (a[r][1]-a[l][1]);

        // binary search
        x=(l+r) / 2;
        if(v<a[x][1]) {
            r=x-1;
        } else {
            l=x+1;
        }
        if((v>a[x-1][1] && v <= a[x][1]) || l>r) {
            break;
        }
    }
    if(v>a[x-1][1] && v <= a[x][1]) {
        return x;
    } else {
        return num_feather_verts;
    }
}

int plx_find_upper_bound(float v, float(*a)[2], int num_feather_verts) {
    int x;
    int l;
    int r;
    l=1;
    r=num_feather_verts;
    for(;;) {
        // interpolation style search
        //x=l+(v-a[l][1])*(r-l) / (a[r][1]-a[l][1]);

        // binary search
        x=(l+r) / 2;
        if(v<a[x][1]) {
            r=x-1;
        } else {
            l=x+1;
        }
        if((v>=a[x-1][1] && v < a[x][1]) || l>r) {
            break;
        }
    }
    if(v>=a[x-1][1] && v < a[x][1]) {
        return x-1;
    } else {
        return num_feather_verts;
    }
}

