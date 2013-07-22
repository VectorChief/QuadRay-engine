/******************************************************************************/
/* Copyright (c) 2013 VectorChief (at github, bitbucket, sourceforge)         */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_ALL_OBJ_H
#define RT_ALL_OBJ_H

#include "format.h"

/******************************************************************************/
/*********************************   CAMERA   *********************************/
/******************************************************************************/

rt_CAMERA cm_camera01 =
{
    RT_CAM(PLAIN),

    RT_COL(0xFFFFFFFF),

    {/* amb */
        0.1
    },
    {/* pov */
        1.0
    },
    {/* dpi     dpj     dpk */
        0.5,    0.5,    0.5
    },
    {/* dri     drj     drk */
        1.5,    1.5,    1.5
    },
};

/******************************************************************************/
/**********************************   LIGHT   *********************************/
/******************************************************************************/

rt_LIGHT lt_light01 =
{
    RT_LGT(PLAIN),

    RT_COL(0xFFFFFFFF),

    {/* amb     src */
        0.1,    0.7
    },
};

#endif /* RT_ALL_OBJ_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
