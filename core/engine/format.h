/******************************************************************************/
/* Copyright (c) 2013-2018 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_FORMAT_H
#define RT_FORMAT_H

#include "rtbase.h"

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * format.h: Interface for the scene data format.
 *
 * Definitions provided in this file are used to specify scene data (in a form
 * of C static struct initializers) by storing pointers to previously defined
 * structures and thus building object hierarchy and relations.
 *
 * All surfaces are defined in their local IJK space, where K is an axis
 * of rotational or axial symmetry (also normal for plane) and IJ is a base
 * orthogonal to K. For non-symmetric surfaces, K axis is chosen based on
 * a difference from what IJ axes have in common in terms of surface properties.
 *
 * Recommended naming scheme for C++ types and definitions is given in rtbase.h.
 */

/******************************************************************************/
/*******************************   DEFINITIONS   ******************************/
/******************************************************************************/

/*
 * Runtime optimization flags,
 * define particular flag as 0
 * to turn respective optimization off at compile time.
 */
#define RT_OPTS_THREAD          (1 << 0)
#define RT_OPTS_TILING          (1 << 1)
#define RT_OPTS_TILING_EXT1     (1 << 2)
#define RT_OPTS_FSCALE          (1 << 3)
#define RT_OPTS_TARRAY          (1 << 4)
#define RT_OPTS_VARRAY          (1 << 5) /* 6 reserved for future exts */
#define RT_OPTS_ADJUST          (1 << 7)
#define RT_OPTS_UPDATE          (1 << 8)
#define RT_OPTS_RENDER          (1 << 9)
#define RT_OPTS_SHADOW          (1 << 10)
#define RT_OPTS_SHADOW_EXT1     (1 << 11)
#define RT_OPTS_SHADOW_EXT2     (1 << 12)
#define RT_OPTS_2SIDED          (1 << 13)
#define RT_OPTS_2SIDED_EXT1     (1 << 14)
#define RT_OPTS_2SIDED_EXT2     (1 << 15)
#define RT_OPTS_INSERT          (0 << 16)
#define RT_OPTS_INSERT_EXT1     (0 << 17)
#define RT_OPTS_INSERT_EXT2     (0 << 18)
#define RT_OPTS_REMOVE          (0 << 19)
#define RT_OPTS_GAMMA           (1 << 20) /* turns off Gamma when set to 1 */
#define RT_OPTS_FRESNEL         (1 << 21) /* turns off Fresnel when set to 1 */

/* extra options (update) */
#define RT_OPTS_UPDATE_EXT0     (1 << 26) /* update phases off */
#define RT_OPTS_UPDATE_EXT1     (1 << 27) /* update phase1 single-threadedly */
#define RT_OPTS_UPDATE_EXT2     (1 << 28) /* update phase2 single-threadedly */
#define RT_OPTS_UPDATE_EXT3     (1 << 29) /* update phase3 single-threadedly */
/* extra options (render) */
#define RT_OPTS_RENDER_EXT0     (1 << 30) /* render scene off */
#define RT_OPTS_RENDER_EXT1     (1 << 31) /* render scene single-threadedly */

/* bbox sorting (RT_OPTS_INSERT) and hidden surfaces removal (RT_OPTS_REMOVE)
 * optimizations have been turned off for poor scalability with larger scenes */

#define RT_OPTS_NONE            (                                           \
        RT_OPTS_GAMMA           |                                           \
        RT_OPTS_FRESNEL         )

