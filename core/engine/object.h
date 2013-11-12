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

/* Update flags,
 * some values are hardcoded in rendering backend,
 * change with care! */

/* set for object which has
 * non-trivial axis scaling
 * (other than +/-1.0 scalers) */
#define RT_UPDATE_FLAG_SCL          (1 << 0)

/* set for object which has
 * non-trivial rotation
 * (other than multiple of 90 dgree) */
#define RT_UPDATE_FLAG_ROT          (1 << 1)

/* Select 1st phase of the update,
 * only obj-related fields are updated */
#define RT_UPDATE_FLAG_OBJ          (1 << 2)

/* Select 2nd phase of the update,
 * only srf-related fields are updated */
#define RT_UPDATE_FLAG_SRF          (1 << 3)

/* Structures */

struct rt_VERT
{
    rt_vec4 pos;
};

struct rt_EDGE
{
    rt_cell index[2];
    rt_cell k;
};

struct rt_FACE
{
    rt_cell index[4];
    rt_cell k, i, j;
};

/* Classes */

class rt_Registry;

class rt_Object;
class rt_Camera;
class rt_Light;
class rt_Node;
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

/*
 * Registry is an interface for scene manager to keep track of all objects.
 */
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
     * with get_* before calling put_*, usually done in constructors */
    rt_void         put_cam(rt_Camera *cam)     { cam_head = cam; cam_num++; }
    rt_void         put_lgt(rt_Light *lgt)      { lgt_head = lgt; lgt_num++; }
    rt_void         put_srf(rt_Surface *srf)    { srf_head = srf; srf_num++; }
    rt_void         put_tex(rt_Texture *tex)    { tex_head = tex; tex_num++; }
    rt_void         put_mat(rt_Material *mat)   { mat_head = mat; mat_num++; }
};

/******************************************************************************/
/*********************************   OBJECT   *********************************/
/******************************************************************************/

/*
 * Object is the base for special objects (cameras, lights)
 * and node objects (arrays, surfaces) in the hierarchy.
 * It is mainly responsible for properly passing transform
 * from the root through the branches to all the leafs.
 */
class rt_Object
{
/*  fields */

    public:

    rt_OBJECT          *obj;
    rt_TRANSFORM3D     *trm;

    rt_mat4             inv;
    rt_mat4             mtx;
    rt_real            *pos;
    rt_cell             tag;

    /* non-zero if object itself has
     * non-trivial transform
     * (scaling, rotation or both) */
    rt_cell             obj_has_trm;

    /* non-zero if object's full matrix has
     * non-trivial transform
     * (scaling, rotation or both) */
    rt_cell             mtx_has_trm;

    /* node up in the hierarchy with
     * non-trivial transform,
     * relative to which object has
     * trivial transform */
    rt_Object          *trnode;

    /* object's immediate parent
     * in the hierarchy */
    rt_Object          *parent;

/*  methods */

    protected:

    rt_Object(rt_Object *parent, rt_OBJECT *obj);

    public:

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

/*
 * Camera is a special object that facilitates the rendering of other objects.
 */
class rt_Camera : public rt_Object, public rt_List<rt_Camera>
{
/*  fields */

    public:

    rt_CAMERA          *cam;

    /* orientation basis in world space */
    rt_real            *hor; /* cam's X axis (left-to-right) */
    rt_real            *ver; /* cam's Y axis (top-to-bottom) */
    rt_real            *nrm; /* cam's Z axis (outwards) */

    /* distance from point of view to screen plane */
    rt_real             pov;

    /* rotation internal variables */
    rt_real             hor_sin;
    rt_real             hor_cos;

/*  methods */

    public:

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

/*
 * Light is a special object that influences the rendering of other objects.
 */
class rt_Light : public rt_Object, public rt_List<rt_Light>
{
/*  fields */

    public:

    rt_LIGHT           *lgt;

    rt_SIMD_LIGHT      *s_lgt;

/*  methods */

    public:

    rt_Light(rt_Registry *rg, rt_Object *parent, rt_OBJECT *obj);

    virtual
   ~rt_Light();

    virtual
    rt_void update(rt_long time, rt_mat4 mtx, rt_cell flags);
};

/******************************************************************************/
/**********************************   NODE   **********************************/
/******************************************************************************/

/*
 * Node is an object that itself is or contains renderable.
 * Only node elements can be inserted into backend surface lists.
 */
class rt_Node : public rt_Object
{
/*  fields */

    public:

    rt_Registry        *rg;

    rt_SIMD_SURFACE    *s_srf;

    rt_cell             map[3];
    rt_cell             sgn[3];

    /* bounding sphere center */
    rt_vec4             mid;
    /* bounding sphere radius */
    rt_real             rad;

/*  methods */

    protected:

    rt_Node(rt_Registry *rg, rt_Object *parent, rt_OBJECT *obj,
            rt_cell ssize);

