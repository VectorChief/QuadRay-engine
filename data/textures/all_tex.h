/******************************************************************************/
/* Copyright (c) 2013-2019 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_ALL_TEX_H
#define RT_ALL_TEX_H

#include "format.h"

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * all_tex.h: Common file for all textures.
 *
 * Recommended naming scheme for textures:
 *
 * - Textures can be loaded from external binary image files or embedded
 *   as C static array initializers converted from respective images.
 *   In the latter case static array names start with dt_ prefix
 *   followed by a three-letter code corresponding to texture's type
 *   in a form of ***_ followed by texture's name.
 *
 * - Textures can be of the following types defined in format.h:
 *
 *   tex for PCOLOR, ACOLOR
 *   trn for PALPHA
 *   nrm for NORMAL
 *   dff for DIFFUS
 *   spc for SPECUL
 *   rfl for REFLEC
 *   rfr for REFRAC
 *   lum for LUMINA
 *   dtl for DETAIL
 *
 * - Textures are usually placed in separate files ***_*.h, where *** is
 *   a three-letter code corresponding to texture's type described above
 *   and * is texture's name (for example tex_crate01.h), texture names in this
 *   case follow the same rules as described above (for example dt_tex_crate01).
 *
 * - Binary image files follow the same naming scheme as .h files above
 *   with the exception for file extension, which must correspond
 *   to a particular binary format.
 */

/******************************************************************************/
/***********************************   TEX   **********************************/
/******************************************************************************/

#if RT_EMBED_TEX == 1
#include "tex_crate01.h"
#endif /* RT_EMBED_TEX */

#endif /* RT_ALL_TEX_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