#define RT_OPTS_FULL            (                                           \
        RT_OPTS_THREAD          |                                           \
        RT_OPTS_TILING          |                                           \
        RT_OPTS_TILING_EXT1     |                                           \
        RT_OPTS_FSCALE          |                                           \
        RT_OPTS_TARRAY          |                                           \
        RT_OPTS_VARRAY          |                                           \
        RT_OPTS_ADJUST          |                                           \
        RT_OPTS_UPDATE          |                                           \
        RT_OPTS_RENDER          |                                           \
        RT_OPTS_SHADOW          |                                           \
        RT_OPTS_SHADOW_EXT1     |                                           \
        RT_OPTS_SHADOW_EXT2     |                                           \
        RT_OPTS_2SIDED          |                                           \
        RT_OPTS_2SIDED_EXT1     |                                           \
        RT_OPTS_2SIDED_EXT2     |                                           \
        RT_OPTS_INSERT          |                                           \
        RT_OPTS_INSERT_EXT1     |                                           \
        RT_OPTS_INSERT_EXT2     |                                           \
        RT_OPTS_REMOVE          |                                           \
        RT_OPTS_GAMMA           |                                           \
        RT_OPTS_FRESNEL         )

/*
 * Object tags,
 * some values are hardcoded in rendering backend,
 * change with care!
 */

/* generic array tag,
 * used for textures as well as for surfaces */
#define RT_TAG_ARRAY                       -1

/* surface tags */
#define RT_TAG_PLANE                        0
#define RT_TAG_CYLINDER                     1
#define RT_TAG_SPHERE                       2
#define RT_TAG_CONE                         3
#define RT_TAG_PARABOLOID                   4
#define RT_TAG_HYPERBOLOID                  5
#define RT_TAG_PARACYLINDER                 6
#define RT_TAG_HYPERCYLINDER                7
#define RT_TAG_HYPERPARABOLOID              8
#define RT_TAG_SURFACE_MAX                  9

/* special tags */
#define RT_TAG_CAMERA                       100
#define RT_TAG_LIGHT                        101
#define RT_TAG_MAX                          102

/******************************************************************************/
/*********************************   MACROS   *********************************/
/******************************************************************************/

#define RT_IS_CAMERA(o)                                                     \
        ((o)->tag == RT_TAG_CAMERA)

#define RT_IS_LIGHT(o)                                                      \
        ((o)->tag == RT_TAG_LIGHT)

#define RT_IS_ARRAY(o)                                                      \
        ((o)->tag == RT_TAG_ARRAY)

#define RT_IS_SURFACE(o)                                                    \
        ((o)->tag  > RT_TAG_ARRAY && (o)->tag < RT_TAG_SURFACE_MAX)

#define RT_IS_PLANE(o)                                                      \
        ((o)->tag == RT_TAG_PLANE)

/******************************************************************************/
/********************************   RELATION   ********************************/
/******************************************************************************/

#define RT_REL_MINUS_INNER                 -1 /* subtract srf inner subspace */
#define RT_REL_MINUS_OUTER                 +1 /* subtract srf outer subspace */
#define RT_REL_MINUS_ACCUM                  2 /* subtract accum subspace */
#define RT_REL_INDEX_ARRAY                  3 /* next index in sub-array */

#define RT_REL_BOUND_ARRAY                  4 /* array has bounding volume */
#define RT_REL_UNTIE_ARRAY                  5 /* array has no bounding volume */
#define RT_REL_BOUND_INDEX                  6 /* add index to bounding volume */
#define RT_REL_UNTIE_INDEX                  7 /* remove index from bnd volume */

struct rt_RELATION
{
    rt_si32             obj1;
    rt_si32             rel;
    rt_si32             obj2;
};

/******************************************************************************/
/********************************   TRANSFORM   *******************************/
/******************************************************************************/

struct rt_TRANSFORM2D
{
    rt_vec2             scl;
    rt_real             rot;
    rt_vec2             pos;
};

struct rt_TRANSFORM3D
{
    rt_vec3             scl;
    rt_vec3             rot;
    rt_vec3             pos;
};

/******************************************************************************/
/**********************************   COLOR   *********************************/
/******************************************************************************/

#define RT_COL(val)                                                         \
{                                                                           \
    val,    {0.0f}                                                          \
}

#define RT_COL_HDR(r, g, b, a)                                              \
{                                                                           \
    0x0,    {r, g, b, a}                                                    \
}

struct rt_COL
{
    rt_ui32             val;
    rt_vec4             hdr;
};