    public:

    virtual
   ~rt_Node();

    virtual
    rt_void add_relation(rt_ELEM *lst);
    virtual
    rt_void update(rt_long time, rt_mat4 mtx, rt_cell flags);
};

/******************************************************************************/
/**********************************   ARRAY   *********************************/
/******************************************************************************/

/*
 * Array is a node that contains group of objects
 * under the same branch in the hierarchy.
 * It may contain renderables (surfaces), other arrays and
 * special objects (cameras, lights).
 */
class rt_Array : public rt_Node
{
/*  fields */

    public:

    rt_Object         **obj_arr;
    rt_cell             obj_num;

/*  methods */

    public:

    rt_Array(rt_Registry *rg, rt_Object *parent, rt_OBJECT *obj,
             rt_cell ssize = sizeof(rt_SIMD_SURFACE));

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

/*
 * Surface is a node that represents renderable shapes.
 */
class rt_Surface : public rt_Node, public rt_List<rt_Surface>
{
/*  fields */

    public:

    rt_SURFACE         *srf;

    rt_cell             mp_i;
    rt_cell             mp_j;
    rt_cell             mp_k;
    rt_cell             mp_l;

    /* bounding box,
     * all sides clipped or non-clipped are boundaries */
    rt_vec4             bmin;
    rt_vec4             bmax;

    /* clipping box,
     * non-clipped sides are at respective +/-infinity */
    rt_vec4             cmin;
    rt_vec4             cmax;

    /* bounding box geometry */
    rt_VERT            *verts;
    rt_cell             verts_num;
    rt_EDGE            *edges;
    rt_cell             edges_num;
    rt_FACE            *faces;
    rt_cell             faces_num;

    rt_vec4             sci;
    rt_vec4             scj;
    rt_vec4             sck;

    private:

    rt_Material        *outer;
    rt_Material        *inner;

/*  methods */

    protected:

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
    rt_void adjust_minmax(rt_vec4 smin, rt_vec4 smax,  /* src */
                          rt_vec4 bmin, rt_vec4 bmax,  /* bbox */
                          rt_vec4 cmin, rt_vec4 cmax); /* cbox */

    rt_void direct_minmax(rt_vec4 smin, rt_vec4 smax,  /* src */
                          rt_vec4 dmin, rt_vec4 dmax); /* dst */

    rt_void update_minmax();
    rt_void update_bounds();
};

/******************************************************************************/
/**********************************   PLANE   *********************************/
/******************************************************************************/

/*
 * Plane is a basic 1st order surface.
 */
class rt_Plane : public rt_Surface
{
/*  fields */

    public:

    rt_PLANE           *xpl;

/*  methods */

    public:

    rt_Plane(rt_Registry *rg, rt_Object *parent, rt_OBJECT *obj,
             rt_cell ssize = sizeof(rt_SIMD_SURFACE));

    virtual
   ~rt_Plane();

    virtual
    rt_void update(rt_long time, rt_mat4 mtx, rt_cell flags);
    virtual
    rt_void adjust_minmax(rt_vec4 smin, rt_vec4 smax,  /* src */
                          rt_vec4 bmin, rt_vec4 bmax,  /* bbox */
                          rt_vec4 cmin, rt_vec4 cmax); /* cbox */
};

/******************************************************************************/
/*********************************   QUADRIC   ********************************/
/******************************************************************************/

/*
 * Quadric is the base for all 2nd order surfaces.
 */
class rt_Quadric : public rt_Surface
{
/*  fields */

    public:

/*  methods */

    protected:

    rt_Quadric(rt_Registry *rg, rt_Object *parent, rt_OBJECT *obj,
               rt_cell ssize);

    public:

    virtual
   ~rt_Quadric();

    virtual
    rt_void update(rt_long time, rt_mat4 mtx, rt_cell flags);
    virtual
    rt_void adjust_minmax(rt_vec4 smin, rt_vec4 smax,  /* src */
                          rt_vec4 bmin, rt_vec4 bmax,  /* bbox */
                          rt_vec4 cmin, rt_vec4 cmax); /* cbox */
};

/******************************************************************************/
/********************************   CYLINDER   ********************************/
/******************************************************************************/

/*
 * Cylinder is a basic 2nd order surface.
 */
class rt_Cylinder : public rt_Quadric
{
/*  fields */

    public:

    rt_CYLINDER        *xcl;

/*  methods */

    public:

    rt_Cylinder(rt_Registry *rg, rt_Object *parent, rt_OBJECT *obj,
                rt_cell ssize = sizeof(rt_SIMD_SURFACE));

    virtual
   ~rt_Cylinder();

