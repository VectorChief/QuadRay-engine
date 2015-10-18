/******************************************************************************/
/* Copyright (c) 2013-2015 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_OBJECT_H
#define RT_OBJECT_H

#include "rtbase.h"
#include "rtconf.h"
#include "rtgeom.h"
#include "format.h"
#include "system.h"
#include "tracer.h"

#undef Q /* short name for RT_SIMD_QUADS */
#undef S /* short name for RT_SIMD_WIDTH */

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * object.h: Interface for the object hierarchy.
 *
 * More detailed description of this subsystem is given in object.cpp.
 * Recommended naming scheme for C++ types and definitions is given in rtbase.h.
 */

/******************************************************************************/
/*******************************   DEFINITIONS   ******************************/
/******************************************************************************/

/*
 * Camera actions.
 */
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

/*
 * Update flags,
 * some values are hardcoded in rendering backend,
 * change with care!
 */

/* set for object which has
 * non-trivial scaling after rotation
 * (other than +/-1.0 scalers) */
#define RT_UPDATE_FLAG_SCL          (1 << 0)

/* set for object which has
 * non-trivial rotation
 * (other than multiple of 90 dgree) */
#define RT_UPDATE_FLAG_ROT          (1 << 1)

/* set for object which has
 * some of its parents changed */
#define RT_UPDATE_FLAG_OBJ          (1 << 2)

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
class rt_ParaCylinder;
class rt_HyperCylinder;
class rt_HyperParaboloid;

class rt_Texture;
class rt_Material;

/******************************************************************************/
/********************************   REGISTRY   ********************************/
/******************************************************************************/

/*
 * Registry is an interface for the scene manager to keep track of all objects.
 */
class rt_Registry : public rt_Heap
{
/*  fields */

    protected:

    rt_Camera          *cam_head;
    rt_cell             cam_num;

    rt_Light           *lgt_head;
    rt_cell             lgt_num;

    rt_Array           *arr_head;
    rt_cell             arr_num;

    rt_Surface         *srf_head;
    rt_cell             srf_num;

    rt_Texture         *tex_head;
    rt_cell             tex_num;

    rt_Material        *mat_head;
    rt_cell             mat_num;

    public:

    /* optimization flags */
    rt_cell             opts;

    /* reusable relations template
     * for clippers accum segments */
    rt_ELEM            *rel;

/*  methods */

    public:

    rt_Registry(rt_FUNC_ALLOC f_alloc, rt_FUNC_FREE f_free) :
                    rt_Heap(f_alloc, f_free),
                    cam_head(RT_NULL), cam_num(0),
                    lgt_head(RT_NULL), lgt_num(0),
                    arr_head(RT_NULL), arr_num(0),
                    srf_head(RT_NULL), srf_num(0),
                    tex_head(RT_NULL), tex_num(0),
                    mat_head(RT_NULL), mat_num(0),
                    opts(RT_OPTS_FULL), rel(RT_NULL) { }

    virtual
   ~rt_Registry() { }

    rt_Camera      *get_cam() { return cam_head; }
    rt_Light       *get_lgt() { return lgt_head; }
    rt_Array       *get_arr() { return arr_head; }
    rt_Surface     *get_srf() { return srf_head; }
    rt_Texture     *get_tex() { return tex_head; }
    rt_Material    *get_mat() { return mat_head; }

    /* object's "next" field (from rt_List) must be initialized
     * with get_* before calling put_*, usually done in constructors */
    rt_void         put_cam(rt_Camera *cam)     { cam_head = cam; cam_num++; }
    rt_void         put_lgt(rt_Light *lgt)      { lgt_head = lgt; lgt_num++; }
    rt_void         put_arr(rt_Array *arr)      { arr_head = arr; arr_num++; }
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

    protected:

    /* original object pointer */
    rt_OBJECT          *obj;
    /* original transform data */
    rt_TRANSFORM3D      otm;

    /* matrix pointer
     * for/from the hierarchy */
    rt_mat4            *pmtx;

    /* axis mapping for trivial transform */
    rt_cell             map[4];
    rt_cell             sgn[4];
    rt_vec4             scl;

