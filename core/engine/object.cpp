/******************************************************************************/
/* Copyright (c) 2013-2025 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#include <string.h>

#include "object.h"
#include "rtimag.h"

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * object.cpp: Implementation of the object hierarchy.
 *
 * Main companion file of the engine responsible for instantiating and managing
 * the object hierarchy. It contains a definition of Object class (the root
 * of the hierarchy) and its derivative classes along with a set of algorithms
 * needed to construct and update per-object fields and cross-object relations.
 *
 * Object handles the following parts of the update initiated by the engine:
 * 0.5 phase (sequential) - hierarchical update of arrays' transform matrices
 * - update object's transform (via animator in the scene), time, flags
 * - compute array's transform matrix from the root down to the leaf objects
 * 1st phase (multi-threaded) - update surfaces' transform matrices, data fields
 * - compute array's inverse transform matrix needed in backend (tracer.cpp)
 * - compute surface's transform matrix from the immediate parent array
 * - compute surface's inverse transform matrix, backend-related SIMD fields
 * 2nd phase (multi-threaded) - update surfaces' clip lists, bounds, tile lists
 * - update surface's bounding and clipping boxes, bounding volume (sphere),
 *   taking into account surfaces from custom clippers list updated in 1st phase
 * 2.5 phase (sequential) - hierarchical update of arrays' bounds from surfaces
 * - update array's bounding box and volume structures (bvbox, trbox, inbox)
 *   from surfaces' bounding boxes and sub-arrays' bounding boxes
 *
 * In order to avoid cross-dependencies on the engine, object file contains
 * a definition of Registry interface inherited by the engine's Scene class,
 * instance of which is then passed to objects' constructors and serves as
 * both objects registry and memory heap (system.cpp).
 *
 * Registry's heap allocations are not allowed in multi-threaded phases
 * as SceneThread's heaps are used in this case to avoid race conditions.
 */

/******************************************************************************/
/*********************************   OBJECT   *********************************/
/******************************************************************************/

/*
 * Allocate object in custom heap.
 */
rt_pntr rt_Object::operator new(size_t size, rt_Heap *hp)
{
    return hp->alloc(size, RT_ALIGN);
}

rt_void rt_Object::operator delete(rt_pntr ptr)
{

}

/*
 * Instantiate object.
 */
rt_Object::rt_Object(rt_Registry *rg, rt_Object *parent, rt_OBJECT *obj)
{
    if (obj == RT_NULL)
    {
        throw rt_Exception("null-pointer in object");
    }

    this->rg = rg;

    this->obj = obj;
    /* save original transform data */
    this->otm = obj->trm;
    this->trm = &obj->trm;
    this->pos = this->mtx[3];
    this->tag = obj->obj.tag;

    /* reset matrix pointer
     * for/from the hierarchy */
    this->pmtx = RT_NULL;

    /* reset object's changed status
     * along with transform flags */
    this->obj_changed = 0;
    this->obj_has_trm = 0;
    this->mtx_has_trm = 0;

    /* set object's immediate parent
     * reset trnode and bvnode pointers */
    this->parent = parent;
    this->trnode = RT_NULL;
    this->bvnode = RT_NULL;

    /* init bvbox used in arrays for outer part of split bvnode if present,
     * used as generic boundary in other objects */
    bvbox = (rt_BOUND *)rg->alloc(RT_IS_SURFACE(this) ?
                        sizeof(rt_SHAPE) : sizeof(rt_BOUND), RT_QUAD_ALIGN);

    memset(bvbox, 0, sizeof(rt_BOUND));
    bvbox->obj = this;
    bvbox->tag = this->tag;
    bvbox->pinv = &this->inv;
    bvbox->pmtx = &this->mtx;
    bvbox->pos = this->mtx[3];
    bvbox->map = this->map;
    bvbox->sgn = this->sgn;
    bvbox->opts = &rg->opts;

    obj->time = -1;
}

/*
 * Build relations template based on given template "lst" from scene data.
 */
rt_void rt_Object::add_relation(rt_ELEM *lst)
{

}

/*
 * Update bvnode pointer with given "mode".
 */
rt_void rt_Object::update_bvnode(rt_Object *bvnode, rt_bool mode)
{
    /* bvnode cannot be its own bvnode,
     * there is no bvnode for boundless surfaces */
    if (bvnode == this || bvbox->verts_num == 0)
    {
        return;
    }

    /* enable bvnode */
    if (mode == RT_TRUE)
    {
        if (this->bvnode == RT_NULL)
        {
            this->bvnode = bvnode;
        }
        /* allow re-bounding objects to inner bvnodes */
        else
        {
            rt_Object *par;

            /* check if previously set bvnode is new bvnode's parent */
            for (par = bvnode->parent; par != RT_NULL; par = par->parent)
            {
                if (this->bvnode == par)
                {
                    this->bvnode = bvnode;
                    break;
                }
            }
        }
    }
    else
    /* disable bvnode */
    if (mode == RT_FALSE)
    {
        if (this->bvnode == bvnode)
        {
            this->bvnode = RT_NULL;
        }
    }
}

/*
 * Update object's status with given "time", "flags" and "trnode".
 */
rt_void rt_Object::update_status(rt_time time, rt_si32 flags,
                                 rt_Object *trnode)
{
    /* animator is called only once for object
     * instances sharing the same scene data,
     * part of sequential update (phase 0.5)
     * as the code below is not thread-safe */
    if (obj->f_anim != RT_NULL && obj->time != time)
    {
        obj->f_anim(time, obj->time < 0 ? 0 : obj->time, trm, RT_NULL);
    }

    /* always update time in scene data to distinguish
     * between first update and all subsequent updates,
     * even if animator is not present */
    obj->time = time;

    /* inherit changed status from the hierarchy */
    obj_changed = (flags & RT_UPDATE_FLAG_OBJ);

    /* update changed status for all object
     * instances sharing the same scene data,
     * even though animator is called only once */
    if (obj->f_anim != RT_NULL)
    {
        obj_changed |= RT_UPDATE_FLAG_OBJ;
    }

    if (obj_changed == 0)
    {
        return;
    }

    /* inherit transform flags from the hierarchy */
    obj_has_trm = (flags & RT_UPDATE_FLAG_SCL) |
                  (flags & RT_UPDATE_FLAG_ROT);

    /* set object's trnode from the hierarchy
     * (node up in the hierarchy with non-trivial transform,
     * relative to which object has trivial transform) */
    this->trnode = trnode;
}

/*
 * Update object's matrix with given "mtx".
 */
rt_void rt_Object::update_matrix(rt_mat4 mtx)
{
    if (obj_changed == 0)
    {
        return;
    }

    /* determine object's own transform for transform caching,
     * which allows to apply single matrix transform
     * in rendering backend to array of objects
     * with trivial transform relative to array node */
    rt_si32 i, c;

    /* reset object's own transform flags */
    mtx_has_trm = 0;

    /* determine if object itself has
     * non-trivial scaling */
    rt_real fsc[] = {-1.0f, +1.0f};

    for (i = 0, c = 0; i < RT_ARR_SIZE(fsc); i++)
    {
        if (trm->scl[RT_X] == fsc[i]) c++;
        if (trm->scl[RT_Y] == fsc[i]) c++;
        if (trm->scl[RT_Z] == fsc[i]) c++;
    }

    mtx_has_trm |= (c == 3) ? 0 : RT_UPDATE_FLAG_SCL;

    /* determine if object itself has
     * non-trivial rotation */
    rt_real frt[] = {-270.0f, -180.0f, -90.0f, 0.0f, +90.0f, +180.0f, +270.0f};

    for (i = 0, c = 0; i < RT_ARR_SIZE(frt); i++)
    {
        if (trm->rot[RT_X] == frt[i]) c++;
        if (trm->rot[RT_Y] == frt[i]) c++;
        if (trm->rot[RT_Z] == frt[i]) c++;
    }

    mtx_has_trm |= (c == 3) ? 0 : RT_UPDATE_FLAG_ROT;

    /* check if object's own matrix doesn't have rotation */
    if ((mtx_has_trm & RT_UPDATE_FLAG_ROT) == 0)
    {
        rt_mat4 trm_mtx;
        matrix_from_transform(trm_mtx, trm, RT_TRUE);
        matrix_mul_matrix(this->mtx, mtx, trm_mtx);

        if (obj_has_trm == RT_UPDATE_FLAG_SCL)
        {
            mtx_has_trm = obj_has_trm;
            obj_has_trm = 0;
        }

        rt_si32 i, j;

        /* determine axis mapping for trivial transform
         * (multiple of 90 degree rotation, scalers),
         * applicable to objects without trnode or with trnode
         * other than the object itself (transform caching),
         * scalers before rotation do not qualify for trnode
         * as solvers handle them without transform matrix */
        for (i = 0; i < 3; i++)
        {
            for (j = 0; j < 3; j++)
            {
                if ((this->mtx[i][0] != 0.0f) == (iden4[j][0] != 0.0f)
                &&  (this->mtx[i][1] != 0.0f) == (iden4[j][1] != 0.0f)
                &&  (this->mtx[i][2] != 0.0f) == (iden4[j][2] != 0.0f))
                {
                    map[i] = j;
                    sgn[i] = RT_SIGN(this->mtx[i][j]);
                    scl[j] = RT_FABS(this->mtx[i][j]);
                }
            }
        }

        map[RT_L] = RT_W;
        sgn[RT_L] = 1;
        scl[RT_W] = 1.0f;
    }

    /* check if object's own matrix has non-trivial rotation */
    if ((mtx_has_trm & RT_UPDATE_FLAG_ROT) != 0
    &&  trnode == RT_NULL)
    {
        rt_mat4 trm_mtx;
        matrix_from_transform(trm_mtx, trm, RT_FALSE);
        matrix_mul_matrix(this->mtx, mtx, trm_mtx);
    }
    if ((mtx_has_trm & RT_UPDATE_FLAG_ROT) != 0
    &&  trnode != RT_NULL)
    {
        rt_mat4 trm_mtx, tmp_mtx;
        matrix_from_transform(trm_mtx, trm, RT_FALSE);
        matrix_mul_matrix(tmp_mtx, trnode->mtx, mtx);
        matrix_mul_matrix(this->mtx, tmp_mtx, trm_mtx);
    }
    if ((mtx_has_trm & RT_UPDATE_FLAG_ROT) != 0)
    {
        trnode = this;
        obj_has_trm |= RT_UPDATE_FLAG_ROT;

        /* axis mapping for trivial transform */
        map[RT_I] = RT_X;
        map[RT_J] = RT_Y;
        map[RT_K] = RT_Z;
        map[RT_L] = RT_W;

        sgn[RT_I] = 1;
        sgn[RT_J] = 1;
        sgn[RT_K] = 1;
        sgn[RT_L] = 1;

        scl[RT_X] = trm->scl[RT_X];
        scl[RT_Y] = trm->scl[RT_Y];
        scl[RT_Z] = trm->scl[RT_Z];
        scl[RT_W] = 1.0f;
    }

    if ((obj_has_trm & RT_UPDATE_FLAG_ROT) != 0
#if RT_OPTS_FSCALE != 0
    && (rg->opts & RT_OPTS_FSCALE) == 0
#endif /* RT_OPTS_FSCALE */
       )
    {
        obj_has_trm |= RT_UPDATE_FLAG_SCL;
    }

    if (trnode != RT_NULL && trnode != this
#if RT_OPTS_TARRAY != 0
    && ((rg->opts & RT_OPTS_TARRAY) == 0 || tag > RT_TAG_SURFACE_MAX)
#endif /* RT_OPTS_TARRAY */
       )
    {
        rt_mat4 tmp_mtx;
        matrix_mul_matrix(tmp_mtx, trnode->mtx, this->mtx);
        memcpy(this->mtx, tmp_mtx, sizeof(rt_mat4));

        trnode = this;
        obj_has_trm |= mtx_has_trm;

        /* axis mapping for trivial transform */
        map[RT_I] = RT_X;
        map[RT_J] = RT_Y;
        map[RT_K] = RT_Z;
        map[RT_L] = RT_W;

        sgn[RT_I] = 1;
        sgn[RT_J] = 1;
        sgn[RT_K] = 1;
        sgn[RT_L] = 1;

        scl[RT_X] = 1.0f;
        scl[RT_Y] = 1.0f;
        scl[RT_Z] = 1.0f;
        scl[RT_W] = 1.0f;
    }

    /* set bvbox's trnode for rtgeom */
    bvbox->trnode = trnode != RT_NULL ? trnode->bvbox : RT_NULL;

    /* axis mapping shorteners */
    mp_i = map[RT_I];
    mp_j = map[RT_J];
    mp_k = map[RT_K];
    mp_l = map[RT_L];
}

/*
 * Update object with given "time", "flags", "trnode" and matrix "mtx".
 */
rt_void rt_Object::update_object(rt_time time, rt_si32 flags,
                                 rt_Object *trnode, rt_mat4 mtx)
{
    update_status(time, flags, trnode);

    update_matrix(mtx);
}

/*
 * Update SIMD and other data fields.
 */
rt_void rt_Object::update_fields()
{

}

/*
 * Deinitialize object.
 */
rt_Object::~rt_Object()
{
    /* restore original transform data */
    obj->trm = otm;
}

/******************************************************************************/
/*********************************   CAMERA   *********************************/
/******************************************************************************/

/*
 * Instantiate camera object.
 */
rt_Camera::rt_Camera(rt_Registry *rg, rt_Object *parent, rt_OBJECT *obj) :

    rt_Object(rg, parent, obj),
    rt_List<rt_Camera>(rg->get_cam())
{
    rg->put_cam(this);

    this->cam = (rt_CAMERA *)obj->obj.pobj;

    if (cam->col.val != 0x0)
    {
        cam->col.hdr[RT_R] = ((cam->col.val >> 0x10) & 0xFF) / 255.0f;
        cam->col.hdr[RT_G] = ((cam->col.val >> 0x08) & 0xFF) / 255.0f;
        cam->col.hdr[RT_B] = ((cam->col.val >> 0x00) & 0xFF) / 255.0f;
    }

    hor = this->mtx[0];
    ver = this->mtx[1];
    nrm = this->mtx[2];

    pov = cam->vpt[0] <= 0.0f ? 1.0f : /* default pov */
          cam->vpt[0] <= 2 * RT_CLIP_THRESHOLD ? /* minimum positive pov */
                         2 * RT_CLIP_THRESHOLD : cam->vpt[0];

    /* reset camera's changed status */
    cam_changed = 0;
}

/*
 * Update object with given "time", "flags", "trnode" and matrix "mtx".
 */
rt_void rt_Camera::update_object(rt_time time, rt_si32 flags,
                                 rt_Object *trnode, rt_mat4 mtx)
{
    update_status(time, flags | cam_changed, trnode);

    /* set matrix pointer
     * from immediate parent array */
    pmtx = (rt_mat4 *)mtx;
}

/*
 * Update SIMD and other data fields.
 */
rt_void rt_Camera::update_fields()
{
    if (obj_changed == 0)
    {
        return;
    }

    /* pass matrix pointer
     * from immediate parent array */
    update_matrix(*pmtx);

    RT_VEC3_SET(bvbox->mid, pos);

    rt_Object::update_fields();

    hor_sin = RT_SINA(trm->rot[RT_Z]);
    hor_cos = RT_COSA(trm->rot[RT_Z]);

    /* reset camera's changed status */
    cam_changed = 0;
}

/*
 * Update camera with given "time" and "action".
 */
rt_void rt_Camera::update_action(rt_time time, rt_si32 action)
{
    rt_real t = (time - obj->time) / 50.0f;

    switch (action)
    {
        /* vertical movement */
        case RT_CAMERA_MOVE_UP:
        trm->pos[RT_Z] += cam->dps[RT_K] * t;
        break;

        case RT_CAMERA_MOVE_DOWN:
        trm->pos[RT_Z] -= cam->dps[RT_K] * t;
        break;

        /* horizontal movement */
        case RT_CAMERA_MOVE_LEFT:
        trm->pos[RT_X] -= cam->dps[RT_I] * t * hor_cos;
        trm->pos[RT_Y] -= cam->dps[RT_I] * t * hor_sin;
        break;

        case RT_CAMERA_MOVE_RIGHT:
        trm->pos[RT_X] += cam->dps[RT_I] * t * hor_cos;
        trm->pos[RT_Y] += cam->dps[RT_I] * t * hor_sin;
        break;

        case RT_CAMERA_MOVE_BACK:
        trm->pos[RT_X] += cam->dps[RT_J] * t * hor_sin;
        trm->pos[RT_Y] -= cam->dps[RT_J] * t * hor_cos;
        break;

        case RT_CAMERA_MOVE_FORWARD:
        trm->pos[RT_X] -= cam->dps[RT_J] * t * hor_sin;
        trm->pos[RT_Y] += cam->dps[RT_J] * t * hor_cos;
        break;

        /* horizontal rotation */
        case RT_CAMERA_ROTATE_LEFT:
        trm->rot[RT_Z] += cam->drt[RT_I] * t;
        if (trm->rot[RT_Z] >= +180.0f)
        {
            trm->rot[RT_Z] -= +360.0f;
        }
        break;

        case RT_CAMERA_ROTATE_RIGHT:
        trm->rot[RT_Z] -= cam->drt[RT_I] * t;
        if (trm->rot[RT_Z] <= -180.0f)
        {
            trm->rot[RT_Z] += +360.0f;
        }
        break;

        /* vertical rotation */
        case RT_CAMERA_ROTATE_UP:
        if (trm->rot[RT_X] <  0.0f)
        {
            trm->rot[RT_X] += cam->drt[RT_J] * t;
            if (trm->rot[RT_X] >  0.0f)
                trm->rot[RT_X] =  0.0f;
        }
        break;

        case RT_CAMERA_ROTATE_DOWN:
        if (trm->rot[RT_X] > -180.0f)
        {
            trm->rot[RT_X] -= cam->drt[RT_J] * t;
            if (trm->rot[RT_X] < -180.0f)
                trm->rot[RT_X] = -180.0f;
        }
        break;

        default:
        break;
    }

    /* set camera's changed status */
    cam_changed = RT_UPDATE_FLAG_OBJ;
}

/*
 * Deinitialize camera object.
 */
rt_Camera::~rt_Camera()
{

}

/******************************************************************************/
/**********************************   LIGHT   *********************************/
/******************************************************************************/

/*
 * Instantiate light object.
 */
rt_Light::rt_Light(rt_Registry *rg, rt_Object *parent, rt_OBJECT *obj) :

    rt_Object(rg, parent, obj),
    rt_List<rt_Light>(rg->get_lgt())
{
    rg->put_lgt(this);

    this->lgt = (rt_LIGHT *)obj->obj.pobj;

    if (lgt->col.val != 0x0)
    {
        lgt->col.hdr[RT_R] = ((lgt->col.val >> 0x10) & 0xFF) / 255.0f;
        lgt->col.hdr[RT_G] = ((lgt->col.val >> 0x08) & 0xFF) / 255.0f;
        lgt->col.hdr[RT_B] = ((lgt->col.val >> 0x00) & 0xFF) / 255.0f;
    }

/*  rt_SIMD_LIGHT */

    s_lgt = (rt_SIMD_LIGHT *)rg->alloc(sizeof(rt_SIMD_LIGHT), RT_SIMD_ALIGN);

    RT_SIMD_SET(s_lgt->t_max, 1.0f);

    RT_SIMD_SET(s_lgt->col_r, lgt->col.hdr[RT_R] * lgt->lum[1]);
    RT_SIMD_SET(s_lgt->col_g, lgt->col.hdr[RT_G] * lgt->lum[1]);
    RT_SIMD_SET(s_lgt->col_b, lgt->col.hdr[RT_B] * lgt->lum[1]);
    RT_SIMD_SET(s_lgt->l_src, lgt->lum[1]);

    RT_SIMD_SET(s_lgt->a_qdr, lgt->atn[3]);
    RT_SIMD_SET(s_lgt->a_lnr, lgt->atn[2]);
    RT_SIMD_SET(s_lgt->a_cnt, lgt->atn[1] + 1.0f);
    RT_SIMD_SET(s_lgt->a_rng, lgt->atn[0]);

    ((rt_Array *)parent)->col.hdr[RT_R] += s_lgt->col_r[0];
    ((rt_Array *)parent)->col.hdr[RT_G] += s_lgt->col_g[0];
    ((rt_Array *)parent)->col.hdr[RT_B] += s_lgt->col_b[0];

    ((rt_Array *)parent)->col.hdr[RT_R] += lgt->col.hdr[RT_R] * lgt->lum[0];
    ((rt_Array *)parent)->col.hdr[RT_G] += lgt->col.hdr[RT_G] * lgt->lum[0];
    ((rt_Array *)parent)->col.hdr[RT_B] += lgt->col.hdr[RT_B] * lgt->lum[0];

    ((rt_Array *)parent)->col.hdr[RT_A] += lgt->lum[0] + lgt->lum[1];
}

