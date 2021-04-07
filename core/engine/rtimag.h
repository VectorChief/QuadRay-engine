/******************************************************************************/
/* Copyright (c) 2013-2021 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTIMAG_H
#define RT_RTIMAG_H

#include "rtbase.h"
#include "format.h"
#include "system.h"

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtimag.h: Interface for the image utils library.
 *
 * More detailed description of this subsystem is given in rtimag.cpp.
 * Recommended naming scheme for C++ types and definitions is given in rtbase.h.
 */

/******************************************************************************/
/*******************************   DEFINITIONS   ******************************/
/******************************************************************************/

#if RT_EMBED_FILEIO == 1 && RT_EMBED_TEX == 0
#error "RT_EMBED_TEX must be enabled if RT_EMBED_FILEIO is enabled"
#endif /* RT_EMBED_FILEIO, RT_EMBED_TEX */

#define RT_PATH_TEXTURES        RT_PATH_TOSTR(RT_PATH)"data/textures/"

/******************************************************************************/
/********************************   TEXTURE   *********************************/
/******************************************************************************/

/*
 * Load image from file to memory.
 */
rt_void load_image(rt_Heap *hp, rt_pstr name, rt_TEX *tx);

/*
 * Save image from memory to file.
 */
rt_void save_image(rt_Heap *hp, rt_pstr name, rt_TEX *tx);

/*
 * Convert image from file to C static array initializer format.
 */
rt_si32 convert_image(rt_Heap *hp, rt_pstr name);

#endif /* RT_RTIMAG_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
