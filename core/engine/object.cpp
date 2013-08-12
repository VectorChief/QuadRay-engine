/******************************************************************************/
/* Copyright (c) 2013 VectorChief (at github, bitbucket, sourceforge)         */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#include <string.h>

#include "object.h"
#include "rtgeom.h"
#include "rtload.h"
#include "system.h"

/******************************************************************************/
/*********************************   OBJECT   *********************************/
/******************************************************************************/

rt_Object::rt_Object(rt_Object *parent, rt_OBJECT *obj)
{
    if (obj == RT_NULL)
    {
        throw rt_Exception("NULL pointer in Object");
    }

    this->obj = obj;
    this->trm = &obj->trm;
    pos = this->mtx[3];
    this->tag = obj->obj.tag;

    this->parent = parent;
}

rt_void rt_Object::add_relation(rt_ELEM *lst)
{

}

rt_void rt_Object::update(rt_long time, rt_mat4 mtx, rt_cell flags)
{
    if (obj->f_anim != RT_NULL)
    {
        obj->f_anim(time, obj->time, trm, RT_NULL);
    }

    obj->time = time;

    rt_mat4 obj_mtx;
    matrix_from_transform(obj_mtx, trm);
    matrix_mul_matrix(this->mtx, mtx, obj_mtx);
}

rt_Object::~rt_Object()
{

}

/******************************************************************************/
/*********************************   CAMERA   *********************************/
/******************************************************************************/

rt_Camera::rt_Camera(rt_Registry *rg, rt_Object *parent, rt_OBJECT *obj) :

    rt_Object(parent, obj),
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

    user_input = 0;
}

rt_void rt_Camera::update(rt_long time, rt_mat4 mtx, rt_cell flags)
{
    rt_Object::update(time, mtx, flags | user_input);

    pov = cam->vpt[0] ? cam->vpt[0] : 1.0f;

    hor_sin = RT_SINA(trm->rot[RT_Z]);
    hor_cos = RT_COSA(trm->rot[RT_Z]);

    ver_sin = RT_SINA(trm->rot[RT_X]);
    ver_cos = RT_COSA(trm->rot[RT_X]);

    user_input = 0;
}

rt_void rt_Camera::update(rt_long time, rt_cell action)
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

    user_input = 1;
}

rt_Camera::~rt_Camera()
{

}

/******************************************************************************/
/**********************************   LIGHT   *********************************/
/******************************************************************************/

rt_Light::rt_Light(rt_Registry *rg, rt_Object *parent, rt_OBJECT *obj) :

    rt_Object(parent, obj),
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

    RT_SIMD_SET(s_lgt->a_qdr, lgt->atn[3]);
    RT_SIMD_SET(s_lgt->a_lnr, lgt->atn[2]);
    RT_SIMD_SET(s_lgt->a_cnt, lgt->atn[1] + 1.0f);
    RT_SIMD_SET(s_lgt->a_rng, lgt->atn[0]);
}

rt_void rt_Light::update(rt_long time, rt_mat4 mtx, rt_cell flags)
{
    rt_Object::update(time, mtx, flags);

    RT_SIMD_SET(s_lgt->pos_x, pos[RT_X]);
    RT_SIMD_SET(s_lgt->pos_y, pos[RT_Y]);
    RT_SIMD_SET(s_lgt->pos_z, pos[RT_Z]);
}

/******************************************************************************/
/**********************************   ARRAY   *********************************/
/******************************************************************************/

#define RT_ACCUM_ENTER  -1
#define RT_ACCUM_LEAVE  +1