/*
 * Update object with given "time", "flags", "trnode" and matrix "mtx".
 */
rt_void rt_Light::update_object(rt_time time, rt_si32 flags,
                                rt_Object *trnode, rt_mat4 mtx)
{
    update_status(time, flags, trnode);

    /* set matrix pointer
     * from immediate parent array */
    pmtx = (rt_mat4 *)mtx;
}

/*
 * Update SIMD and other data fields.
 */
rt_void rt_Light::update_fields()
{
    if (obj_changed == 0)
    {
        return;
    }

    /* pass matrix pointer
     * from immediate parent array */
    update_matrix(*pmtx);

    RT_VEC3_SET(bvbox->mid, pos);

    rt_Object::update_fields();

    RT_SIMD_SET(s_lgt->pos_x, pos[RT_X]);
    RT_SIMD_SET(s_lgt->pos_y, pos[RT_Y]);
    RT_SIMD_SET(s_lgt->pos_z, pos[RT_Z]);
}

/*
 * Deinitialize light object.
 */
rt_Light::~rt_Light()
{

}

/******************************************************************************/
/**********************************   NODE   **********************************/
/******************************************************************************/

static
rt_EDGE bx_edges[] = 
{
    {0x0, 0x1},
    {0x1, 0x2},
    {0x2, 0x3},
    {0x3, 0x0},
    {0x0, 0x4},
    {0x1, 0x5},
    {0x2, 0x6},
    {0x3, 0x7},
    {0x7, 0x6},
    {0x6, 0x5},
    {0x5, 0x4},
    {0x4, 0x7},
};

static
rt_FACE bx_faces[] = 
{
    {0x0, 0x1, 0x2, 0x3},
    {0x0, 0x4, 0x5, 0x1},
    {0x1, 0x5, 0x6, 0x2},
    {0x2, 0x6, 0x7, 0x3},
    {0x3, 0x7, 0x4, 0x0},
    {0x7, 0x6, 0x5, 0x4},
};

/*
 * Instantiate node object.
 */
rt_Node::rt_Node(rt_Registry *rg, rt_Object *parent,
                 rt_OBJECT *obj, rt_si32 ssize) :

    rt_Object(rg, parent, obj)
{
    /* reset relations template */
    rel = RT_NULL;

    /* validate surface size */
    ssize = RT_MAX(ssize, sizeof(rt_SIMD_SURFACE));

/*  rt_SIMD_SURFACE */

    s_srf = (rt_SIMD_SURFACE *)rg->alloc(ssize, RT_SIMD_ALIGN);
    memset(s_srf, 0, ssize);
    s_srf->srf_t[3] = tag;

    /* allocate SIMD-buffers (with actual number of threads) */
    if ((rg->opts & RT_OPTS_BUFFERS) == 0)
    {
        s_srf->msc_p[0] = rg->alloc(RT_BUFFER_POOL*rg->thr_num, RT_SIMD_ALIGN);
        memset(s_srf->msc_p[0], 255, RT_BUFFER_POOL*rg->thr_num);
    }

#if 0 /* surface's misc pointers description */

    s_srf->srf_t[0];    /* surf ptr, filled in update0 */
    s_srf->srf_t[1];    /* norm ptr, filled in update0 */
    s_srf->srf_t[2];    /* clip ptr, filled in update0 */
    s_srf->srf_t[3];    /* surf tag */

    s_srf->msc_p[0];    /* SIMD-buffers */
    s_srf->msc_p[1];    /* surf flg, filled in update0 */
    s_srf->msc_p[2];    /* custom clippers */
    s_srf->msc_p[3];    /* trnode's simd ptr */

    s_srf->mat_p[0];    /* outer material */
    s_srf->mat_p[1];    /* outer material props */
    s_srf->mat_p[2];    /* inner material */
    s_srf->mat_p[3];    /* inner material props */

    s_srf->lst_p[0];    /* outer lights/shadows */
    s_srf->lst_p[1];    /* outer surfaces for rfl/rfr */
    s_srf->lst_p[2];    /* inner lights/shadows */
    s_srf->lst_p[3];    /* inner surfaces for rfl/rfr */

#endif /* surface's misc pointers description */

    RT_SIMD_SET(s_srf->sbase, 0);
    RT_SIMD_SET(s_srf->smask, (rt_uelm)0x80000000 << (RT_ELEMENT - 32));
    RT_SIMD_SET(s_srf->c_def, -1);

    RT_SIMD_SET(s_srf->srf_p, (rt_uelm)((rt_ui64)(rt_uptr)s_srf & 0xFFFFFFFF));
    RT_SIMD_SET(s_srf->srf_h, (rt_uelm)((rt_ui64)(rt_uptr)s_srf >> 32));

    RT_SIMD_SET(s_srf->srf_o, 0); /* relies on RT_FLAG_SIDE_OUTER definition */
    RT_SIMD_SET(s_srf->srf_i, 1); /* relies on RT_FLAG_SIDE_INNER definition */

    RT_SIMD_SET(s_srf->d_eps, RT_DEPS_THRESHOLD);
    RT_SIMD_SET(s_srf->t_eps, RT_TEPS_THRESHOLD);
}

/*
 * Build relations template based on given template "lst" from scene data.
 */
rt_void rt_Node::add_relation(rt_ELEM *lst)
{
    rt_Object::add_relation(lst);
}

/*
 * Update bvnode pointer with given "mode".
 */
rt_void rt_Node::update_bvnode(rt_Object *bvnode, rt_bool mode)
{
    rt_Object::update_bvnode(bvnode, mode);
}

/*
 * Update object's status with given "time", "flags" and "trnode".
 */
rt_void rt_Node::update_status(rt_time time, rt_si32 flags,
                               rt_Object *trnode)
{
    rt_Object::update_status(time, flags, trnode);
}

/*
 * Update object's matrix with given "mtx".
 */
rt_void rt_Node::update_matrix(rt_mat4 mtx)
{
    if (obj_changed == 0)
    {
        return;
    }

    rt_Object::update_matrix(mtx);
}

/*
 * Update object with given "time", "flags", "trnode" and matrix "mtx".
 */
rt_void rt_Node::update_object(rt_time time, rt_si32 flags,
                               rt_Object *trnode, rt_mat4 mtx)
{
    update_status(time, flags, trnode);

    update_matrix(mtx);
}

/*
 * Update SIMD and other data fields.
 */
rt_void rt_Node::update_fields()
{
    if (obj_changed == 0)
    {
        return;
    }

    rt_Object::update_fields();

    /* compute object's inverted transform matrix and store
     * its values into backend fields along with current position */
    if (trnode == this)
    {
        matrix_inverse(inv, this->mtx);

        RT_SIMD_SET(s_srf->tci_x, inv[RT_X][RT_I]);
        RT_SIMD_SET(s_srf->tci_y, inv[RT_Y][RT_I]);
        RT_SIMD_SET(s_srf->tci_z, inv[RT_Z][RT_I]);

        RT_SIMD_SET(s_srf->tcj_x, inv[RT_X][RT_J]);
        RT_SIMD_SET(s_srf->tcj_y, inv[RT_Y][RT_J]);
        RT_SIMD_SET(s_srf->tcj_z, inv[RT_Z][RT_J]);

        RT_SIMD_SET(s_srf->tck_x, inv[RT_X][RT_K]);
        RT_SIMD_SET(s_srf->tck_y, inv[RT_Y][RT_K]);
        RT_SIMD_SET(s_srf->tck_z, inv[RT_Z][RT_K]);
    }

    RT_SIMD_SET(s_srf->pos_x, pos[RT_X]);
    RT_SIMD_SET(s_srf->pos_y, pos[RT_Y]);
    RT_SIMD_SET(s_srf->pos_z, pos[RT_Z]);
}

/*
 * Update bounding box and volume geometry.
 */
rt_void rt_Node::update_bbgeom(rt_BOUND *box)
{
    /* check bbox geometry limits */
    if (box->verts_num > RT_VERTS_LIMIT
    ||  box->edges_num > RT_EDGES_LIMIT
    ||  box->faces_num > RT_FACES_LIMIT)
    {
        throw rt_Exception("bbox geometry limits exceeded");
    }

    /* check bbox ownership */
    if (box->obj != this)
    {
        throw rt_Exception("incorrect box in update_bbgeom");
    }

    do /* use "do {break} while(0)" instead of "goto label" */
    {
        /* bvbox is always in world space,
         * thus skip transform (in else branch) even if trnode is present */
        if (trnode != RT_NULL && !(RT_IS_ARRAY(this)
        && ((rt_Array *)this)->bvbox == box))
        {
            rt_mat4 *pmtx = &trnode->mtx;

            rt_vec4 vt0;
            vt0[mp_i] = box->bmax[mp_i];
            vt0[mp_j] = box->bmax[mp_j];
            vt0[mp_k] = box->bmax[mp_k];
            vt0[mp_l] = 1.0f; /* takes "pos" in "mtx" into account */

            rt_vec4 vt1;
            vt1[mp_i] = box->bmin[mp_i];
            vt1[mp_j] = box->bmax[mp_j];
            vt1[mp_k] = box->bmax[mp_k];
            vt1[mp_l] = 1.0f; /* takes "pos" in "mtx" into account */

            rt_vec4 vt2;
            vt2[mp_i] = box->bmin[mp_i];
            vt2[mp_j] = box->bmin[mp_j];
            vt2[mp_k] = box->bmax[mp_k];
            vt2[mp_l] = 1.0f; /* takes "pos" in "mtx" into account */

            rt_vec4 vt3;
            vt3[mp_i] = box->bmax[mp_i];
            vt3[mp_j] = box->bmin[mp_j];
            vt3[mp_k] = box->bmax[mp_k];
            vt3[mp_l] = 1.0f; /* takes "pos" in "mtx" into account */

            matrix_mul_vector(box->verts[0x0].pos, *pmtx, vt0);
            matrix_mul_vector(box->verts[0x1].pos, *pmtx, vt1);
            matrix_mul_vector(box->verts[0x2].pos, *pmtx, vt2);
            matrix_mul_vector(box->verts[0x3].pos, *pmtx, vt3);

            box->edges[0x0].k = 3;
            box->edges[0x1].k = 3;
            box->edges[0x2].k = 3;
            box->edges[0x3].k = 3;

            box->faces[0x0].k = 3;
            box->faces[0x0].i = 3;
            box->faces[0x0].j = 3;

            if (RT_IS_PLANE(this))
            {
                break;
            }

            rt_vec4 vt4;
            vt4[mp_i] = box->bmax[mp_i];
            vt4[mp_j] = box->bmax[mp_j];
            vt4[mp_k] = box->bmin[mp_k];
            vt4[mp_l] = 1.0f; /* takes "pos" in "mtx" into account */

            rt_vec4 vt5;
            vt5[mp_i] = box->bmin[mp_i];
            vt5[mp_j] = box->bmax[mp_j];
            vt5[mp_k] = box->bmin[mp_k];
            vt5[mp_l] = 1.0f; /* takes "pos" in "mtx" into account */

            rt_vec4 vt6;
            vt6[mp_i] = box->bmin[mp_i];
            vt6[mp_j] = box->bmin[mp_j];
            vt6[mp_k] = box->bmin[mp_k];
            vt6[mp_l] = 1.0f; /* takes "pos" in "mtx" into account */

            rt_vec4 vt7;
            vt7[mp_i] = box->bmax[mp_i];
            vt7[mp_j] = box->bmin[mp_j];
            vt7[mp_k] = box->bmin[mp_k];
            vt7[mp_l] = 1.0f; /* takes "pos" in "mtx" into account */

            matrix_mul_vector(box->verts[0x4].pos, *pmtx, vt4);
            matrix_mul_vector(box->verts[0x5].pos, *pmtx, vt5);
            matrix_mul_vector(box->verts[0x6].pos, *pmtx, vt6);
            matrix_mul_vector(box->verts[0x7].pos, *pmtx, vt7);

            box->edges[0x4].k = 3;
            box->edges[0x5].k = 3;
            box->edges[0x6].k = 3;
            box->edges[0x7].k = 3;

            box->edges[0x8].k = 3;
            box->edges[0x9].k = 3;
            box->edges[0xA].k = 3;
            box->edges[0xB].k = 3;

            box->faces[0x1].k = 3;
            box->faces[0x1].i = 3;
            box->faces[0x1].j = 3;

            box->faces[0x2].k = 3;
            box->faces[0x2].i = 3;
            box->faces[0x2].j = 3;

            box->faces[0x3].k = 3;
            box->faces[0x3].i = 3;
            box->faces[0x3].j = 3;

            box->faces[0x4].k = 3;
            box->faces[0x4].i = 3;
            box->faces[0x4].j = 3;

            box->faces[0x5].k = 3;
            box->faces[0x5].i = 3;
            box->faces[0x5].j = 3;
        }
        else
        {
            box->verts[0x0].pos[mp_i] = box->bmax[mp_i];
            box->verts[0x0].pos[mp_j] = box->bmax[mp_j];
            box->verts[0x0].pos[mp_k] = box->bmax[mp_k];
            box->verts[0x0].pos[mp_l] = 1.0f;

            box->verts[0x1].pos[mp_i] = box->bmin[mp_i];
            box->verts[0x1].pos[mp_j] = box->bmax[mp_j];
            box->verts[0x1].pos[mp_k] = box->bmax[mp_k];
            box->verts[0x1].pos[mp_l] = 1.0f;

            box->verts[0x2].pos[mp_i] = box->bmin[mp_i];
            box->verts[0x2].pos[mp_j] = box->bmin[mp_j];
            box->verts[0x2].pos[mp_k] = box->bmax[mp_k];
            box->verts[0x2].pos[mp_l] = 1.0f;

            box->verts[0x3].pos[mp_i] = box->bmax[mp_i];
            box->verts[0x3].pos[mp_j] = box->bmin[mp_j];
            box->verts[0x3].pos[mp_k] = box->bmax[mp_k];
            box->verts[0x3].pos[mp_l] = 1.0f;

            box->edges[0x0].k = mp_i;
            box->edges[0x1].k = mp_j;
            box->edges[0x2].k = mp_i;
            box->edges[0x3].k = mp_j;

            box->faces[0x0].k = mp_k;
            box->faces[0x0].i = mp_i;
            box->faces[0x0].j = mp_j;

            if (RT_IS_PLANE(this))
            {
                break;
            }

            box->verts[0x4].pos[mp_i] = box->bmax[mp_i];
            box->verts[0x4].pos[mp_j] = box->bmax[mp_j];
            box->verts[0x4].pos[mp_k] = box->bmin[mp_k];
            box->verts[0x4].pos[mp_l] = 1.0f;

            box->verts[0x5].pos[mp_i] = box->bmin[mp_i];
            box->verts[0x5].pos[mp_j] = box->bmax[mp_j];
            box->verts[0x5].pos[mp_k] = box->bmin[mp_k];
            box->verts[0x5].pos[mp_l] = 1.0f;

            box->verts[0x6].pos[mp_i] = box->bmin[mp_i];
            box->verts[0x6].pos[mp_j] = box->bmin[mp_j];
            box->verts[0x6].pos[mp_k] = box->bmin[mp_k];
            box->verts[0x6].pos[mp_l] = 1.0f;

            box->verts[0x7].pos[mp_i] = box->bmax[mp_i];
            box->verts[0x7].pos[mp_j] = box->bmin[mp_j];
            box->verts[0x7].pos[mp_k] = box->bmin[mp_k];
            box->verts[0x7].pos[mp_l] = 1.0f;

            box->edges[0x4].k = mp_k;
            box->edges[0x5].k = mp_k;
            box->edges[0x6].k = mp_k;
            box->edges[0x7].k = mp_k;

            box->edges[0x8].k = mp_i;
            box->edges[0x9].k = mp_j;
            box->edges[0xA].k = mp_i;
            box->edges[0xB].k = mp_j;

            box->faces[0x1].k = mp_j;
            box->faces[0x1].i = mp_k;
            box->faces[0x1].j = mp_i;

            box->faces[0x2].k = mp_i;
            box->faces[0x2].i = mp_k;
            box->faces[0x2].j = mp_j;

            box->faces[0x3].k = mp_j;
            box->faces[0x3].i = mp_k;
            box->faces[0x3].j = mp_i;

            box->faces[0x4].k = mp_i;
            box->faces[0x4].i = mp_k;
            box->faces[0x4].j = mp_j;

            box->faces[0x5].k = mp_k;
            box->faces[0x5].i = mp_i;
            box->faces[0x5].j = mp_j;
        }
    }
    while (0);

    RT_VEC3_SET_VAL1(box->mid, 0.0f);
    box->rad = 0.0f;

    /* this function isn't called
     * if "box->verts_num == 0" */
    rt_si32 i;
    rt_real f = 1.0f / (rt_real)box->verts_num;

    for (i = 0; i < box->verts_num; i++)
    {
        RT_VEC3_MAD_VAL1(box->mid, box->verts[i].pos, f);
    }

    for (i = 0; i < box->verts_num; i++)
    {
        rt_vec4 dff_vec;
        RT_VEC3_SUB(dff_vec, box->mid, box->verts[i].pos);
        rt_real dff_dot = RT_VEC3_DOT(dff_vec, dff_vec);

        if (box->rad < dff_dot)
        {
            box->rad = dff_dot;
        }
    }

    box->rad = RT_SQRT(box->rad);

#if RT_OPTS_REMOVE != 0
    if ((rg->opts & RT_OPTS_REMOVE) != 0)
    {
        if (RT_IS_PLANE(box) && *((rt_SHAPE *)box)->ptr == RT_NULL)
        {
            /* plane bbox's only face */
            box->fln = 1;

            /* plane bbox's only face in minmax data format:
             * (1 - min, 2 - max) << (axis_index * 2) */
            box->flm = 3 << (mp_k * 2);

            /* plane bbox's only face in face index format:
             * 1 << face_index (0th index in terms of bx_faces) */
            box->flf = 1 << 0;
        }
        else
        if (RT_IS_ARRAY(box) && box->flm != 0)
        {
            /* convert array bbox's flags
             * from minmax data format to face index format */
            box->flf = bbox_flag(box->map, box->flm);

            rt_si32 c = 0, i;

            for (i = 0; i < 6; i++)
            {
                c += (box->flf & (1 << i)) != 0;
            }

            /* number of array bbox's fully covered (by plane) faces */
            box->fln = c;
        }
    }
#endif /* RT_OPTS_REMOVE */
}

/*
 * Deinitialize node object.
 */
rt_Node::~rt_Node()
{

}

/******************************************************************************/
/**********************************   ARRAY   *********************************/
/******************************************************************************/

static
rt_MATERIAL mt_glass01_array01 =
{
    RT_MAT(LIGHT),

    RT_TEX(PCOLOR, 0xFFFF8F00),

    {/* dff     spc     pow */
        1.0,    0.0,    1.0
    },
    {/* rfl     trn     rfr */
        0.0,    0.9,    1.0
    },
};

