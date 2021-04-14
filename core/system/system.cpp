/******************************************************************************/
/* Copyright (c) 2013-2021 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#if RT_EMBED_STDOUT == 0
#include <stdio.h>
#endif /* RT_EMBED_STDOUT */

#include "system.h"

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * system.cpp: Implementation of the system layer.
 *
 * System layer of the engine responsible for file I/O operations,
 * fast linear memory heap allocations, error and info logging
 * as well as definitions of List template and Exception classes.
 */

/******************************************************************************/
/**********************************   FILE   **********************************/
/******************************************************************************/

/*
 * Instantiate and open file with given "name" and I/O "mode".
 */
rt_File::rt_File(rt_pstr name, rt_pstr mode)
{
#if RT_EMBED_FILEIO == 0
    file = RT_NULL;
    if (name != RT_NULL && mode != RT_NULL)
    {
        file = fopen(name, mode);
    }
#endif /* RT_EMBED_FILEIO */
}

/*
 * Set file position to "offset" bytes from "origin".
 */
rt_si32 rt_File::seek(rt_cell offset, rt_si32 origin)
{
    return
#if RT_EMBED_FILEIO == 0
    file != RT_NULL ? fseek(file, offset, origin) :
#endif /* RT_EMBED_FILEIO */
    0;
}

/*
 * Load "num" elements of "size" bytes into "data" buffer.
 */
rt_size rt_File::load(rt_pntr data, rt_size size, rt_size num)
{
    return
#if RT_EMBED_FILEIO == 0
    file != RT_NULL ? fread(data, size, num, file) :
#endif /* RT_EMBED_FILEIO */
    0;
}

/*
 * Save "num" elements of "size" bytes from "data" buffer.
 */
rt_size rt_File::save(rt_pntr data, rt_size size, rt_size num)
{
    return
#if RT_EMBED_FILEIO == 0
    file != RT_NULL ? fwrite(data, size, num, file) :
#endif /* RT_EMBED_FILEIO */
    0;
}

/*
 * Print formatted string with variable number of arguments.
 */
rt_si32 rt_File::fprint(rt_pstr format, ...)
{
    rt_si32 ret = 0;
#if RT_EMBED_FILEIO == 0
    va_list args;
    va_start(args, format);
    if (file != RT_NULL)
    {
        ret = vfprintf(file, format, args);
        fflush(file);
    }
    va_end(args);
#endif /* RT_EMBED_FILEIO */
    return ret;
}

/*
 * Print formatted string with given list of arguments.
 */
rt_si32 rt_File::vprint(rt_pstr format, va_list args)
{
    rt_si32 ret = 0;
#if RT_EMBED_FILEIO == 0
    if (file != RT_NULL)
    {
        ret = vfprintf(file, format, args);
        fflush(file);
    }
#endif /* RT_EMBED_FILEIO */
    return ret;
}

/*
 * Return error code.
 */
rt_si32 rt_File::error()
{
    return 
#if RT_EMBED_FILEIO == 0
    file == RT_NULL ? 1 :
#endif /* RT_EMBED_FILEIO */
    0;
}

/*
 * Deinitialize file after flushing and closing it.
 */
rt_File::~rt_File()
{
#if RT_EMBED_FILEIO == 0
    if (file != RT_NULL)
    {
        fflush(file);
        fclose(file);
    }
    file = RT_NULL;
#endif /* RT_EMBED_FILEIO */
}

/******************************************************************************/
/**********************************   HEAP   **********************************/
/******************************************************************************/

/*
 * Instantiate heap with platform-specific alloc/free functions.
 */
rt_Heap::rt_Heap(rt_FUNC_ALLOC f_alloc, rt_FUNC_FREE f_free)
{
    this->f_alloc = f_alloc;
    this->f_free  = f_free;

    /* init heap */
    head = RT_NULL;
    obj_head = RT_NULL;
    chunk_alloc(0, RT_ALIGN);
}

/*
 * Allocate new chunk at least "size" bytes with given "align",
 * and link it to the list as head.
 */
