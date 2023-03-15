/******************************************************************************/
/* Copyright (c) 2013 VectorChief (at github, bitbucket, sourceforge)         */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTBASE_H
#define RT_RTBASE_H

#if   defined (RT_WIN32) /* Win32, MSVC ------------------------------------- */

#pragma warning (disable : 4244) /* VC++ 6.0: conversion from double to float */
#pragma warning (disable : 4305) /* VC++ 6.0: truncation from double to float */
#pragma warning (disable : 4291) /* VC++ 6.0: operator new no matching delete */
#pragma warning (disable : 4731) /* VC++ 9.0: frame ptr modified by assembler */
#pragma warning (disable : 4838) /* VS 2017:  conversion from double to float */
#pragma warning (disable : 4996) /* VS 2017:  secure form of string functions */

#endif /* ----------------- OS specific ------------------------------------- */

/* Generic types */

typedef float               rt_real;

typedef char                rt_char;
typedef int                 rt_bool;
typedef int                 rt_cell;
typedef long                rt_long;

typedef void                rt_void;
typedef void               *rt_pntr;

/* Generic definitions */

#define RT_NULL   ((rt_pntr)0)

#define RT_FALSE            0
#define RT_TRUE             1

/* Generic macros */

#define RT_ARR_SIZE(a)      (sizeof(a)/sizeof(a[0]))

#define RT_MIN(a, b)        ((a) < (b) ? (a) : (b))
#define RT_MAX(a, b)        ((a) > (b) ? (a) : (b))

#endif /* RT_RTBASE_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/