static
rt_SIDE sd_array01 =
{
/* OUTER        RT_U,       RT_V    */
/* scl */   {    1.0,        1.0    },
/* rot */              0.0           ,
/* pos */   {    0.0,        0.0    },

/* mat */   &mt_glass01_array01,
};

/*
 * Instantiate array object.
 */
rt_Array::rt_Array(rt_Registry *rg, rt_Object *parent,
                   rt_OBJECT *obj, rt_si32 ssize) :

    rt_Node(rg, parent, obj, ssize),
    rt_List<rt_Array>(rg->get_arr())
{
    rg->put_arr(this);

    /* reset array's changed status */
    arr_changed = 0;
    scn_changed = 0;

    /* reset array's accumulated light */
    memset(&col, 0, sizeof(rt_COL));

    /* init bvbox used for outer part of split bvnode if present */
    if (RT_TRUE)
    {
        bvbox->verts_num = 8;
        bvbox->verts = (rt_VERT *)
                     rg->alloc(bvbox->verts_num * sizeof(rt_VERT), RT_ALIGN);

        bvbox->edges_num = RT_ARR_SIZE(bx_edges);
        bvbox->edges = (rt_EDGE *)
                     rg->alloc(bvbox->edges_num * sizeof(rt_EDGE), RT_ALIGN);
        memcpy(bvbox->edges, bx_edges, bvbox->edges_num * sizeof(rt_EDGE));

        bvbox->faces_num = RT_ARR_SIZE(bx_faces);
        bvbox->faces = (rt_FACE *)
                     rg->alloc(bvbox->faces_num * sizeof(rt_FACE), RT_ALIGN);
        memcpy(bvbox->faces, bx_faces, bvbox->faces_num * sizeof(rt_FACE));
    }

    /* init trbox used for trnode if present and has contents outside bvnode,
     * in which case bvnode is split */
    trbox = (rt_BOUND *)rg->alloc(sizeof(rt_BOUND), RT_QUAD_ALIGN);

    memset(trbox, 0, sizeof(rt_BOUND));
    trbox->obj = this;
    trbox->tag = this->tag;
    trbox->pinv = &this->inv;
    trbox->pmtx = &this->mtx;
    trbox->pos = this->mtx[3];
    trbox->map = this->map;
    trbox->sgn = this->sgn;
    trbox->opts = &rg->opts;

    if (RT_TRUE)
    {
        trbox->verts_num = 8;
        trbox->verts = (rt_VERT *)
                     rg->alloc(trbox->verts_num * sizeof(rt_VERT), RT_ALIGN);

        trbox->edges_num = RT_ARR_SIZE(bx_edges);
        trbox->edges = (rt_EDGE *)
                     rg->alloc(trbox->edges_num * sizeof(rt_EDGE), RT_ALIGN);
        memcpy(trbox->edges, bx_edges, trbox->edges_num * sizeof(rt_EDGE));

        trbox->faces_num = RT_ARR_SIZE(bx_faces);
        trbox->faces = (rt_FACE *)
                     rg->alloc(trbox->faces_num * sizeof(rt_FACE), RT_ALIGN);
        memcpy(trbox->faces, bx_faces, trbox->faces_num * sizeof(rt_FACE));
    }

    /* init inbox used for inner part of split bvnode if present,
     * or trnode if it doesn't have contents outside bvnode */
    inbox = (rt_BOUND *)rg->alloc(sizeof(rt_BOUND), RT_QUAD_ALIGN);

    memset(inbox, 0, sizeof(rt_BOUND));
    inbox->obj = this;
    inbox->tag = this->tag;
    inbox->pinv = &this->inv;
    inbox->pmtx = &this->mtx;
    inbox->pos = this->mtx[3];
    inbox->map = this->map;
    inbox->sgn = this->sgn;
    inbox->opts = &rg->opts;

    if (RT_TRUE)
    {
        inbox->verts_num = 8;
        inbox->verts = (rt_VERT *)
                     rg->alloc(inbox->verts_num * sizeof(rt_VERT), RT_ALIGN);

        inbox->edges_num = RT_ARR_SIZE(bx_edges);
        inbox->edges = (rt_EDGE *)
                     rg->alloc(inbox->edges_num * sizeof(rt_EDGE), RT_ALIGN);
        memcpy(inbox->edges, bx_edges, inbox->edges_num * sizeof(rt_EDGE));

        inbox->faces_num = RT_ARR_SIZE(bx_faces);
        inbox->faces = (rt_FACE *)
                     rg->alloc(inbox->faces_num * sizeof(rt_FACE), RT_ALIGN);
        memcpy(inbox->faces, bx_faces, inbox->faces_num * sizeof(rt_FACE));
    }

    /* process array's objects */
    rt_OBJECT *arr = (rt_OBJECT *)obj->obj.pobj;

    obj_num = obj->obj.obj_num;
    obj_arr = (rt_Object **)rg->alloc(obj_num * sizeof(rt_Object *), RT_ALIGN);

    rt_si32 i, j; /* j - for skipping unsupported object tags */

    /* instantiate every object in array from scene data,
     * including sub-arrays (recursive) */
    for (i = 0, j = 0; i < obj->obj.obj_num; i++, j++)
    {
        switch (arr[i].obj.tag)
        {
            case RT_TAG_CAMERA:
            obj_arr[j] = new(rg) rt_Camera(rg, this, &arr[i]);
            break;

            case RT_TAG_LIGHT:
            obj_arr[j] = new(rg) rt_Light(rg, this, &arr[i]);
            break;

            case RT_TAG_ARRAY:
            obj_arr[j] = new(rg) rt_Array(rg, this, &arr[i]);
            break;

            case RT_TAG_PLANE:
            obj_arr[j] = new(rg) rt_Plane(rg, this, &arr[i]);
            break;

            case RT_TAG_CYLINDER:
            obj_arr[j] = new(rg) rt_Cylinder(rg, this, &arr[i]);
            break;

            case RT_TAG_SPHERE:
            obj_arr[j] = new(rg) rt_Sphere(rg, this, &arr[i]);
            break;

            case RT_TAG_CONE:
            obj_arr[j] = new(rg) rt_Cone(rg, this, &arr[i]);
            break;

            case RT_TAG_PARABOLOID:
            obj_arr[j] = new(rg) rt_Paraboloid(rg, this, &arr[i]);
            break;

            case RT_TAG_HYPERBOLOID:
            obj_arr[j] = new(rg) rt_Hyperboloid(rg, this, &arr[i]);
            break;

            case RT_TAG_PARACYLINDER:
            obj_arr[j] = new(rg) rt_ParaCylinder(rg, this, &arr[i]);
            break;

            case RT_TAG_HYPERCYLINDER:
            obj_arr[j] = new(rg) rt_HyperCylinder(rg, this, &arr[i]);
            break;

            case RT_TAG_HYPERPARABOLOID:
            obj_arr[j] = new(rg) rt_HyperParaboloid(rg, this, &arr[i]);
            break;

            default:
            j--;
            obj_num--;
            break;
        }
    }

    /* assign accumulated light to emitting surfaces */
    for (i = 0; i < obj_num; i++)
    {
        rt_SIMD_MATERIAL *s_mat = RT_NULL;

        if (RT_IS_SURFACE(obj_arr[i]))
        {
            s_mat = ((rt_Surface *)obj_arr[i])->outer->s_mat;

            if (((rt_Surface *)obj_arr[i])->outer->props & RT_PROP_LIGHT)
            {
                RT_SIMD_SET(s_mat->col_r, col.hdr[RT_R] * 100.0f);
                RT_SIMD_SET(s_mat->col_g, col.hdr[RT_G] * 100.0f);
                RT_SIMD_SET(s_mat->col_b, col.hdr[RT_B] * 100.0f);
                RT_SIMD_SET(s_mat->e_src, col.hdr[RT_A] * 100.0f);
            }
            else
            {
                RT_SIMD_SET(s_mat->col_r, 0.0f);
                RT_SIMD_SET(s_mat->col_g, 0.0f);
                RT_SIMD_SET(s_mat->col_b, 0.0f);
                RT_SIMD_SET(s_mat->e_src, 0.0f);
            }

            s_mat = ((rt_Surface *)obj_arr[i])->inner->s_mat;

            if (((rt_Surface *)obj_arr[i])->inner->props & RT_PROP_LIGHT)
            {
                RT_SIMD_SET(s_mat->col_r, col.hdr[RT_R] * 100.0f);
                RT_SIMD_SET(s_mat->col_g, col.hdr[RT_G] * 100.0f);
                RT_SIMD_SET(s_mat->col_b, col.hdr[RT_B] * 100.0f);
                RT_SIMD_SET(s_mat->e_src, col.hdr[RT_A] * 100.0f);
            }
            else
            {
                RT_SIMD_SET(s_mat->col_r, 0.0f);
                RT_SIMD_SET(s_mat->col_g, 0.0f);
                RT_SIMD_SET(s_mat->col_b, 0.0f);
                RT_SIMD_SET(s_mat->e_src, 0.0f);
            }
        }
    }

    /* process array's relations */
    rt_RELATION *rel = obj->obj.prel;

    /* maintain reusable relations template list linked via "simd"
     * used via "ptr", so that list elements are not reallocated */
    rt_ELEM *lst = RT_NULL, *prv = RT_NULL, **ptr = &rg->rel;
    rt_si32 acc  = 0;

    rt_Object **obj_arr_l = obj_arr; /* left  sub-array */
    rt_Object **obj_arr_r = obj_arr; /* right sub-array */

    rt_si32     obj_num_l = obj_num; /* left  sub-array size */
    rt_si32     obj_num_r = obj_num; /* right sub-array size */

    /* build relations templates (custom clippers) from scene data */
    for (i = 0; i < obj->obj.rel_num; i++)
    {
        if (rel[i].obj1 >= obj_num_l
        ||  rel[i].obj2 >= obj_num_r)
        {
            continue;
        }

        rt_ELEM *elm = RT_NULL;

        rt_Object *obj = RT_NULL;
        rt_Array *arr = RT_NULL;
        rt_bool mode = RT_FALSE;

        switch (rel[i].rel)
        {
            case RT_REL_INDEX_ARRAY:
            if (rel[i].obj1 >= 0 && rel[i].obj2 >= -1
            &&  RT_IS_ARRAY(obj_arr_l[rel[i].obj1]))
            {
                rt_Array *arr = (rt_Array *)obj_arr_l[rel[i].obj1];
                obj_arr_l = arr->obj_arr; /* select left  sub-array */
                obj_num_l = arr->obj_num;   /* for next left  index */
            }
            if (rel[i].obj1 >= -1 && rel[i].obj2 >= 0
            &&  RT_IS_ARRAY(obj_arr_r[rel[i].obj2]))
            {
                rt_Array *arr = (rt_Array *)obj_arr_r[rel[i].obj2];
                obj_arr_r = arr->obj_arr; /* select right sub-array */
                obj_num_r = arr->obj_num;   /* for next right index */
            }
            break;

            case RT_REL_MINUS_INNER:
            case RT_REL_MINUS_OUTER:
            if (rel[i].obj1 == -1 && rel[i].obj2 >= 0 && acc == 0)
            {
                acc = 1;
                elm = *ptr != RT_NULL ? *ptr :
                      (rt_ELEM *)rg->alloc(sizeof(rt_ELEM), RT_QUAD_ALIGN);
                if (*ptr == RT_NULL)
                {
                   *ptr = elm;
                    elm->simd = RT_NULL;
                }
                /* use original marker value as elements are inserted
                 * into the list's tail here and the original relations
                 * template from scene data is inverted twice, first
                 * in surface's "add_relation" and second in engine's "sclip",
                 * thus accum markers will end up in correct order */
                elm->data = RT_ACCUM_ENTER;
                elm->temp = RT_NULL; /* accum marker */
                elm->next = RT_NULL;
                lst = elm;
                prv = elm;
                ptr = RT_GET_ADR(elm->simd);
            }
            if (rel[i].obj1 >= -1 && rel[i].obj2 >= 0)
            {
                elm = *ptr != RT_NULL ? *ptr :
                      (rt_ELEM *)rg->alloc(sizeof(rt_ELEM), RT_QUAD_ALIGN);
                if (*ptr == RT_NULL)
                {
                   *ptr = elm;
                    elm->simd = RT_NULL;
                }
                elm->data = rel[i].rel;
                elm->temp = obj_arr_r[rel[i].obj2]->bvbox;
                elm->next = RT_NULL;
                obj_arr_r = obj_arr; /* reset right sub-array after use */
                obj_num_r = obj_num;
            }
            if (rel[i].obj1 == -1 && rel[i].obj2 >= 0)
            {
                prv->next = elm;
                prv = elm;
                ptr = RT_GET_ADR(elm->simd);
            }
            break;

            case RT_REL_MINUS_ACCUM:
            if (rel[i].obj1 >= 0 && rel[i].obj2 == -1 && acc == 1)
            {
                acc = 0;
                elm = *ptr != RT_NULL ? *ptr :
                      (rt_ELEM *)rg->alloc(sizeof(rt_ELEM), RT_QUAD_ALIGN);
                if (*ptr == RT_NULL)
                {
                   *ptr = elm;
                    elm->simd = RT_NULL;
                }
                /* use original marker value as elements are inserted
                 * into the list's tail here and the original relations
                 * template from scene data is inverted twice, first
                 * in surface's "add_relation" and second in engine's "sclip",
                 * thus accum markers will end up in correct order */
                elm->data = RT_ACCUM_LEAVE;
                elm->temp = RT_NULL; /* accum marker */
                elm->next = RT_NULL;
                prv->next = elm;
                elm = lst;
                lst = RT_NULL;
                prv = RT_NULL;
                ptr = &rg->rel;
            }
            break;

            case RT_REL_BOUND_ARRAY:
            if (rel[i].obj1 == -1 && rel[i].obj2 == -1)
            {
                obj = arr = this;
                mode = RT_TRUE;
            }
            if (rel[i].obj1 == -1 && rel[i].obj2 >= 0
            &&  RT_IS_ARRAY(obj_arr_r[rel[i].obj2]))
            {
                obj = arr = (rt_Array *)obj_arr_r[rel[i].obj2];
                mode = RT_TRUE;
            }
            break;

            case RT_REL_UNTIE_ARRAY:
            if (rel[i].obj1 == -1 && rel[i].obj2 == -1)
            {
                obj = arr = this;
                mode = RT_FALSE;
            }
            if (rel[i].obj1 == -1 && rel[i].obj2 >= 0
            &&  RT_IS_ARRAY(obj_arr_r[rel[i].obj2]))
            {
                obj = arr = (rt_Array *)obj_arr_r[rel[i].obj2];
                mode = RT_FALSE;
            }
            break;

            case RT_REL_BOUND_INDEX:
            if (rel[i].obj1 == -1 && rel[i].obj2 >= 0)
            {
                obj = obj_arr_r[rel[i].obj2];
                arr = this;
                mode = RT_TRUE;
            }
            if (rel[i].obj1 >= 0 && rel[i].obj2 >= 0
            &&  RT_IS_ARRAY(obj_arr_l[rel[i].obj1]))
            {
                obj = obj_arr_r[rel[i].obj2];
                arr = (rt_Array *)obj_arr_l[rel[i].obj1];
                mode = RT_TRUE;
            }
            break;

            case RT_REL_UNTIE_INDEX:
            if (rel[i].obj1 == -1 && rel[i].obj2 >= 0)
            {
                obj = obj_arr_r[rel[i].obj2];
                arr = this;
                mode = RT_FALSE;
            }
            if (rel[i].obj1 >= 0 && rel[i].obj2 >= 0
            &&  RT_IS_ARRAY(obj_arr_l[rel[i].obj1]))
            {
                obj = obj_arr_r[rel[i].obj2];
                arr = (rt_Array *)obj_arr_l[rel[i].obj1];
                mode = RT_FALSE;
            }
            break;

            default:
            break;
        }

        if (rel[i].obj1 >= 0 && elm != RT_NULL)
        {
            obj_arr_l[rel[i].obj1]->add_relation(elm);
            obj_arr_l = obj_arr; /* reset left  sub-array after use */
            obj_num_l = obj_num;
        }
        if (obj != RT_NULL && arr != RT_NULL)
        {
#if RT_OPTS_VARRAY != 0
            if ((rg->opts & RT_OPTS_VARRAY) != 0)
            {
                obj->update_bvnode(arr, mode);
            }
#endif /* RT_OPTS_VARRAY */
            if (rel[i].obj1 >= 0)
            {
                obj_arr_l = obj_arr; /* reset left  sub-array after use */
                obj_num_l = obj_num;
            }
            if (rel[i].obj2 >= 0)
            {
                obj_arr_r = obj_arr; /* reset right sub-array after use */
                obj_num_r = obj_num;
            }
        }
    }

    /* init outer side material */
    outer = new(rg) rt_Material(rg, &sd_array01, &mt_glass01_array01);

    /* init inner side material */
    inner = new(rg) rt_Material(rg, &sd_array01, &mt_glass01_array01);

    /* validate surface size */
    ssize = RT_MAX(ssize, sizeof(rt_SIMD_SURFACE));

/*  rt_SIMD_SURFACE */

    s_inb = (rt_SIMD_SURFACE *)rg->alloc(ssize, RT_SIMD_ALIGN);
    memset(s_inb, 0, ssize);
    s_inb->srf_t[3] = RT_TAG_SURFACE_MAX;

    s_inb->mat_p[0] = outer->s_mat;
    s_inb->mat_p[1] = (rt_pntr)(rt_word)outer->props;
    s_inb->mat_p[2] = inner->s_mat;
    s_inb->mat_p[3] = (rt_pntr)(rt_word)inner->props;

    RT_SIMD_SET(s_srf->sbase, 0);
    RT_SIMD_SET(s_srf->smask, (rt_uelm)0x80000000 << (RT_ELEMENT - 32));
    RT_SIMD_SET(s_srf->c_def, -1);

    RT_SIMD_SET(s_inb->d_eps, RT_DEPS_THRESHOLD);
    RT_SIMD_SET(s_inb->t_eps, RT_TEPS_THRESHOLD);

/*  rt_SIMD_SURFACE */

    s_bvb = (rt_SIMD_SURFACE *)rg->alloc(ssize, RT_SIMD_ALIGN);
    memset(s_bvb, 0, ssize);
    s_bvb->srf_t[3] = RT_TAG_SURFACE_MAX;

    s_bvb->mat_p[0] = outer->s_mat;
    s_bvb->mat_p[1] = (rt_pntr)(rt_word)outer->props;
    s_bvb->mat_p[2] = inner->s_mat;
    s_bvb->mat_p[3] = (rt_pntr)(rt_word)inner->props;

    RT_SIMD_SET(s_srf->sbase, 0);
    RT_SIMD_SET(s_srf->smask, (rt_uelm)0x80000000 << (RT_ELEMENT - 32));
    RT_SIMD_SET(s_srf->c_def, -1);

    RT_SIMD_SET(s_bvb->d_eps, RT_DEPS_THRESHOLD);
    RT_SIMD_SET(s_bvb->t_eps, RT_TEPS_THRESHOLD);
}

/*
 * Build relations template based on given template "lst" from scene data.
 */
rt_void rt_Array::add_relation(rt_ELEM *lst)
{
    rt_Node::add_relation(lst);

    rt_si32 i;

    for (i = 0; i < obj_num; i++)
    {
        obj_arr[i]->add_relation(lst);
    }
}

/*
 * Update bvnode pointer for all sub-objects,
 * including sub-arrays (recursive).
 */
rt_void rt_Array::update_bvnode(rt_Object *bvnode, rt_bool mode)
{
    rt_Node::update_bvnode(bvnode, mode);

    rt_si32 i;

    for (i = 0; i < obj_num; i++)
    {
        obj_arr[i]->update_bvnode(bvnode, mode);
    }
}

/*
 * Update object's status with given "time", "flags" and "trnode".
 */