rt_void rt_Heap::chunk_alloc(rt_size size, rt_ui32 align)
{
    /* compute align and new chunk's size */
    rt_size mask = align > 0 ? align - 1 : 0;
    rt_size real_size = size + mask + sizeof(rt_CHUNK) + (RT_CHUNK_SIZE - 1);
    real_size = (real_size / RT_CHUNK_SIZE) * RT_CHUNK_SIZE;
    rt_CHUNK *chunk = (rt_CHUNK *)f_alloc(real_size);

    /* check for out of memory */
    if (chunk == RT_NULL)
    {
        throw rt_Exception("out of memory in heap's chunk_alloc");
    }

    /* prepare new chunk */
    chunk->ptr = (rt_byte *)chunk + sizeof(rt_CHUNK);
    chunk->ptr = (rt_byte *)(((rt_size)chunk->ptr + mask) & ~mask);
    chunk->end = (rt_byte *)chunk + real_size;
    chunk->size = real_size;
    chunk->next = head;

    head = chunk;
}

/*
 * Reserve given "size" bytes of memory with given "align",
 * move heap pointer ahead for the next alloc.
 */
rt_pntr rt_Heap::alloc(rt_size size, rt_ui32 align)
{
    rt_byte *ptr = (rt_byte *)reserve(size, align);

    head->ptr = ptr + size;

    return ptr;
}

/*
 * Reserve given "size" bytes of memory with given "align",
 * don't move heap pointer. Next alloc will begin in reserved area.
 */
rt_pntr rt_Heap::reserve(rt_size size, rt_ui32 align)
{
    /* compute align */
    rt_size mask = align > 0 ? align - 1 : 0;
    rt_byte *ptr = (rt_byte *)(((rt_size)head->ptr + mask) & ~mask);

    /* allocate bigger chunk, if current doesn't fit */
    if (head->end < ptr + size)
    {
        chunk_alloc(size, align);
        ptr = head->ptr;
    }
    /* move heap pointer to newly aligned "ptr" */
    else
    {
        head->ptr = ptr;
    }

    return ptr;
}

/*
 * Release all allocs made after given "ptr" was reserved.
 * Next alloc will begin from "ptr" if align and size fits.
 */
rt_pntr rt_Heap::release(rt_pntr ptr)
{
    /* search chunk where "ptr" belongs,
     * free chunks allocated afterwards */
    while (head != RT_NULL && (ptr < head + 1 || ptr >= head->end))
    {
        rt_pntr *obj = &obj_head;

        /* traverse the list of free objects */
        while (*obj != RT_NULL)
        {
            /* remove free object from the chunk */
            if (*obj >= head + 1 && *obj < head->end)
            {
                *((rt_si32 *)*obj - 2) = 0; /* clear object's "magic" */
                *obj = *(rt_pntr *)*obj; /* remove object from the list */
                continue;
            }

            /* move to the next object */
            obj = (rt_pntr *)*obj;
        }

        /* release chunk */
        rt_CHUNK *chunk = head->next;
        f_free(head, head->size);
        head = chunk;
    }

    /* reset heap pointer to "ptr" */
    if (head != RT_NULL && ptr >= head + 1 && ptr < head->end)
    {
        rt_pntr *obj = &obj_head;

        /* traverse the list of free objects */
        while (*obj != RT_NULL)
        {
            /* remove free object after "ptr" */
            if (*obj >= ptr && *obj < head->end)
            {
                *((rt_si32 *)*obj - 2) = 0; /* clear object's "magic" */
                *obj = *(rt_pntr *)*obj; /* remove object from the list */
                continue;
            }

            /* move to the next object */
            obj = (rt_pntr *)*obj;
        }

        /* reset heap pointer */
        head->ptr = (rt_byte *)ptr;
        return ptr;
    }

    /* chunk with "ptr" was not found */
    return RT_NULL;
}

/*
 * Allocate given "size" bytes of memory with given "align",
 * search the list of free objects, move heap pointer otherwise.
 */
