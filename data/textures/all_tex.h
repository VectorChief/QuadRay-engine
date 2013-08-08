/******************************************************************************/
/* Copyright (c) 2013 VectorChief (at github, bitbucket, sourceforge)         */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_ALL_TEX_H
#define RT_ALL_TEX_H

#include "format.h"

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/* Recommended name prefixes for texture data types defined in format.h:
 *
 * - tex for PCOLOR, ACOLOR
 * - trn for PALPHA
 * - nrm for NORMAL
 * - dff for DIFFUS
 * - spc for SPECUL
 * - rfl for REFLEC
 * - rfr for REFRAC
 * - lum for LUMINA
 * - dtl for DETAIL
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