    /* axis mapping shorteners */
    rt_cell             mp_i;
    rt_cell             mp_j;
    rt_cell             mp_k;
    rt_cell             mp_l;

    public:

    /* registry pointer */
    rt_Registry        *rg;

    /* bounding box and volume,
     * used in arrays for outer part
     * of split bvnode if present,
     * used as generic boundary in other objects */
    rt_BOUND           *bvbox;

    /* object's transform and tag */
    rt_TRANSFORM3D     *trm;
    rt_cell             tag;

    /* transform matrices */
    rt_mat4             inv;
    rt_mat4             mtx;
    rt_real            *pos;

    /* non-zero if object itself or
     * some of its parents changed */
    rt_cell             obj_changed;

    /* non-zero if object itself or
     * some of its parents has
     * non-trivial transform
     * (rotation or scaling after rotation) */
    rt_cell             obj_has_trm;

    /* non-zero if object's own matrix has
     * non-trivial transform
     * (rotation or scaling after rotation) */
    rt_cell             mtx_has_trm;

    /* object's immediate parent
     * in the hierarchy */
    rt_Object          *parent;

    /* node up in the hierarchy with
     * non-trivial transform,
     * relative to which object has
     * trivial transform */
    rt_Object          *trnode;

    /* node up in the hierarchy with
     * bounding volume enabled,
     * to which object contributes
     * its own bounding volume */
    rt_Object          *bvnode;

/*  methods */

    protected:

    rt_void update_status(rt_time time, rt_cell flags, rt_Object *trnode);

    rt_void update_matrix(rt_mat4 mtx);

    rt_Object(rt_Registry *rg, rt_Object *parent, rt_OBJECT *obj);

    public:

    virtual
   ~rt_Object();

    virtual
    rt_void add_relation(rt_ELEM *lst);
    virtual
    rt_void update_bvnode(rt_Object *bvnode, rt_bool mode);

    virtual
    rt_void update_object(rt_time time, rt_cell flags,
                          rt_Object *trnode, rt_mat4 mtx);
    virtual
    rt_void update_fields();
};

/******************************************************************************/
/*********************************   CAMERA   *********************************/
/******************************************************************************/

/*
 * Camera is a special object which facilitates the rendering of other objects.
 */
class rt_Camera : public rt_Object, public rt_List<rt_Camera>
{
/*  fields */

    private:

    /* rotation internal variables */
    rt_real             hor_sin;
    rt_real             hor_cos;

    /* non-zero if camera was changed by action */
    rt_cell             cam_changed;

    public:

    rt_CAMERA          *cam;

    /* orientation basis in world space */
    rt_real            *hor; /* cam's X axis (left-to-right) */
    rt_real            *ver; /* cam's Y axis (top-to-bottom) */
    rt_real            *nrm; /* cam's Z axis (outwards) */

    /* distance from point of view to screen plane */
    rt_real             pov;

/*  methods */

    public:

    rt_Camera(rt_Registry *rg, rt_Object *parent, rt_OBJECT *obj);

    virtual
   ~rt_Camera();

    virtual
    rt_void update_object(rt_time time, rt_cell flags,
                          rt_Object *trnode, rt_mat4 mtx);
    virtual
    rt_void update_fields();

    rt_void update_action(rt_time time, rt_cell action);
};

/******************************************************************************/
/**********************************   LIGHT   *********************************/
/******************************************************************************/

/*
 * Light is a special object which influences the rendering of other objects.
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
    rt_void update_object(rt_time time, rt_cell flags,
                          rt_Object *trnode, rt_mat4 mtx);
    virtual
    rt_void update_fields();
};

/******************************************************************************/
/**********************************   NODE   **********************************/
/******************************************************************************/

/*
 * Node is an object which itself is or contains renderable (surface).
 * Only node elements can be inserted into backend surface lists.
 */
class rt_Node : public rt_Object
{
/*  fields */

    protected:

    /* per-side materials */
    rt_Material        *outer;
    rt_Material        *inner;

    public:

