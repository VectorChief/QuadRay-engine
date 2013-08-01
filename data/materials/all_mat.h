/******************************************************************************/
/* Copyright (c) 2013 VectorChief (at github, bitbucket, sourceforge)         */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_ALL_MAT_H
#define RT_ALL_MAT_H

#include "format.h"

/******************************************************************************/
/**********************************   TILES   *********************************/
/******************************************************************************/

rt_word dt_tile01[2][2] =
{
    0xFFFFFFFF, 0xFF888800,
    0xFF222222, 0xFFFFFFFF,
};

rt_MATERIAL mt_tile01 =
{
    RT_MAT(PLAIN),

    RT_TEX_BIND(PCOLOR, &dt_tile01),

    {/* dff     spc     pow */
        1.0,    0.0,    1.0
    },
    {/* rfl     trn     rfr */
        0.0,    0.0,    1.0
    },
};

/******************************************************************************/
/**********************************   PLAIN   *********************************/
/******************************************************************************/

rt_MATERIAL mt_blue01 =
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

rt_MATERIAL mt_blue02 =
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

rt_MATERIAL mt_cyan01 =
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

rt_MATERIAL mt_gray01 =
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

rt_MATERIAL mt_gray02 =
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

rt_MATERIAL mt_green01 =
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

rt_MATERIAL mt_orange01 =
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

rt_MATERIAL mt_pink01 =
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

rt_MATERIAL mt_white01 =
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

/******************************************************************************/
/**********************************   METAL   *********************************/
/******************************************************************************/

rt_MATERIAL mt_metal_pink01 =
{
    RT_MAT(METAL),

    RT_TEX(PCOLOR, 0xFFF6C6C6),

    {/* dff     spc     pow */
        1.0,    0.0,    1.0
    },
    {/* rfl     trn     rfr */
        0.5,    0.0,    1.0
    },
};

rt_MATERIAL mt_metal_cyan01 =
{
    RT_MAT(METAL),

    RT_TEX(PCOLOR, 0xFFA0F0D0),

    {/* dff     spc     pow */
        1.0,    0.0,    1.0
    },
    {/* rfl     trn     rfr */
        0.5,    0.0,    1.0
    },
};

/******************************************************************************/
/**********************************   GLASS   *********************************/
/******************************************************************************/

rt_MATERIAL mt_glass_orange01 =
{
    RT_MAT(PLAIN),

    RT_TEX(PCOLOR, 0xFFFF8F00),

    {/* dff     spc     pow */
        1.0,    0.0,    1.0
    },
    {/* rfl     trn     rfr */
        0.0,    0.5,    1.0
    },
};

rt_MATERIAL mt_air_to_glass_blue02 =
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

rt_MATERIAL mt_glass_blue02_to_air =
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

/******************************************************************************/
/**********************************   LIGHT   *********************************/
/******************************************************************************/

rt_MATERIAL mt_bulb01 =
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
