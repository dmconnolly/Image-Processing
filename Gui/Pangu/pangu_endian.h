/* Copyright (c) University of Dundee, 2001-2016 */


/* Simple compile-time tests to decide what format floating point */
/* numbers are stored in and what the endian-ness is. */
#ifndef PANGU_ENDIAN_H_INCLUDED
#define PANGU_ENDIAN_H_INCLUDED


/* PC-based systems (note _WIN32 is defined on x64 as well as x86!) */
#if defined(__MSDOS__) || defined(__WIN32__) || defined(WIN32) || defined(_WIN32) || defined(__NT__) || defined(__OS2__) || defined(__i386__) || defined(__x86_64)
#define HW_LITTLE_ENDIAN
#endif


/* Everything else is unrecognised: error */
#if !defined(HW_LITTLE_ENDIAN) && !defined(HW_BIG_ENDIAN)
#error UPDATE pangu_endian.h to recognise your system!
#endif


#endif