    /* reusable relations template
     * for arrays and surfaces */
    rt_ELEM            *rel;

    /* surface SIMD struct,
     * used for trnode if present */
    rt_SIMD_SURFACE    *s_srf;

/*  methods */

    protected:

    rt_void update_status(rt_time time, rt_cell flags, rt_Object *trnode);

    rt_void update_matrix(rt_mat4 mtx);

    rt_void update_bbgeom(rt_BOUND *box);

    rt_Node(rt_Registry *rg, rt_Object *parent, rt_OBJECT *obj,
            rt_cell ssize);

    public:

    virtual
   ~rt_Node();

    virtual
    rt_void add_relation(rt_ELEM *lst);
    virtual
    rt_void update_bvnode(rt_Object *bvnode, rt_bool mode);

    virtual
    rt_void update_object(rt_time time, rt_cell flags,
                          rt_Object *trnode, rt_mat4 mtx);
    virtual
    rt_void update_fields();
};

/******************************************************************************/
/**********************************   ARRAY   *********************************/
/******************************************************************************/

/*
 * Array is a node which contains group of objects
 * under the same branch in the hierarchy.
 * It may contain renderables (surfaces), other arrays (recursive)
 * and special objects (cameras, lights).
 */
class rt_Array : public rt_Node, public rt_List<rt_Array>
{
/*  fields */

    private:

    /* scalers matrix */
    rt_mat4             scm;

    public:

    /* array of objects */
    rt_Object         **obj_arr;
    rt_cell             obj_num;

    /* non-zero if array itself or
     * some of its sub-objects changed */
    rt_cell             arr_changed;

    /* bounding box and volume,
     * used for trnode if present
     * and has contents outside bvnode,
     * in which case bvnode is split */
    rt_BOUND           *trbox;

    /* bounding box and volume,
     * used for inner part of split bvnode
     * or trnode if it doesn't have contents
     * outside bvnode */
    rt_BOUND           *inbox;

    /* surface SIMD struct,
     * used for bvbox part of bvnode */
    rt_SIMD_SURFACE    *s_bvb;

    /* surface SIMD struct,
     * used for inbox part of bvnode */
    rt_SIMD_SURFACE    *s_inb;

/*  methods */

    protected:

    rt_void update_status(rt_time time, rt_cell flags, rt_Object *trnode);

    rt_void update_matrix(rt_mat4 mtx);

    public:

    rt_Array(rt_Registry *rg, rt_Object *parent, rt_OBJECT *obj,
             rt_cell ssize = 0);

    virtual
   ~rt_Array();

    virtual
    rt_void add_relation(rt_ELEM *lst);
    virtual
    rt_void update_bvnode(rt_Object *bvnode, rt_bool mode);

    virtual
    rt_void update_object(rt_time time, rt_cell flags,
                          rt_Object *trnode, rt_mat4 mtx);
    virtual
    rt_void update_fields();

    rt_void update_bounds();
};

/******************************************************************************/
/*********************************   SURFACE   ********************************/
/******************************************************************************/

/*
 * Surface is a node which represents renderable shapes.
 */
class rt_Surface : public rt_Node, public rt_List<rt_Surface>
{
/*  fields */

    protected:

    rt_SURFACE         *srf;

    /* non-zero if surface itself or
     * some of its clippers changed */
    rt_cell             srf_changed;

    public:

    /* top of the trnode/bvnode
     * sequence on the branch */
    rt_ELEM            *top;

    /* trnode element for cases
     * where bvnode is not allowed */
    rt_ELEM            *trn;

    /* surface shape extension to
     * bounding box and volume */
    rt_SHAPE           *shape;

/*  methods */

    protected:

    virtual
    rt_void adjust_minmax(rt_vec4 smin, rt_vec4 smax,  /* src */
                          rt_vec4 bmin, rt_vec4 bmax,  /* bbox */
                          rt_vec4 cmin, rt_vec4 cmax); /* cbox */

    rt_void invert_minmax(rt_vec4 smin, rt_vec4 smax,  /* src */
                          rt_vec4 dmin, rt_vec4 dmax); /* dst */