/******************************************************************************/
/*********************************   TEXTURE   ********************************/
/******************************************************************************/

#define RT_TEX_PCOLOR                       0   /* plain-color,     XRGB */

#define RT_TEX_HDR_PCOLOR                   10  /* plain-color,     XRGB */

/* default HDR format is 32-bit fp, add other variants later if needed */

#define RT_TEX(tag, val)                                                    \
{                                                                           \
    RT_TEX_##tag,           RT_COL(val),                                    \
    RT_NULL,                0,                                              \
    RT_NULL,                0,                                              \
    0,                      0                                               \
}

#define RT_TEX_HDR(tag, r, g, b, a)                                         \
{                                                                           \
    RT_TEX_HDR_##tag,       RT_COL_HDR(r, g, b  a),                         \
    RT_NULL,                0,                                              \
    RT_NULL,                0,                                              \
    0,                      0                                               \
}

#define RT_TEX_LOAD(tag, pstr)                                              \
{                                                                           \
    RT_TEX_##tag,           RT_COL(0x0),                                    \
    (rt_pntr)pstr,          0,                                              \
    RT_NULL,                0,                                              \
    0,                      0                                               \
}

#define RT_TEX_BIND(tag, ptex)                                              \
{                                                                           \
    RT_TEX_##tag,           RT_COL(0x0),                                    \
   *ptex,                   0,                                              \
    RT_NULL,                0,                                              \
    RT_ARR_SIZE(**ptex),    RT_ARR_SIZE(*ptex)                              \
}

#define RT_TEX_ARRAY(parr)                                                  \
{/* using generic array tag here */                                         \
    RT_TAG_ARRAY,           RT_COL(0x0),                                    \
   *parr,                   RT_ARR_SIZE(*parr),                             \
    RT_NULL,                0,                                              \
    0,                      0                                               \
}

#define RT_TEX_ARRAY_REL(parr, prel)                                        \
{/* using generic array tag here */                                         \
    RT_TAG_ARRAY,           RT_COL(0x0),                                    \
   *parr,                   RT_ARR_SIZE(*parr),                             \
   *prel,                   RT_ARR_SIZE(*prel),                             \
    0,                      0                                               \
}

struct rt_TEX
{
    rt_si32             tag;
    rt_COL              col;

    rt_void            *ptex;
    rt_si32             tex_num;

    rt_RELATION        *prel;
    rt_si32             rel_num;

    rt_si32             x_dim;
    rt_si32             y_dim;
};

/* texture arrays are not currently implemented in the engine */

struct rt_TEXTURE
{
    /* TRANSFORM2D (implicit) */
    rt_vec2             scl;
    rt_real             rot;
    rt_vec2             pos;

    rt_TEX              tex;
    rt_real             wgt; /* texture's weight in the array */
};

/******************************************************************************/
/********************************   MATERIAL   ********************************/
/******************************************************************************/

#define RT_MAT_PLAIN                        0
#define RT_MAT_LIGHT                        1
#define RT_MAT_METAL                        2

#define RT_MAT(tag)                         RT_MAT_##tag

struct rt_MATERIAL
{
    rt_si32             tag;
    rt_TEX              tex;

    rt_real             lgt[4];
    rt_real             prp[4];
};

struct rt_SIDE
{
    /* TRANSFORM2D (implicit) */
    rt_vec2             scl;
    rt_real             rot;
    rt_vec2             pos;

    rt_MATERIAL        *pmat;
};

/******************************************************************************/
/*********************************   OBJECT   *********************************/
/******************************************************************************/

struct rt_OBJ
{
    rt_si32             tag;

    rt_void            *pobj;
    rt_si32             obj_num;

    rt_RELATION        *prel;
    rt_si32             rel_num;

    rt_MATERIAL        *pmat_outer; /* srf material override */
    rt_MATERIAL        *pmat_inner; /* srf material override */
};