rt_Array::rt_Array(rt_Registry *rg, rt_Object *parent, rt_OBJECT *obj) :

    rt_Object(parent, obj)
{
    this->rg = rg;

    obj_num = 0;
    obj_arr = RT_NULL;

    if (obj->obj.tag != RT_TAG_ARRAY)
    {
        return;
    }

    obj_num = obj->obj.obj_num;
    obj_arr = (rt_Object **)rg->alloc(obj_num * sizeof(rt_Object *), RT_ALIGN);

    rt_OBJECT *arr = (rt_OBJECT *)obj->obj.pobj;

    rt_cell i, j; /* j - for skipping unsupported object tags */

    for (i = 0, j = 0; i < obj->obj.obj_num; i++, j++)
    {
        switch (arr[i].obj.tag)
        {
            case RT_TAG_CAMERA:
            obj_arr[j] = new rt_Camera(rg, this, &arr[i]);
            break;

            case RT_TAG_LIGHT:
            obj_arr[j] = new rt_Light(rg, this, &arr[i]);
            break;

            case RT_TAG_ARRAY:
            obj_arr[j] = new rt_Array(rg, this, &arr[i]);
            break;

            case RT_TAG_PLANE:
            obj_arr[j] = new rt_Plane(rg, this, &arr[i]);
            break;

            case RT_TAG_CYLINDER:
            obj_arr[j] = new rt_Cylinder(rg, this, &arr[i]);
            break;

            case RT_TAG_SPHERE:
            obj_arr[j] = new rt_Sphere(rg, this, &arr[i]);
            break;

            case RT_TAG_CONE:
            obj_arr[j] = new rt_Cone(rg, this, &arr[i]);
            break;

            case RT_TAG_PARABOLOID:
            obj_arr[j] = new rt_Paraboloid(rg, this, &arr[i]);
            break;

            case RT_TAG_HYPERBOLOID:
            obj_arr[j] = new rt_Hyperboloid(rg, this, &arr[i]);
            break;

            default:
            j--;
            obj_num--;
            break;
        }
    }

    if (obj->obj.rel_num > 0)
    {
        rt_RELATION *rel = obj->obj.prel;

        rt_ELEM *lst = RT_NULL;
        rt_cell acc  = 0;

        rt_Object **obj_arr_l = obj_arr; /* left  sub-array */
        rt_Object **obj_arr_r = obj_arr; /* right sub-array */

        rt_cell     obj_num_l = obj_num; /* left  sub-array size */
        rt_cell     obj_num_r = obj_num; /* right sub-array size */

        rt_cell i;

        for (i = 0; i < obj->obj.rel_num; i++)
        {
            if (rel[i].obj1 >= obj_num_l
            ||  rel[i].obj2 >= obj_num_r)
            {
                continue;
            }

            rt_ELEM *elm = RT_NULL;

            switch (rel[i].rel)
            {
                case RT_REL_INDEX_ARRAY:
                if (rel[i].obj1 >= 0 && rel[i].obj2 >= -1)
                {
                    rt_Object *obj = (rt_Object *)obj_arr_l[rel[i].obj1];

                    if (RT_IS_ARRAY(obj))
                    {
                        rt_Array *arr = (rt_Array *)obj;
                        obj_arr_l = arr->obj_arr; /* select left  sub-array */
                        obj_num_l = arr->obj_num;   /* for next left  index */
                    }
                }
                if (rel[i].obj1 >= -1 && rel[i].obj2 >= 0)
                {
                    rt_Object *obj = (rt_Object *)obj_arr_r[rel[i].obj2];

                    if (RT_IS_ARRAY(obj))
                    {
                        rt_Array *arr = (rt_Array *)obj;
                        obj_arr_r = arr->obj_arr; /* select right sub-array */
                        obj_num_r = arr->obj_num;   /* for next right index */
                    }
                }
                break;

                case RT_REL_MINUS_INNER:
                case RT_REL_MINUS_OUTER:
                if (rel[i].obj1 == -1 && rel[i].obj2 >= 0 && acc == 0)
                {
                    acc = 1;
                    elm = (rt_ELEM *)rg->alloc(sizeof(rt_ELEM), RT_ALIGN);
                    elm->data = RT_ACCUM_ENTER;
                    elm->simd = RT_NULL;
                    elm->temp = RT_NULL; /* accum marker */
                    elm->next = lst;
                    lst = elm;
                }
                if (rel[i].obj1 >= -1 && rel[i].obj2 >= 0)
                {
                    elm = (rt_ELEM *)rg->alloc(sizeof(rt_ELEM), RT_ALIGN);
                    elm->data = rel[i].rel;
                    elm->simd = RT_NULL;
                    elm->temp = obj_arr_r[rel[i].obj2];
                    elm->next = RT_NULL;
                    obj_arr_r = obj_arr; /* reset right sub-array after use */
                    obj_num_r = obj_num;
                }
                if (rel[i].obj1 == -1 && rel[i].obj2 >= 0)
                {
                    elm->next = lst;
                    lst = elm;
                }
                break;

                case RT_REL_MINUS_ACCUM:
                if (rel[i].obj1 >= 0 && rel[i].obj2 == -1 && acc == 1)
                {
                    acc = 0;
                    elm = (rt_ELEM *)rg->alloc(sizeof(rt_ELEM), RT_ALIGN);
                    elm->data = RT_ACCUM_LEAVE;
                    elm->simd = RT_NULL;
                    elm->temp = RT_NULL; /* accum marker */
                    elm->next = lst;
                    lst = RT_NULL;
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
        }
    }
}

rt_void rt_Array::add_relation(rt_ELEM *lst)
{
    rt_Object::add_relation(lst);

    rt_cell i;

    for (i = 0; i < obj_num; i++)
    {
        obj_arr[i]->add_relation(lst);
    }
}

rt_void rt_Array::update(rt_long time, rt_mat4 mtx, rt_cell flags)
{
    rt_Object::update(time, mtx, flags);

    rt_cell i;

    for (i = 0; i < obj_num; i++)
    {
        obj_arr[i]->update(time, this->mtx, flags);
    }
}

rt_Array::~rt_Array()
{
    rt_cell i;

    for (i = 0; i < obj_num; i++)
    {
        delete obj_arr[i];
    }
}

/******************************************************************************/
/*********************************   SURFACE   ********************************/
/******************************************************************************/

rt_Surface::rt_Surface(rt_Registry *rg, rt_Object *parent,
                       rt_OBJECT *obj, rt_cell ssize) :

    rt_Object(parent, obj),
    rt_List<rt_Surface>(rg->get_srf())
{
    this->rg = rg;

    rg->put_srf(this);

    this->srf = (rt_SURFACE *)obj->obj.pobj;

    this->shift = 0;

/*  rt_SIMD_SURFACE */

    s_srf = (rt_SIMD_SURFACE *)rg->alloc(ssize, RT_SIMD_ALIGN);

    this->outer = new rt_Material(rg, &srf->side_outer,
                    obj->obj.pmat_outer ? obj->obj.pmat_outer :
                                          srf->side_outer.pmat);

    this->inner = new rt_Material(rg, &srf->side_inner,
                    obj->obj.pmat_inner ? obj->obj.pmat_inner :
                                          srf->side_inner.pmat);

    s_srf->mat_p[0] = outer->s_mat;
    s_srf->mat_p[1] = (rt_pntr)outer->props;
    s_srf->mat_p[2] = inner->s_mat;
    s_srf->mat_p[3] = (rt_pntr)inner->props;

    s_srf->srf_p[0] = RT_NULL; /* surf ptr, filled in update0 */
    s_srf->srf_p[1] = RT_NULL; /* reserved */
    s_srf->srf_p[2] = RT_NULL; /* clip ptr, filled in update0 */
    s_srf->srf_p[3] = (rt_pntr)tag; /* tag */

    s_srf->msc_p[0] = RT_NULL; /* reserved */
    s_srf->msc_p[1] = RT_NULL; /* reserved */
    s_srf->msc_p[2] = RT_NULL; /* custom clippers */
    s_srf->msc_p[3] = RT_NULL; /* reserved */

    s_srf->lst_p[0] = RT_NULL; /* outer lights/shadows */
    s_srf->lst_p[1] = RT_NULL; /* outer surfaces for rfl/rfr */
    s_srf->lst_p[2] = RT_NULL; /* inner lights/shadows */
    s_srf->lst_p[3] = RT_NULL; /* inner surfaces for rfl/rfr */

    RT_SIMD_SET(s_srf->sbase, 0x00000000);
    RT_SIMD_SET(s_srf->smask, 0x80000000);

    RT_SIMD_SET(s_srf->c_tmp, 0xFFFFFFFF);
}

rt_void rt_Surface::add_relation(rt_ELEM *lst)
{
    rt_Object::add_relation(lst);

    for (; lst != RT_NULL; lst = lst->next)
    {
        rt_ELEM *elm = RT_NULL;
        rt_cell rel = lst->data;
        rt_Object *obj = (rt_Object *)lst->temp;

        if (obj == RT_NULL)
        {
            elm = (rt_ELEM *)rg->alloc(sizeof(rt_ELEM), RT_ALIGN);
            elm->data = rel;
            elm->simd = RT_NULL; /* accum marker */
            elm->temp = RT_NULL;
            elm->next = (rt_ELEM *)s_srf->msc_p[2];
            s_srf->msc_p[2] = elm;
        }
        else
        if (RT_IS_ARRAY(obj))
        {
            rt_Array *arr = (rt_Array *)obj;
            rt_cell i;

            for (i = 0; i < arr->obj_num; i++)
            {
                elm = (rt_ELEM *)rg->alloc(sizeof(rt_ELEM), RT_ALIGN);
                elm->data = rel;
                elm->simd = RT_NULL;
                elm->temp = arr->obj_arr[i];
                elm->next = RT_NULL;

                add_relation(elm);
            }
        }
        else
        if (RT_IS_SURFACE(obj))
        {
            rt_Surface *srf = (rt_Surface *)obj;

            elm = (rt_ELEM *)rg->alloc(sizeof(rt_ELEM), RT_ALIGN);
            elm->data = rel;
            elm->simd = srf->s_srf;
            elm->temp = srf;
            elm->next = (rt_ELEM *)s_srf->msc_p[2];

            s_srf->msc_p[2] = elm;
        }
    }
}

rt_void rt_Surface::update(rt_long time, rt_mat4 mtx, rt_cell flags)
{
    rt_Object::update(time, mtx, flags);

    rt_cell match = 0;

    rt_cell i, j;

    /* determine axis mapping for trivial transform
     * (multiple of 90 degree rotation, +/-1.0 scalers) */
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
                match++;
            }
        }
    }

    shift = 0;

    /* if object has non-trivial transform
     * all rotation and scaling is already in the matrix,
     * reset axis mapping to identity */
    if (match < 3
    /* axis scaling goes through generic
     * matrix transform for now (slow) */
    ||  scl[RT_X] != 1.0f
    ||  scl[RT_Y] != 1.0f
    ||  scl[RT_Z] != 1.0f)
    {
        shift = 3;

        map[RT_I] = RT_X;
        sgn[RT_I] = 1;
        scl[RT_X] = 1.0f;

        map[RT_J] = RT_Y;
        sgn[RT_J] = 1;
        scl[RT_Y] = 1.0f;

        map[RT_K] = RT_Z;
        sgn[RT_K] = 1;
        scl[RT_Z] = 1.0f;
    }

    /* axis mapping shorteners */
    mp_i = map[RT_I];
    mp_j = map[RT_J];
    mp_k = map[RT_K];

    /*---------------------------------*/

    update_minmax();

    s_srf->a_map[RT_I] = (mp_i + shift) << 4;
    s_srf->a_map[RT_J] = (mp_j + shift) << 4;
    s_srf->a_map[RT_K] = (mp_k + shift) << 4;
    s_srf->a_map[RT_L] = shift > 0 ? 1 : 0;

    s_srf->a_sgn[RT_I] = (sgn[RT_I] > 0 ? 0 : 1) << 4;
    s_srf->a_sgn[RT_J] = (sgn[RT_J] > 0 ? 0 : 1) << 4;
    s_srf->a_sgn[RT_K] = (sgn[RT_K] > 0 ? 0 : 1) << 4;
    s_srf->a_sgn[RT_L] = 0;

    s_srf->min_t[RT_X] = cmin[RT_X] == -RT_INF ? 0 : 1;
    s_srf->min_t[RT_Y] = cmin[RT_Y] == -RT_INF ? 0 : 1;
    s_srf->min_t[RT_Z] = cmin[RT_Z] == -RT_INF ? 0 : 1;

    s_srf->max_t[RT_X] = cmax[RT_X] == +RT_INF ? 0 : 1;
    s_srf->max_t[RT_Y] = cmax[RT_Y] == +RT_INF ? 0 : 1;
    s_srf->max_t[RT_Z] = cmax[RT_Z] == +RT_INF ? 0 : 1;

    rt_vec3  zro = {0.0f, 0.0f, 0.0f};
    rt_real *pps = shift > 0 ? zro : pos;

    RT_SIMD_SET(s_srf->min_x, bmin[RT_X] - pps[RT_X]);
    RT_SIMD_SET(s_srf->min_y, bmin[RT_Y] - pps[RT_Y]);
    RT_SIMD_SET(s_srf->min_z, bmin[RT_Z] - pps[RT_Z]);

    RT_SIMD_SET(s_srf->max_x, bmax[RT_X] - pps[RT_X]);
    RT_SIMD_SET(s_srf->max_y, bmax[RT_Y] - pps[RT_Y]);
    RT_SIMD_SET(s_srf->max_z, bmax[RT_Z] - pps[RT_Z]);

    RT_SIMD_SET(s_srf->pos_x, pos[RT_X]);
    RT_SIMD_SET(s_srf->pos_y, pos[RT_Y]);
    RT_SIMD_SET(s_srf->pos_z, pos[RT_Z]);

    if (shift > 0)
    {
        matrix_inverse(this->inv, this->mtx);

        RT_SIMD_SET(s_srf->tci_x, this->inv[RT_X][RT_I]);
        RT_SIMD_SET(s_srf->tci_y, this->inv[RT_Y][RT_I]);
        RT_SIMD_SET(s_srf->tci_z, this->inv[RT_Z][RT_I]);

        RT_SIMD_SET(s_srf->tcj_x, this->inv[RT_X][RT_J]);
        RT_SIMD_SET(s_srf->tcj_y, this->inv[RT_Y][RT_J]);
        RT_SIMD_SET(s_srf->tcj_z, this->inv[RT_Z][RT_J]);

        RT_SIMD_SET(s_srf->tck_x, this->inv[RT_X][RT_K]);
        RT_SIMD_SET(s_srf->tck_y, this->inv[RT_Y][RT_K]);
        RT_SIMD_SET(s_srf->tck_z, this->inv[RT_Z][RT_K]);
    }
}

