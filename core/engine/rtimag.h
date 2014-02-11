/******************************************************************************/
/* Copyright (c) 2013-2014 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTIMAG_H
#define RT_RTIMAG_H

#include "rtbase.h"
#include "format.h"
#include "object.h"

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtimag.h: Interface for the image library.
 *
 * More detailed description of this subsystem is given in rtimag.cpp.
 * Recommended naming scheme for C++ types and definitions is given in rtbase.h.
 */

/******************************************************************************/
/********************************   TEXTURE   *********************************/
/******************************************************************************/

/*
 * Load texture from file to memory.
 */
rt_void load_texture(rt_Registry *rg, rt_pstr name, rt_TEX *tx);

/*
 * Convert texture from file to static array initializer.
 * Parameter fullpath must be editable char array.
 */
rt_void convert_texture(rt_char *fullpath);

#endif /* RT_RTIMAG_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