    rt_void direct_minmax(rt_vec4 smin, rt_vec4 smax,  /* src */
                          rt_vec4 dmin, rt_vec4 dmax); /* dst */

    rt_void recalc_minmax(rt_vec4 smin, rt_vec4 smax,  /* src */
                          rt_vec4 bmin, rt_vec4 bmax,  /* bbox */
                          rt_vec4 cmin, rt_vec4 cmax); /* cbox */

    rt_void update_minmax();

    rt_Surface(rt_Registry *rg, rt_Object *parent, rt_OBJECT *obj,
               rt_cell ssize);

    public:

    virtual
   ~rt_Surface();

    virtual
    rt_void add_relation(rt_ELEM *lst);

    virtual
    rt_void update_object(rt_time time, rt_cell flags,
                          rt_Object *trnode, rt_mat4 mtx);
    virtual
    rt_void update_fields();

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

    private:

    rt_PLANE           *xpl;

/*  methods */

    protected:

    virtual
    rt_void adjust_minmax(rt_vec4 smin, rt_vec4 smax,  /* src */
                          rt_vec4 bmin, rt_vec4 bmax,  /* bbox */
                          rt_vec4 cmin, rt_vec4 cmax); /* cbox */

    public:

    rt_Plane(rt_Registry *rg, rt_Object *parent, rt_OBJECT *obj,
             rt_cell ssize = 0);

    virtual
   ~rt_Plane();

    virtual
    rt_void update_fields();
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

    protected:

/*  methods */

    protected:

    virtual
    rt_void adjust_minmax(rt_vec4 smin, rt_vec4 smax,  /* src */
                          rt_vec4 bmin, rt_vec4 bmax,  /* bbox */
                          rt_vec4 cmin, rt_vec4 cmax); /* cbox */

    rt_Quadric(rt_Registry *rg, rt_Object *parent, rt_OBJECT *obj,
               rt_cell ssize);

    public:

    virtual
   ~rt_Quadric();

    virtual
    rt_void update_fields();

    rt_void commit_fields();
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

    private:

    rt_CYLINDER        *xcl;

/*  methods */

    protected:

    virtual
    rt_void adjust_minmax(rt_vec4 smin, rt_vec4 smax,  /* src */
                          rt_vec4 bmin, rt_vec4 bmax,  /* bbox */
                          rt_vec4 cmin, rt_vec4 cmax); /* cbox */

    public:

    rt_Cylinder(rt_Registry *rg, rt_Object *parent, rt_OBJECT *obj,
                rt_cell ssize = 0);

    virtual
   ~rt_Cylinder();

    virtual
    rt_void update_fields();
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

    private:

    rt_SPHERE          *xsp;

/*  methods */

    protected:

    virtual
    rt_void adjust_minmax(rt_vec4 smin, rt_vec4 smax,  /* src */
                          rt_vec4 bmin, rt_vec4 bmax,  /* bbox */
                          rt_vec4 cmin, rt_vec4 cmax); /* cbox */

    public:

    rt_Sphere(rt_Registry *rg, rt_Object *parent, rt_OBJECT *obj,
              rt_cell ssize = 0);

    virtual
   ~rt_Sphere();

    virtual
    rt_void update_fields();
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

    private:

    rt_CONE            *xcn;

/*  methods */

    protected:

    virtual
    rt_void adjust_minmax(rt_vec4 smin, rt_vec4 smax,  /* src */
                          rt_vec4 bmin, rt_vec4 bmax,  /* bbox */
                          rt_vec4 cmin, rt_vec4 cmax); /* cbox */

    public:

    rt_Cone(rt_Registry *rg, rt_Object *parent, rt_OBJECT *obj,
            rt_cell ssize = 0);

    virtual
   ~rt_Cone();

    virtual
    rt_void update_fields();
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

    private:

    rt_PARABOLOID      *xpb;

/*  methods */

    protected:

    virtual
    rt_void adjust_minmax(rt_vec4 smin, rt_vec4 smax,  /* src */
                          rt_vec4 bmin, rt_vec4 bmax,  /* bbox */
                          rt_vec4 cmin, rt_vec4 cmax); /* cbox */

