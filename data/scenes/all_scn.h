/******************************************************************************/
/* Copyright (c) 2013-2022 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_ALL_SCN_H
#define RT_ALL_SCN_H

#include "format.h"

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * all_scn.h: Common file for all scenes.
 *
 * Recommended naming scheme for scenes:
 *
 * - All scene namespaces start with scn_ followed by scene's specific name,
 *   which is usually demo or test followed by the scene number.
 *
 * - Every scene must have SCENE object defined in format.h called sc_root.
 *   The root object contains references to the rest of the objects tree
 *   and is used by the engine to instantiate the scene in run time.
 *
 * - Every scene must have at least one CAMERA object defined in format.h.
 *
 * - Scenes are usually placed in separate files scn_*.h, where * is
 *   scene's name (for example scn_demo01.h), scene names in this
 *   case follow the same rules as described above (for example scn_demo01)
 */

/******************************************************************************/
/***********************************   SCN   **********************************/
/******************************************************************************/

#include "scn_demo01.h"
#include "scn_demo02.h"
#include "scn_demo03.h"

#endif /* RT_ALL_SCN_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