rt_void rt_Surface::direct_minmax(rt_vec3 smin, rt_vec3 smax, /* src */
                                  rt_vec3 dmin, rt_vec3 dmax) /* dst */
{
    rt_vec3 tmin, tmax; /* tmp */

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

    rt_vec3  zro = {0.0f, 0.0f, 0.0f};
    rt_real *pps = shift > 0 ? zro : pos;

    dmin[RT_X] = tmin[RT_X] == -RT_INF ? -RT_INF : tmin[RT_X] + pps[RT_X];
    dmin[RT_Y] = tmin[RT_Y] == -RT_INF ? -RT_INF : tmin[RT_Y] + pps[RT_Y];
    dmin[RT_Z] = tmin[RT_Z] == -RT_INF ? -RT_INF : tmin[RT_Z] + pps[RT_Z];

    dmax[RT_X] = tmax[RT_X] == +RT_INF ? +RT_INF : tmax[RT_X] + pps[RT_X];
    dmax[RT_Y] = tmax[RT_Y] == +RT_INF ? +RT_INF : tmax[RT_Y] + pps[RT_Y];
    dmax[RT_Z] = tmax[RT_Z] == +RT_INF ? +RT_INF : tmax[RT_Z] + pps[RT_Z];
}

rt_void rt_Surface::update_minmax()
{
    rt_vec3 min, max;

    min[RT_I] = srf->min[RT_I];
    min[RT_J] = srf->min[RT_J];
    min[RT_K] = srf->min[RT_K];

    max[RT_I] = srf->max[RT_I];
    max[RT_J] = srf->max[RT_J];
    max[RT_K] = srf->max[RT_K];

    direct_minmax(min, max, bmin, bmax);
    direct_minmax(min, max, cmin, cmax);
}