rt_pntr rt_Heap::obj_alloc(rt_size size, rt_ui32 align)
{
    rt_pntr *obj = &obj_head;

    /* each object must be capable of holding a pointer */
    size = RT_MAX(size, 8);

    /* search the list of free objects */
    while (*obj != RT_NULL)
    {
        /* compute align */
        rt_size mask = align > 0 ? align - 1 : 0;
        rt_byte *real_ptr = (rt_byte *)(((rt_size)*obj + mask) & ~mask);
        rt_si32 real_size = *((rt_si32 *)*obj - 1); /* fetch size from obj */
        real_size -= (rt_si32)(real_ptr - (rt_byte *)*obj); /* size-=align */

        /* found matching object */                  /* check 1-FREE-OBJ */
        if (real_size >= size && *((rt_si32 *)*obj - 2) == 0x1F3EE0B7)
        {
            *((rt_si32 *)*obj - 2) = 0; /* clear object's "magic" */
            *obj = *(rt_pntr *)*obj; /* remove object from the list */

            *((rt_si32 *)real_ptr - 1) = real_size; /* size behind pointer */
            *((rt_si32 *)real_ptr - 2) = 0x1600D0B7; /* stamp 1-GODD-OBJ */

            return real_ptr;
        }

        /* move to the next object */
        obj = (rt_pntr *)*obj;
    }

    rt_byte *ptr = (rt_byte *)reserve(size + RT_MAX(8, align), align);

    head->ptr = ptr + size + RT_MAX(8, align);

    ptr += RT_MAX(8, align);

    *((rt_si32 *)ptr - 1) = size;      /* size behind pointer */
    *((rt_si32 *)ptr - 2) = 0x1600D0B7; /* stamp 1-GODD-OBJ */

    return ptr;
}

/*
 * Release object at given "ptr",
 * add its memory to the list of free objects.
 */
rt_pntr rt_Heap::obj_free(rt_pntr ptr)
{
    /* check object at "ptr" */
    if (*((rt_si32 *)ptr - 2) == 0x1600D0B7) /* check 1-GOOD-OBJ */
    {
        *((rt_si32 *)ptr - 2)  = 0x1F3EE0B7; /* stamp 1-FREE-OBJ */

        /* add object to the list */
        *(rt_pntr *)ptr = obj_head;
        obj_head = ptr;
        return ptr;
    }

    /* object at "ptr" was not found */
    return RT_NULL;
}

/*
 * Deinitialize heap.
 */
rt_Heap::~rt_Heap()
{
    /* release all free objects from the list */
    while (obj_head != RT_NULL)
    {
        *((rt_si32 *)obj_head - 2) = 0; /* clear object's "magic" */
        obj_head = *(rt_pntr *)obj_head; /* remove object from the list */
    }

    /* free all chunks from the list */
    while (head != RT_NULL)
    {
        rt_CHUNK *chunk = head->next;
        f_free(head, head->size);
        head = chunk;
    }
}

/******************************************************************************/
/*********************************   LOGGING   ********************************/
/******************************************************************************/

rt_bool g_print = RT_FALSE;

rt_File g_log_file(RT_PATH_DUMP_LOG, "w+");
rt_File g_err_file(RT_PATH_DUMP_ERR, "w+");

/*
 * Print log into stdout and default log file.
 */
rt_void print_log(rt_pstr format, ...)
{
    va_list args;
#if RT_EMBED_STDOUT == 0
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
#endif /* RT_EMBED_STDOUT */
    va_start(args, format);
    g_log_file.vprint(format, args);
    va_end(args);
}

/*
 * Print err into stdout and default err file.
 */
rt_void print_err(rt_pstr format, ...)
{
    va_list args;
#if RT_EMBED_STDOUT == 0
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
#endif /* RT_EMBED_STDOUT */
    va_start(args, format);
    g_err_file.vprint(format, args);
    va_end(args);
}

rt_FUNC_PRINT_LOG   f_print_log = print_log;
rt_FUNC_PRINT_ERR   f_print_err = print_err;

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
