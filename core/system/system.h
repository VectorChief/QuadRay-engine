/******************************************************************************/
/* Copyright (c) 2013-2014 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_SYSTEM_H
#define RT_SYSTEM_H

#include <stdio.h>
#include <stdarg.h>

#include "rtbase.h"
#include "rtconf.h"

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * system.h: Interface for the system layer.
 *
 * More detailed description of this subsystem is given in system.cpp.
 * Recommended naming scheme for C++ types and definitions is given in rtbase.h.
 */

/******************************************************************************/
/*******************************   DEFINITIONS   ******************************/
/******************************************************************************/

/* Classes */

class rt_File;
class rt_Heap;

template <class rt_Class>
class rt_List;

class rt_Exception;
class rt_LogRedirect;

/******************************************************************************/
/**********************************   FILE   **********************************/
/******************************************************************************/

/*
 * File encapsulates all file I/O operations.
 */
class rt_File
{
/*  fields */

    private:

    FILE               *file;

/*  methods */

    public:

    rt_File(rt_pstr name, rt_pstr mode);

    virtual
   ~rt_File();

    rt_cell seek(rt_cell offset, rt_cell origin);
    rt_word load(rt_pntr data, rt_word size, rt_word num);
    rt_word save(rt_pntr data, rt_word size, rt_word num);
    rt_cell fprint(rt_pstr format, ...);
    rt_cell vprint(rt_pstr format, va_list args);
    rt_cell error(); /* 0 - no error */
};

/******************************************************************************/
/**********************************   HEAP   **********************************/
/******************************************************************************/

/* memory chunk header */
struct rt_CHUNK
{
    rt_byte            *ptr;
    rt_byte            *end;
    rt_word             size;
    rt_CHUNK           *next;
};

typedef rt_pntr (*rt_FUNC_ALLOC)(rt_word size);
typedef rt_void (*rt_FUNC_FREE)(rt_pntr ptr);

/*
 * Heap manages fast linear allocs with the ability to release
 * group of allocs made after a checkpoint in the past.
 */
class rt_Heap
{
/*  fields */

    private:

    rt_CHUNK           *head;

    rt_void chunk_alloc(rt_word size, rt_word align);

    protected:

    rt_FUNC_ALLOC      f_alloc;
    rt_FUNC_FREE       f_free;

/*  methods */

    public:

    rt_Heap(rt_FUNC_ALLOC f_alloc, rt_FUNC_FREE f_free);

    virtual
   ~rt_Heap();

    rt_pntr alloc(rt_word size, rt_word align);
    rt_pntr reserve(rt_word size, rt_word align);
    rt_pntr release(rt_pntr ptr);
};

/******************************************************************************/
/**********************************   LIST   **********************************/
/******************************************************************************/

/*
 * List is a template used for linking other high-level objects into lists,
 * while preserving strict typization.
 */
template <class rt_Class>
class rt_List
{
/*  fields */

    public:

    rt_Class           *next;

/*  methods */

    protected:

    rt_List(rt_Class *next) { this->next = next; }
};

/******************************************************************************/
/********************************   EXCEPTION   *******************************/
/******************************************************************************/

/*
 * Exception contains description of a run-time error.
 */
class rt_Exception
{
/*  fields */

    public:

    rt_pstr err;

/*  methods */

    public:

    rt_Exception(rt_pstr err) { this->err = err; }

    virtual
   ~rt_Exception() { }
};

/******************************************************************************/
/*********************************   LOGGING   ********************************/
/******************************************************************************/

extern rt_bool              g_print;

extern rt_File              g_log_file;
extern rt_File              g_err_file;

typedef rt_void (*rt_FUNC_PRINT_LOG)(rt_pstr format, ...);
typedef rt_void (*rt_FUNC_PRINT_ERR)(rt_pstr format, ...);

extern rt_FUNC_PRINT_LOG    f_print_log;
extern rt_FUNC_PRINT_ERR    f_print_err;

#define RT_LOGI             f_print_log
#define RT_LOGE             f_print_err

/*
 * LogRedirect is an interface for scene manager
 * used for replacing default log functions.
 */
class rt_LogRedirect /* must be first in scene init */
{
/*  fields */

    protected:

/*  methods */

    protected:

    rt_LogRedirect(rt_FUNC_PRINT_LOG f_print_log,
                   rt_FUNC_PRINT_ERR f_print_err)
    { 
        if (f_print_log != RT_NULL)
          ::f_print_log = f_print_log;

        if (f_print_err != RT_NULL)
          ::f_print_err = f_print_err;
    }
};

#endif /* RT_SYSTEM_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