rt_void rt_Array::update_status(rt_time time, rt_si32 flags,
                                rt_Object *trnode)
{
    /* trigger update of the whole hierarchy
     * when called for the first time or
     * requested explicitly via "time == -1" */
#if RT_OPTS_UPDATE != 0
    if ((rg->opts & RT_OPTS_UPDATE) == 0
    || (obj->time == -1 && parent == RT_NULL))
#endif /* RT_OPTS_UPDATE */
    {
        flags |= RT_UPDATE_FLAG_OBJ;
    }

    rt_Node::update_status(time, flags, trnode);

    /* inherit array's changed status from object */
    arr_changed = obj_changed;
}

/*
 * Update object's matrix with given "mtx".
 */
rt_void rt_Array::update_matrix(rt_mat4 mtx)
{
    if (obj_changed == 0)
    {
        return;
    }

    rt_Node::update_matrix(mtx);

    /* reset bvbox's trnode for rtgeom
     * as array's bvbox is always in world space,
     * note that bvbox's pos is still relative to trnode if present */
    bvbox->trnode = RT_NULL;

    /* set trbox's trnode for rtgeom */
    trbox->trnode = trnode != RT_NULL ? ((rt_Array *)trnode)->trbox : RT_NULL;

    /* set inbox's trnode for rtgeom */
    inbox->trnode = trnode != RT_NULL ? ((rt_Array *)trnode)->inbox : RT_NULL;

    /* set matrix pointer for sub-objects
     * to array's transform matrix */
    pmtx = &this->mtx;

    /* if array node has non-trivial transform (trnode)
     * put scalers before rotation into a separate matrix
     * with main diagonal for passing to sub-objects */
    if (trnode == this)
    {
        if (obj_changed)
        {
            memcpy(scm, iden4, sizeof(rt_mat4));

            scm[0][0] = scl[0];
            scm[1][1] = scl[1];
            scm[2][2] = scl[2];
        }

        /* set matrix pointer for sub-objects
         * to scalers matrix */
        pmtx = &scm;
    }
}

/*
 * Update object with given "time", "flags", "trnode" and matrix "mtx".
 */
rt_void rt_Array::update_object(rt_time time, rt_si32 flags,
                                rt_Object *trnode, rt_mat4 mtx)
{
    update_status(time, flags, trnode);

    update_matrix(mtx);

    scn_changed = 0;

    rt_si32 i;

    /* update every object in array including sub-arrays (recursive),
     * pass array's own transform flags, changed status,
     * updated trnode and matrix pointer for sub-objects */
    for (i = 0; i < obj_num; i++)
    {
        obj_arr[i]->update_object(time, flags | mtx_has_trm | obj_changed,
                                  this->trnode, *pmtx);

        if (RT_IS_ARRAY(obj_arr[i]))
        {
            scn_changed |= ((rt_Array *)obj_arr[i])->scn_changed;
        }
        else
        {
            scn_changed |= obj_arr[i]->obj_changed;
        }
    }
}

/*
 * Update SIMD and other data fields.
 */
rt_void rt_Array::update_fields()
{
    if (obj_changed == 0)
    {
        return;
    }

    rt_Node::update_fields();

    /* if surface or some of its parents has non-trivial transform,
     * select aux vector fields for axis mapping in backend structures */
    rt_si32 shift = trnode != RT_NULL ? 3 : 0;

    s_srf->a_map[RT_I] = (mp_i + shift) * RT_SIMD_QUADS * 16;
    s_srf->a_map[RT_J] = (mp_j + shift) * RT_SIMD_QUADS * 16;
    s_srf->a_map[RT_K] = (mp_k + shift) * RT_SIMD_QUADS * 16;
    s_srf->a_map[RT_L] = obj_has_trm;

    s_srf->a_sgn[RT_I] = (sgn[RT_I] >= 0 ? 0 : 1) * RT_SIMD_QUADS * 16;
    s_srf->a_sgn[RT_J] = (sgn[RT_J] >= 0 ? 0 : 1) * RT_SIMD_QUADS * 16;
    s_srf->a_sgn[RT_K] = (sgn[RT_K] >= 0 ? 0 : 1) * RT_SIMD_QUADS * 16;
    s_srf->a_sgn[RT_L] = shift * RT_SIMD_QUADS * 16;

    /* trnode's simd ptr is needed in rendering backend
     * to check if surface and its clippers belong to the same trnode */
    s_srf->msc_p[3] = trnode == RT_NULL ?
                                RT_NULL : ((rt_Node *)trnode)->s_srf;

    s_inb->a_map[RT_I] = (mp_i + shift) * RT_SIMD_QUADS * 16;
    s_inb->a_map[RT_J] = (mp_j + shift) * RT_SIMD_QUADS * 16;
    s_inb->a_map[RT_K] = (mp_k + shift) * RT_SIMD_QUADS * 16;
    s_inb->a_map[RT_L] = obj_has_trm;

    s_inb->a_sgn[RT_I] = (sgn[RT_I] >= 0 ? 0 : 1) * RT_SIMD_QUADS * 16;
    s_inb->a_sgn[RT_J] = (sgn[RT_J] >= 0 ? 0 : 1) * RT_SIMD_QUADS * 16;
    s_inb->a_sgn[RT_K] = (sgn[RT_K] >= 0 ? 0 : 1) * RT_SIMD_QUADS * 16;
    s_inb->a_sgn[RT_L] = shift * RT_SIMD_QUADS * 16;

    /* trnode's simd ptr is needed in rendering backend
     * to check if surface and its clippers belong to the same trnode */
    s_inb->msc_p[3] = trnode == RT_NULL ?
                                RT_NULL : ((rt_Node *)trnode)->s_srf;

    RT_SIMD_SET(s_inb->scj_x, 0.0f);
    RT_SIMD_SET(s_inb->scj_y, 0.0f);
    RT_SIMD_SET(s_inb->scj_z, 0.0f);

    s_bvb->a_map[RT_I] = RT_X * RT_SIMD_QUADS * 16;
    s_bvb->a_map[RT_J] = RT_Y * RT_SIMD_QUADS * 16;
    s_bvb->a_map[RT_K] = RT_Z * RT_SIMD_QUADS * 16;
    s_bvb->a_map[RT_L] = 0;

    s_bvb->a_sgn[RT_I] = 0;
    s_bvb->a_sgn[RT_J] = 0;
    s_bvb->a_sgn[RT_K] = 0;
    s_bvb->a_sgn[RT_L] = 0;

    /* trnode's simd ptr is needed in rendering backend
     * to check if surface and its clippers belong to the same trnode */
    s_bvb->msc_p[3] = RT_NULL;

    RT_SIMD_SET(s_bvb->scj_x, 0.0f);
    RT_SIMD_SET(s_bvb->scj_y, 0.0f);
    RT_SIMD_SET(s_bvb->scj_z, 0.0f);
}

/*
 * Update bounding box and volume along with related SIMD fields.
 */
rt_void rt_Array::update_bounds()
{
    if (arr_changed == 0)
    {
        return;
    }

    /* reset all boxes for array */
    RT_VEC3_SET_VAL1(bvbox->bmin, +RT_INF);
    RT_VEC3_SET_VAL1(bvbox->bmax, -RT_INF);
    bvbox->rad = 0.0f;
    bvbox->fln = 0;
    bvbox->flm = 0;
    bvbox->flf = 0;

    RT_VEC3_SET_VAL1(trbox->bmin, +RT_INF);
    RT_VEC3_SET_VAL1(trbox->bmax, -RT_INF);
    trbox->rad = 0.0f;
    trbox->fln = 0;
    trbox->flm = 0;
    trbox->flf = 0;

    RT_VEC3_SET_VAL1(inbox->bmin, +RT_INF);
    RT_VEC3_SET_VAL1(inbox->bmax, -RT_INF);
    inbox->rad = 0.0f;
    inbox->fln = 0;
    inbox->flm = 0;
    inbox->flf = 0;

    rt_si32 i, j, k;
    rt_BOUND *src_box, *dst_box;

    /* run through array's sub-objects,
     * including sub-arrays (recursive) */
    for (i = 0; i < obj_num; i++)
    {
        rt_Node *nd = RT_NULL;
        rt_Array *arr = RT_NULL;

        if (RT_IS_ARRAY(obj_arr[i]))
        {
            nd = (rt_Node *)obj_arr[i];
            arr = (rt_Array *)nd;
            arr->update_bounds();
        }
        else
        if (RT_IS_SURFACE(obj_arr[i]))
        {
            nd = (rt_Node *)obj_arr[i];
        }
        else
        {
            continue;
        }

        /* contribute bounds to trnode's trbox or bvnode's inbox */
        src_box = dst_box = RT_NULL;
        arr = nd->trnode != nd ?
              (rt_Array *)nd->trnode : RT_NULL;

        if (arr != RT_NULL)
        {
            src_box = RT_IS_SURFACE(nd) ? nd->bvbox : ((rt_Array *)nd)->inbox;
            dst_box = nd->bvnode != RT_NULL && nd->bvnode->trnode == arr ?
                      ((rt_Array *)nd->bvnode)->inbox : arr->trbox;
        }

        if (src_box != RT_NULL && src_box->rad != 0.0f && dst_box != RT_NULL)
        {
            if (src_box->rad != RT_INF)
            {
                /* contribute minmax data directly (fast) */
#if RT_OPTS_REMOVE != 0
                if ((rg->opts & RT_OPTS_REMOVE) != 0
                &&  RT_IS_PLANE(src_box) && src_box->flm != 0)
                {
                    rt_si32 b = 0, c = 0, m = 3;

                    for (k = 0; k < 3; k++)
                    {
                        if (src_box->map[RT_K] == k)
                        {
                            m = k;
                        }

                        if (dst_box->bmin[k] > src_box->bmin[k])
                        {
                            dst_box->bmin[k] = src_box->bmin[k];
                            dst_box->flm &= 2 << (k * 2);
                            if (k == m)
                            {
                                c |= 1;
                            }
                        }
                        else
                        if (dst_box->bmin[k] < src_box->bmin[k])
                        {
                            if (k != m)
                            {
                                b = 1;
                            }
                        }
                        else
                        {
                            if (k == m)
                            {
                                c |= 1;
                            }
                        }
                        if (dst_box->bmax[k] < src_box->bmax[k])
                        {
                            dst_box->bmax[k] = src_box->bmax[k];
                            dst_box->flm &= 1 << (k * 2);
                            if (k == m)
                            {
                                c |= 2;
                            }
                        }
                        else
                        if (dst_box->bmax[k] > src_box->bmax[k])
                        {
                            if (k != m)
                            {
                                b = 1;
                            }
                        }
                        else
                        {
                            if (k == m)
                            {
                                c |= 2;
                            }
                        }
                    }

                    if (b == 0 && m < 3)
                    {
                        dst_box->flm |= c << (m * 2);
                    }
                }
                else
                if ((rg->opts & RT_OPTS_REMOVE) != 0)
                {
                    for (k = 0; k < 3; k++)
                    {
                        if (dst_box->bmin[k] > src_box->bmin[k])
                        {
                            dst_box->bmin[k] = src_box->bmin[k];
                            dst_box->flm &= 2 << (k * 2);
                        }
                        if (dst_box->bmax[k] < src_box->bmax[k])
                        {
                            dst_box->bmax[k] = src_box->bmax[k];
                            dst_box->flm &= 1 << (k * 2);
                        }
                    }
                }
                else
#endif /* RT_OPTS_REMOVE */
                {
                    for (k = 0; k < 3; k++)
                    {
                        if (dst_box->bmin[k] > src_box->bmin[k])
                        {
                            dst_box->bmin[k] = src_box->bmin[k];
                        }
                        if (dst_box->bmax[k] < src_box->bmax[k])
                        {
                            dst_box->bmax[k] = src_box->bmax[k];
                        }
                    }
                }
            }
            /* use rad temporarily as a tag for
             * "empty" (rad == 0.0f),
             * "finite" (0.0f < rad < RT_INF),
             * "infinite" (rad == RT_INF) */
            if (dst_box->rad < src_box->rad)
            {
                dst_box->rad = src_box->rad;
            }
        }

        /* contribute bounds to bvnode's bvbox */
        src_box = dst_box = RT_NULL;
        arr = (rt_Array *)nd->bvnode;

        if (arr != RT_NULL)
        {
            src_box = RT_IS_SURFACE(nd) && nd->trnode == arr->trnode && 
                      nd->trnode != nd && nd->trnode != RT_NULL ?
                      RT_NULL : nd->bvbox;
            dst_box = arr->bvbox;
        }

        if (src_box != RT_NULL && src_box->rad != 0.0f && dst_box != RT_NULL
        && (nd->trnode == RT_NULL || RT_IS_ARRAY(nd)))
        {
            if (src_box->rad != RT_INF)
            {
                /* contribute minmax data directly (fast) */
#if RT_OPTS_REMOVE != 0
                if ((rg->opts & RT_OPTS_REMOVE) != 0
                &&  RT_IS_PLANE(src_box) && src_box->flm != 0)
                {
                    rt_si32 b = 0, c = 0, m = 3;

                    for (k = 0; k < 3; k++)
                    {
                        if (src_box->map[RT_K] == k)
                        {
                            m = k;
                        }

                        if (dst_box->bmin[k] > src_box->bmin[k])
                        {
                            dst_box->bmin[k] = src_box->bmin[k];
                            dst_box->flm &= 2 << (k * 2);
                            if (k == m)
                            {
                                c |= 1;
                            }
                        }
                        else
                        if (dst_box->bmin[k] < src_box->bmin[k])
                        {
                            if (k != m)
                            {
                                b = 1;
                            }
                        }
                        else
                        {
                            if (k == m)
                            {
                                c |= 1;
                            }
                        }
                        if (dst_box->bmax[k] < src_box->bmax[k])
                        {
                            dst_box->bmax[k] = src_box->bmax[k];
                            dst_box->flm &= 1 << (k * 2);
                            if (k == m)
                            {
                                c |= 2;
                            }
                        }
                        else
                        if (dst_box->bmax[k] > src_box->bmax[k])
                        {
                            if (k != m)
                            {
                                b = 1;
                            }
                        }
                        else
                        {
                            if (k == m)
                            {
                                c |= 2;
                            }
                        }
                    }

                    if (b == 0 && m < 3)
                    {
                        dst_box->flm |= c << (m * 2);
                    }
                }
                else
                if ((rg->opts & RT_OPTS_REMOVE) != 0)
                {
                    for (k = 0; k < 3; k++)
                    {
                        if (dst_box->bmin[k] > src_box->bmin[k])
                        {
                            dst_box->bmin[k] = src_box->bmin[k];
                            dst_box->flm &= 2 << (k * 2);
                        }
                        if (dst_box->bmax[k] < src_box->bmax[k])
                        {
                            dst_box->bmax[k] = src_box->bmax[k];
                            dst_box->flm &= 1 << (k * 2);
                        }
                    }
                }
                else
#endif /* RT_OPTS_REMOVE */
                {
                    for (k = 0; k < 3; k++)
                    {
                        if (dst_box->bmin[k] > src_box->bmin[k])
                        {
                            dst_box->bmin[k] = src_box->bmin[k];
                        }
                        if (dst_box->bmax[k] < src_box->bmax[k])
                        {
                            dst_box->bmax[k] = src_box->bmax[k];
                        }
                    }
                }
            }
            /* use rad temporarily as a tag for
             * "empty" (rad == 0.0f),
             * "finite" (0.0f < rad < RT_INF),
             * "infinite" (rad == RT_INF) */
            if (dst_box->rad < src_box->rad)
            {
                dst_box->rad = src_box->rad;
            }
        }

        if (arr != RT_NULL)
        {
            src_box = RT_IS_ARRAY(nd) ? nd->trnode != nd ||
                      ((rt_Array *)nd)->trbox->rad != 0.0f ||
                      ((rt_Array *)nd)->bvbox->rad == 0.0f ?
                      ((rt_Array *)nd)->inbox : RT_NULL : src_box;
        }

        if (src_box != RT_NULL && src_box->rad != 0.0f && dst_box != RT_NULL
        &&  nd->trnode != RT_NULL && nd->trnode != arr->trnode)
        {
            if (src_box->rad != RT_INF)
            {
                /* contribute transformed vertex data (8x slower) */
                for (j = 0; j < src_box->verts_num; j++)
                {
                    for (k = 0; k < 3; k++)
                    {
                        if (dst_box->bmin[k] > src_box->verts[j].pos[k])
                        {
                            dst_box->bmin[k] = src_box->verts[j].pos[k];
                        }
                        if (dst_box->bmax[k] < src_box->verts[j].pos[k])
                        {
                            dst_box->bmax[k] = src_box->verts[j].pos[k];
                        }
                    }
                }
            }
            /* use rad temporarily as a tag for
             * "empty" (rad == 0.0f),
             * "finite" (0.0f < rad < RT_INF),
             * "infinite" (rad == RT_INF) */
            if (dst_box->rad < src_box->rad)
            {
                dst_box->rad = src_box->rad;
            }
        }
    }

    /* update inbox's geometry */
    if (inbox->rad != 0.0f && inbox->rad != RT_INF)
    {
        update_bbgeom(inbox);

        RT_SIMD_SET(s_inb->pos_x, (inbox->bmin[RT_X]+inbox->bmax[RT_X])*0.5f);
        RT_SIMD_SET(s_inb->pos_y, (inbox->bmin[RT_Y]+inbox->bmax[RT_Y])*0.5f);
        RT_SIMD_SET(s_inb->pos_z, (inbox->bmin[RT_Z]+inbox->bmax[RT_Z])*0.5f);

        rt_vec4 dff;
        RT_VEC3_SUB(dff, inbox->bmax, inbox->bmin);

        RT_SIMD_SET(s_inb->sci_w, 0.75f); /* unit cube's radius squared */
        RT_SIMD_SET(s_inb->sci_x, 1.0f / (dff[RT_X] * dff[RT_X]));
        RT_SIMD_SET(s_inb->sci_y, 1.0f / (dff[RT_Y] * dff[RT_Y]));
        RT_SIMD_SET(s_inb->sci_z, 1.0f / (dff[RT_Z] * dff[RT_Z]));

        /* contribute trnode array's inbox to trbox if trbox has contents,
         * trbox has priority over bvbox here as bvbox might get split */
        if (trnode == this && trbox->rad != 0.0f)
        {
            src_box = inbox;
            dst_box = trbox;

            if (src_box->rad != RT_INF)
            {
                /* contribute minmax data directly (fast) */
                for (k = 0; k < 3; k++)
                {
                    if (dst_box->bmin[k] > src_box->bmin[k])
                    {
                        dst_box->bmin[k] = src_box->bmin[k];
                    }
                    if (dst_box->bmax[k] < src_box->bmax[k])
                    {
                        dst_box->bmax[k] = src_box->bmax[k];
                    }
                }
            }
            /* use rad temporarily as a tag for
             * "empty" (rad == 0.0f),
             * "finite" (0.0f < rad < RT_INF),
             * "infinite" (rad == RT_INF) */
            if (dst_box->rad < src_box->rad)
            {
                dst_box->rad = src_box->rad;
            }
        }
        else
        /* contribute trnode array's inbox to bvbox if bvbox has contents
         * while trbox is empty, trbox has priority as defined in "snode" */
        if (trnode == this && bvbox->rad != 0.0f)
        {
            src_box = inbox;
            dst_box = bvbox;

            if (src_box->rad != RT_INF)
            {
                /* contribute transformed vertex data (8x slower) */
                for (j = 0; j < src_box->verts_num; j++)
                {
                    for (k = 0; k < 3; k++)
                    {
                        if (dst_box->bmin[k] > src_box->verts[j].pos[k])
                        {
                            dst_box->bmin[k] = src_box->verts[j].pos[k];
                        }
                        if (dst_box->bmax[k] < src_box->verts[j].pos[k])
                        {
                            dst_box->bmax[k] = src_box->verts[j].pos[k];
                        }
                    }
                }
            }
            /* use rad temporarily as a tag for
             * "empty" (rad == 0.0f),
             * "finite" (0.0f < rad < RT_INF),
             * "infinite" (rad == RT_INF) */
            if (dst_box->rad < src_box->rad)
            {
                dst_box->rad = src_box->rad;
            }
        }
        else
        if (trnode == this)
        {
            s_inb->a_map[RT_I] = RT_X * RT_SIMD_QUADS * 16;
            s_inb->a_map[RT_J] = RT_Y * RT_SIMD_QUADS * 16;
            s_inb->a_map[RT_K] = RT_Z * RT_SIMD_QUADS * 16;
            s_inb->a_map[RT_L] = 0;

            s_inb->a_sgn[RT_I] = 0;
            s_inb->a_sgn[RT_J] = 0;
            s_inb->a_sgn[RT_K] = 0;
            s_inb->a_sgn[RT_L] = 0;

            /* trnode's simd ptr is needed in rendering backend
             * to check if surface and its clippers belong to the same trnode */
            s_inb->msc_p[3] = RT_NULL;

            RT_SIMD_SET(s_inb->pos_x, inbox->mid[RT_X]);
            RT_SIMD_SET(s_inb->pos_y, inbox->mid[RT_Y]);
            RT_SIMD_SET(s_inb->pos_z, inbox->mid[RT_Z]);

            RT_SIMD_SET(s_inb->sci_w, inbox->rad * inbox->rad);
            RT_SIMD_SET(s_inb->sci_x, 1.0f);
            RT_SIMD_SET(s_inb->sci_y, 1.0f);
            RT_SIMD_SET(s_inb->sci_z, 1.0f);
        }
    }

    /* update bvbox's geometry */
    if (bvbox->rad != 0.0f && bvbox->rad != RT_INF)
    {
        update_bbgeom(bvbox);

        RT_SIMD_SET(s_bvb->pos_x, (bvbox->bmin[RT_X]+bvbox->bmax[RT_X])*0.5f);
        RT_SIMD_SET(s_bvb->pos_y, (bvbox->bmin[RT_Y]+bvbox->bmax[RT_Y])*0.5f);
        RT_SIMD_SET(s_bvb->pos_z, (bvbox->bmin[RT_Z]+bvbox->bmax[RT_Z])*0.5f);

        rt_vec4 dff;
        RT_VEC3_SUB(dff, bvbox->bmax, bvbox->bmin);

        RT_SIMD_SET(s_bvb->sci_w, 0.75f); /* unit cube's radius squared */
        RT_SIMD_SET(s_bvb->sci_x, 1.0f / (dff[RT_X] * dff[RT_X]));
        RT_SIMD_SET(s_bvb->sci_y, 1.0f / (dff[RT_Y] * dff[RT_Y]));
        RT_SIMD_SET(s_bvb->sci_z, 1.0f / (dff[RT_Z] * dff[RT_Z]));
    }

    /* update trbox's geometry */
    if (trbox->rad != 0.0f && trbox->rad != RT_INF)
    {
        update_bbgeom(trbox);
    }
}

