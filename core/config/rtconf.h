/******************************************************************************/
/* Copyright (c) 2013-2015 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTCONF_H
#define RT_RTCONF_H

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtconf.h: Main configuration file.
 *
 * Definitions provided in this file are used to configure different subsystems
 * of the engine by specifying paths, thresholds and optimization flags.
 */

/******************************************************************************/
/*******************************   DEFINITIONS   ******************************/
/******************************************************************************/

#if RT_DEBUG == 1

#define RT_STACK_DEPTH          10 /* context stack depth for secondary rays */
#define RT_THREADS_NUM          16 /* number of threads for update and render */

#else  /* RT_DEBUG */

#define RT_STACK_DEPTH          10 /* context stack depth for secondary rays */
#define RT_THREADS_NUM          16 /* number of threads for update and render */

#endif /* RT_DEBUG */

#define RT_CHUNK_SIZE           4096 /* heap chunk size granularity */

#define RT_PATH_STRFY(p)        #p
#define RT_PATH_TOSTR(p)        RT_PATH_STRFY(p)
#define RT_PATH_TEXTURES        RT_PATH_TOSTR(RT_PATH)"data/textures/"
#define RT_PATH_DUMP            RT_PATH_TOSTR(RT_PATH)"dump/"
#define RT_PATH_DUMP_LOG        RT_PATH_TOSTR(RT_PATH)"dump/log.txt"
#define RT_PATH_DUMP_ERR        RT_PATH_TOSTR(RT_PATH)"dump/err.txt"

#if RT_EMBED_FILEIO == 1 && RT_EMBED_TEX == 0
#error "RT_EMBED_TEX must be enabled if RT_EMBED_FILEIO is enabled"
#endif /* RT_EMBED_FILEIO, RT_EMBED_TEX */

#define RT_VERTS_LIMIT          8  /* maximum number of verts for bbox */
#define RT_EDGES_LIMIT          12 /* maximum number of edges for bbox */
#define RT_FACES_LIMIT          6  /* maximum number of faces for bbox */

#define RT_TILE_W               8  /* screen tile width  in pixels (%4 == 0) */
#define RT_TILE_H               8  /* screen tile height in pixels */

#define RT_TILE_THRESHOLD       0.2f
#define RT_LINE_THRESHOLD       0.01f
#define RT_CLIP_THRESHOLD       0.01f
#define RT_CULL_THRESHOLD       0.0001f

#define RT_DEPS_THRESHOLD       0.00000000001f /* <- maximum for two-plane */
#define RT_TEPS_THRESHOLD       0.0000001f /* <- minimum for roots sorting */

/*
 * Runtime optimization flags,
 * define particular flag as 0
 * to turn respective optimization off at compile time.
 */
#define RT_OPTS_NONE            0
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
        RT_OPTS_REMOVE          )

#endif /* RT_RTCONF_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