typedef rt_void (*rt_FUNC_ANIM3D)(rt_time time, rt_time last_time,
                                  rt_TRANSFORM3D *trm, rt_pntr pobj);

struct rt_OBJECT
{
    rt_TRANSFORM3D      trm;
    rt_OBJ              obj;
    rt_FUNC_ANIM3D      f_anim;
    rt_time             time;
};

static /* needed for strict typization */
rt_si32 AR_(rt_OBJECT *pobj)
{
    return RT_TAG_ARRAY;
}

#define RT_OBJ_ARRAY(parr)                                                  \
{                                                                           \
    AR_(*parr),                                                             \
   *parr,                   RT_ARR_SIZE(*parr),                             \
    RT_NULL,                0,                                              \
    RT_NULL,                RT_NULL                                         \
}

static /* needed for strict typization */
rt_si32 RL_(rt_OBJECT *pobj, rt_RELATION *prel)
{
    return RT_TAG_ARRAY;
}

#define RT_OBJ_ARRAY_REL(parr, prel)                                        \
{                                                                           \
    RL_(*parr, *prel),                                                      \
   *parr,                   RT_ARR_SIZE(*parr),                             \
   *prel,                   RT_ARR_SIZE(*prel),                             \
    RT_NULL,                RT_NULL                                         \
}

/******************************************************************************/
/*********************************   CAMERA   *********************************/
/******************************************************************************/

#define RT_CAM_PLAIN                        0

#define RT_CAM(tag)                         RT_CAM_##tag

struct rt_CAMERA
{
    rt_si32             tag;
    rt_COL              col;    /* global ambient color */
    rt_real             lum[1]; /* global ambient intensity */

    rt_real             vpt[1]; /* viewport (pov: distance from screen) */
    rt_vec3             dps;    /* delta position (per unit of time) */
    rt_vec3             drt;    /* delta rotation (per unit of time) */
};

static /* needed for strict typization */
rt_si32 CM_(rt_CAMERA *pobj)
{
    return RT_TAG_CAMERA;
}

#define RT_OBJ_CAMERA(pobj)                                                 \
{                                                                           \
    CM_(pobj),                                                              \
    pobj,                   1,                                              \
    RT_NULL,                0,                                              \
    RT_NULL,                RT_NULL                                         \
}

/******************************************************************************/
/**********************************   LIGHT   *********************************/
/******************************************************************************/

#define RT_LGT_PLAIN                        0

#define RT_LGT(tag)                         RT_LGT_##tag

struct rt_LIGHT
{
    rt_si32             tag;
    rt_COL              col;    /* light's color */
    rt_real             lum[2]; /* light's ambient and source intensity */
    rt_real             atn[4]; /* light's attenuation properties */
};

static /* needed for strict typization */
rt_si32 LT_(rt_LIGHT *pobj)
{
    return RT_TAG_LIGHT;
}

#define RT_OBJ_LIGHT(pobj)                                                  \
{                                                                           \
    LT_(pobj),                                                              \
    pobj,                   1,                                              \
    RT_NULL,                0,                                              \
    RT_NULL,                RT_NULL                                         \
}

/******************************************************************************/
/*********************************   SURFACE   ********************************/
/******************************************************************************/

struct rt_SURFACE
{
    rt_vec3             min;
    rt_vec3             max;

    rt_SIDE             side_outer;
    rt_SIDE             side_inner;
};

/******************************************************************************/
/**********************************   PLANE   *********************************/
/******************************************************************************/

struct rt_PLANE
{
    rt_SURFACE          srf;
};

static /* needed for strict typization */
rt_si32 PL_(rt_PLANE *pobj)
{
    return RT_TAG_PLANE;
}

#define RT_OBJ_PLANE(pobj)                                                  \
{                                                                           \
    PL_(pobj),                                                              \
    pobj,                   1,                                              \
    RT_NULL,                0,                                              \
    RT_NULL,                RT_NULL                                         \
}