/*
 * Deinitialize array object.
 */
rt_Array::~rt_Array()
{
    rt_si32 i;

    for (i = 0; i < obj_num; i++)
    {
        delete obj_arr[i];
    }

    delete outer;
    delete inner;
}

/******************************************************************************/
/*********************************   SURFACE   ********************************/
/******************************************************************************/

/*
 * Instantiate surface object.
 */
rt_Surface::rt_Surface(rt_Registry *rg, rt_Object *parent,
                       rt_OBJECT *obj, rt_si32 ssize) :

    rt_Node(rg, parent, obj, ssize),
    rt_List<rt_Surface>(rg->get_srf())
{
    rg->put_srf(this);

    srf = (rt_SURFACE *)obj->obj.pobj;

    /* reset surface's changed status */
    srf_changed = 0;

    /* init outer side material */
    outer = new(rg) rt_Material(rg, &srf->side_outer,
                    obj->obj.pmat_outer ? obj->obj.pmat_outer :
                                          srf->side_outer.pmat);

    /* init inner side material */
    inner = new(rg) rt_Material(rg, &srf->side_inner,
                    obj->obj.pmat_inner ? obj->obj.pmat_inner :
                                          srf->side_inner.pmat);

    /* init surface's bvbox used for tiling, rtgeom and array's bounds */
    bvbox->rad = RT_INF;

    /* init surface's shape used for rtgeom */
    shape = (rt_SHAPE *)bvbox;
    shape->ptr = (rt_pntr*)&s_srf->msc_p[2];

/*  rt_SIMD_SURFACE */

    s_srf->mat_p[0] = outer->s_mat;
    s_srf->mat_p[1] = (rt_pntr)(rt_word)outer->props;
    s_srf->mat_p[2] = inner->s_mat;
    s_srf->mat_p[3] = (rt_pntr)(rt_word)inner->props;
}

/*
 * Build relations template based on given template "lst" from scene data.
 */
rt_void rt_Surface::add_relation(rt_ELEM *lst)
{
    rt_Node::add_relation(lst);

    /* init surface's relations template */
    rt_ELEM **ptr = RT_GET_ADR(rel);

    /* build surface's relations template from given template "lst",
     * as surface's relations template is inverted in engine's "sclip"
     * and elements are inserted into the list's head here,
     * the original relations template from scene data is inverted twice,
     * thus accum enter/leave markers will end up in correct order */
    for (; lst != RT_NULL; lst = lst->next)
    {
        rt_ELEM *elm = RT_NULL;
        rt_cell rel = lst->data;
        rt_Object *obj = lst->temp == RT_NULL ? RT_NULL :
                         (rt_Object *)((rt_BOUND *)lst->temp)->obj;

        if (obj == RT_NULL)
        {
            /* alloc new element for accum marker */
            elm = (rt_ELEM *)rg->alloc(sizeof(rt_ELEM), RT_QUAD_ALIGN);
            elm->data = rel;
            elm->simd = RT_NULL; /* accum marker */
            elm->temp = RT_NULL;
            /* insert element as list's head */
            elm->next = *ptr;
           *ptr = elm;
        }
        else
        if (RT_IS_ARRAY(obj))
        {
            rt_Array *arr = (rt_Array *)obj;
            rt_si32 i;

            /* init array's relations template
             * used to avoid unnecessary allocs */
            rt_ELEM **ptr = RT_GET_ADR(arr->rel);

            /* populate array element with sub-objects */
            for (i = 0; i < arr->obj_num; i++)
            {
                elm = *ptr != RT_NULL ? *ptr :
                      (rt_ELEM *)rg->alloc(sizeof(rt_ELEM), RT_QUAD_ALIGN);
                if (*ptr == RT_NULL)
                {
                   *ptr = elm;
                    elm->simd = RT_NULL;
                }
                elm->data = rel;
                elm->temp = arr->obj_arr[i]->bvbox;
                elm->next = RT_NULL;

                add_relation(elm);
            }
        }
        else
        if (RT_IS_SURFACE(obj))
        {
            rt_Surface *srf = (rt_Surface *)obj;

            /* alloc new element for srf */
            elm = (rt_ELEM *)rg->alloc(sizeof(rt_ELEM), RT_QUAD_ALIGN);
            elm->data = rel;
            elm->simd = srf->s_srf;
            elm->temp = srf->bvbox;
            /* insert element as list's head */
            elm->next = *ptr;
           *ptr = elm;
        }
    }
}

/*
 * Update object with given "time", "flags", "trnode" and matrix "mtx".
 */
rt_void rt_Surface::update_object(rt_time time, rt_si32 flags,
                                  rt_Object *trnode, rt_mat4 mtx)
{
    update_status(time, flags, trnode);

    /* set matrix pointer
     * from immediate parent array */
    pmtx = (rt_mat4 *)mtx;
}

/*
 * Update SIMD and other data fields.
 */
rt_void rt_Surface::update_fields()
{
    if (obj_changed == 0)
    {
        return;
    }

    /* pass matrix pointer
     * from immediate parent array */
    update_matrix(*pmtx);

    rt_Node::update_fields();

    /* if surface or some of its parents has non-trivial transform,
     * select aux vector fields for axis mapping in backend structures */
    rt_si32 shift = trnode != RT_NULL ? 3 : 0;

    s_srf->a_map[RT_I] = (mp_i + shift) * RT_SIMD_QUADS * 16;
    s_srf->a_map[RT_J] = (mp_j + shift) * RT_SIMD_QUADS * 16;
    s_srf->a_map[RT_K] = (mp_k + shift) * RT_SIMD_QUADS * 16;
    s_srf->a_map[RT_L] = obj_has_trm;

    s_srf->a_sgn[RT_I] = (sgn[RT_I] >= 0 ? 0 : 1) * RT_SIMD_QUADS * 16;
    s_srf->a_sgn[RT_J] = (sgn[RT_J] >= 0 ? 0 : 1) * RT_SIMD_QUADS * 16;
    s_srf->a_sgn[RT_K] = (sgn[RT_K] >= 0 ? 0 : 1) * RT_SIMD_QUADS * 16;
    s_srf->a_sgn[RT_L] = shift * RT_SIMD_QUADS * 16;

    /* trnode's simd ptr is needed in rendering backend
     * to check if surface and its clippers belong to the same trnode */
    s_srf->msc_p[3] = trnode == RT_NULL ?
                                RT_NULL : ((rt_Node *)trnode)->s_srf;
}

/*
 * Adjust local space bounding and clipping boxes according to surface shape.
 */
rt_void rt_Surface::adjust_minmax(rt_vec4 smin, rt_vec4 smax, /* src */
                                  rt_vec4 bmin, rt_vec4 bmax, /* bbox */
                                  rt_vec4 cmin, rt_vec4 cmax) /* cbox */
{
    /* cbox adjust below is not currently used in rtgeom's "clip_side"
       as all custom clippers are considered surface holes for now */
    if (cmin != RT_NULL && cmax != RT_NULL)
    {
        cmin[RT_I] = smin[RT_I] > srf->min[RT_I] ? -RT_INF : smin[RT_I];
        cmin[RT_J] = smin[RT_J] > srf->min[RT_J] ? -RT_INF : smin[RT_J];
        cmin[RT_K] = smin[RT_K] > srf->min[RT_K] ? -RT_INF : smin[RT_K];

        cmax[RT_I] = smax[RT_I] < srf->max[RT_I] ? +RT_INF : smax[RT_I];
        cmax[RT_J] = smax[RT_J] < srf->max[RT_J] ? +RT_INF : smax[RT_J];
        cmax[RT_K] = smax[RT_K] < srf->max[RT_K] ? +RT_INF : smax[RT_K];
    }
}

/*
 * Transform sub-world space bounding or clipping box to local space
 * by applying axis mapping (trivial transform).
 * The sub-world space doesn't include trnode's transform matrix,
 * which is used to compute world space bounding box geometry,
 * thus minmax data always remains axis-aligned within sub-world space
 * up to trnode, or within world space if trnode is not present. 
 */
rt_void rt_Surface::invert_minmax(rt_vec4 smin, rt_vec4 smax, /* src */
                                  rt_vec4 dmin, rt_vec4 dmax) /* dst */
{
    rt_vec4 tmin, tmax; /* tmp */

    rt_vec4  zro = {0.0f, 0.0f, 0.0f, 0.0f};
    rt_real *pps = trnode == this ? zro : pos;

    tmin[RT_X] = smin[RT_X] == -RT_INF ? -RT_INF : smin[RT_X] - pps[RT_X];
    tmin[RT_Y] = smin[RT_Y] == -RT_INF ? -RT_INF : smin[RT_Y] - pps[RT_Y];
    tmin[RT_Z] = smin[RT_Z] == -RT_INF ? -RT_INF : smin[RT_Z] - pps[RT_Z];

    tmax[RT_X] = smax[RT_X] == +RT_INF ? +RT_INF : smax[RT_X] - pps[RT_X];
    tmax[RT_Y] = smax[RT_Y] == +RT_INF ? +RT_INF : smax[RT_Y] - pps[RT_Y];
    tmax[RT_Z] = smax[RT_Z] == +RT_INF ? +RT_INF : smax[RT_Z] - pps[RT_Z];

    tmin[RT_X] = tmin[RT_X] == -RT_INF ? -RT_INF : tmin[RT_X] / scl[RT_X];
    tmin[RT_Y] = tmin[RT_Y] == -RT_INF ? -RT_INF : tmin[RT_Y] / scl[RT_Y];
    tmin[RT_Z] = tmin[RT_Z] == -RT_INF ? -RT_INF : tmin[RT_Z] / scl[RT_Z];

    tmax[RT_X] = tmax[RT_X] == +RT_INF ? +RT_INF : tmax[RT_X] / scl[RT_X];
    tmax[RT_Y] = tmax[RT_Y] == +RT_INF ? +RT_INF : tmax[RT_Y] / scl[RT_Y];
    tmax[RT_Z] = tmax[RT_Z] == +RT_INF ? +RT_INF : tmax[RT_Z] / scl[RT_Z];

    dmin[RT_I] = sgn[RT_I] > 0 ? +tmin[mp_i] : -tmax[mp_i];
    dmin[RT_J] = sgn[RT_J] > 0 ? +tmin[mp_j] : -tmax[mp_j];
    dmin[RT_K] = sgn[RT_K] > 0 ? +tmin[mp_k] : -tmax[mp_k];

    dmax[RT_I] = sgn[RT_I] > 0 ? +tmax[mp_i] : -tmin[mp_i];
    dmax[RT_J] = sgn[RT_J] > 0 ? +tmax[mp_j] : -tmin[mp_j];
    dmax[RT_K] = sgn[RT_K] > 0 ? +tmax[mp_k] : -tmin[mp_k];
}

/*
 * Transform local space bounding or clipping box to sub-world space
 * by applying axis mapping (trivial transform).
 * The sub-world space doesn't include trnode's transform matrix,
 * which is used to compute world space bounding box geometry,
 * thus minmax data always remains axis-aligned within sub-world space
 * up to trnode, or within world space if trnode is not present. 
 */
rt_void rt_Surface::direct_minmax(rt_vec4 smin, rt_vec4 smax, /* src */
                                  rt_vec4 dmin, rt_vec4 dmax) /* dst */
{
    rt_vec4 tmin, tmax; /* tmp */

    rt_vec4  zro = {0.0f, 0.0f, 0.0f, 0.0f};
    rt_real *pps = trnode == this ? zro : pos;

    tmin[mp_i] = sgn[RT_I] > 0 ? +smin[RT_I] : -smax[RT_I];
    tmin[mp_j] = sgn[RT_J] > 0 ? +smin[RT_J] : -smax[RT_J];
    tmin[mp_k] = sgn[RT_K] > 0 ? +smin[RT_K] : -smax[RT_K];

    tmax[mp_i] = sgn[RT_I] > 0 ? +smax[RT_I] : -smin[RT_I];
    tmax[mp_j] = sgn[RT_J] > 0 ? +smax[RT_J] : -smin[RT_J];
    tmax[mp_k] = sgn[RT_K] > 0 ? +smax[RT_K] : -smin[RT_K];

    tmin[RT_X] = tmin[RT_X] == -RT_INF ? -RT_INF : tmin[RT_X] * scl[RT_X];
    tmin[RT_Y] = tmin[RT_Y] == -RT_INF ? -RT_INF : tmin[RT_Y] * scl[RT_Y];
    tmin[RT_Z] = tmin[RT_Z] == -RT_INF ? -RT_INF : tmin[RT_Z] * scl[RT_Z];

    tmax[RT_X] = tmax[RT_X] == +RT_INF ? +RT_INF : tmax[RT_X] * scl[RT_X];
    tmax[RT_Y] = tmax[RT_Y] == +RT_INF ? +RT_INF : tmax[RT_Y] * scl[RT_Y];
    tmax[RT_Z] = tmax[RT_Z] == +RT_INF ? +RT_INF : tmax[RT_Z] * scl[RT_Z];

    dmin[RT_X] = tmin[RT_X] == -RT_INF ? -RT_INF : tmin[RT_X] + pps[RT_X];
    dmin[RT_Y] = tmin[RT_Y] == -RT_INF ? -RT_INF : tmin[RT_Y] + pps[RT_Y];
    dmin[RT_Z] = tmin[RT_Z] == -RT_INF ? -RT_INF : tmin[RT_Z] + pps[RT_Z];

    dmax[RT_X] = tmax[RT_X] == +RT_INF ? +RT_INF : tmax[RT_X] + pps[RT_X];
    dmax[RT_Y] = tmax[RT_Y] == +RT_INF ? +RT_INF : tmax[RT_Y] + pps[RT_Y];
    dmax[RT_Z] = tmax[RT_Z] == +RT_INF ? +RT_INF : tmax[RT_Z] + pps[RT_Z];
}

/*
 * Recalculate bounding and clipping boxes based on given "src" box.
 */
rt_void rt_Surface::recalc_minmax(rt_vec4 smin, rt_vec4 smax,  /* src */
                                  rt_vec4 bmin, rt_vec4 bmax,  /* bbox */
                                  rt_vec4 cmin, rt_vec4 cmax)  /* cbox */
{
    rt_vec4 tmin, tmax;
    rt_vec4 lmin, lmax;

    rt_real *pmin = RT_NULL;
    rt_real *pmax = RT_NULL;

    /* accumulate bbox adjustments into cbox */
    if (smin != RT_NULL && smax != RT_NULL
    &&  bmin == RT_NULL && bmax == RT_NULL)
    {
        invert_minmax(smin, smax, tmin, tmax);

        bmin = lmin;
        bmax = lmax;

        pmin = cmin;
        pmax = cmax;

        cmin = RT_NULL;
        cmax = RT_NULL;
    }
    else
    /* apply bbox adjustments from cbox */
    if (smin != RT_NULL && smax != RT_NULL
    &&  cmin != RT_NULL && cmax != RT_NULL)
    {
        invert_minmax(smin, smax, tmin, tmax);

        RT_VEC3_MAX(tmin, tmin, srf->min);
        RT_VEC3_MIN(tmax, tmax, srf->max);
    }
    else
    /* init bbox with original axis clippers */
    if (smin == RT_NULL && smax == RT_NULL)
    {
        RT_VEC3_SET(tmin, srf->min);
        RT_VEC3_SET(tmax, srf->max);
    }

    adjust_minmax(tmin, tmax, bmin, bmax, cmin, cmax);

    /* accumulate bbox adjustments into cbox */
    if (pmin != RT_NULL && pmax != RT_NULL)
    {
        tmin[RT_I] = tmin[RT_I] == bmin[RT_I] ? -RT_INF : bmin[RT_I];
        tmin[RT_J] = tmin[RT_J] == bmin[RT_J] ? -RT_INF : bmin[RT_J];
        tmin[RT_K] = tmin[RT_K] == bmin[RT_K] ? -RT_INF : bmin[RT_K];

        tmax[RT_I] = tmax[RT_I] == bmax[RT_I] ? +RT_INF : bmax[RT_I];
        tmax[RT_J] = tmax[RT_J] == bmax[RT_J] ? +RT_INF : bmax[RT_J];
        tmax[RT_K] = tmax[RT_K] == bmax[RT_K] ? +RT_INF : bmax[RT_K];

        direct_minmax(tmin, tmax, tmin, tmax);

        RT_VEC3_MAX(pmin, pmin, tmin);
        RT_VEC3_MIN(pmax, pmax, tmax);

        bmin = RT_NULL;
        bmax = RT_NULL;
    }

    if (bmin != RT_NULL && bmax != RT_NULL)
    {
        direct_minmax(bmin, bmax, bmin, bmax);
    }

    if (cmin != RT_NULL && cmax != RT_NULL)
    {
        direct_minmax(cmin, cmax, cmin, cmax);
    }
}

/*
 * Update bounding and clipping boxes data.
 */
