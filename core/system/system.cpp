/******************************************************************************/
/* Copyright (c) 2013 VectorChief (at github, bitbucket, sourceforge)         */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#include "system.h"

/******************************************************************************/
/**********************************   FILE   **********************************/
/******************************************************************************/

rt_File::rt_File(rt_pstr name, rt_pstr mode)
{
    file = RT_NULL;
#if RT_EMBED == 0
    if (name != RT_NULL && mode != RT_NULL)
    {
        file = fopen(name, mode);
    }
#endif /* RT_EMBED */
}

rt_cell rt_File::seek(rt_cell offset, rt_cell origin)
{
    return
#if RT_EMBED == 0
    file ? fseek(file, offset, origin) :
#endif /* RT_EMBED */
    0;
}

rt_word rt_File::read(rt_pntr data, rt_word size, rt_word num)
{
    return
#if RT_EMBED == 0
    file ? fread(data, size, num, file) :
#endif /* RT_EMBED */
    0;
}

rt_word rt_File::write(rt_pntr data, rt_word size, rt_word num)
{
    return
#if RT_EMBED == 0
    file ? fwrite(data, size, num, file) :
#endif /* RT_EMBED */
    0;
}

rt_cell rt_File::print(rt_pstr format, ...)
{
    rt_cell ret = 0;
#if RT_EMBED == 0
    va_list args;

    va_start(args, format);
    ret = vfprintf(file, format, args);
    va_end(args);

    fflush(file);
#endif /* RT_EMBED */
    return ret;
}

rt_cell rt_File::error()
{
    return file == RT_NULL;
}

rt_File::~rt_File()
{
#if RT_EMBED == 0
    if (file != RT_NULL)
    {
        fflush(file);
        fclose(file);
    }
#endif /* RT_EMBED */
    file = RT_NULL;
}

/******************************************************************************/
/**********************************   HEAP   **********************************/
/******************************************************************************/

rt_Heap::rt_Heap(rt_FUNC_ALLOC f_alloc, rt_FUNC_FREE f_free)
{
    this->f_alloc = f_alloc;
    this->f_free  = f_free;

    head = RT_NULL;
    chunk_alloc(0, RT_ALIGN);
}

rt_void rt_Heap::chunk_alloc(rt_word size, rt_word align)
{
    rt_word mask = align > 0 ? align - 1 : 0;
    rt_word real_size = size + mask + sizeof(rt_CHUNK) + (RT_CHUNK_SIZE - 1);
    real_size = (real_size / RT_CHUNK_SIZE) * RT_CHUNK_SIZE;
    rt_CHUNK *chunk = (rt_CHUNK *)f_alloc(real_size);

    chunk->ptr = (rt_byte *)chunk + sizeof(rt_CHUNK);
    chunk->ptr = (rt_byte *)(((rt_word)chunk->ptr + mask) & ~mask);
    chunk->end = (rt_byte *)chunk + real_size;
    chunk->size = real_size;
    chunk->next = head;

    head = chunk;
}

rt_pntr rt_Heap::alloc(rt_word size, rt_word align)
{
    rt_word mask = align > 0 ? align - 1 : 0;
    rt_byte *ptr = (rt_byte *)(((rt_word)head->ptr + mask) & ~mask);

    if (head->end < ptr + size)
    {
        chunk_alloc(size, align);
        ptr = head->ptr;
    }

    head->ptr = ptr + size;

    return ptr;
}

rt_Heap::~rt_Heap()
{
    while (head != RT_NULL)
    {
        rt_CHUNK *chunk = head->next;
        f_free(head);
        head = chunk;
    }
}

/******************************************************************************/
/********************************   LOGGING   *********************************/
/******************************************************************************/

rt_File g_log_file("dump/log.txt", "w+");
rt_File g_err_file("dump/err.txt", "w+");

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