rt_Surface::~rt_Surface()
{
    delete outer;
    delete inner;
}

/******************************************************************************/
/**********************************   PLANE   *********************************/
/******************************************************************************/

rt_Plane::rt_Plane(rt_Registry *rg, rt_Object *parent,
                   rt_OBJECT *obj, rt_cell ssize) :

    rt_Surface(rg, parent, obj, RT_MAX(ssize, sizeof(rt_SIMD_PLANE)))
{
    this->xpl = (rt_PLANE *)obj->obj.pobj;

/*  rt_SIMD_PLANE */

    rt_SIMD_PLANE *s_xpl = (rt_SIMD_PLANE *)s_srf;

    RT_SIMD_SET(s_xpl->nrm_k, +1.0f);
}

/******************************************************************************/
/********************************   QUADRIC   *********************************/
/******************************************************************************/

rt_Quadric::rt_Quadric(rt_Registry *rg, rt_Object *parent,
                       rt_OBJECT *obj, rt_cell ssize) :

    rt_Surface(rg, parent, obj, ssize)
{

}

/******************************************************************************/
/********************************   CYLINDER   ********************************/
/******************************************************************************/

rt_Cylinder::rt_Cylinder(rt_Registry *rg, rt_Object *parent,
                         rt_OBJECT *obj, rt_cell ssize) :

    rt_Quadric(rg, parent, obj,
               RT_MAX(ssize, sizeof(rt_SIMD_CYLINDER)))
{
    this->xcl = (rt_CYLINDER *)obj->obj.pobj;

/*  rt_SIMD_CYLINDER */

    rt_SIMD_CYLINDER *s_xcl = (rt_SIMD_CYLINDER *)s_srf;

    rt_real rad = RT_FABS(xcl->rad);

    RT_SIMD_SET(s_xcl->rad_2, rad * rad);
    RT_SIMD_SET(s_xcl->i_rad, 1.0f / rad);
}