rt_void rt_Surface::update_minmax()
{
    /* inherit surface's changed status from object */
    srf_changed = obj_changed;

    /* init custom clippers list */
    rt_ELEM *elm = (rt_ELEM *)s_srf->msc_p[2];

    /* no custom clippers or
     * surface itself has non-trivial transform */
#if RT_OPTS_ADJUST != 0
    if ((rg->opts & RT_OPTS_ADJUST) == 0
    ||  elm == RT_NULL || trnode == this)
#endif /* RT_OPTS_ADJUST */
    {
        /* calculate bbox and cbox based on 
         * original axis clippers and surface shape */
        recalc_minmax(RT_NULL,     RT_NULL,
                      shape->bmin, shape->bmax,
                      shape->cmin, shape->cmax);
        return;
    }

    rt_si32 skip = 0;

    /* run through custom clippers list */
    for (; elm != RT_NULL; elm = elm->next)
    {
        rt_Object *obj = elm->temp == RT_NULL ? RT_NULL :
                         (rt_Object *)((rt_BOUND *)elm->temp)->obj;

        /* skip clip accum segments in the list */
        if (obj == RT_NULL)
        {
            skip = 1 - skip;
        }

        if (obj == RT_NULL || skip == 1
        ||  RT_IS_ARRAY(obj)
        ||  RT_IS_PLANE(obj)
        ||  obj->trnode != trnode
        ||  elm->data != RT_REL_MINUS_OUTER)
        {
            continue;
        }

        /* update surface's changed status from clippers */
        srf_changed |= obj->obj_changed;
    }

    if (srf_changed == 0)
    {
        return;
    }

    /* first calculate only bbox based on 
     * original axis clippers and surface shape */
    recalc_minmax(RT_NULL,     RT_NULL,
                  shape->bmin, shape->bmax,
                  RT_NULL,     RT_NULL);

    /* prepare cbox as temporary storage
     * for bbox adjustments by custom clippers */
    RT_VEC3_SET_VAL1(shape->cmin, -RT_INF);
    RT_VEC3_SET_VAL1(shape->cmax, +RT_INF);

    /* reinit custom clippers list */
    elm = (rt_ELEM *)s_srf->msc_p[2];

    skip = 0;

    /* run through custom clippers list */
    for (; elm != RT_NULL; elm = elm->next)
    {
        rt_Object *obj = elm->temp == RT_NULL ? RT_NULL :
                         (rt_Object *)((rt_BOUND *)elm->temp)->obj;

        /* skip clip accum segments in the list */
        if (obj == RT_NULL)
        {
            skip = 1 - skip;
        }

        if (obj == RT_NULL || skip == 1
        ||  RT_IS_ARRAY(obj)
        ||  RT_IS_PLANE(obj)
        ||  obj->trnode != trnode
        ||  elm->data != RT_REL_MINUS_OUTER)
        {
            continue;
        }

        rt_Surface *srf = (rt_Surface *)obj;

        /* accumulate bbox adjustments
         * from individual outer clippers into cbox */
        srf->recalc_minmax(shape->bmin, shape->bmax,
                           RT_NULL,     RT_NULL,
                           shape->cmin, shape->cmax);
    }

    /* apply bbox adjustments accumulated in cbox,
     * calculate final bbox and cbox for the surface */
    recalc_minmax(shape->cmin, shape->cmax,
                  shape->bmin, shape->bmax,
                  shape->cmin, shape->cmax);
}

/*
 * Update bounding box and volume along with related SIMD fields.
 */
rt_void rt_Surface::update_bounds()
{
    update_minmax();

    if (srf_changed == 0)
    {
        return;
    }

    rt_Object *par;

    /* update array's changed status from sub-objects,
     * as the same non-zero value is only set but not checked
     * it is safe to perform this update from multi-threaded phase,
     * the updated value is then checked in the next sequential phase */
    for (par = parent; par != RT_NULL; par = par->parent)
    {
        ((rt_Array *)par)->arr_changed |= srf_changed;
    }

    /* update bvbox's geometry */
    if (bvbox->verts_num != 0)
    {
        update_bbgeom(bvbox);
    }

    s_srf->min_t[RT_X] = shape->cmin[RT_X] == -RT_INF ? 0 : 1;
    s_srf->min_t[RT_Y] = shape->cmin[RT_Y] == -RT_INF ? 0 : 1;
    s_srf->min_t[RT_Z] = shape->cmin[RT_Z] == -RT_INF ? 0 : 1;

    s_srf->max_t[RT_X] = shape->cmax[RT_X] == +RT_INF ? 0 : 1;
    s_srf->max_t[RT_Y] = shape->cmax[RT_Y] == +RT_INF ? 0 : 1;
    s_srf->max_t[RT_Z] = shape->cmax[RT_Z] == +RT_INF ? 0 : 1;

    rt_vec4  zro = {0.0f, 0.0f, 0.0f, 0.0f};
    rt_real *pps = trnode == this ? zro : pos;

    RT_SIMD_SET(s_srf->min_x, shape->bmin[RT_X] - pps[RT_X]);
    RT_SIMD_SET(s_srf->min_y, shape->bmin[RT_Y] - pps[RT_Y]);
    RT_SIMD_SET(s_srf->min_z, shape->bmin[RT_Z] - pps[RT_Z]);

    RT_SIMD_SET(s_srf->max_x, shape->bmax[RT_X] - pps[RT_X]);
    RT_SIMD_SET(s_srf->max_y, shape->bmax[RT_Y] - pps[RT_Y]);
    RT_SIMD_SET(s_srf->max_z, shape->bmax[RT_Z] - pps[RT_Z]);
}

/*
 * Deinitialize surface object.
 */
rt_Surface::~rt_Surface()
{
    delete outer;
    delete inner;
}

/******************************************************************************/
/**********************************   PLANE   *********************************/
/******************************************************************************/

/*
 * Instantiate plane surface object.
 */
rt_Plane::rt_Plane(rt_Registry *rg, rt_Object *parent,
                   rt_OBJECT *obj, rt_si32 ssize) :

    rt_Surface(rg, parent, obj, ssize)
{
    xpl = (rt_PLANE *)obj->obj.pobj;

    /* init surface's bvbox used for tiling, rtgeom and array's bounds */
    if (srf->min[RT_I] != -RT_INF
    &&  srf->max[RT_I] != +RT_INF
    &&  srf->min[RT_J] != -RT_INF
    &&  srf->max[RT_J] != +RT_INF)
    {
        bvbox->verts_num = 4;
        bvbox->verts = (rt_VERT *)
                     rg->alloc(bvbox->verts_num * sizeof(rt_VERT), RT_ALIGN);

        bvbox->edges_num = 4;
        bvbox->edges = (rt_EDGE *)
                     rg->alloc(bvbox->edges_num * sizeof(rt_EDGE), RT_ALIGN);
        memcpy(bvbox->edges, bx_edges, bvbox->edges_num * sizeof(rt_EDGE));

        bvbox->faces_num = 1;
        bvbox->faces = (rt_FACE *)
                     rg->alloc(bvbox->faces_num * sizeof(rt_FACE), RT_ALIGN);
        memcpy(bvbox->faces, bx_faces, bvbox->faces_num * sizeof(rt_FACE));
    }
}

/*
 * Update SIMD and other data fields.
 */
rt_void rt_Plane::update_fields()
{
    if (obj_changed == 0)
    {
        return;
    }

    rt_Surface::update_fields();

    /* apply axis scalers to texturing */

    rt_real asc[2], isc[2];

    asc[RT_U] = scl[mp_i];
    asc[RT_V] = scl[mp_j];

    isc[RT_U] = 1.0f / asc[RT_U];
    isc[RT_V] = 1.0f / asc[RT_V];

    rt_si32 *map;
    rt_real *scl, *pos;
    rt_SIMD_MATERIAL *s_mat;

    map = outer->map;
    scl = outer->scl;
    pos = outer->sd->pos;
    s_mat = outer->s_mat;

    RT_SIMD_SET(s_mat->xscal, scl[RT_X] * isc[map[RT_X]]);
    RT_SIMD_SET(s_mat->yscal, scl[RT_Y] * isc[map[RT_Y]]);

    RT_SIMD_SET(s_mat->xoffs, pos[map[RT_X]] * asc[map[RT_X]]);
    RT_SIMD_SET(s_mat->yoffs, pos[map[RT_Y]] * asc[map[RT_Y]]);

    map = inner->map;
    scl = inner->scl;
    pos = inner->sd->pos;
    s_mat = inner->s_mat;

    RT_SIMD_SET(s_mat->xscal, scl[RT_X] * isc[map[RT_X]]);
    RT_SIMD_SET(s_mat->yscal, scl[RT_Y] * isc[map[RT_Y]]);

    RT_SIMD_SET(s_mat->xoffs, pos[map[RT_X]] * asc[map[RT_X]]);
    RT_SIMD_SET(s_mat->yoffs, pos[map[RT_Y]] * asc[map[RT_Y]]);

    /* set surface shape */

    RT_VEC3_SET_VAL1(shape->sci, 0.0f);
    shape->sci[RT_W] = 0.0f;

    RT_VEC3_SET_VAL1(shape->scj, 0.0f);
    shape->scj[RT_W] = 0.0f;

    RT_VEC3_SET_VAL1(shape->sck, 0.0f);
    shape->sck[RT_W] = 0.0f;

    shape->sck[mp_k] = (rt_real)sgn[RT_K];
}

/*
 * Adjust local space bounding and clipping boxes according to surface shape.
 */
rt_void rt_Plane::adjust_minmax(rt_vec4 smin, rt_vec4 smax, /* src */
                                rt_vec4 bmin, rt_vec4 bmax, /* bbox */
                                rt_vec4 cmin, rt_vec4 cmax) /* cbox */
{
    rt_Surface::adjust_minmax(smin, smax, bmin, bmax, cmin, cmax);

    rt_bool cb = RT_FALSE; /* distinguish self-adjust from clip-adjust */

    if (cmin != RT_NULL && cmax != RT_NULL)
    {
        cmin[RT_K] = -RT_INF;

        cmax[RT_K] = +RT_INF;

        cb = RT_TRUE; /* self-adjust if cbox is passed */
    }

    if (bmin != RT_NULL && bmax != RT_NULL)
    {
        bmin[RT_I] = smin[RT_I];
        bmin[RT_J] = smin[RT_J];
        bmin[RT_K] = 0.0f;

        bmax[RT_I] = smax[RT_I];
        bmax[RT_J] = smax[RT_J];
        bmax[RT_K] = 0.0f;
    }
}

/*
 * Deinitialize plane surface object.
 */
rt_Plane::~rt_Plane()
{

}

/******************************************************************************/
/*********************************   QUADRIC   ********************************/
/******************************************************************************/

/*
 * Instantiate quadric surface object.
 */
rt_Quadric::rt_Quadric(rt_Registry *rg, rt_Object *parent,
                       rt_OBJECT *obj, rt_si32 ssize) :

    rt_Surface(rg, parent, obj, ssize)
{

}

/*
 * Update SIMD and other data fields.
 */
rt_void rt_Quadric::update_fields()
{
    if (obj_changed == 0)
    {
        return;
    }

    rt_Surface::update_fields();

    RT_VEC3_SET_VAL1(shape->sci, 1.0f);
    shape->sci[RT_W] = 0.0f;

    RT_VEC3_SET_VAL1(shape->scj, 0.0f);
    shape->scj[RT_W] = 0.0f;

    RT_VEC3_SET_VAL1(shape->sck, 0.0f);
    shape->sck[RT_W] = 0.0f;
}

/*
 * Commit SIMD fields after update in sub-classes.
 */
rt_void rt_Quadric::commit_fields()
{
    if (obj_changed == 0)
    {
        return;
    }

    rt_vec4 isc;

    isc[RT_X] = 1.0f / scl[RT_X];
    isc[RT_Y] = 1.0f / scl[RT_Y];
    isc[RT_Z] = 1.0f / scl[RT_Z];

    shape->sci[RT_X] *= isc[RT_X] * isc[RT_X];
    shape->sci[RT_Y] *= isc[RT_Y] * isc[RT_Y];
    shape->sci[RT_Z] *= isc[RT_Z] * isc[RT_Z];

    shape->scj[RT_X] *= isc[RT_X];
    shape->scj[RT_Y] *= isc[RT_Y];
    shape->scj[RT_Z] *= isc[RT_Z];

    RT_SIMD_SET(s_srf->sci_x, shape->sci[RT_X]);
    RT_SIMD_SET(s_srf->sci_y, shape->sci[RT_Y]);
    RT_SIMD_SET(s_srf->sci_z, shape->sci[RT_Z]);
    RT_SIMD_SET(s_srf->sci_w, shape->sci[RT_W]);

    RT_SIMD_SET(s_srf->scj_x, shape->scj[RT_X] * 0.5f);
    RT_SIMD_SET(s_srf->scj_y, shape->scj[RT_Y] * 0.5f);
    RT_SIMD_SET(s_srf->scj_z, shape->scj[RT_Z] * 0.5f);
}

/*
 * Adjust local space bounding and clipping boxes according to surface shape.
 */
rt_void rt_Quadric::adjust_minmax(rt_vec4 smin, rt_vec4 smax, /* src */
                                  rt_vec4 bmin, rt_vec4 bmax, /* bbox */
                                  rt_vec4 cmin, rt_vec4 cmax) /* cbox */
{
    rt_Surface::adjust_minmax(smin, smax, bmin, bmax, cmin, cmax);
}

/*
 * Deinitialize quadric surface object.
 */
rt_Quadric::~rt_Quadric()
{

}

/******************************************************************************/
/********************************   CYLINDER   ********************************/
/******************************************************************************/

/*
 * Instantiate cylinder surface object.
 */
rt_Cylinder::rt_Cylinder(rt_Registry *rg, rt_Object *parent,
                         rt_OBJECT *obj, rt_si32 ssize) :

    rt_Quadric(rg, parent, obj, ssize)
{
    xcl = (rt_CYLINDER *)obj->obj.pobj;

    /* init surface's bvbox used for tiling, rtgeom and array's bounds */
    if (srf->min[RT_K] != -RT_INF
    &&  srf->max[RT_K] != +RT_INF)
    {
        bvbox->verts_num = 8;
        bvbox->verts = (rt_VERT *)
                     rg->alloc(bvbox->verts_num * sizeof(rt_VERT), RT_ALIGN);

        bvbox->edges_num = RT_ARR_SIZE(bx_edges);
        bvbox->edges = (rt_EDGE *)
                     rg->alloc(bvbox->edges_num * sizeof(rt_EDGE), RT_ALIGN);
        memcpy(bvbox->edges, bx_edges, bvbox->edges_num * sizeof(rt_EDGE));

        bvbox->faces_num = RT_ARR_SIZE(bx_faces);
        bvbox->faces = (rt_FACE *)
                     rg->alloc(bvbox->faces_num * sizeof(rt_FACE), RT_ALIGN);
        memcpy(bvbox->faces, bx_faces, bvbox->faces_num * sizeof(rt_FACE));
    }
}

/*
 * Update SIMD and other data fields.
 */
rt_void rt_Cylinder::update_fields()
{
    if (obj_changed == 0)
    {
        return;
    }

    rt_Quadric::update_fields();

    shape->sci[mp_k] = 0.0f;
    shape->sci[RT_W] = xcl->rad * xcl->rad;

    rt_Quadric::commit_fields();
}

/*
 * Adjust local space bounding and clipping boxes according to surface shape.
 */
rt_void rt_Cylinder::adjust_minmax(rt_vec4 smin, rt_vec4 smax, /* src */
                                   rt_vec4 bmin, rt_vec4 bmax, /* bbox */
                                   rt_vec4 cmin, rt_vec4 cmax) /* cbox */
{
    rt_Quadric::adjust_minmax(smin, smax, bmin, bmax, cmin, cmax);

    rt_real rad = RT_FABS(xcl->rad);

    rt_bool cb = RT_FALSE; /* distinguish self-adjust from clip-adjust */

    if (cmin != RT_NULL && cmax != RT_NULL)
    {
        cmin[RT_I] = cmin[RT_I] <= -rad ? -RT_INF : cmin[RT_I];
        cmin[RT_J] = cmin[RT_J] <= -rad ? -RT_INF : cmin[RT_J];

        cmax[RT_I] = cmax[RT_I] >= +rad ? +RT_INF : cmax[RT_I];
        cmax[RT_J] = cmax[RT_J] >= +rad ? +RT_INF : cmax[RT_J];

        cb = RT_TRUE; /* self-adjust if cbox is passed */
    }

    if (bmin != RT_NULL && bmax != RT_NULL)
    {
        bmin[RT_I] = RT_MAX(smin[RT_I], -rad);
        bmin[RT_J] = RT_MAX(smin[RT_J], -rad);
        bmin[RT_K] = smin[RT_K];

        bmax[RT_I] = RT_MIN(smax[RT_I], +rad);
        bmax[RT_J] = RT_MIN(smax[RT_J], +rad);
        bmax[RT_K] = smax[RT_K];
    }
}

/*
 * Deinitialize cylinder surface object.
 */
rt_Cylinder::~rt_Cylinder()
{

}

/******************************************************************************/
/*********************************   SPHERE   *********************************/
/******************************************************************************/

/*
 * Instantiate sphere surface object.
 */
rt_Sphere::rt_Sphere(rt_Registry *rg, rt_Object *parent,
                     rt_OBJECT *obj, rt_si32 ssize) :

    rt_Quadric(rg, parent, obj, ssize)
{
    xsp = (rt_SPHERE *)obj->obj.pobj;

    /* init surface's bvbox used for tiling, rtgeom and array's bounds */
    if (RT_TRUE)
    {
        bvbox->verts_num = 8;
        bvbox->verts = (rt_VERT *)
                     rg->alloc(bvbox->verts_num * sizeof(rt_VERT), RT_ALIGN);

        bvbox->edges_num = RT_ARR_SIZE(bx_edges);
        bvbox->edges = (rt_EDGE *)
                     rg->alloc(bvbox->edges_num * sizeof(rt_EDGE), RT_ALIGN);
        memcpy(bvbox->edges, bx_edges, bvbox->edges_num * sizeof(rt_EDGE));

        bvbox->faces_num = RT_ARR_SIZE(bx_faces);
        bvbox->faces = (rt_FACE *)
                     rg->alloc(bvbox->faces_num * sizeof(rt_FACE), RT_ALIGN);
        memcpy(bvbox->faces, bx_faces, bvbox->faces_num * sizeof(rt_FACE));
    }
}

/*
 * Update SIMD and other data fields.
 */
rt_void rt_Sphere::update_fields()
{
    if (obj_changed == 0)
    {
        return;
    }

    rt_Quadric::update_fields();

    shape->sci[RT_W] = xsp->rad * xsp->rad;

    rt_Quadric::commit_fields();
}

/*
 * Adjust local space bounding and clipping boxes according to surface shape.
 */