#define RT_OBJ_PLANE_MAT(pobj, pmat_outer, pmat_inner)                      \
{                                                                           \
    PL_(pobj),                                                              \
    pobj,                   1,                                              \
    RT_NULL,                0,                                              \
    pmat_outer,             pmat_inner                                      \
}

/******************************************************************************/
/********************************   CYLINDER   ********************************/
/******************************************************************************/

struct rt_CYLINDER
{
    rt_SURFACE          srf;
    rt_real             rad;
};

static /* needed for strict typization */
rt_si32 CL_(rt_CYLINDER *pobj)
{
    return RT_TAG_CYLINDER;
}

#define RT_OBJ_CYLINDER(pobj)                                               \
{                                                                           \
    CL_(pobj),                                                              \
    pobj,                   1,                                              \
    RT_NULL,                0,                                              \
    RT_NULL,                RT_NULL                                         \
}

#define RT_OBJ_CYLINDER_MAT(pobj, pmat_outer, pmat_inner)                   \
{                                                                           \
    CL_(pobj),                                                              \
    pobj,                   1,                                              \
    RT_NULL,                0,                                              \
    pmat_outer,             pmat_inner                                      \
}

/******************************************************************************/
/*********************************   SPHERE   *********************************/
/******************************************************************************/

struct rt_SPHERE
{
    rt_SURFACE          srf;
    rt_real             rad;
};

static /* needed for strict typization */
rt_si32 SP_(rt_SPHERE *pobj)
{
    return RT_TAG_SPHERE;
}

#define RT_OBJ_SPHERE(pobj)                                                 \
{                                                                           \
    SP_(pobj),                                                              \
    pobj,                   1,                                              \
    RT_NULL,                0,                                              \
    RT_NULL,                RT_NULL                                         \
}

#define RT_OBJ_SPHERE_MAT(pobj, pmat_outer, pmat_inner)                     \
{                                                                           \
    SP_(pobj),                                                              \
    pobj,                   1,                                              \
    RT_NULL,                0,                                              \
    pmat_outer,             pmat_inner                                      \
}

/******************************************************************************/
/**********************************   CONE   **********************************/
/******************************************************************************/

struct rt_CONE
{
    rt_SURFACE          srf;
    rt_real             rat;
};

static /* needed for strict typization */
rt_si32 CN_(rt_CONE *pobj)
{
    return RT_TAG_CONE;
}

#define RT_OBJ_CONE(pobj)                                                   \
{                                                                           \
    CN_(pobj),                                                              \
    pobj,                   1,                                              \
    RT_NULL,                0,                                              \
    RT_NULL,                RT_NULL                                         \
}

#define RT_OBJ_CONE_MAT(pobj, pmat_outer, pmat_inner)                       \
{                                                                           \
    CN_(pobj),                                                              \
    pobj,                   1,                                              \
    RT_NULL,                0,                                              \
    pmat_outer,             pmat_inner                                      \
}

/******************************************************************************/
/*******************************   PARABOLOID   *******************************/
/******************************************************************************/

struct rt_PARABOLOID
{
    rt_SURFACE          srf;
    rt_real             par;
};

static /* needed for strict typization */
rt_si32 PB_(rt_PARABOLOID *pobj)
{
    return RT_TAG_PARABOLOID;
}

#define RT_OBJ_PARABOLOID(pobj)                                             \
{                                                                           \
    PB_(pobj),                                                              \
    pobj,                   1,                                              \
    RT_NULL,                0,                                              \
    RT_NULL,                RT_NULL                                         \
}

#define RT_OBJ_PARABOLOID_MAT(pobj, pmat_outer, pmat_inner)                 \
{                                                                           \
    PB_(pobj),                                                              \
    pobj,                   1,                                              \
    RT_NULL,                0,                                              \
    pmat_outer,             pmat_inner                                      \
}

/******************************************************************************/
/*******************************   HYPERBOLOID   ******************************/
/******************************************************************************/

struct rt_HYPERBOLOID
{
    rt_SURFACE          srf;
    rt_real             rat;
    rt_real             hyp;
};