/******************************************************************************/
/*********************************   SPHERE   *********************************/
/******************************************************************************/

rt_Sphere::rt_Sphere(rt_Registry *rg, rt_Object *parent,
                     rt_OBJECT *obj, rt_cell ssize) :

    rt_Quadric(rg, parent, obj,
               RT_MAX(ssize, sizeof(rt_SIMD_SPHERE)))
{
    this->xsp = (rt_SPHERE *)obj->obj.pobj;

/*  rt_SIMD_SPHERE */

    rt_SIMD_SPHERE *s_xsp = (rt_SIMD_SPHERE *)s_srf;

    rt_real rad = RT_FABS(xsp->rad);

    RT_SIMD_SET(s_xsp->rad_2, rad * rad);
    RT_SIMD_SET(s_xsp->i_rad, 1.0f / rad);
}

/******************************************************************************/
/**********************************   CONE   **********************************/
/******************************************************************************/

rt_Cone::rt_Cone(rt_Registry *rg, rt_Object *parent,
                 rt_OBJECT *obj, rt_cell ssize) :

    rt_Quadric(rg, parent, obj,
               RT_MAX(ssize, sizeof(rt_SIMD_CONE)))
{
    this->xcn = (rt_CONE *)obj->obj.pobj;

/*  rt_SIMD_CONE */

    rt_SIMD_CONE *s_xcn = (rt_SIMD_CONE *)s_srf;

    rt_real rat = RT_FABS(xcn->rat);

    RT_SIMD_SET(s_xcn->rat_2, rat * rat);
    RT_SIMD_SET(s_xcn->i_rat, 1.0f / (rat * sqrtf(rat * rat + 1.0f)));
}

