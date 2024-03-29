/******************************************************************************/
/* Copyright (c) 2013-2025 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_ALL_MAT_H
#define RT_ALL_MAT_H

#include "format.h"

#include "all_tex.h"

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * all_mat.h: Common file for all materials.
 *
 * Recommended naming scheme for materials:
 *
 * - All material names start with mt_ followed by two parts.
 *
 * - 1st part is material's property set denoted by material's type followed
 *   by a number designating a particular set of properties.
 *   It is recommended to use the same numbers for identical property sets
 *   across all material types. Some property sets might be assigned
 *   custom material types as their names (not present in format.h),
 *   in this case it is recommended to keep these names consistent
 *   across all materials.
 *
 * - 2nd part is material's color or texture set.
 *   Colors and textures are denoted by their respective names followed
 *   by a number designating a particular set of RGB values for color
 *   or particular combination of bitmaps for texture arrays.
 *   It is recommended to use the same numbers for identical colors or
 *   texture sets across all materials.
 *
 * - Complex materials can be placed in separate files mat_*.h, where * is
 *   material's name (for example mat_plain01_crate01.h), material names
 *   in this case follow the same rules as described above
 *   (for example mt_plain01_crate01)
 */

/******************************************************************************/
/**********************************   CRATE   *********************************/
/******************************************************************************/

rt_MATERIAL mt_plain01_crate01 =
{
    RT_MAT(PLAIN),

#if RT_EMBED_TEX == 1
    RT_TEX_BIND(PCOLOR, &dt_tex_crate01),
#else /* RT_EMBED_TEX */
    RT_TEX_LOAD(PCOLOR, "tex_crate01.bmp"),
#endif /* RT_EMBED_TEX */

    {/* dff     spc     pow */
        1.0,    0.0,    1.0
    },
    {/* rfl     trn     rfr */
        0.0,    0.0,    1.0
    },
};

/******************************************************************************/
/**********************************   TILES   *********************************/
/******************************************************************************/

rt_ui32 dt_tex_tile01[2][2] =
{
    0xFFFFFFFF, 0xFF888800,
    0xFF222222, 0xFFFFFFFF,
};

rt_MATERIAL mt_plain01_tile01 =
{
    RT_MAT(PLAIN),

    RT_TEX_BIND(PCOLOR, &dt_tex_tile01),

    {/* dff     spc     pow */
        1.0,    0.0,    1.0
    },
    {/* rfl     trn     rfr */
        0.0,    0.0,    1.0
    },
};

rt_MATERIAL mt_plain03_tile01 =
{
    RT_MAT(PLAIN),

    RT_TEX_BIND(PCOLOR, &dt_tex_tile01),

    {/* dff     spc     pow */
        1.0,    0.0,    1.0
    },
    {/* rfl     trn     rfr */
        0.5,    0.0,    0.1
    },
};

/******************************************************************************/
/**********************************   PLAIN   *********************************/
/******************************************************************************/

rt_MATERIAL mt_plain01_blue01 =
{
    RT_MAT(PLAIN),

    RT_TEX(PCOLOR, 0xFF4343F3),

    {/* dff     spc     pow */
        1.0,    0.0,    1.0
    },
    {/* rfl     trn     rfr */
        0.0,    0.0,    1.0
    },
};

rt_MATERIAL mt_plain01_blue02 =
{
    RT_MAT(PLAIN),

    RT_TEX(PCOLOR, 0xFF0080D0),

    {/* dff     spc     pow */
        1.0,    0.0,    1.0
    },
    {/* rfl     trn     rfr */
        0.0,    0.0,    1.0
    },
};

rt_MATERIAL mt_plain01_cyan01 =
{
    RT_MAT(PLAIN),

    RT_TEX(PCOLOR, 0xFFA0F0D0),

    {/* dff     spc     pow */
        1.0,    0.0,    1.0
    },
    {/* rfl     trn     rfr */
        0.0,    0.0,    1.0
    },
};

