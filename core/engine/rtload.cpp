/******************************************************************************/
/* Copyright (c) 2013 VectorChief (at github, bitbucket, sourceforge)         */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#include <string.h>

#include "rtload.h"
#include "system.h"

/******************************************************************************/
/********************************   TEXTURE   *********************************/
/******************************************************************************/

/* Load texture from file to memory.
 */
rt_void load_texture(rt_Registry *rg, rt_pstr name, rt_TEX *tx)
{
    rt_word boffset;
    rt_cell bwidth, bheight;
    rt_half bdepth;

    rt_File *f = RT_NULL;
    rt_word *p = RT_NULL;
    rt_cell i, n;

    tx->ptex = RT_NULL;
    tx->x_dim = 0;
    tx->y_dim = 0;

    rt_pstr path = RT_PATH_TEXTURES;
    rt_cell len = strlen(path);
    rt_char *fullpath = (rt_char *)rg->alloc(len + strlen(name) + 1, 0);

    strcpy(fullpath, path);
    strcpy(fullpath + len, name);

    do
    {
        f = new rt_File(fullpath, "rb");

        if (f->error() != 0)
        {
            break;
        }
        if (f->seek(10, SEEK_SET) != 0)
        {
            break;
        }
        if (f->read(&boffset, sizeof(boffset), 1) != 1)
        {
            break;
        }
        if (f->seek(18, SEEK_SET) != 0)
        {
            break;
        }
        if (f->read(&bwidth, sizeof(bwidth), 1) != 1)
        {
            break;
        }
        if (f->seek(22, SEEK_SET) != 0)
        {
            break;
        }
        if (f->read(&bheight, sizeof(bheight), 1) != 1)
        {
            break;
        }
        if (f->seek(28, SEEK_SET) != 0)
        {
            break;
        }
        if (f->read(&bdepth, sizeof(bdepth), 1) != 1)
        {
            break;
        }
        if (bdepth != 24)
        {
            break;
        }

        n = bwidth * bheight;

        tx->ptex = rg->alloc(n * sizeof(rt_word), RT_ALIGN);
        if (tx->ptex == RT_NULL)
        {
            break;
        }

        tx->x_dim = bwidth;
        tx->y_dim = bheight;

        f->seek(boffset, SEEK_SET);

        for (i = 0, p = (rt_word *)tx->ptex; i < n; i++, p++)
        {
            if (f->read(p, 3, 1) != 1)
            {
                break;
            }
        }

        if (i < n)
        {
            break;
        }

        delete f;
        return;
    }
    while (0);

    if (f != NULL)
    {
        delete f;
    }

    throw rt_Exception("Failed to load texture");
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