/******************************************************************************/
/*******************************   PARABOLOID   *******************************/
/******************************************************************************/

rt_Paraboloid::rt_Paraboloid(rt_Registry *rg, rt_Object *parent,
                             rt_OBJECT *obj, rt_cell ssize) :

    rt_Quadric(rg, parent, obj,
               RT_MAX(ssize, sizeof(rt_SIMD_PARABOLOID)))
{
    this->xpb = (rt_PARABOLOID *)obj->obj.pobj;

/*  rt_SIMD_PARABOLOID */

    rt_SIMD_PARABOLOID *s_xpb = (rt_SIMD_PARABOLOID *)s_srf;

    rt_real par = xpb->par;

    RT_SIMD_SET(s_xpb->par_2, par / 2.0f);
    RT_SIMD_SET(s_xpb->i_par, par * par / 4.0f);
    RT_SIMD_SET(s_xpb->par_k, par);
    RT_SIMD_SET(s_xpb->one_k, 1.0f);
}

/******************************************************************************/
/*******************************   HYPERBOLOID   ******************************/
/******************************************************************************/

rt_Hyperboloid::rt_Hyperboloid(rt_Registry *rg, rt_Object *parent,
                               rt_OBJECT *obj, rt_cell ssize) :

    rt_Quadric(rg, parent, obj,
               RT_MAX(ssize, sizeof(rt_SIMD_HYPERBOLOID)))
{
    this->xhb = (rt_HYPERBOLOID *)obj->obj.pobj;

/*  rt_SIMD_HYPERBOLOID */

    rt_SIMD_HYPERBOLOID *s_xhb = (rt_SIMD_HYPERBOLOID *)s_srf;

    rt_real rat = xhb->rat;
    rt_real hyp = xhb->hyp;

    RT_SIMD_SET(s_xhb->rat_2, rat * rat);
    RT_SIMD_SET(s_xhb->i_rat, (1.0f + rat * rat) * rat * rat);
    RT_SIMD_SET(s_xhb->hyp_k, hyp);
    RT_SIMD_SET(s_xhb->one_k, 1.0f);
}