rt_MATERIAL mt_plain01_gray01 =
{
    RT_MAT(PLAIN),

    RT_TEX(PCOLOR, 0xFF838383),

    {/* dff     spc     pow */
        1.0,    0.0,    1.0
    },
    {/* rfl     trn     rfr */
        0.0,    0.0,    1.0
    },
};

rt_MATERIAL mt_plain01_gray02 =
{
    RT_MAT(PLAIN),

    RT_TEX(PCOLOR, 0xFF434343),

    {/* dff     spc     pow */
        1.0,    0.0,    1.0
    },
    {/* rfl     trn     rfr */
        0.0,    0.0,    1.0
    },
};

rt_MATERIAL mt_plain01_green01 =
{
    RT_MAT(PLAIN),

    RT_TEX(PCOLOR, 0xFF2FAF3F),

    {/* dff     spc     pow */
        1.0,    0.0,    1.0
    },
    {/* rfl     trn     rfr */
        0.0,    0.0,    1.0
    },
};

rt_MATERIAL mt_plain01_orange01 =
{
    RT_MAT(PLAIN),

    RT_TEX(PCOLOR, 0xFFFF8F00),

    {/* dff     spc     pow */
        1.0,    0.0,    1.0
    },
    {/* rfl     trn     rfr */
        0.0,    0.0,    1.0
    },
};

rt_MATERIAL mt_plain02_orange01 =
{
    RT_MAT(PLAIN),

    RT_TEX(PCOLOR, 0xFFFF8F00),

    {/* dff     spc     pow */
        0.5,    0.5,   32.0
    },
    {/* rfl     trn     rfr */
        0.0,    0.0,    1.0
    },
};

rt_MATERIAL mt_plain01_pink01 =
{
    RT_MAT(PLAIN),

    RT_TEX(PCOLOR, 0xFFF6C6C6),

    {/* dff     spc     pow */
        1.0,    0.0,    1.0
    },
    {/* rfl     trn     rfr */
        0.0,    0.0,    1.0
    },
};

rt_MATERIAL mt_plain02_pink01 =
{
    RT_MAT(PLAIN),

    RT_TEX(PCOLOR, 0xFFF6C6C6),

    {/* dff     spc     pow */
        0.5,    0.5,   32.0
    },
    {/* rfl     trn     rfr */
        0.0,    0.0,    1.0
    },
};

rt_MATERIAL mt_plain01_red01 =
{
    RT_MAT(PLAIN),

    RT_TEX(PCOLOR, 0xFFF63F2F),

    {/* dff     spc     pow */
        1.0,    0.0,    1.0
    },
    {/* rfl     trn     rfr */
        0.0,    0.0,    1.0
    },
};

rt_MATERIAL mt_plain02_red01 =
{
    RT_MAT(PLAIN),

    RT_TEX(PCOLOR, 0xFFF63F2F),

    {/* dff     spc     pow */
        0.5,    0.5,   32.0
    },
    {/* rfl     trn     rfr */
        0.0,    0.0,    1.0
    },
};

rt_MATERIAL mt_plain01_white01 =
{
    RT_MAT(PLAIN),

    RT_TEX(PCOLOR, 0xFFAFAFAF),

    {/* dff     spc     pow */
        1.0,    0.0,    1.0
    },
    {/* rfl     trn     rfr */
        0.0,    0.0,    1.0
    },
};

rt_MATERIAL mt_plain03_white01 =
{
    RT_MAT(PLAIN),

    RT_TEX(PCOLOR, 0xFFAFAFAF),

    {/* dff     spc     pow */
        1.0,    0.0,    1.0
    },
    {/* rfl     trn     rfr */
        0.5,    0.0,    0.1
    },
};

/******************************************************************************/
/**********************************   METAL   *********************************/
/******************************************************************************/

