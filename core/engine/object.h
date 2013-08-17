/******************************************************************************/
/* Copyright (c) 2013 VectorChief (at github, bitbucket, sourceforge)         */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_OBJECT_H
#define RT_OBJECT_H

#include "rtbase.h"
#include "format.h"
#include "system.h"
#include "tracer.h"

/******************************************************************************/
/*******************************   DEFINITIONS   ******************************/
/******************************************************************************/

/* Camera actions */

#define RT_CAMERA_CROUCH            0
#define RT_CAMERA_JUMP              1

#define RT_CAMERA_MOVE_DOWN         2
#define RT_CAMERA_MOVE_UP           3
#define RT_CAMERA_MOVE_LEFT         4
#define RT_CAMERA_MOVE_RIGHT        5
#define RT_CAMERA_MOVE_BACK         6
#define RT_CAMERA_MOVE_FORWARD      7

#define RT_CAMERA_LEAN_LEFT         8
#define RT_CAMERA_LEAN_RIGHT        9

#define RT_CAMERA_ROTATE_DOWN       10
#define RT_CAMERA_ROTATE_UP         11
#define RT_CAMERA_ROTATE_LEFT       12
#define RT_CAMERA_ROTATE_RIGHT      13

/* Structures */

struct rt_VERT
{
    rt_vec4 pos;
};

struct rt_EDGE
{
    rt_cell index[2];
};

struct rt_FACE
{
    rt_cell index[4];
};

/* Classes */

class rt_Registry;

class rt_Object;
class rt_Camera;
class rt_Light;
class rt_Array;
class rt_Surface;
class rt_Plane;
class rt_Quadric;
class rt_Cylinder;
class rt_Sphere;
class rt_Cone;
class rt_Paraboloid;
class rt_Hyperboloid;

class rt_Texture;
class rt_Material;

/******************************************************************************/
/********************************   REGISTRY   ********************************/
/******************************************************************************/

class rt_Registry : public rt_Heap
{
/*  fields */

    protected:

    rt_Camera          *cam_head;
    rt_cell             cam_num;

    rt_Light           *lgt_head;
    rt_cell             lgt_num;

    rt_Surface         *srf_head;
    rt_cell             srf_num;

    rt_Texture         *tex_head;
    rt_cell             tex_num;

    rt_Material        *mat_head;
    rt_cell             mat_num;

/*  methods */

    public:

    rt_Registry(rt_FUNC_ALLOC f_alloc, rt_FUNC_FREE f_free) :
                    rt_Heap(f_alloc, f_free),
                    cam_head(RT_NULL), cam_num(0),
                    lgt_head(RT_NULL), lgt_num(0),
                    srf_head(RT_NULL), srf_num(0),
                    tex_head(RT_NULL), tex_num(0),
                    mat_head(RT_NULL), mat_num(0) { }

    virtual
   ~rt_Registry() { }

    rt_Camera      *get_cam() { return cam_head; }
    rt_Light       *get_lgt() { return lgt_head; }
    rt_Surface     *get_srf() { return srf_head; }
    rt_Texture     *get_tex() { return tex_head; }
    rt_Material    *get_mat() { return mat_head; }

    /* object's "next" field (from rt_List) must be initialized
     * with get_* before calling put_*, usually done in constructors. */
    rt_void         put_cam(rt_Camera *cam)     { cam_head = cam; cam_num++; }
    rt_void         put_lgt(rt_Light *lgt)      { lgt_head = lgt; lgt_num++; }
    rt_void         put_srf(rt_Surface *srf)    { srf_head = srf; srf_num++; }
    rt_void         put_tex(rt_Texture *tex)    { tex_head = tex; tex_num++; }
    rt_void         put_mat(rt_Material *mat)   { mat_head = mat; mat_num++; }
};

/******************************************************************************/
/*********************************   OBJECT   *********************************/
/******************************************************************************/

class rt_Object
{
    public:

/*  fields */

    rt_OBJECT          *obj;
    rt_TRANSFORM3D     *trm;

    rt_mat4             inv;
    rt_mat4             mtx;
    rt_real            *pos;
    rt_cell             tag;

    /* object's immediate parent
     * in the hierarchy */
    rt_Object          *parent;

/*  methods */

    rt_Object(rt_Object *parent, rt_OBJECT *obj);

    virtual
   ~rt_Object();

    virtual
    rt_void add_relation(rt_ELEM *lst);
    virtual
    rt_void update(rt_long time, rt_mat4 mtx, rt_cell flags);
};

/******************************************************************************/
/*********************************   CAMERA   *********************************/
/******************************************************************************/

class rt_Camera : public rt_Object, public rt_List<rt_Camera>
{
    public:

/*  fields */

    rt_CAMERA          *cam;