/******************************************************************************/
/********************************   MATERIAL   ********************************/
/******************************************************************************/

rt_Texture::rt_Texture(rt_Registry *rg, rt_pstr name) :

    rt_List<rt_Texture>(rg->get_tex())
{
    rg->put_tex(this);

    this->name = name;

    load_texture(rg, name, &tex);
}

/* For surface's UV coords
 *  to texture's XY coords mapping
 */
#define RT_U                0
#define RT_V                1

rt_Material::rt_Material(rt_Registry *rg, rt_SIDE *sd, rt_MATERIAL *mat) :

    rt_List<rt_Material>(rg->get_mat())
{
    if (mat == RT_NULL)
    {
        throw rt_Exception("NULL pointer in Material");
    }

    rg->put_mat(this);

    this->mat = mat;

    resolve_texture(rg);

    rt_TEX *tx = &mat->tex;

    props  = 0;
    props |= tx->x_dim == 1 && tx->y_dim == 1 ? 0 : RT_PROP_TEXTURE;
    props |= mat->prp[0] == 0.0f ? 0 : RT_PROP_REFLECT;
    props |= mat->prp[2] == 1.0f ? 0 : RT_PROP_REFRACT;
    props |= mat->lgt[1] == 0.0f ? 0 : RT_PROP_SPECULAR;
    props |= mat->prp[1] == 0.0f ? RT_PROP_OPAQUE : 0;
    props |= mat->prp[1] == 1.0f ? RT_PROP_TRANSP : 0;
    props |= mat->tag == RT_MAT_LIGHT ? RT_PROP_LIGHT : RT_PROP_NORMAL;
    props |= mat->tag == RT_MAT_METAL ? RT_PROP_METAL : 0;

    mtx[0][0] = +RT_COSA(sd->rot);
    mtx[0][1] = +RT_SINA(sd->rot);
    mtx[1][0] = -RT_SINA(sd->rot);
    mtx[1][1] = +RT_COSA(sd->rot);

    rt_cell map[2], sgn[2];
    rt_cell match = 0;

    rt_cell i, j;

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
            rg->alloc(sizeof(rt_SIMD_MATERIAL),
                                RT_SIMD_ALIGN);

    s_mat->t_map[RT_X] = map[RT_X] << 4;
    s_mat->t_map[RT_Y] = map[RT_Y] << 4;
    s_mat->t_map[2] = 0;
    s_mat->t_map[3] = 0;

    RT_SIMD_SET(s_mat->xscal, tx->x_dim / sd->scl[RT_X] * sgn[RT_X]);
    RT_SIMD_SET(s_mat->yscal, tx->y_dim / sd->scl[RT_Y] * sgn[RT_Y]);

    RT_SIMD_SET(s_mat->xoffs, sd->pos[map[RT_X]]);
    RT_SIMD_SET(s_mat->yoffs, sd->pos[map[RT_Y]]);

    rt_word x_mask = tx->x_dim - 1;
    rt_word y_mask = tx->y_dim - 1;

    RT_SIMD_SET(s_mat->xmask, x_mask);
    RT_SIMD_SET(s_mat->ymask, y_mask);

    rt_cell x_dim = tx->x_dim;
    rt_cell x_lg2 = 0;
    while (x_dim >>= 1)
    {
        x_lg2++;
    }

    s_mat->yshft[0] = x_lg2;
    s_mat->yshft[1] = 0;
    s_mat->yshft[2] = 0;
    s_mat->yshft[3] = 0;

    RT_SIMD_SET(s_mat->tex_p, tx->ptex);
    RT_SIMD_SET(s_mat->cmask, 0xFF);

    RT_SIMD_SET(s_mat->l_dff, mat->lgt[0]);
    RT_SIMD_SET(s_mat->l_spc, mat->lgt[1]);
    RT_SIMD_SET(s_mat->l_pow, (rt_word)mat->lgt[2]);
    RT_SIMD_SET(s_mat->pow_p, RT_NULL);

    RT_SIMD_SET(s_mat->c_rfl, mat->prp[0]);
    RT_SIMD_SET(s_mat->c_trn, mat->prp[1]);
    RT_SIMD_SET(s_mat->c_rfr, mat->prp[2]);

    RT_SIMD_SET(s_mat->rfr_2, mat->prp[2] * mat->prp[2]);
    RT_SIMD_SET(s_mat->c_one, 1.0f);
}

rt_void rt_Material::resolve_texture(rt_Registry *rg)
{
    rt_TEX *tx = &mat->tex;

    if (tx->x_dim == 0 && tx->y_dim == 0 && tx->ptex == RT_NULL)
    {
        tx->ptex  = &tx->col.val;
        tx->x_dim = 1;
        tx->y_dim = 1;
    }

    if (tx->x_dim == 0 && tx->y_dim == 0 && tx->ptex != RT_NULL)
    {
        rt_pstr name = (rt_pstr)tx->ptex;
        rt_Texture *tex = NULL;

        for (tex = rg->get_tex(); tex != NULL; tex = tex->next)
        {
            if (strcmp(name, tex->name) == 0)
            {
                break;
            }
        }

        if (tex == NULL)
        {
            tex = new rt_Texture(rg, name);
        }

        *tx = tex->tex;
    }
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