rt_MATERIAL mt_metal01_cyan01 =
{
    RT_MAT(METAL),

    RT_TEX(PCOLOR, 0xFFA0F0D0),

    {/* dff     spc     pow */
        1.0,    0.0,    1.0
    },
    {/* rfl     trn     rfr     ext */
        0.5,    0.0,    1.0,    9.0
    },
};

rt_MATERIAL mt_metal01_pink01 =
{
    RT_MAT(METAL),

    RT_TEX(PCOLOR, 0xFFF6C6C6),

    {/* dff     spc     pow */
        1.0,    0.0,    1.0
    },
    {/* rfl     trn     rfr     ext */
        0.5,    0.0,    1.0,    9.0
    },
};

rt_MATERIAL mt_metal02_orange01 =
{
    RT_MAT(METAL),

    RT_TEX(PCOLOR, 0xFFFF8F00),

    {/* dff     spc     pow */
        0.5,    0.5,   32.0
    },
    {/* rfl     trn     rfr     ext */
        0.0,    0.0,    1.0,    9.0
    },
};

rt_MATERIAL mt_metal02_pink01 =
{
    RT_MAT(METAL),

    RT_TEX(PCOLOR, 0xFFF6C6C6),

    {/* dff     spc     pow */
        0.5,    0.5,   32.0
    },
    {/* rfl     trn     rfr     ext */
        0.0,    0.0,    1.0,    9.0
    },
};

rt_MATERIAL mt_metal03_nickel01 =
{
    RT_MAT(METAL),

    RT_TEX(PCOLOR, 0xFFA0A0A0),

    {/* dff     spc     pow */
        1.0,    0.0,    1.0
    },
    {/* rfl     trn     rfr     ext */
        1.0,    0.0,    1.0,    9.0
    },
};

/******************************************************************************/
/**********************************   GLASS   *********************************/
/******************************************************************************/

rt_MATERIAL mt_glass01_orange01 =
{
    RT_MAT(PLAIN),

    RT_TEX(PCOLOR, 0xFFFF8F00),

    {/* dff     spc     pow */
        1.0,    0.0,    1.0
    },
    {/* rfl     trn     rfr     ext */
        0.0,    0.5,    1.0,    0.67
    },
};

rt_MATERIAL mt_air_to_glass01_blue02 =
{
    RT_MAT(PLAIN),

    RT_TEX(PCOLOR, 0xFF0080D0),

    {/* dff     spc     pow */
        1.0,    0.0,    1.0
    },
    {/* rfl     trn     rfr */
        0.0,    0.5,    0.67
    },
};

rt_MATERIAL mt_glass01_to_air_blue02 =
{
    RT_MAT(PLAIN),

    RT_TEX(PCOLOR, 0xFF0080D0),

    {/* dff     spc     pow */
        1.0,    0.0,    1.0
    },
    {/* rfl     trn     rfr */
        0.0,    0.5,    1.5
    },
};

rt_MATERIAL mt_air_to_glass03 =
{
    RT_MAT(PLAIN),

    RT_TEX(PCOLOR, 0xFFFFFFFF),

    {/* dff     spc     pow */
        1.0,    0.0,    1.0
    },
    {/* rfl     trn     rfr */
        0.0,    1.0,    0.67
    },
};

rt_MATERIAL mt_glass03_to_air =
{
    RT_MAT(PLAIN),

    RT_TEX(PCOLOR, 0xFFFFFFFF),

    {/* dff     spc     pow */
        1.0,    0.0,    1.0
    },
    {/* rfl     trn     rfr */
        0.0,    1.0,    1.5
    },
};

/******************************************************************************/
/**********************************   LIGHT   *********************************/
/******************************************************************************/

rt_MATERIAL mt_light01_bulb01 =
{
    RT_MAT(LIGHT),

    RT_TEX(PCOLOR, 0xFFFFFFFF),

    {/* dff     spc     pow */
        1.0,    0.0,    1.0
    },
    {/* rfl     trn     rfr */
        0.0,    0.0,    1.0
    },
};

#endif /* RT_ALL_MAT_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