rt_void rt_Sphere::adjust_minmax(rt_vec4 smin, rt_vec4 smax, /* src */
                                 rt_vec4 bmin, rt_vec4 bmax, /* bbox */
                                 rt_vec4 cmin, rt_vec4 cmax) /* cbox */
{
    rt_Quadric::adjust_minmax(smin, smax, bmin, bmax, cmin, cmax);

    rt_real top, r = RT_FABS(xsp->rad);
    rt_real rad[3] = {r, r, r};
    rt_si32 i, j, k;

    for (k = 0; k < 3; k++)
    {
        top = smin[k] > 0.0f ? +smin[k] : smax[k] < 0.0f ? -smax[k] : 0.0f;
        r = RT_SQRT(RT_MAX(xsp->rad * xsp->rad - top * top, 0.0f));

        i = (k + 1) % 3;
        if (rad[i] > r)
        {
            rad[i] = r;
        }

        j = (k + 2) % 3;
        if (rad[j] > r)
        {
            rad[j] = r;
        }
    }

    rt_bool cb = RT_FALSE; /* distinguish self-adjust from clip-adjust */

    if (cmin != RT_NULL && cmax != RT_NULL)
    {
        cmin[RT_I] = cmin[RT_I] <= -rad[RT_I] ? -RT_INF : cmin[RT_I];
        cmin[RT_J] = cmin[RT_J] <= -rad[RT_J] ? -RT_INF : cmin[RT_J];
        cmin[RT_K] = cmin[RT_K] <= -rad[RT_K] ? -RT_INF : cmin[RT_K];

        cmax[RT_I] = cmax[RT_I] >= +rad[RT_I] ? +RT_INF : cmax[RT_I];
        cmax[RT_J] = cmax[RT_J] >= +rad[RT_J] ? +RT_INF : cmax[RT_J];
        cmax[RT_K] = cmax[RT_K] >= +rad[RT_K] ? +RT_INF : cmax[RT_K];

        cb = RT_TRUE; /* self-adjust if cbox is passed */
    }

    if (bmin != RT_NULL && bmax != RT_NULL)
    {
        bmin[RT_I] = RT_MAX(smin[RT_I], -rad[RT_I]);
        bmin[RT_J] = RT_MAX(smin[RT_J], -rad[RT_J]);
        bmin[RT_K] = RT_MAX(smin[RT_K], -rad[RT_K]);

        bmax[RT_I] = RT_MIN(smax[RT_I], +rad[RT_I]);
        bmax[RT_J] = RT_MIN(smax[RT_J], +rad[RT_J]);
        bmax[RT_K] = RT_MIN(smax[RT_K], +rad[RT_K]);
    }
}

/*
 * Deinitialize sphere surface object.
 */
rt_Sphere::~rt_Sphere()
{

}

/******************************************************************************/
/**********************************   CONE   **********************************/
/******************************************************************************/

/*
 * Instantiate cone surface object.
 */
rt_Cone::rt_Cone(rt_Registry *rg, rt_Object *parent,
                 rt_OBJECT *obj, rt_si32 ssize) :

    rt_Quadric(rg, parent, obj, ssize)
{
    xcn = (rt_CONE *)obj->obj.pobj;

    /* init surface's bvbox used for tiling, rtgeom and array's bounds */
    if ((srf->min[RT_I] != -RT_INF
    &&   srf->max[RT_I] != +RT_INF
    &&   srf->min[RT_J] != -RT_INF
    &&   srf->max[RT_J] != +RT_INF)
    ||  (srf->min[RT_K] != -RT_INF
    &&   srf->max[RT_K] != +RT_INF))
    {
        bvbox->verts_num = 8;
        bvbox->verts = (rt_VERT *)
                     rg->alloc(bvbox->verts_num * sizeof(rt_VERT), RT_ALIGN);

        bvbox->edges_num = RT_ARR_SIZE(bx_edges);
        bvbox->edges = (rt_EDGE *)
                     rg->alloc(bvbox->edges_num * sizeof(rt_EDGE), RT_ALIGN);
        memcpy(bvbox->edges, bx_edges, bvbox->edges_num * sizeof(rt_EDGE));

        bvbox->faces_num = RT_ARR_SIZE(bx_faces);
        bvbox->faces = (rt_FACE *)
                     rg->alloc(bvbox->faces_num * sizeof(rt_FACE), RT_ALIGN);
        memcpy(bvbox->faces, bx_faces, bvbox->faces_num * sizeof(rt_FACE));
    }
}

/*
 * Update SIMD and other data fields.
 */
rt_void rt_Cone::update_fields()
{
    if (obj_changed == 0)
    {
        return;
    }

    rt_Quadric::update_fields();

    shape->sci[mp_k] = -(xcn->rat * xcn->rat);

    rt_Quadric::commit_fields();
}

/*
 * Adjust local space bounding and clipping boxes according to surface shape.
 */
rt_void rt_Cone::adjust_minmax(rt_vec4 smin, rt_vec4 smax, /* src */
                               rt_vec4 bmin, rt_vec4 bmax, /* bbox */
                               rt_vec4 cmin, rt_vec4 cmax) /* cbox */
{
    rt_Quadric::adjust_minmax(smin, smax, bmin, bmax, cmin, cmax);

    rt_real rat = RT_FABS(xcn->rat);
    rt_real top = RT_MAX(RT_FABS(smin[RT_K]), RT_FABS(smax[RT_K]));
    rt_real rad = top != RT_INF ? top * rat : RT_INF;

    rt_real mxi = RT_MAX(RT_FABS(smin[RT_I]), RT_FABS(smax[RT_I]));
    rt_real mxj = RT_MAX(RT_FABS(smin[RT_J]), RT_FABS(smax[RT_J]));
            top = RT_MIN(mxi != RT_INF && mxj != RT_INF ?
                  RT_SQRT(mxi * mxi + mxj * mxj) / rat : top, top);

    rt_bool cb = RT_FALSE; /* distinguish self-adjust from clip-adjust */

    if (cmin != RT_NULL && cmax != RT_NULL)
    {
        cmin[RT_I] = cmin[RT_I] <= -rad ? -RT_INF : cmin[RT_I];
        cmin[RT_J] = cmin[RT_J] <= -rad ? -RT_INF : cmin[RT_J];
        cmin[RT_K] = cmin[RT_K] <  -top ? -RT_INF : cmin[RT_K];

        cmax[RT_I] = cmax[RT_I] >= +rad ? +RT_INF : cmax[RT_I];
        cmax[RT_J] = cmax[RT_J] >= +rad ? +RT_INF : cmax[RT_J];
        cmax[RT_K] = cmax[RT_K] >  +top ? +RT_INF : cmax[RT_K];

        cb = RT_TRUE; /* self-adjust if cbox is passed */
    }

    if (bmin != RT_NULL && bmax != RT_NULL)
    {
        bmin[RT_I] = RT_MAX(smin[RT_I], -rad);
        bmin[RT_J] = RT_MAX(smin[RT_J], -rad);
        bmin[RT_K] = cb == RT_TRUE ? RT_MAX(smin[RT_K], -top) : smin[RT_K];

        bmax[RT_I] = RT_MIN(smax[RT_I], +rad);
        bmax[RT_J] = RT_MIN(smax[RT_J], +rad);
        bmax[RT_K] = cb == RT_TRUE ? RT_MIN(smax[RT_K], +top) : smax[RT_K];
    }
}

/*
 * Deinitialize cone surface object.
 */
rt_Cone::~rt_Cone()
{

}

/******************************************************************************/
/*******************************   PARABOLOID   *******************************/
/******************************************************************************/

/*
 * Instantiate paraboloid surface object.
 */
rt_Paraboloid::rt_Paraboloid(rt_Registry *rg, rt_Object *parent,
                             rt_OBJECT *obj, rt_si32 ssize) :

    rt_Quadric(rg, parent, obj, ssize)
{
    xpb = (rt_PARABOLOID *)obj->obj.pobj;

    /* init surface's bvbox used for tiling, rtgeom and array's bounds */
    if ((srf->min[RT_I] != -RT_INF
    &&   srf->max[RT_I] != +RT_INF
    &&   srf->min[RT_J] != -RT_INF
    &&   srf->max[RT_J] != +RT_INF)
    ||  (srf->min[RT_K] != -RT_INF && xpb->par < 0.0f)
    ||  (srf->max[RT_K] != +RT_INF && xpb->par > 0.0f))
    {
        bvbox->verts_num = 8;
        bvbox->verts = (rt_VERT *)
                     rg->alloc(bvbox->verts_num * sizeof(rt_VERT), RT_ALIGN);

        bvbox->edges_num = RT_ARR_SIZE(bx_edges);
        bvbox->edges = (rt_EDGE *)
                     rg->alloc(bvbox->edges_num * sizeof(rt_EDGE), RT_ALIGN);
        memcpy(bvbox->edges, bx_edges, bvbox->edges_num * sizeof(rt_EDGE));

        bvbox->faces_num = RT_ARR_SIZE(bx_faces);
        bvbox->faces = (rt_FACE *)
                     rg->alloc(bvbox->faces_num * sizeof(rt_FACE), RT_ALIGN);
        memcpy(bvbox->faces, bx_faces, bvbox->faces_num * sizeof(rt_FACE));
    }
}

/*
 * Update SIMD and other data fields.
 */
rt_void rt_Paraboloid::update_fields()
{
    if (obj_changed == 0)
    {
        return;
    }

    rt_Quadric::update_fields();

    shape->sci[mp_k] = 0.0f;
    shape->scj[mp_k] = xpb->par * (rt_real)sgn[RT_K];

    rt_Quadric::commit_fields();
}

/*
 * Adjust local space bounding and clipping boxes according to surface shape.
 */
rt_void rt_Paraboloid::adjust_minmax(rt_vec4 smin, rt_vec4 smax, /* src */
                                     rt_vec4 bmin, rt_vec4 bmax, /* bbox */
                                     rt_vec4 cmin, rt_vec4 cmax) /* cbox */
{
    rt_Quadric::adjust_minmax(smin, smax, bmin, bmax, cmin, cmax);

    rt_real par = xpb->par;
    rt_real top = RT_MAX(par < 0.0f ? -smin[RT_K] : +smax[RT_K], 0.0f);
    rt_real rad = top != RT_INF ? RT_SQRT(top * RT_FABS(par)) : RT_INF;

    rt_real mxi = RT_MAX(RT_FABS(smin[RT_I]), RT_FABS(smax[RT_I]));
    rt_real mxj = RT_MAX(RT_FABS(smin[RT_J]), RT_FABS(smax[RT_J]));
            top = RT_MIN(mxi != RT_INF && mxj != RT_INF ?
                  (mxi * mxi + mxj * mxj) / RT_FABS(par) : top, top);

    rt_bool cb = RT_FALSE; /* distinguish self-adjust from clip-adjust */

    if (cmin != RT_NULL && cmax != RT_NULL)
    {
        cmin[RT_I] = cmin[RT_I] <= -rad ? -RT_INF : cmin[RT_I];
        cmin[RT_J] = cmin[RT_J] <= -rad ? -RT_INF : cmin[RT_J];
        cmin[RT_K] = (cmin[RT_K] <= 0.0f && par > 0.0f) ||
                     (cmin[RT_K] <  -top && par < 0.0f)
                                        ? -RT_INF : cmin[RT_K];

        cmax[RT_I] = cmax[RT_I] >= +rad ? +RT_INF : cmax[RT_I];
        cmax[RT_J] = cmax[RT_J] >= +rad ? +RT_INF : cmax[RT_J];
        cmax[RT_K] = (cmax[RT_K] >= 0.0f && par < 0.0f) ||
                     (cmax[RT_K] >  +top && par > 0.0f)
                                        ? +RT_INF : cmax[RT_K];

        cb = RT_TRUE; /* self-adjust if cbox is passed */
    }

    if (bmin != RT_NULL && bmax != RT_NULL)
    {
        bmin[RT_I] = RT_MAX(smin[RT_I], -rad);
        bmin[RT_J] = RT_MAX(smin[RT_J], -rad);
        bmin[RT_K] = par > 0.0f ?
                     RT_MAX(smin[RT_K], 0.0f) :
                     cb == RT_TRUE ?
                     RT_MAX(smin[RT_K], -top) : smin[RT_K];

        bmax[RT_I] = RT_MIN(smax[RT_I], +rad);
        bmax[RT_J] = RT_MIN(smax[RT_J], +rad);
        bmax[RT_K] = par < 0.0f ?
                     RT_MIN(smax[RT_K], 0.0f) :
                     cb == RT_TRUE ?
                     RT_MIN(smax[RT_K], +top) : smax[RT_K];
    }
}

/*
 * Deinitialize paraboloid surface object.
 */
rt_Paraboloid::~rt_Paraboloid()
{

}

/******************************************************************************/
/*******************************   HYPERBOLOID   ******************************/
/******************************************************************************/

/*
 * Instantiate hyperboloid surface object.
 */
rt_Hyperboloid::rt_Hyperboloid(rt_Registry *rg, rt_Object *parent,
                               rt_OBJECT *obj, rt_si32 ssize) :

    rt_Quadric(rg, parent, obj, ssize)
{
    xhb = (rt_HYPERBOLOID *)obj->obj.pobj;

    /* init surface's bvbox used for tiling, rtgeom and array's bounds */
    if ((srf->min[RT_I] != -RT_INF
    &&   srf->max[RT_I] != +RT_INF
    &&   srf->min[RT_J] != -RT_INF
    &&   srf->max[RT_J] != +RT_INF)
    ||  (srf->min[RT_K] != -RT_INF
    &&   srf->max[RT_K] != +RT_INF))
    {
        bvbox->verts_num = 8;
        bvbox->verts = (rt_VERT *)
                     rg->alloc(bvbox->verts_num * sizeof(rt_VERT), RT_ALIGN);

        bvbox->edges_num = RT_ARR_SIZE(bx_edges);
        bvbox->edges = (rt_EDGE *)
                     rg->alloc(bvbox->edges_num * sizeof(rt_EDGE), RT_ALIGN);
        memcpy(bvbox->edges, bx_edges, bvbox->edges_num * sizeof(rt_EDGE));

        bvbox->faces_num = RT_ARR_SIZE(bx_faces);
        bvbox->faces = (rt_FACE *)
                     rg->alloc(bvbox->faces_num * sizeof(rt_FACE), RT_ALIGN);
        memcpy(bvbox->faces, bx_faces, bvbox->faces_num * sizeof(rt_FACE));
    }
}

/*
 * Update SIMD and other data fields.
 */
rt_void rt_Hyperboloid::update_fields()
{
    if (obj_changed == 0)
    {
        return;
    }

    rt_Quadric::update_fields();

    shape->sci[mp_k] = -(xhb->rat * xhb->rat);
    shape->sci[RT_W] = xhb->hyp;

    rt_Quadric::commit_fields();
}

/*
 * Adjust local space bounding and clipping boxes according to surface shape.
 */
rt_void rt_Hyperboloid::adjust_minmax(rt_vec4 smin, rt_vec4 smax, /* src */
                                      rt_vec4 bmin, rt_vec4 bmax, /* bbox */
                                      rt_vec4 cmin, rt_vec4 cmax) /* cbox */
{
    rt_Quadric::adjust_minmax(smin, smax, bmin, bmax, cmin, cmax);

    rt_real rat = RT_FABS(xhb->rat);
    rt_real hyp = xhb->hyp;
    rt_real top = RT_MAX(RT_FABS(smin[RT_K]), RT_FABS(smax[RT_K]));
    rt_real rad = top != RT_INF ? RT_SQRT(top * top * rat * rat + hyp) : RT_INF;

    rt_real mxi = RT_MAX(RT_FABS(smin[RT_I]), RT_FABS(smax[RT_I]));
    rt_real mxj = RT_MAX(RT_FABS(smin[RT_J]), RT_FABS(smax[RT_J]));
            top = RT_MIN(mxi != RT_INF && mxj != RT_INF ?
                  RT_SQRT(mxi * mxi + mxj * mxj - hyp) / rat : top, top);

    rt_bool cb = RT_FALSE; /* distinguish self-adjust from clip-adjust */

    if (cmin != RT_NULL && cmax != RT_NULL)
    {
        cmin[RT_I] = cmin[RT_I] <= -rad ? -RT_INF : cmin[RT_I];
        cmin[RT_J] = cmin[RT_J] <= -rad ? -RT_INF : cmin[RT_J];
        cmin[RT_K] = cmin[RT_K] <  -top ? -RT_INF : cmin[RT_K];

        cmax[RT_I] = cmax[RT_I] >= +rad ? +RT_INF : cmax[RT_I];
        cmax[RT_J] = cmax[RT_J] >= +rad ? +RT_INF : cmax[RT_J];
        cmax[RT_K] = cmax[RT_K] >  +top ? +RT_INF : cmax[RT_K];

        cb = RT_TRUE; /* self-adjust if cbox is passed */
    }

    if (bmin != RT_NULL && bmax != RT_NULL)
    {
        bmin[RT_I] = RT_MAX(smin[RT_I], -rad);
        bmin[RT_J] = RT_MAX(smin[RT_J], -rad);
        bmin[RT_K] = cb == RT_TRUE ? RT_MAX(smin[RT_K], -top) : smin[RT_K];

        bmax[RT_I] = RT_MIN(smax[RT_I], +rad);
        bmax[RT_J] = RT_MIN(smax[RT_J], +rad);
        bmax[RT_K] = cb == RT_TRUE ? RT_MIN(smax[RT_K], +top) : smax[RT_K];
    }
}

/*
 * Deinitialize hyperboloid surface object.
 */
rt_Hyperboloid::~rt_Hyperboloid()
{

}

/******************************************************************************/
/******************************   PARACYLINDER   ******************************/
/******************************************************************************/

/*
 * Instantiate paracylinder surface object.
 */
rt_ParaCylinder::rt_ParaCylinder(rt_Registry *rg, rt_Object *parent,
                                 rt_OBJECT *obj, rt_si32 ssize) :

    rt_Quadric(rg, parent, obj, ssize)
{
    xpc = (rt_PARACYLINDER *)obj->obj.pobj;

    /* init surface's bvbox used for tiling, rtgeom and array's bounds */
    if ((srf->min[RT_J] != -RT_INF
    &&   srf->max[RT_J] != +RT_INF)
    && ((srf->min[RT_I] != -RT_INF
    &&   srf->max[RT_I] != +RT_INF)
    ||  (srf->min[RT_K] != -RT_INF && xpc->par < 0.0f)
    ||  (srf->max[RT_K] != +RT_INF && xpc->par > 0.0f)))
    {
        bvbox->verts_num = 8;
        bvbox->verts = (rt_VERT *)
                     rg->alloc(bvbox->verts_num * sizeof(rt_VERT), RT_ALIGN);

        bvbox->edges_num = RT_ARR_SIZE(bx_edges);
        bvbox->edges = (rt_EDGE *)
                     rg->alloc(bvbox->edges_num * sizeof(rt_EDGE), RT_ALIGN);
        memcpy(bvbox->edges, bx_edges, bvbox->edges_num * sizeof(rt_EDGE));

        bvbox->faces_num = RT_ARR_SIZE(bx_faces);
        bvbox->faces = (rt_FACE *)
                     rg->alloc(bvbox->faces_num * sizeof(rt_FACE), RT_ALIGN);
        memcpy(bvbox->faces, bx_faces, bvbox->faces_num * sizeof(rt_FACE));
    }
}

/*
 * Update SIMD and other data fields.
 */
rt_void rt_ParaCylinder::update_fields()
{
    if (obj_changed == 0)
    {
        return;
    }

    rt_Quadric::update_fields();

    shape->sci[mp_j] = 0.0f;
    shape->sci[mp_k] = 0.0f;
    shape->scj[mp_k] = xpc->par * (rt_real)sgn[RT_K];

    rt_Quadric::commit_fields();
}

/*
 * Adjust local space bounding and clipping boxes according to surface shape.
 */
rt_void rt_ParaCylinder::adjust_minmax(rt_vec4 smin, rt_vec4 smax, /* src */
                                       rt_vec4 bmin, rt_vec4 bmax, /* bbox */
                                       rt_vec4 cmin, rt_vec4 cmax) /* cbox */
{
    rt_Quadric::adjust_minmax(smin, smax, bmin, bmax, cmin, cmax);

    rt_real par = xpc->par;
    rt_real top = RT_MAX(par < 0.0f ? -smin[RT_K] : +smax[RT_K], 0.0f);
    rt_real rad = top != RT_INF ? RT_SQRT(top * RT_FABS(par)) : RT_INF;

    rt_real mxi = RT_MAX(RT_FABS(smin[RT_I]), RT_FABS(smax[RT_I]));
            top = RT_MIN(mxi != RT_INF ?
                  (mxi * mxi) / RT_FABS(par) : top, top);

    rt_bool cb = RT_FALSE; /* distinguish self-adjust from clip-adjust */

    if (cmin != RT_NULL && cmax != RT_NULL)
    {
        cmin[RT_I] = cmin[RT_I] <= -rad ? -RT_INF : cmin[RT_I];
        cmin[RT_K] = (cmin[RT_K] <= 0.0f && par > 0.0f) ||
                     (cmin[RT_K] <  -top && par < 0.0f)
                                        ? -RT_INF : cmin[RT_K];

        cmax[RT_I] = cmax[RT_I] >= +rad ? +RT_INF : cmax[RT_I];
        cmax[RT_K] = (cmax[RT_K] >= 0.0f && par < 0.0f) ||
                     (cmax[RT_K] >  +top && par > 0.0f)
                                        ? +RT_INF : cmax[RT_K];

        cb = RT_TRUE; /* self-adjust if cbox is passed */
    }

    if (bmin != RT_NULL && bmax != RT_NULL)
    {
        bmin[RT_I] = RT_MAX(smin[RT_I], -rad);
        bmin[RT_J] = smin[RT_J];
        bmin[RT_K] = par > 0.0f ?
                     RT_MAX(smin[RT_K], 0.0f) :
                     cb == RT_TRUE ?
                     RT_MAX(smin[RT_K], -top) : smin[RT_K];

        bmax[RT_I] = RT_MIN(smax[RT_I], +rad);
        bmax[RT_J] = smax[RT_J];
        bmax[RT_K] = par < 0.0f ?
                     RT_MIN(smax[RT_K], 0.0f) :
                     cb == RT_TRUE ?
                     RT_MIN(smax[RT_K], +top) : smax[RT_K];
    }
}