static /* needed for strict typization */
rt_si32 HB_(rt_HYPERBOLOID *pobj)
{
    return RT_TAG_HYPERBOLOID;
}

#define RT_OBJ_HYPERBOLOID(pobj)                                            \
{                                                                           \
    HB_(pobj),                                                              \
    pobj,                   1,                                              \
    RT_NULL,                0,                                              \
    RT_NULL,                RT_NULL                                         \
}

#define RT_OBJ_HYPERBOLOID_MAT(pobj, pmat_outer, pmat_inner)                \
{                                                                           \
    HB_(pobj),                                                              \
    pobj,                   1,                                              \
    RT_NULL,                0,                                              \
    pmat_outer,             pmat_inner                                      \
}

/******************************************************************************/
/******************************   PARACYLINDER   ******************************/
/******************************************************************************/

struct rt_PARACYLINDER
{
    rt_SURFACE          srf;
    rt_real             par;
};

static /* needed for strict typization */
rt_si32 PC_(rt_PARACYLINDER *pobj)
{
    return RT_TAG_PARACYLINDER;
}

#define RT_OBJ_PARACYLINDER(pobj)                                           \
{                                                                           \
    PC_(pobj),                                                              \
    pobj,                   1,                                              \
    RT_NULL,                0,                                              \
    RT_NULL,                RT_NULL                                         \
}

#define RT_OBJ_PARACYLINDER_MAT(pobj, pmat_outer, pmat_inner)               \
{                                                                           \
    PC_(pobj),                                                              \
    pobj,                   1,                                              \
    RT_NULL,                0,                                              \
    pmat_outer,             pmat_inner                                      \
}

/******************************************************************************/
/******************************   HYPERCYLINDER   *****************************/
/******************************************************************************/

struct rt_HYPERCYLINDER
{
    rt_SURFACE          srf;
    rt_real             rat;
    rt_real             hyp;
};

static /* needed for strict typization */
rt_si32 HC_(rt_HYPERCYLINDER *pobj)
{
    return RT_TAG_HYPERCYLINDER;
}

#define RT_OBJ_HYPERCYLINDER(pobj)                                          \
{                                                                           \
    HC_(pobj),                                                              \
    pobj,                   1,                                              \
    RT_NULL,                0,                                              \
    RT_NULL,                RT_NULL                                         \
}

#define RT_OBJ_HYPERCYLINDER_MAT(pobj, pmat_outer, pmat_inner)              \
{                                                                           \
    HC_(pobj),                                                              \
    pobj,                   1,                                              \
    RT_NULL,                0,                                              \
    pmat_outer,             pmat_inner                                      \
}

/******************************************************************************/
/*****************************   HYPERPARABOLOID   ****************************/
/******************************************************************************/

struct rt_HYPERPARABOLOID
{
    rt_SURFACE          srf;
    rt_real             pr1;
    rt_real             pr2;
};

static /* needed for strict typization */
rt_si32 HP_(rt_HYPERPARABOLOID *pobj)
{
    return RT_TAG_HYPERPARABOLOID;
}

#define RT_OBJ_HYPERPARABOLOID(pobj)                                        \
{                                                                           \
    HP_(pobj),                                                              \
    pobj,                   1,                                              \
    RT_NULL,                0,                                              \
    RT_NULL,                RT_NULL                                         \
}

#define RT_OBJ_HYPERPARABOLOID_MAT(pobj, pmat_outer, pmat_inner)            \
{                                                                           \
    HP_(pobj),                                                              \
    pobj,                   1,                                              \
    RT_NULL,                0,                                              \
    pmat_outer,             pmat_inner                                      \
}

/******************************************************************************/
/**********************************   SCENE   *********************************/
/******************************************************************************/

struct rt_SCENE
{
    rt_OBJ              root;
    rt_ui32             opts; /* set flags to disable runtime opts per scene */
    rt_pntr             lock;
};

#endif /* RT_FORMAT_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