    virtual
    rt_void update(rt_long time, rt_mat4 mtx, rt_cell flags);
    virtual
    rt_void adjust_minmax(rt_vec4 smin, rt_vec4 smax,  /* src */
                          rt_vec4 bmin, rt_vec4 bmax,  /* bbox */
                          rt_vec4 cmin, rt_vec4 cmax); /* cbox */
};

/******************************************************************************/
/*********************************   SPHERE   *********************************/
/******************************************************************************/

/*
 * Sphere is a basic 2nd order surface.
 */
class rt_Sphere : public rt_Quadric
{
/*  fields */

    public:

    rt_SPHERE          *xsp;

/*  methods */

    public:

    rt_Sphere(rt_Registry *rg, rt_Object *parent, rt_OBJECT *obj,
              rt_cell ssize = sizeof(rt_SIMD_SURFACE));

    virtual
   ~rt_Sphere();

    virtual
    rt_void update(rt_long time, rt_mat4 mtx, rt_cell flags);
    virtual
    rt_void adjust_minmax(rt_vec4 smin, rt_vec4 smax,  /* src */
                          rt_vec4 bmin, rt_vec4 bmax,  /* bbox */
                          rt_vec4 cmin, rt_vec4 cmax); /* cbox */
};

/******************************************************************************/
/**********************************   CONE   **********************************/
/******************************************************************************/

/*
 * Cone is a basic 2nd order surface.
 */
class rt_Cone : public rt_Quadric
{
/*  fields */

    public:

    rt_CONE            *xcn;

/*  methods */

    public:

    rt_Cone(rt_Registry *rg, rt_Object *parent, rt_OBJECT *obj,
            rt_cell ssize = sizeof(rt_SIMD_SURFACE));

    virtual
   ~rt_Cone();

    virtual
    rt_void update(rt_long time, rt_mat4 mtx, rt_cell flags);
    virtual
    rt_void adjust_minmax(rt_vec4 smin, rt_vec4 smax,  /* src */
                          rt_vec4 bmin, rt_vec4 bmax,  /* bbox */
                          rt_vec4 cmin, rt_vec4 cmax); /* cbox */
};

/******************************************************************************/
/*******************************   PARABOLOID   *******************************/
/******************************************************************************/

/*
 * Paraboloid is a basic 2nd order surface.
 */
class rt_Paraboloid : public rt_Quadric
{
/*  fields */

    public:

    rt_PARABOLOID      *xpb;

/*  methods */

    public:

    rt_Paraboloid(rt_Registry *rg, rt_Object *parent, rt_OBJECT *obj,
                  rt_cell ssize = sizeof(rt_SIMD_SURFACE));

    virtual
   ~rt_Paraboloid();

    virtual
    rt_void update(rt_long time, rt_mat4 mtx, rt_cell flags);
    virtual
    rt_void adjust_minmax(rt_vec4 smin, rt_vec4 smax,  /* src */
                          rt_vec4 bmin, rt_vec4 bmax,  /* bbox */
                          rt_vec4 cmin, rt_vec4 cmax); /* cbox */
};

/******************************************************************************/
/*******************************   HYPERBOLOID   ******************************/
/******************************************************************************/

/*
 * Hyperboloid is a basic 2nd order surface.
 */
class rt_Hyperboloid : public rt_Quadric
{
/*  fields */

    public:

    rt_HYPERBOLOID     *xhb;

/*  methods */

    public:

    rt_Hyperboloid(rt_Registry *rg, rt_Object *parent, rt_OBJECT *obj,
                   rt_cell ssize = sizeof(rt_SIMD_SURFACE));

    virtual
   ~rt_Hyperboloid();

    virtual
    rt_void update(rt_long time, rt_mat4 mtx, rt_cell flags);
    virtual
    rt_void adjust_minmax(rt_vec4 smin, rt_vec4 smax,  /* src */
                          rt_vec4 bmin, rt_vec4 bmax,  /* bbox */
                          rt_vec4 cmin, rt_vec4 cmax); /* cbox */
};

/******************************************************************************/
/********************************   MATERIAL   ********************************/
/******************************************************************************/

/*
 * Texture contains data for images loaded from external files
 * in order to keep track of them and re-use them.
 */
class rt_Texture : public rt_List<rt_Texture>
{
/*  fields */

    public:

    rt_TEX              tex;

    rt_pstr             name;

/*  methods */

    public:

    rt_Texture(rt_Registry *rg, rt_pstr name);

    virtual
   ~rt_Texture();
};

/*
 * Material represents set of properties for a single side of a surface.
 */
class rt_Material : public rt_List<rt_Material>
{
/*  fields */

    public:

    rt_MATERIAL        *mat;

    rt_SIMD_MATERIAL   *s_mat;
    rt_cell             props;

    rt_mat2             mtx;

/*  methods */

    public:

    rt_Material(rt_Registry *rg, rt_SIDE *sd, rt_MATERIAL *mat);

    virtual
   ~rt_Material();

    rt_void resolve_texture(rt_Registry *rg);
};

#endif /* RT_OBJECT_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
