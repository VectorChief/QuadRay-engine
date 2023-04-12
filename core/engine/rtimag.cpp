/******************************************************************************/
/* Copyright (c) 2013-2023 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#if RT_EMBED_FILEIO == 0
#include <stdio.h>
#endif /* RT_EMBED_FILEIO */
#include <string.h>

#include "rtimag.h"

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

/* endian-agnostic serialization from little-endian BMP format */

#define RT_LOAD_H(h, p)                                                     \
        h = (((rt_byte *)(p))[0] << 0x00) |                                 \
            (((rt_byte *)(p))[1] << 0x08)

#define RT_LOAD_W(w, p)                                                     \
        w = (((rt_byte *)(p))[0] << 0x00) |                                 \
            (((rt_byte *)(p))[1] << 0x08) |                                 \
            (((rt_byte *)(p))[2] << 0x10) |                                 \
            (((rt_byte *)(p))[3] << 0x18)

/*
 * Load image from file to memory.
 */
rt_void load_image(rt_Heap *hp, rt_pstr name, rt_TEX *tx)
{
#if RT_EMBED_FILEIO == 0
    rt_ui32 *p = RT_NULL;
    rt_si32 i, k, n;

    rt_pstr path = RT_PATH_TEXTURES;
    rt_size len = strlen(path);
    rt_char *fullpath = (rt_char *)hp->alloc(len + strlen(name) + 1, 0);

    strcpy(fullpath, path);
    strcpy(fullpath + len, name);

    rt_File fl(fullpath, "rb");
    rt_File *f = &fl;

    /* release memory for temporary fullpath string,
     * would also release all allocs made after fullpath */
    hp->release(fullpath);

    rt_ui32 boffset, bzero, buf;
    rt_si32 bwidth, bheight;
    rt_ui16 bdepth, bplanes, bsig;

    tx->ptex = RT_NULL;
    tx->x_dim = 0;
    tx->y_dim = 0;

    do /* use "do {break} while(0)" instead of "goto label" */
    {
        if (f->error() != 0)
        {
            break;
        }

        if (f->load(&buf, sizeof(bsig), 1) != 1)
        {
            break;
        }
        RT_LOAD_H(bsig, &buf);

        if (f->seek(10, SEEK_SET) != 0)
        {
            break;
        }

        if (f->load(&buf, sizeof(boffset), 1) != 1)
        {
            break;
        }
        RT_LOAD_W(boffset, &buf);

        if (f->seek(18, SEEK_SET) != 0)
        {
            break;
        }

        if (f->load(&buf, sizeof(bwidth), 1) != 1)
        {
            break;
        }
        RT_LOAD_W(bwidth, &buf);

        if (f->load(&buf, sizeof(bheight), 1) != 1)
        {
            break;
        }
        RT_LOAD_W(bheight, &buf);

        if (f->load(&buf, sizeof(bplanes), 1) != 1)
        {
            break;
        }
        RT_LOAD_H(bplanes, &buf);

        if (f->load(&buf, sizeof(bdepth), 1) != 1)
        {
            break;
        }
        RT_LOAD_H(bdepth, &buf);

        if (f->load(&buf, sizeof(bzero), 1) != 1)
        {
            break;
        }
        RT_LOAD_W(bzero, &buf);

        if (bsig != 0x4D42 || bplanes != 1 || bdepth != 24 || bzero != 0)
        {
            break;
        }

        tx->x_dim = RT_ABS32(bwidth);
        tx->y_dim = RT_ABS32(bheight);

        n = tx->x_dim * tx->y_dim;

        tx->ptex = hp->alloc(n * sizeof(rt_ui32), RT_ALIGN);
        if (tx->ptex == RT_NULL)
        {
            break;
        }

        f->seek(boffset, SEEK_SET);

        for (i = 0, p = (rt_ui32 *)tx->ptex; i < n; i++, p++)
        {
            if (f->load(&buf, 3, 1) != 1)
            {
                break;
            }
            RT_LOAD_W(*p, &buf);
            if ((i+1) % bwidth == 0) /* <- temp fix for tex's stride */
            {
                k = ((bwidth*3+3)/4)*4 - (bwidth*3);
                if (k != 0 && f->load(&buf, k, 1) != 1)
                {
                    break;
                }
            }
        }

        if (i < n)
        {
            break;
        }

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

    throw rt_Exception("failed to load image");
#endif /* RT_EMBED_FILEIO */
}

/* endian-agnostic serialization to little-endian BMP format */

#define RT_SAVE_H(h, p)                                                     \
        ((rt_byte *)(p))[0] = ((h) >> 0x00) & 0xFF;                         \
        ((rt_byte *)(p))[1] = ((h) >> 0x08) & 0xFF

#define RT_SAVE_W(w, p)                                                     \
        ((rt_byte *)(p))[0] = ((w) >> 0x00) & 0xFF;                         \
        ((rt_byte *)(p))[1] = ((w) >> 0x08) & 0xFF;                         \
        ((rt_byte *)(p))[2] = ((w) >> 0x10) & 0xFF;                         \
        ((rt_byte *)(p))[3] = ((w) >> 0x18) & 0xFF

/*
 * Save image from memory to file.
 */
rt_void save_image(rt_Heap *hp, rt_pstr name, rt_TEX *tx)
{
#if RT_EMBED_FILEIO == 0
    rt_ui32 *p = RT_NULL;
    rt_si32 i, k, n;

    rt_pstr path = RT_PATH_DUMP;
    rt_size len = strlen(path);
    rt_char *fullpath = (rt_char *)hp->alloc(len + strlen(name) + 1, 0);

    strcpy(fullpath, path);
    strcpy(fullpath + len, name);

    rt_File fl(fullpath, "wb");
    rt_File *f = &fl;

    /* release memory for temporary fullpath string,
     * would also release all allocs made after fullpath */
    hp->release(fullpath);

    rt_ui32 boffset = 54, binfo = 40, bmeter = 4000, bzero = 0;
    rt_si32 bwidth = RT_ABS32(tx->x_dim), bheight = RT_ABS32(tx->y_dim);
    rt_ui32 bsize = boffset + bwidth * bheight * 3, buf;
    rt_ui16 bdepth = 24, bplanes = 1, bsig = 0x4D42;

    do /* use "do {break} while(0)" instead of "goto label" */
    {
        if (f->error() != 0)
        {
            break;
        }

        RT_SAVE_H(bsig, &buf);
        if (f->save(&buf, sizeof(bsig), 1) != 1)
        {
            break;
        }

        RT_SAVE_W(bsize, &buf);
        if (f->save(&buf, sizeof(bsize), 1) != 1)
        {
            break;
        }

        RT_SAVE_W(bzero, &buf);
        if (f->save(&buf, sizeof(bzero), 1) != 1)
        {
            break;
        }

        RT_SAVE_W(boffset, &buf);
        if (f->save(&buf, sizeof(boffset), 1) != 1)
        {
            break;
        }

        RT_SAVE_W(binfo, &buf);
        if (f->save(&buf, sizeof(binfo), 1) != 1)
        {
            break;
        }

        RT_SAVE_W(tx->x_dim, &buf);
        if (f->save(&buf, sizeof(tx->x_dim), 1) != 1)
        {
            break;
        }

        RT_SAVE_W(tx->y_dim, &buf);
        if (f->save(&buf, sizeof(tx->y_dim), 1) != 1)
        {
            break;
        }

        RT_SAVE_H(bplanes, &buf);
        if (f->save(&buf, sizeof(bplanes), 1) != 1)
        {
            break;
        }

        RT_SAVE_H(bdepth, &buf);
        if (f->save(&buf, sizeof(bdepth), 1) != 1)
        {
            break;
        }

        RT_SAVE_W(bzero, &buf);
        if (f->save(&buf, sizeof(bzero), 1) != 1)
        {
            break;
        }

        bsize -= boffset;

        RT_SAVE_W(bsize, &buf);
        if (f->save(&buf, sizeof(bsize), 1) != 1)
        {
            break;
        }

        RT_SAVE_W(bmeter, &buf);
        if (f->save(&buf, sizeof(bmeter), 1) != 1)
        {
            break;
        }

        RT_SAVE_W(bmeter, &buf);
        if (f->save(&buf, sizeof(bmeter), 1) != 1)
        {
            break;
        }

        RT_SAVE_W(bzero, &buf);
        if (f->save(&buf, sizeof(bzero), 1) != 1)
        {
            break;
        }

        RT_SAVE_W(bzero, &buf);
        if (f->save(&buf, sizeof(bzero), 1) != 1)
        {
            break;
        }

        n = bwidth * bheight;

        for (i = 0, p = (rt_ui32 *)tx->ptex; i < n; i++, p++)
        {
            RT_SAVE_W(*p, &buf);
            if (f->save(&buf, 3, 1) != 1)
            {
                break;
            }
            if ((i+1) % bwidth == 0) /* <- temp fix for tex's stride */
            {
                p += RT_MAX(tx->tex_num, bwidth) - bwidth;
                k = ((bwidth*3+3)/4)*4 - (bwidth*3); buf = 0;
                if (k != 0 && f->save(&buf, k, 1) != 1)
                {
                    break;
                }
            }
        }

        if (i < n)
        {
            break;
        }

        return;
    }
    while (0);

    throw rt_Exception("failed to save image");
#endif /* RT_EMBED_FILEIO */
}

/*
 * Convert image from file to C static array initializer format.
 */
rt_si32 convert_image(rt_Heap *hp, rt_pstr name)
{
#if RT_EMBED_FILEIO == 0
    rt_ui32 *p = RT_NULL;
    rt_si32 i, n, r = 0;

    rt_pstr path = RT_PATH_TEXTURES;
    rt_size len = strlen(path), dot = len;
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

        rt_File fl(fullpath, "w+");
        rt_File *f = &fl;

        if (f->error() != 0)
        {
            break;
        }

        fullpath[dot] = 0;

        f->fprint("rt_ui32 dt_%s[%d][%d] =\n", &fullpath[len],
                                               tx->y_dim, tx->x_dim);
        f->fprint("{");

        n = tx->x_dim * tx->y_dim;

        for (i = 0, p = (rt_ui32 *)tx->ptex; i < n; i++, p++)
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

        r = 1;
    }
    while (0);

    /* release memory for temporary fullpath string,
     * would also release all allocs made after fullpath */
    hp->release(fullpath);

    return r;
#endif /* RT_EMBED_FILEIO */
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
