/******************************************************************************/
/* Copyright (c) 2013-2014 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#include "system.h"

/******************************************************************************/
/**********************************   FILE   **********************************/
/******************************************************************************/

/*
 * Instantiate file with given "name" and I/O "mode".
 */
rt_File::rt_File(rt_pstr name, rt_pstr mode)
{
    file = RT_NULL;
#if RT_EMBED_FILEIO == 0
    if (name != RT_NULL && mode != RT_NULL)
    {
        file = fopen(name, mode);
    }
#endif /* RT_EMBED_FILEIO */
}

/*
 * Set file position to "offset" bytes from "origin".
 */
rt_cell rt_File::seek(rt_cell offset, rt_cell origin)
{
    return
#if RT_EMBED_FILEIO == 0
    file ? fseek(file, offset, origin) :
#endif /* RT_EMBED_FILEIO */
    0;
}

/*
 * Read "num" elements of "size" bytes into "data" buffer.
 */
rt_word rt_File::read(rt_pntr data, rt_word size, rt_word num)
{
    return
#if RT_EMBED_FILEIO == 0
    file ? fread(data, size, num, file) :
#endif /* RT_EMBED_FILEIO */
    0;
}

/*
 * Write "num" elements of "size" bytes from "data" buffer.
 */
rt_word rt_File::write(rt_pntr data, rt_word size, rt_word num)
{
    return
#if RT_EMBED_FILEIO == 0
    file ? fwrite(data, size, num, file) :
#endif /* RT_EMBED_FILEIO */
    0;
}

/*
 * Write formatted string with variable number of arguments.
 */
rt_cell rt_File::print(rt_pstr format, ...)
{
    rt_cell ret = 0;
#if RT_EMBED_FILEIO == 0
    va_list args;
    va_start(args, format);
    ret = vfprintf(file, format, args);
    va_end(args);
    fflush(file);
#endif /* RT_EMBED_FILEIO */
    return ret;
}

/*
 * Write formatted string with given list of arguments.
 */
rt_cell rt_File::vprint(rt_pstr format, va_list args)
{
    rt_cell ret = 0;
#if RT_EMBED_FILEIO == 0
    ret = vfprintf(file, format, args);
    fflush(file);
#endif /* RT_EMBED_FILEIO */
    return ret;
}

/*
 * Return error code.
 */
rt_cell rt_File::error()
{
    return file == RT_NULL;
}

/*
 * Destroy file.
 */
rt_File::~rt_File()
{
#if RT_EMBED_FILEIO == 0
    if (file != RT_NULL)
    {
        fflush(file);
        fclose(file);
    }
#endif /* RT_EMBED_FILEIO */
    file = RT_NULL;
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
    chunk_alloc(0, RT_ALIGN);
}

/*
 * Allocate new chunk at least "size" bytes with given "align",
 * and link it to the list as head.
 */
rt_void rt_Heap::chunk_alloc(rt_word size, rt_word align)
{
    /* compute align and new chunk's size */
    rt_word mask = align > 0 ? align - 1 : 0;
    rt_word real_size = size + mask + sizeof(rt_CHUNK) + (RT_CHUNK_SIZE - 1);
    real_size = (real_size / RT_CHUNK_SIZE) * RT_CHUNK_SIZE;
    rt_CHUNK *chunk = (rt_CHUNK *)f_alloc(real_size);

    /* prepare new chunk */
    chunk->ptr = (rt_byte *)chunk + sizeof(rt_CHUNK);
    chunk->ptr = (rt_byte *)(((rt_word)chunk->ptr + mask) & ~mask);
    chunk->end = (rt_byte *)chunk + real_size;
    chunk->size = real_size;
    chunk->next = head;

    head = chunk;
}

/*
 * Reserve given "size" bytes of memory with given "align",
 * move heap pointer ahead for next alloc.
 */
rt_pntr rt_Heap::alloc(rt_word size, rt_word align)
{
    rt_byte *ptr = (rt_byte *)reserve(size, align);

    head->ptr = ptr + size;

    return ptr;
}

/*
 * Reserve given "size" bytes of memory with given "align",
 * don't move heap pointer. Next alloc will begin in reserved area.
 */
rt_pntr rt_Heap::reserve(rt_word size, rt_word align)
{
    /* compute align */
    rt_word mask = align > 0 ? align - 1 : 0;
    rt_byte *ptr = (rt_byte *)(((rt_word)head->ptr + mask) & ~mask);

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
 * Next alloc will start from "ptr" if align and size fits.
 */
rt_pntr rt_Heap::release(rt_pntr ptr)
{
    /* search chunk where "ptr" belongs,
     * free chunks allocated afterwards */
    while (head != RT_NULL && (ptr < head + 1 || ptr >= head->end))
    {
        rt_CHUNK *chunk = head->next;
        f_free(head);
        head = chunk;
    }

    /* reset heap pointer to "ptr" */
    if (head != RT_NULL && ptr >= head + 1 && ptr < head->end)
    {
        head->ptr = (rt_byte *)ptr;
        return ptr;
    }

    /* chunk with "ptr" was not found */
    return RT_NULL;
}

/*
 * Destroy heap.
 */
rt_Heap::~rt_Heap()
{
    /* free all chunks in the list */
    while (head != RT_NULL)
    {
        rt_CHUNK *chunk = head->next;
        f_free(head);
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
 * Print info log into stdout and default info log file.
 */
rt_void print_log(rt_pstr format, ...)
{
    va_list args;
    va_start(args, format);
#if RT_EMBED_STDOUT == 0
    vprintf(format, args);
#endif /* RT_EMBED_STDOUT */
    g_log_file.vprint(format, args);
    va_end(args);
}

/*
 * Print error log into stdout and default error log file.
 */
rt_void print_err(rt_pstr format, ...)
{
    va_list args;
    va_start(args, format);
#if RT_EMBED_STDOUT == 0
    vprintf(format, args);
#endif /* RT_EMBED_STDOUT */
    g_err_file.vprint(format, args);
    va_end(args);
}

rt_FUNC_PRINT_LOG   f_print_log = print_log;
rt_FUNC_PRINT_ERR   f_print_err = print_err;

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