/*
 * Deinitialize paracylinder surface object.
 */
rt_ParaCylinder::~rt_ParaCylinder()
{

}

/******************************************************************************/
/******************************   HYPERCYLINDER   *****************************/
/******************************************************************************/

/*
 * Instantiate hypercylinder surface object.
 */
rt_HyperCylinder::rt_HyperCylinder(rt_Registry *rg, rt_Object *parent,
                                   rt_OBJECT *obj, rt_si32 ssize) :

    rt_Quadric(rg, parent, obj, ssize)
{
    xhc = (rt_HYPERCYLINDER *)obj->obj.pobj;

    /* init surface's bvbox used for tiling, rtgeom and array's bounds */
    if ((srf->min[RT_J] != -RT_INF
    &&   srf->max[RT_J] != +RT_INF)
    && ((srf->min[RT_I] != -RT_INF
    &&   srf->max[RT_I] != +RT_INF)
    ||  (srf->min[RT_K] != -RT_INF
    &&   srf->max[RT_K] != +RT_INF)))
    {
        bvbox->verts_num = 8;
        bvbox->verts = (rt_VERT *)
                     rg->alloc(bvbox->verts_num * sizeof(rt_VERT), RT_ALIGN);

        bvbox->edges_num = RT_ARR_SIZE(bx_edges);
        bvbox->edges = (rt_EDGE *)
                     rg->alloc(bvbox->edges_num * sizeof(rt_EDGE), RT_ALIGN);
        memcpy(bvbox->edges, bx_edges, bvbox->edges_num * sizeof(rt_EDGE));

        bvbox->faces_num = RT_ARR_SIZE(bx_faces);
        bvbox->faces = (rt_FACE *)
                     rg->alloc(bvbox->faces_num * sizeof(rt_FACE), RT_ALIGN);
        memcpy(bvbox->faces, bx_faces, bvbox->faces_num * sizeof(rt_FACE));
    }
}

/*
 * Update SIMD and other data fields.
 */
rt_void rt_HyperCylinder::update_fields()
{
    if (obj_changed == 0)
    {
        return;
    }

    rt_Quadric::update_fields();

    shape->sci[mp_j] = 0.0f;
    shape->sci[mp_k] = -(xhc->rat * xhc->rat);
    shape->sci[RT_W] = xhc->hyp;

    rt_Quadric::commit_fields();
}

/*
 * Adjust local space bounding and clipping boxes according to surface shape.
 */
rt_void rt_HyperCylinder::adjust_minmax(rt_vec4 smin, rt_vec4 smax, /* src */
                                        rt_vec4 bmin, rt_vec4 bmax, /* bbox */
                                        rt_vec4 cmin, rt_vec4 cmax) /* cbox */
{
    rt_Quadric::adjust_minmax(smin, smax, bmin, bmax, cmin, cmax);

    rt_real rat = RT_FABS(xhc->rat);
    rt_real hyp = xhc->hyp;
    rt_real top = RT_MAX(RT_FABS(smin[RT_K]), RT_FABS(smax[RT_K]));
    rt_real rad = top != RT_INF ? RT_SQRT(top * top * rat * rat + hyp) : RT_INF;

    rt_real mxi = RT_MAX(RT_FABS(smin[RT_I]), RT_FABS(smax[RT_I]));
            top = RT_MIN(mxi != RT_INF ?
                  RT_SQRT(mxi * mxi - hyp) / rat : top, top);

    rt_bool cb = RT_FALSE; /* distinguish self-adjust from clip-adjust */

    if (cmin != RT_NULL && cmax != RT_NULL)
    {
        cmin[RT_I] = cmin[RT_I] <= -rad ? -RT_INF : cmin[RT_I];
        cmin[RT_K] = cmin[RT_K] <  -top ? -RT_INF : cmin[RT_K];

        cmax[RT_I] = cmax[RT_I] >= +rad ? +RT_INF : cmax[RT_I];
        cmax[RT_K] = cmax[RT_K] >  +top ? +RT_INF : cmax[RT_K];

        cb = RT_TRUE; /* self-adjust if cbox is passed */
    }

    if (bmin != RT_NULL && bmax != RT_NULL)
    {
        bmin[RT_I] = RT_MAX(smin[RT_I], -rad);
        bmin[RT_J] = smin[RT_J];
        bmin[RT_K] = cb == RT_TRUE ? RT_MAX(smin[RT_K], -top) : smin[RT_K];

        bmax[RT_I] = RT_MIN(smax[RT_I], +rad);
        bmax[RT_J] = smax[RT_J];
        bmax[RT_K] = cb == RT_TRUE ? RT_MIN(smax[RT_K], +top) : smax[RT_K];
    }
}

/*
 * Deinitialize hypercylinder surface object.
 */
rt_HyperCylinder::~rt_HyperCylinder()
{

}

/******************************************************************************/
/*****************************   HYPERPARABOLOID   ****************************/
/******************************************************************************/

/*
 * Instantiate hyperparaboloid surface object.
 */
rt_HyperParaboloid::rt_HyperParaboloid(rt_Registry *rg, rt_Object *parent,
                                       rt_OBJECT *obj, rt_si32 ssize) :

    rt_Quadric(rg, parent, obj, ssize)
{
    xhp = (rt_HYPERPARABOLOID *)obj->obj.pobj;

    /* init surface's bvbox used for tiling, rtgeom and array's bounds */
    if (srf->min[RT_I] != -RT_INF
    &&  srf->max[RT_I] != +RT_INF
    &&  srf->min[RT_J] != -RT_INF
    &&  srf->max[RT_J] != +RT_INF)
    {
        bvbox->verts_num = 8;
        bvbox->verts = (rt_VERT *)
                     rg->alloc(bvbox->verts_num * sizeof(rt_VERT), RT_ALIGN);

        bvbox->edges_num = RT_ARR_SIZE(bx_edges);
        bvbox->edges = (rt_EDGE *)
                     rg->alloc(bvbox->edges_num * sizeof(rt_EDGE), RT_ALIGN);
        memcpy(bvbox->edges, bx_edges, bvbox->edges_num * sizeof(rt_EDGE));

        bvbox->faces_num = RT_ARR_SIZE(bx_faces);
        bvbox->faces = (rt_FACE *)
                     rg->alloc(bvbox->faces_num * sizeof(rt_FACE), RT_ALIGN);
        memcpy(bvbox->faces, bx_faces, bvbox->faces_num * sizeof(rt_FACE));
    }
}

/*
 * Update SIMD and other data fields.
 */
rt_void rt_HyperParaboloid::update_fields()
{
    if (obj_changed == 0)
    {
        return;
    }

    rt_Quadric::update_fields();

    shape->sci[mp_i] = 1.0f / +RT_FABS(xhp->pr1);
    shape->sci[mp_j] = 1.0f / -RT_FABS(xhp->pr2);
    shape->sci[mp_k] = 0.0f;
    shape->scj[mp_k] = 1.0f * (rt_real)sgn[RT_K];

    rt_Quadric::commit_fields();
}

/*
 * Adjust local space bounding and clipping boxes according to surface shape.
 */
rt_void rt_HyperParaboloid::adjust_minmax(rt_vec4 smin, rt_vec4 smax, /* src */
                                          rt_vec4 bmin, rt_vec4 bmax, /* bbox */
                                          rt_vec4 cmin, rt_vec4 cmax) /* cbox */
{
    rt_Quadric::adjust_minmax(smin, smax, bmin, bmax, cmin, cmax);

    rt_real rd1 = RT_MAX(-smin[RT_I], +smax[RT_I]);
    rt_real rd2 = RT_MAX(-smin[RT_J], +smax[RT_J]);
    rt_real tp1 = rd1 * rd1 / RT_FABS(xhp->pr1);
    rt_real tp2 = rd2 * rd2 / RT_FABS(xhp->pr2);

    rt_bool cb = RT_FALSE; /* distinguish self-adjust from clip-adjust */

    if (cmin != RT_NULL && cmax != RT_NULL)
    {
        cmin[RT_K] = cmin[RT_K] <= -tp2 ? -RT_INF : cmin[RT_K];

        cmax[RT_K] = cmax[RT_K] >= +tp1 ? +RT_INF : cmax[RT_K];

        cb = RT_TRUE; /* self-adjust if cbox is passed */
    }

    if (bmin != RT_NULL && bmax != RT_NULL)
    {
        bmin[RT_I] = smin[RT_I];
        bmin[RT_J] = smin[RT_J];
        bmin[RT_K] = cb == RT_TRUE ? RT_MAX(smin[RT_K], -tp2) : smin[RT_K];

        bmax[RT_I] = smax[RT_I];
        bmax[RT_J] = smax[RT_J];
        bmax[RT_K] = cb == RT_TRUE ? RT_MIN(smax[RT_K], +tp1) : smax[RT_K];
    }
}

/*
 * Deinitialize hyperparaboloid surface object.
 */
rt_HyperParaboloid::~rt_HyperParaboloid()
{

}

/******************************************************************************/
/********************************   MATERIAL   ********************************/
/******************************************************************************/

/*
 * Allocate texture in custom heap.
 */
rt_pntr rt_Texture::operator new(size_t size, rt_Heap *hp)
{
    return hp->alloc(size, RT_ALIGN);
}

rt_void rt_Texture::operator delete(rt_pntr ptr)
{

}

/*
 * Instantiate texture to keep track of loaded textures.
 */
rt_Texture::rt_Texture(rt_Registry *rg, rt_pstr name) :

    rt_List<rt_Texture>(rg->get_tex())
{
    rg->put_tex(this);

    this->name = name;

    load_image(rg, name, &tex);
}

/*
 * Deinitialize texture.
 */
rt_Texture::~rt_Texture()
{

}

/*
 * Allocate material in custom heap.
 */
rt_pntr rt_Material::operator new(size_t size, rt_Heap *hp)
{
    return hp->alloc(size, RT_ALIGN);
}

rt_void rt_Material::operator delete(rt_pntr ptr)
{

}

/*
 * Instantiate material.
 */
rt_Material::rt_Material(rt_Registry *rg, rt_SIDE *sd, rt_MATERIAL *mat) :

    rt_List<rt_Material>(rg->get_mat())
{
    if (mat == RT_NULL)
    {
        throw rt_Exception("null-pointer in material");
    }

    rg->put_mat(this);

    this->sd  = sd;
    this->mat = mat;

    rt_TEX *tx = &mat->tex;
    otx.x_dim = otx.y_dim = -1;

    /* save original texture data */
    if ((tx->x_dim == 0 && tx->y_dim == 0)
#if (RT_POINTER - RT_ADDRESS) != 0
    || (rt_full)tx->ptex >= (rt_full)(0x80000000 - tx->x_dim * tx->y_dim * 4)
#endif /* (RT_POINTER - RT_ADDRESS) */
       )
    {
        otx = *tx;
    }

    resolve_texture(rg);

    props  = RT_PROP_NORMAL;
    props |= -((rg->opts & RT_OPTS_GAMMA) == 0) & RT_PROP_GAMMA;
    props |= -((rg->opts & RT_OPTS_FRESNEL) == 0) & RT_PROP_FRESNEL;
    props |= mat->tag == RT_MAT_LIGHT ? RT_PROP_LIGHT : 0;
    props |= mat->tag == RT_MAT_METAL ? RT_PROP_METAL : 0;
    props |= mat->prp[1] == 0.0f ? RT_PROP_OPAQUE : 0;
    props |= mat->prp[1] == 1.0f ? RT_PROP_TRANSP : 0;
    props |= tx->x_dim == 1 && tx->y_dim == 1 ? 0 : RT_PROP_TEXTURE;
    props |= mat->prp[0] == 0.0f ? 0 : RT_PROP_REFLECT;
    props |= mat->prp[2] == 1.0f ? 0 : RT_PROP_REFRACT;
    props |= mat->lgt[0] == 0.0f ? 0 : RT_PROP_DIFFUSE;
    props |= mat->lgt[1] == 0.0f ? 0 : RT_PROP_SPECULAR;

    if (mat->prp[0] + mat->prp[1] >= 1.0f)
    {
        props &= ~RT_PROP_DIFFUSE;
        props &= ~RT_PROP_SPECULAR;
    }

    mtx[0][0] = +RT_COSA(sd->rot);
    mtx[0][1] = +RT_SINA(sd->rot);
    mtx[1][0] = -RT_SINA(sd->rot);
    mtx[1][1] = +RT_COSA(sd->rot);

    rt_si32 sgn[2];
    rt_si32 match = 0;

    rt_si32 i, j;

    for (i = 0; i < 2; i++)
    {
        for (j = 0; j < 2; j++)
        {
            if (RT_FABS(this->mtx[i][0]) == iden4[j][0]
            &&  RT_FABS(this->mtx[i][1]) == iden4[j][1])
            {
                map[i] = j;
                sgn[i] = RT_SIGN(this->mtx[i][j]);
                match++;
            }
        }
    }

    if (match < 2)
    {
        map[RT_X] = RT_U;
        sgn[RT_X] = 1;

        map[RT_Y] = RT_V;
        sgn[RT_Y] = 1;
    }

/*  rt_SIMD_MATERIAL */

    s_mat = (rt_SIMD_MATERIAL *)
            rg->alloc(sizeof(rt_SIMD_MATERIAL), RT_SIMD_ALIGN);

    s_mat->t_map[RT_X] = map[RT_X] * RT_SIMD_QUADS * 16;
    s_mat->t_map[RT_Y] = map[RT_Y] * RT_SIMD_QUADS * 16;
    s_mat->t_map[2] = 0;
    s_mat->t_map[3] = 0;

    scl[RT_X] = tx->x_dim / (sd->scl[RT_X] * sgn[RT_X]);
    scl[RT_Y] = tx->y_dim / (sd->scl[RT_Y] * sgn[RT_Y]);

    RT_SIMD_SET(s_mat->xscal, scl[RT_X]);
    RT_SIMD_SET(s_mat->yscal, scl[RT_Y]);

    RT_SIMD_SET(s_mat->xoffs, sd->pos[map[RT_X]]);
    RT_SIMD_SET(s_mat->yoffs, sd->pos[map[RT_Y]]);

    rt_ui32 x_mask = tx->x_dim - 1;
    rt_ui32 y_mask = tx->y_dim - 1;

    RT_SIMD_SET(s_mat->xmask, x_mask);
    RT_SIMD_SET(s_mat->ymask, y_mask);

    rt_si32 x_dim = tx->x_dim;
    rt_si32 x_lg2 = 0;
    while (x_dim >>= 1)
    {
        x_lg2++;
    }

    RT_SIMD_SET(s_mat->yshft, 0);
    s_mat->yshft[0] = x_lg2;

    s_mat->tex_p[0] = tx->ptex;
    RT_SIMD_SET(s_mat->gpc10, (rt_real)RT_PI);
    RT_SIMD_SET(s_mat->clamp, (rt_real)255);
    RT_SIMD_SET(s_mat->cmask, (rt_elem)255);

    rt_real f = 1.0f - (mat->prp[0] + mat->prp[1]);
    f = RT_MAX(f, 0.0f);

    RT_SIMD_SET(s_mat->l_dff, mat->lgt[0] * f);
    RT_SIMD_SET(s_mat->l_spc, mat->lgt[1] * f);
    s_mat->l_pow[0] = (rt_ui32)(mat->lgt[2] * 16.0f); /* fixed-point 28.4-bit */

    RT_SIMD_SET(s_mat->c_rfl, mat->prp[0]);
    RT_SIMD_SET(s_mat->c_trn, mat->prp[1]);
    RT_SIMD_SET(s_mat->c_rfr, mat->prp[2]);
    RT_SIMD_SET(s_mat->rfr_2, mat->prp[2] * mat->prp[2]);
    RT_SIMD_SET(s_mat->c_rcp, 1.0f / mat->prp[2]);
    RT_SIMD_SET(s_mat->ext_2, mat->prp[3] * mat->prp[3]);

    if (mat->prp[1] == 0.0f || mat->prp[1] == 1.0f || mat->prp[2] != 1.0f)
    {
        return;
    }

    RT_SIMD_SET(s_mat->c_rfr, mat->prp[3]);
    RT_SIMD_SET(s_mat->rfr_2, mat->prp[3] * mat->prp[3]);
}

/*
 * Validate texture fields by checking whether
 * texture color was defined in place,
 * texture data needs to be loaded from external file or
 * texture data was bound from local array.
 */
rt_void rt_Material::resolve_texture(rt_Registry *rg)
{
    rt_TEX *tx = &mat->tex;

    /* texture color is defined in place */
    if (tx->x_dim == 0 && tx->y_dim == 0 && tx->ptex == RT_NULL)
    {
        tx->ptex  = &tx->col.val;
        tx->x_dim = 1;
        tx->y_dim = 1;
    }

    /* texture load is requested */
    if (tx->x_dim == 0 && tx->y_dim == 0 && tx->ptex != RT_NULL)
    {
        rt_pstr name = (rt_pstr)tx->ptex;
        rt_Texture *tex = RT_NULL;

        /* traverse list of loaded textures (slow, implement hashmap later)
         * and check if requested texture already exists */
        for (tex = rg->get_tex(); tex != RT_NULL; tex = tex->next)
        {
            if (strcmp(name, tex->name) == 0)
            {
                break;
            }
        }

        if (tex == RT_NULL)
        {
            tex = new(rg) rt_Texture(rg, name);
        }

        *tx = tex->tex;
    }

    /* texture bind doesn't need extra validation
     * except for allowed address range for backend */
#if (RT_POINTER - RT_ADDRESS) != 0 && RT_DEBUG >= 2

    RT_LOGI("TEX_P PTR = %016" PR_Z "X\n", (rt_full)tx->ptex);

#endif /* (RT_POINTER - RT_ADDRESS) && RT_DEBUG */

#if (RT_POINTER - RT_ADDRESS) != 0

    if ((rt_full)tx->ptex >= (rt_full)(0x80000000 - tx->x_dim * tx->y_dim * 4))
    {
        rt_pntr pnew = rg->alloc(tx->x_dim * tx->y_dim * 4, RT_ALIGN);
        memcpy(pnew, tx->ptex, tx->x_dim * tx->y_dim * 4);
        tx->ptex = pnew;
    }

    if ((rt_full)tx->ptex >= (rt_full)(0x80000000 - tx->x_dim * tx->y_dim * 4))
    {
        throw rt_Exception("address exceeded allowed range in material");
    }

#endif /* (RT_POINTER - RT_ADDRESS) */
}

/*
 * Deinitialize material.
 */
rt_Material::~rt_Material()
{
    rt_TEX *tx = &mat->tex;

    /* restore original texture data */
    if (otx.x_dim != -1 && otx.y_dim != -1)
    {
        *tx = otx;
    }
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
