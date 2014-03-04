/******************************************************************************/
/* Copyright (c) 2013-2014 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#include <string.h>

#include "rtimag.h"
#include "rtconf.h"

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtimag.cpp: Implementation of the image utils library.
 *
 * Utility file for the engine responsible for image loading, saving and
 * conversion to C static array initializer format suitable for embedding.
 *
 * Utility file names are usually in the form of rt****.cpp/h,
 * while core engine parts are located in ******.cpp/h files.
 */

/******************************************************************************/
/********************************   TEXTURE   *********************************/
/******************************************************************************/

/*
 * Load image from file to memory.
 */
rt_void load_image(rt_Heap *hp, rt_pstr name, rt_TEX *tx)
{
    rt_word boffset, bzero;
    rt_cell bwidth, bheight;
    rt_half bdepth, bplanes, bsig;

    rt_File *f = RT_NULL;
    rt_word *p = RT_NULL;
    rt_cell i, n;

    tx->ptex = RT_NULL;
    tx->x_dim = 0;
    tx->y_dim = 0;

    rt_pstr path = RT_PATH_TEXTURES;
    rt_cell len = strlen(path);
    rt_char *fullpath = (rt_char *)hp->alloc(len + strlen(name) + 1, 0);

    strcpy(fullpath, path);
    strcpy(fullpath + len, name);

    do
    {
        f = new rt_File(fullpath, "rb");

        if (f->error() != 0)
        {
            break;
        }
        if (f->load(&bsig, sizeof(bsig), 1) != 1)
        {
            break;
        }
        if (f->seek(10, SEEK_SET) != 0)
        {
            break;
        }
        if (f->load(&boffset, sizeof(boffset), 1) != 1)
        {
            break;
        }
        if (f->seek(18, SEEK_SET) != 0)
        {
            break;
        }
        if (f->load(&bwidth, sizeof(bwidth), 1) != 1)
        {
            break;
        }
        if (f->load(&bheight, sizeof(bheight), 1) != 1)
        {
            break;
        }
        if (f->load(&bplanes, sizeof(bplanes), 1) != 1)
        {
            break;
        }
        if (f->load(&bdepth, sizeof(bdepth), 1) != 1)
        {
            break;
        }
        if (f->load(&bzero, sizeof(bzero), 1) != 1)
        {
            break;
        }
        if (bsig != 0x4D42 || bplanes != 1 || bdepth != 24 || bzero != 0)
        {
            break;
        }

        tx->x_dim = RT_ABS(bwidth);
        tx->y_dim = RT_ABS(bheight);

        n = tx->x_dim * tx->y_dim;

        tx->ptex = hp->alloc(n * sizeof(rt_word), RT_ALIGN);
        if (tx->ptex == RT_NULL)
        {
            break;
        }

        f->seek(boffset, SEEK_SET);

        for (i = 0, p = (rt_word *)tx->ptex; i < n; i++, p++)
        {
            if (f->load(p, 3, 1) != 1)
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

    if (f != RT_NULL)
    {
        delete f;
    }

    throw rt_Exception("failed to load image");
}

/*
 * Save image from memory to file.
 */
rt_void save_image(rt_Heap *hp, rt_pstr name, rt_TEX *tx)
{
    rt_word boffset = 54, binfo = 40, bmeter = 4000, bzero = 0;
    rt_cell bwidth = RT_ABS(tx->x_dim), bheight = RT_ABS(tx->y_dim);
    rt_word bsize = boffset + bwidth * bheight * 3;
    rt_half bdepth = 24, bplanes = 1, bsig = 0x4D42;

    rt_File *f = RT_NULL;
    rt_word *p = RT_NULL;
    rt_cell i, n;

    rt_pstr path = RT_PATH_DUMP;
    rt_cell len = strlen(path);
    rt_char *fullpath = (rt_char *)hp->alloc(len + strlen(name) + 1, 0);

    strcpy(fullpath, path);
    strcpy(fullpath + len, name);

    do
    {
        f = new rt_File(fullpath, "wb");

        if (f->error() != 0)
        {
            break;
        }
        if (f->save(&bsig, sizeof(bsig), 1) != 1)
        {
            break;
        }
        if (f->save(&bsize, sizeof(bsize), 1) != 1)
        {
            break;
        }
        if (f->save(&bzero, sizeof(bzero), 1) != 1)
        {
            break;
        }
        if (f->save(&boffset, sizeof(boffset), 1) != 1)
        {
            break;
        }
        if (f->save(&binfo, sizeof(binfo), 1) != 1)
        {
            break;
        }
        if (f->save(&tx->x_dim, sizeof(tx->x_dim), 1) != 1)
        {
            break;
        }
        if (f->save(&tx->y_dim, sizeof(tx->y_dim), 1) != 1)
        {
            break;
        }
        if (f->save(&bplanes, sizeof(bplanes), 1) != 1)
        {
            break;
        }
        if (f->save(&bdepth, sizeof(bdepth), 1) != 1)
        {
            break;
        }
        if (f->save(&bzero, sizeof(bzero), 1) != 1)
        {
            break;
        }

        bsize -= boffset;

        if (f->save(&bsize, sizeof(bsize), 1) != 1)
        {
            break;
        }
        if (f->save(&bmeter, sizeof(bmeter), 1) != 1)
        {
            break;
        }
        if (f->save(&bmeter, sizeof(bmeter), 1) != 1)
        {
            break;
        }
        if (f->save(&bzero, sizeof(bzero), 1) != 1)
        {
            break;
        }
        if (f->save(&bzero, sizeof(bzero), 1) != 1)
        {
            break;
        }

        n = bwidth * bheight;

        for (i = 0, p = (rt_word *)tx->ptex; i < n; i++, p++)
        {
            if (f->save(p, 3, 1) != 1)
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

    if (f != RT_NULL)
    {
        delete f;
    }

    throw rt_Exception("failed to save image");
}

/*
 * Convert image from file to C static array initializer format.
 * Parameter fullpath must be editable char array.
 */
rt_void convert_image(rt_char *fullpath)
{
    rt_word boffset;
    rt_cell bwidth, bheight;
    rt_half bdepth;

    rt_File *f = RT_NULL;
    rt_File *o = RT_NULL;
    rt_char dig[] = "0123456789ABCDEF";
    rt_char s[10];
    rt_word p;
    rt_cell i, n, k, len = strlen(fullpath);

    if (strcmp(&fullpath[len - 4], ".bmp"))
    {
        RT_LOGI("?");
        return;
    }

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
        if (f->load(&boffset, sizeof(boffset), 1) != 1)
        {
            break;
        }
        if (f->seek(18, SEEK_SET) != 0)
        {
            break;
        }
        if (f->load(&bwidth, sizeof(bwidth), 1) != 1)
        {
            break;
        }
        if (f->seek(22, SEEK_SET) != 0)
        {
            break;
        }
        if (f->load(&bheight, sizeof(bheight), 1) != 1)
        {
            break;
        }
        if (f->seek(28, SEEK_SET) != 0)
        {
            break;
        }
        if (f->load(&bdepth, sizeof(bdepth), 1) != 1)
        {
            break;
        }
        if (bdepth != 24)
        {
            break;
        }

        fullpath[len - 3] = 'h';
        fullpath[len - 2] =  0;
        fullpath[len - 1] =  0;

        o = new rt_File(fullpath, "w+");

        if (o->error() != 0)
        {
            break;
        }

        fullpath[len - 4] = 0;
        for (len = len - 4; len > 0 && fullpath[len - 1] != '\\'
                                    && fullpath[len - 1] != '/'; len--);

        o->fprint("rt_word dt_%s[%d][%d] =\n", &fullpath[len],
                                               bheight, bwidth);
        o->fprint("{");

        n = bwidth * bheight;

        f->seek(boffset, SEEK_SET);

        for (i = 0, p = 0; i < n; i++)
        {
            if (f->load(&p, 3, 1) != 1)
            {
                break;
            }

            if (i % 6 == 0)
            {
                o->fprint("\n   ");
            }

            for (k = 0; k < 8; k++)
            {
                s[k] = dig[p >> (28 - k * 4) & 0xF];
            }
            s[k] = 0;

            o->fprint(" 0x%s,", s);
        }

        o->fprint("\n};\n");

        if (i < n)
        {
            break;
        }

        delete f;
        delete o;

        RT_LOGI(".");
        return;
    }
    while (0);

    if (f != RT_NULL)
    {
        delete f;
    }
    if (o != RT_NULL)
    {
        delete o;
    }

    RT_LOGI("x");
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
