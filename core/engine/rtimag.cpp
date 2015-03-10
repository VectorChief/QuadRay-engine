/******************************************************************************/
/* Copyright (c) 2013-2015 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#if RT_EMBED_FILEIO == 0
#include <stdio.h>
#endif /* RT_EMBED_FILEIO */
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
#if RT_EMBED_FILEIO == 0
    rt_File *f = RT_NULL;
    rt_word *p = RT_NULL;
    rt_cell i, n;

    rt_pstr path = RT_PATH_TEXTURES;
    rt_cell len = strlen(path);
    rt_char *fullpath = (rt_char *)hp->alloc(len + strlen(name) + 1, 0);

    strcpy(fullpath, path);
    strcpy(fullpath + len, name);

    f = new rt_File(fullpath, "rb");

    /* release memory for temporary fullpath string,
     * would also release all allocs made after fullpath */
    hp->release(fullpath);

    rt_word boffset, bzero;
    rt_cell bwidth, bheight;
    rt_half bdepth, bplanes, bsig;

    tx->ptex = RT_NULL;
    tx->x_dim = 0;
    tx->y_dim = 0;

    do /* use "do {break} while(0)" instead of "goto label" */
    {
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

    if (tx->ptex != RT_NULL)
    {
        /* release memory for texture data as loading failed,
         * would also release all allocs made after tx->ptex */
        hp->release(tx->ptex);
        tx->ptex = RT_NULL;
    }

    delete f;
    throw rt_Exception("failed to load image");
#endif /* RT_EMBED_FILEIO */
}

/*
 * Save image from memory to file.
 */
rt_void save_image(rt_Heap *hp, rt_pstr name, rt_TEX *tx)
{
#if RT_EMBED_FILEIO == 0
    rt_File *f = RT_NULL;
    rt_word *p = RT_NULL;
    rt_cell i, n;

    rt_pstr path = RT_PATH_DUMP;
    rt_cell len = strlen(path);
    rt_char *fullpath = (rt_char *)hp->alloc(len + strlen(name) + 1, 0);

    strcpy(fullpath, path);
    strcpy(fullpath + len, name);

    f = new rt_File(fullpath, "wb");

    /* release memory for temporary fullpath string,
     * would also release all allocs made after fullpath */
    hp->release(fullpath);

    rt_word boffset = 54, binfo = 40, bmeter = 4000, bzero = 0;
    rt_cell bwidth = RT_ABS(tx->x_dim), bheight = RT_ABS(tx->y_dim);
    rt_word bsize = boffset + bwidth * bheight * 3;
    rt_half bdepth = 24, bplanes = 1, bsig = 0x4D42;

    do /* use "do {break} while(0)" instead of "goto label" */
    {
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

    delete f;
    throw rt_Exception("failed to save image");
#endif /* RT_EMBED_FILEIO */
}

/*
 * Convert image from file to C static array initializer format.
 */
rt_void convert_image(rt_Heap *hp, rt_pstr name)
{
#if RT_EMBED_FILEIO == 0
    rt_File *f = RT_NULL;
    rt_word *p = RT_NULL;
    rt_cell i, n;

    rt_pstr path = RT_PATH_TEXTURES;
    rt_cell len = strlen(path), dot = len;
    rt_char *fullpath = (rt_char *)hp->alloc(len + strlen(name) + 3, 0);

    strcpy(fullpath, path);
    strcpy(fullpath + len, name);

    while (fullpath[dot] != 0 && fullpath[dot] != '.') dot++;

    fullpath[dot + 0] = '.';
    fullpath[dot + 1] = 'h';
    fullpath[dot + 2] =  0;

    rt_TEX tex, *tx = &tex;

    do /* use "do {break} while(0)" instead of "goto label" */
    {
        try
        {
            load_image(hp, name, tx);
        }
        catch (rt_Exception e)
        {
            break;
        }

        f = new rt_File(fullpath, "w+");

        if (f == RT_NULL)
        {
            break;
        }
        if (f->error() != 0)
        {
            break;
        }

        fullpath[dot] = 0;

        f->fprint("rt_word dt_%s[%d][%d] =\n", &fullpath[len],
                                               tx->y_dim, tx->x_dim);
        f->fprint("{");

        n = tx->x_dim * tx->y_dim;

        for (i = 0, p = (rt_word *)tx->ptex; i < n; i++, p++)
        {
            if (i % 6 == 0)
            {
                f->fprint("\n   ");
            }

            f->fprint(" 0x%08X,", *p);
        }

        f->fprint("\n};\n");

        if (i < n)
        {
            break;
        }

        /* release memory for temporary fullpath string,
         * would also release all allocs made after fullpath */
        hp->release(fullpath);

        delete f;
        RT_LOGI(".");
        return;
    }
    while (0);

    /* release memory for temporary fullpath string,
     * would also release all allocs made after fullpath */
    hp->release(fullpath);

    delete f;
    RT_LOGI("x");
#endif /* RT_EMBED_FILEIO */
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