    rt_real            *hor;
    rt_real            *ver;
    rt_real            *nrm;

    rt_real             pov;

    rt_real             hor_sin;
    rt_real             hor_cos;

    rt_real             ver_sin;
    rt_real             ver_cos;

    rt_cell             user_input;

/*  methods */

    rt_Camera(rt_Registry *rg, rt_Object *parent, rt_OBJECT *obj);

    virtual
   ~rt_Camera();

    virtual
    rt_void update(rt_long time, rt_cell action);
    virtual
    rt_void update(rt_long time, rt_mat4 mtx, rt_cell flags);
};

/******************************************************************************/
/**********************************   LIGHT   *********************************/
/******************************************************************************/

class rt_Light : public rt_Object, public rt_List<rt_Light>
{
    public:

/*  fields */

    rt_LIGHT           *lgt;

    rt_SIMD_LIGHT      *s_lgt;

/*  methods */

    rt_Light(rt_Registry *rg, rt_Object *parent, rt_OBJECT *obj);

    virtual
    rt_void update(rt_long time, rt_mat4 mtx, rt_cell flags);
};

/******************************************************************************/
/**********************************   ARRAY   *********************************/
/******************************************************************************/

class rt_Array : public rt_Object
{
    public:

/*  fields */

    rt_Registry        *rg;

    rt_Object         **obj_arr;
    rt_cell             obj_num;

/*  methods */

    rt_Array(rt_Registry *rg, rt_Object *parent, rt_OBJECT *obj);

    virtual
   ~rt_Array();

    virtual
    rt_void add_relation(rt_ELEM *lst);
    virtual
    rt_void update(rt_long time, rt_mat4 mtx, rt_cell flags);
};

/******************************************************************************/
/*********************************   SURFACE   ********************************/
/******************************************************************************/

class rt_Surface : public rt_Object, public rt_List<rt_Surface>
{
    public:

/*  fields */

    rt_Registry        *rg;

    rt_SURFACE         *srf;

    rt_cell             mp_i;
    rt_cell             mp_j;
    rt_cell             mp_k;
    rt_cell             mp_l;

    /* bounding box,
     * all sides clipped or non-clipped are boundaries */
    rt_vec3             bmin;
    rt_vec3             bmax;

    /* clipping box,
     * non-clipped sides are at respective +/-infinity */
    rt_vec3             cmin;
    rt_vec3             cmax;

    /* bounding box geometry */
    rt_VERT            *verts;
    rt_cell             verts_num;
    rt_EDGE            *edges;
    rt_cell             edges_num;
    rt_FACE            *faces;
    rt_cell             faces_num;

    rt_SIMD_SURFACE    *s_srf;

    private:

    rt_Material        *outer;
    rt_Material        *inner;

    protected:

    /* enables generic matrix transform if non-zero,
     * selects aux vector sets in backend structures */
    rt_cell             shift;

    rt_cell             map[3];
    rt_cell             sgn[3];
    rt_vec3             scl;

/*  methods */

    rt_Surface(rt_Registry *rg, rt_Object *parent, rt_OBJECT *obj,
               rt_cell ssize);

    public:

    virtual
   ~rt_Surface();

    virtual
    rt_void add_relation(rt_ELEM *lst);
    virtual
    rt_void update(rt_long time, rt_mat4 mtx, rt_cell flags);
    virtual
    rt_void adjust_minmax(rt_vec3 smin, rt_vec3 smax,  /* src */
                          rt_vec3 bmin, rt_vec3 bmax,  /* bbox */
                          rt_vec3 cmin, rt_vec3 cmax); /* cbox */

    rt_void direct_minmax(rt_vec3 smin, rt_vec3 smax,  /* src */
                          rt_vec3 dmin, rt_vec3 dmax); /* dst */

    rt_void update_minmax();
};

/******************************************************************************/
/**********************************   PLANE   *********************************/
/******************************************************************************/

class rt_Plane : public rt_Surface
{
    public:

/*  fields */

    rt_PLANE           *xpl;

/*  methods */

    rt_Plane(rt_Registry *rg, rt_Object *parent, rt_OBJECT *obj,
             rt_cell ssize = sizeof(rt_SIMD_SURFACE));

    virtual
    rt_void update(rt_long time, rt_mat4 mtx, rt_cell flags);
    virtual
    rt_void adjust_minmax(rt_vec3 smin, rt_vec3 smax,  /* src */
                          rt_vec3 bmin, rt_vec3 bmax,  /* bbox */
                          rt_vec3 cmin, rt_vec3 cmax); /* cbox */
};

/******************************************************************************/
/*********************************   QUADRIC   ********************************/
/******************************************************************************/

class rt_Quadric : public rt_Surface
{
    public:

/*  fields */

/*  methods */

    protected:

    rt_Quadric(rt_Registry *rg, rt_Object *parent, rt_OBJECT *obj,
               rt_cell ssize);

    virtual
    rt_void update(rt_long time, rt_mat4 mtx, rt_cell flags);
    virtual
    rt_void adjust_minmax(rt_vec3 smin, rt_vec3 smax,  /* src */
                          rt_vec3 bmin, rt_vec3 bmax,  /* bbox */
                          rt_vec3 cmin, rt_vec3 cmax); /* cbox */
};

/******************************************************************************/
/********************************   CYLINDER   ********************************/
/******************************************************************************/

class rt_Cylinder : public rt_Quadric
{
    public:

/*  fields */

    rt_CYLINDER        *xcl;

/*  methods */

    rt_Cylinder(rt_Registry *rg, rt_Object *parent, rt_OBJECT *obj,
                rt_cell ssize = sizeof(rt_SIMD_SURFACE));

    virtual
    rt_void adjust_minmax(rt_vec3 smin, rt_vec3 smax,  /* src */
                          rt_vec3 bmin, rt_vec3 bmax,  /* bbox */
                          rt_vec3 cmin, rt_vec3 cmax); /* cbox */
};

/******************************************************************************/
/*********************************   SPHERE   *********************************/
/******************************************************************************/

class rt_Sphere : public rt_Quadric
{
    public:

/*  fields */

    rt_SPHERE          *xsp;

/*  methods */

    rt_Sphere(rt_Registry *rg, rt_Object *parent, rt_OBJECT *obj,
              rt_cell ssize = sizeof(rt_SIMD_SURFACE));

    virtual
    rt_void adjust_minmax(rt_vec3 smin, rt_vec3 smax,  /* src */
                          rt_vec3 bmin, rt_vec3 bmax,  /* bbox */
                          rt_vec3 cmin, rt_vec3 cmax); /* cbox */
};

/******************************************************************************/
/**********************************   CONE   **********************************/
/******************************************************************************/

class rt_Cone : public rt_Quadric
{
    public:

/*  fields */

    rt_CONE            *xcn;

/*  methods */

    rt_Cone(rt_Registry *rg, rt_Object *parent, rt_OBJECT *obj,
            rt_cell ssize = sizeof(rt_SIMD_SURFACE));

    virtual
    rt_void adjust_minmax(rt_vec3 smin, rt_vec3 smax,  /* src */
                          rt_vec3 bmin, rt_vec3 bmax,  /* bbox */
                          rt_vec3 cmin, rt_vec3 cmax); /* cbox */
};

/******************************************************************************/
/*******************************   PARABOLOID   *******************************/
/******************************************************************************/

class rt_Paraboloid : public rt_Quadric
{
    public:

/*  fields */

    rt_PARABOLOID      *xpb;

/*  methods */

    rt_Paraboloid(rt_Registry *rg, rt_Object *parent, rt_OBJECT *obj,
                  rt_cell ssize = sizeof(rt_SIMD_SURFACE));

    virtual
    rt_void adjust_minmax(rt_vec3 smin, rt_vec3 smax,  /* src */
                          rt_vec3 bmin, rt_vec3 bmax,  /* bbox */
                          rt_vec3 cmin, rt_vec3 cmax); /* cbox */
};

/******************************************************************************/
/*******************************   HYPERBOLOID   ******************************/
/******************************************************************************/

class rt_Hyperboloid : public rt_Quadric
{
    public:

/*  fields */

    rt_HYPERBOLOID     *xhb;

/*  methods */

    rt_Hyperboloid(rt_Registry *rg, rt_Object *parent, rt_OBJECT *obj,
                   rt_cell ssize = sizeof(rt_SIMD_SURFACE));

    virtual
    rt_void adjust_minmax(rt_vec3 smin, rt_vec3 smax,  /* src */
                          rt_vec3 bmin, rt_vec3 bmax,  /* bbox */
                          rt_vec3 cmin, rt_vec3 cmax); /* cbox */
};

/******************************************************************************/
/********************************   MATERIAL   ********************************/
/******************************************************************************/

class rt_Texture : public rt_List<rt_Texture>
{
    public:

/*  fields */

    rt_TEX              tex;

    rt_pstr             name;

/*  methods */

    rt_Texture(rt_Registry *rg, rt_pstr name);
};

class rt_Material : public rt_List<rt_Material>
{
    public:

/*  fields */

    rt_MATERIAL        *mat;

    rt_SIMD_MATERIAL   *s_mat;
    rt_cell             props;

    rt_mat2             mtx;

/*  methods */

    rt_Material(rt_Registry *rg, rt_SIDE *sd, rt_MATERIAL *mat);

    rt_void resolve_texture(rt_Registry *rg);
};

#endif /* RT_OBJECT_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
