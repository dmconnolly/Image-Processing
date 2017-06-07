/*
 * Name:
 *   platform.h
 *
 * Purpose:
 *   Define global platform-specific settings.
 *
 * Copyright:
 *   Copyright (c) University of Dundee, 2001-2016
 *
 * Language:
 *   ANSI C
 *
 * Author:
 *   Martin N Dunstan (mdunstan@computing.dundee.ac.uk)
 *   {add_authors_here}
 *
 * History:
 *   14-Nov-2002 (mnd)
 *      Converted from C++ into C (comments).
 * Future work:
 *   {add_suggestions_here}
 *
 * Bugs:
 *   {add_new_bugs_here}
 */

#ifndef PLATFORM_H_INCLUDED
#define PLATFORM_H_INCLUDED

/*
 * We check for _WIN64 first because _WIN32 is defined on 32-bit and
 * 64-bit Windows platforms. Use _WIN32 if you ever need to know if
 * you're on Windows (32-bit or 64-bit). Note that the _WIN32 branch
 * of this test could probably be removed because it was really only
 * used for older versions of Visual Studio.
 */

/*----------------------------- Windows (64-bit) -----------------------*/
#if defined(_WIN64)

/* MSVC++ 6.0: Identifier truncated to 255 chars in debug information */
#pragma warning (disable:4786)

/* MSVC++ 7.0: Exception specification ignored */
#pragma warning (disable:4290)

/* Stop MSVC++ Complaining about potentially insecure / deprecated functions */
#pragma warning(disable : 4996)

/* Standard headers that we want */
#include <sys/types.h>

/*
 * This stops windows.h from incuding winsock.h, which would otherwise
 * conflict with our use of winsock2.h when we need to. Note that there
 * is no such thing as WIN64_LEAN_AND_MEAN!
 */
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

/* Sleep for a fixed number of seconds */
#include <windows.h> /* Needed for Sleep() */
#define sleep(P)	Sleep((P)*1000)

/* The best that we can do for now ... */
#define usleep(P)	Sleep((P)/1000)


/*
 * Commented out so this compiles under Visual Studio 2013
 * FIXME: Was needed for older versions of Visual Studio
 */
#if 0
/* Oh for platform independence ... */
#include <float.h>

#define isnan(d)	_isnan(d)
#define isinf(d)	_fpclass(d) & (_FPCLASS_NINF | _FPCLASS_PINF)
#define isfinite(d)	!(_fpclass(d) & (_FPCLASS_NINF | _FPCLASS_PINF | _FPCLASS_SNAN | _FPCLASS_QNAN))
#endif

/* Yet more platform dependencies. */
#include <process.h>
#define getpid		_getpid

/* Is stdin attached to a terminal? */
#define isatty(x)	(0)

/* For command execution. */
#define putenv		_putenv

/* Is stdin connected to a terminal? */
#define isatty(x)	(0)

/* Socket stuff */
typedef int socklen_t;

/* Defines int32_t/int64_t */
#include <stdint.h>

/* Needed for compatability with 64-bit Linux */
/*typedef __int32 int32_t;*/
/*typedef unsigned __int32 uint32_t;*/

/* MSVC++ 6.0 doesn't support long long */
typedef __int64 longlong;
typedef unsigned __int64 ulonglong;
#define strtoll _strtoi64

/* More dependencies ... */
#include <direct.h>
#define chdir(d)	_chdir(d)

/* For use with stat() */
#define S_ISDIR(v) ((v) & _S_IFDIR)


/* This provides common large-file support between Windows and UNIX. */
#define fseek64 _fseeki64
#define ftell64 _ftelli64

/*----------------------------- Windows (32-bit) -----------------------*/
#elif defined(_WIN32)

/*
 * We don't want to support 32-bit Windows any more or if we do we would
 * like to have common 32-bit/64-bit support. So this check here is to
 * cause compilation to fail to as a reminder to investigate. This branch
 * is probably only needed for older versions of Visual Studio rather than
 * specifically 32-bit Windows. If this #error triggers then replace the
 * _WIN64 check above with a _WIN32 check and see if everything works.
 * If it does we can purge this branch.
 */
#error FATAL: check if 32-bit specific definitions are necessary


/* MSVC++ 6.0: Identifier truncated to 255 chars in debug information */
#pragma warning (disable:4786)

/* MSVC++ 7.0: Exception specification ignored */
#pragma warning (disable:4290)

/* Stop MSVC++ Complaining about potentially insecure / deprecated functions */
#pragma warning(disable : 4996)

/* Standard headers that we want */
#include <sys/types.h>

/*
 * This stops windows.h from incuding winsock.h, which would otherwise
 * conflict with our use of winsock2.h when we need to.
*/
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

/* Sleep for a fixed number of seconds */
#include <windows.h> /* Needed for Sleep() */
#define sleep(P)	Sleep((P)*1000)

/* The best that we can do for now ... */
#define usleep(P)	Sleep((P)/1000)

/* Oh for platform independence ... */
#include <float.h>
#define isnan(d)	_isnan(d)
#define isinf(d)	_fpclass(d) & (_FPCLASS_NINF | _FPCLASS_PINF)
#define isfinite(d)	!(_fpclass(d) & (_FPCLASS_NINF | _FPCLASS_PINF | _FPCLASS_SNAN | _FPCLASS_QNAN))

/* Yet more platform dependencies. */
#include <process.h>
#define getpid		_getpid

/* Is stdin attached to a terminal? */
#define isatty(x)	(0)

/* For command execution. */
#define putenv		_putenv

/* Is stdin connected to a terminal? */
#define isatty(x)	(0)

/* Socket stuff */
typedef int socklen_t;

/* Defines int32_t/uint32_t */
#include <stdint.h>

/* Needed for compatability with 64-bit Linux */
/*typedef __int32 int32_t;*/
/*typedef unsigned __int32 uint32_t;*/

/* MSVC++ 6.0 doesn't support long long */
typedef __int64 longlong;
typedef unsigned __int64 ulonglong;
#define strtoll _strtoi64

/* More dependencies ... */
#include <direct.h>
#define chdir(d)	_chdir(d)

/* For use with stat() */
#define S_ISDIR(v) ((v) & _S_IFDIR)


/* This provides common large-file support between Windows and UNIX. */
#define fseek64 _fseeki64
#define ftell64 _ftelli64

/*--------------------------------- Linux -------------------------------*/
#else

/*
 * NOTE that we can use #if defined(__x86_64__) to check for 64-bit Linux.
 * However, unless we absolutely need to do so please don't! To help with
 * portability use uint32_t instead of unsigned long etc to fix portability
 * issues where 32-bit code assumed 32-bit longs and has broken indexing
 * or value wrapping or sign extension etc.
 */

/* Assume UNIX-like */
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <dirent.h>

/* Defines int32_t/uint32_t */
#include <stdint.h>

/*
 * Needed because MSVC++ 6.0 doesn't support long long and we need a
 * common type name for all platforms.
 */
typedef long long longlong;
typedef unsigned long long ulonglong;


/*
 * This provides common large-file support between Windows and UNIX.
 * Note that the viewer checks that off_t is large enough so if the
 * viewer runs ok other programs can rely on these functions. Note
 * that fseek64/ftell64() are defined on IRIX using long long but
 * that's not likely to cause us problems.
 */
#define fseek64 fseeko
#define ftell64 ftello


#endif
/*---------------- DO NOT ADD ANY CODE BELOW THIS LINE -----------------*/
#endif