    public:

    rt_Paraboloid(rt_Registry *rg, rt_Object *parent, rt_OBJECT *obj,
                  rt_cell ssize = 0);

    virtual
   ~rt_Paraboloid();

    virtual
    rt_void update_fields();
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

    private:

    rt_HYPERBOLOID     *xhb;

/*  methods */

    protected:

    virtual
    rt_void adjust_minmax(rt_vec4 smin, rt_vec4 smax,  /* src */
                          rt_vec4 bmin, rt_vec4 bmax,  /* bbox */
                          rt_vec4 cmin, rt_vec4 cmax); /* cbox */

    public:

    rt_Hyperboloid(rt_Registry *rg, rt_Object *parent, rt_OBJECT *obj,
                   rt_cell ssize = 0);

    virtual
   ~rt_Hyperboloid();

    virtual
    rt_void update_fields();
};

/******************************************************************************/
/******************************   PARACYLINDER   ******************************/
/******************************************************************************/

/*
 * ParaCylinder is a basic 2nd order surface.
 */
class rt_ParaCylinder : public rt_Quadric
{
/*  fields */

    private:

    rt_PARACYLINDER    *xpc;

/*  methods */

    protected:

    virtual
    rt_void adjust_minmax(rt_vec4 smin, rt_vec4 smax,  /* src */
                          rt_vec4 bmin, rt_vec4 bmax,  /* bbox */
                          rt_vec4 cmin, rt_vec4 cmax); /* cbox */

    public:

    rt_ParaCylinder(rt_Registry *rg, rt_Object *parent, rt_OBJECT *obj,
                    rt_cell ssize = 0);

    virtual
   ~rt_ParaCylinder();

    virtual
    rt_void update_fields();
};

/******************************************************************************/
/******************************   HYPERCYLINDER   *****************************/
/******************************************************************************/

/*
 * HyperCylinder is a basic 2nd order surface.
 */
class rt_HyperCylinder : public rt_Quadric
{
/*  fields */

    private:

    rt_HYPERCYLINDER   *xhc;

/*  methods */

    protected:

    virtual
    rt_void adjust_minmax(rt_vec4 smin, rt_vec4 smax,  /* src */
                          rt_vec4 bmin, rt_vec4 bmax,  /* bbox */
                          rt_vec4 cmin, rt_vec4 cmax); /* cbox */

    public:

    rt_HyperCylinder(rt_Registry *rg, rt_Object *parent, rt_OBJECT *obj,
                     rt_cell ssize = 0);

    virtual
   ~rt_HyperCylinder();

    virtual
    rt_void update_fields();
};

/******************************************************************************/
/*****************************   HYPERPARABOLOID   ****************************/
/******************************************************************************/

/*
 * HyperParaboloid is a basic 2nd order surface.
 */
class rt_HyperParaboloid : public rt_Quadric
{
/*  fields */

    private:

    rt_HYPERPARABOLOID *xhp;

/*  methods */

    protected:

    virtual
    rt_void adjust_minmax(rt_vec4 smin, rt_vec4 smax,  /* src */
                          rt_vec4 bmin, rt_vec4 bmax,  /* bbox */
                          rt_vec4 cmin, rt_vec4 cmax); /* cbox */

    public:

    rt_HyperParaboloid(rt_Registry *rg, rt_Object *parent, rt_OBJECT *obj,
                       rt_cell ssize = 0);

    virtual
   ~rt_HyperParaboloid();

    virtual
    rt_void update_fields();
};

/******************************************************************************/
/********************************   MATERIAL   ********************************/
/******************************************************************************/

/*
 * Texture contains data for images loaded from external files
 * in order to keep track of and re-use them.
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

    private:

    rt_MATERIAL        *mat;
    /* original texture data */
    rt_TEX              otx;

    rt_mat2             mtx;

    public:

    rt_SIDE            *sd;

    rt_cell             map[2];
    rt_real             scl[2];

    rt_SIMD_MATERIAL   *s_mat;
    rt_cell             props;

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
