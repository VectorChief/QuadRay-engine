/******************************************************************************/
/* Copyright (c) 2013 VectorChief (at github, bitbucket, sourceforge)         */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_ALL_MAT_H
#define RT_ALL_MAT_H

#include "format.h"

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

#endif /* RT_ALL_MAT_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
