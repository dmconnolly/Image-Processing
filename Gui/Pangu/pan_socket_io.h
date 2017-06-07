
/*
 * Name:
 *   pan_socket_io.h
 *
 * Purpose:
 *   Socket I/O functions designed for use with the PANGU network protocol.
 *
 * Description:
 *   This file contains low-level functions (pan_socket_read/pan_socket_write)
 *   for sending and receiving data over sockets with basic support for packet
 *   fragmentation. Short reads and short writes must be handled by users of
 *   these functions.
 *
 *   This file also contains medium-level functions for reading and writing
 *   values of specific C data types in a portable format for PANGU systems.
 *   Short reads and short writes must be handled by users of these functions.
 *
 * Notes:
 *   The integer value returned by every read and write function declared
 *   below is the number of bytes successfully read/written. The table below
 *   defines the number of bytes that each supported data type occupies and
 *   it is up to the caller to ensure that the result of these calls matches
 *   the corresponding value in the table:
 *
 *        uchar/char     1
 *        bool           1
 *        short/ushort   2
 *        long/ulong     4
 *        float          4
 *        double         8
 *
 * For readable code use pan_socket_read_*() and pan_socket_write_*(). For
 * optimal code use multiple pan_socket_poke_*() calls into a single buffer
 * followed by a single pan_socket_write(). This gives a 1000x performance
 * boost under Windows and Linux ....
 *
 * Language:
 *   ANSI C 89
 *
 * Author:
 *   Martin N Dunstan (mdunstan@computing.dundee.ac.uk)
 *   {add_authors_here}
 *
 * Copyright:
 *   (c) Space Technology Centre, University of Dundee, 2001-2016.
 *
 * History:
 *   14-Nov-2002 (mnd):
 *      Original version.
 *   15-May-2008 (mnd):
 *      Added double precision support.
 *   {add_changes_here}
 *
 * Future work:
 *   {add_suggestions_here}
 *
 * Bugs:
 *   {add_new_bugs_here}
 */

#ifndef PAN_SOCKET_IO_H_INCLUDED
#define PAN_SOCKET_IO_H_INCLUDED

#include "platform.h"
#include "pangu_endian.h"
#include "socket_stuff.h"

// This is used to enable/disable debug printing.
extern int pan_socket_debug_print;

extern int pan_socket_size_uchar();
extern int pan_socket_read_uchar(SOCKET, unsigned char *);
extern char *pan_socket_peek_uchar(char *, unsigned char *);
extern int pan_socket_write_uchar(SOCKET, unsigned char);
extern char *pan_socket_poke_uchar(char *, unsigned char);

extern int pan_socket_size_char();
extern int pan_socket_read_char(SOCKET, char *);
extern char *pan_socket_peek_char(char *, char *);
extern int pan_socket_write_char(SOCKET, char);
extern char *pan_socket_poke_char(char *, char);

extern int pan_socket_size_bool();
extern int pan_socket_read_bool(SOCKET, char *);
extern char *pan_socket_peek_bool(char *, char *);
extern int pan_socket_write_bool(SOCKET, char);
extern char *pan_socket_poke_bool(char *, char);

extern int pan_socket_size_ushort();
extern int pan_socket_read_ushort(SOCKET, unsigned short *);
extern char *pan_socket_peek_ushort(char *, unsigned short *);
extern int pan_socket_write_ushort(SOCKET, unsigned short);
extern char *pan_socket_poke_ushort(char *, unsigned short);

extern int pan_socket_size_short();
extern int pan_socket_read_short(SOCKET, short *);
extern char *pan_socket_peek_short(char *, short *);
extern int pan_socket_write_short(SOCKET, short);
extern char *pan_socket_poke_short(char *, short);

extern int pan_socket_size_ulong();
extern int pan_socket_read_ulong(SOCKET, unsigned long *);
extern char *pan_socket_peek_ulong(char *, unsigned long *);
extern int pan_socket_write_ulong(SOCKET, unsigned long);
extern char *pan_socket_poke_ulong(char *, unsigned long);

extern int pan_socket_size_long();
extern int pan_socket_read_long(SOCKET, long *);
extern char *pan_socket_peek_long(char *, long *);
extern int pan_socket_write_long(SOCKET, long);
extern char *pan_socket_poke_long(char *, long);

extern int pan_socket_size_float();
extern int pan_socket_read_float(SOCKET, float *);
extern char *pan_socket_peek_float(char *, float *);
extern int pan_socket_write_float(SOCKET, float);
extern char *pan_socket_poke_float(char *, float);

extern int pan_socket_size_double();
extern int pan_socket_read_double(SOCKET, double *);
extern char *pan_socket_peek_double(char *, double *);
extern int pan_socket_write_double(SOCKET, double);
extern char *pan_socket_poke_double(char *, double);

extern int pan_socket_size_string(char *);
extern int pan_socket_read_string(SOCKET, char **);
extern char *pan_socket_peek_string(char *, char **);
extern int pan_socket_write_string(SOCKET, char *);
extern char *pan_socket_poke_string(char *, char *);

/*
 * Low-level support functions.
 */
extern long pan_socket_read(SOCKET, void *, unsigned long);
	/*
	 * pan_socket_read(s, p, n) reads "n" bytes from socket "s" and
	 * stores them in the block of memory pointed to by "p". This is
	 * a low-level socket read function.
	 */

extern int pan_socket_write(SOCKET, void *, unsigned long);
	/*
	 * pan_socket_write(s, p, n) reads "n" bytes from the block of
	 * memory pointed to by "p" and writes them to the socket "s".
	 * This is a low-level socket write function.
	 */

extern char *pan_socket_peek(char *, void *, unsigned long);
	/*
	 * pan_socket_peek(p, d, n) reads "n" bytes from the block of memory
	 * pointed to by "p" and writes them to the memory block at "d" and
	 * returns a pointer to the location after the last byte read.
	 */

extern char *pan_socket_poke(char *, void *, unsigned long);
	/*
	 * pan_socket_poke(d, p, n) reads "n" bytes from the block of memory
	 * pointed to by "p" and writes them to the memory block at "d" and
	 * returns a pointer to the location after the last byte written.
	 */

extern unsigned long float2ulong(float);
	/*
	 * Convert a normalised single-precision float into our own
	 * little-endian format. We ignore issues of NaNs, infinities
	 * and subnormals (platform-specific).
	 */

extern float ulong2float(unsigned long);
	/*
	 * Convert from our little-endian format into a native float.
	 * See float2ulong() above for more details.
	 */

extern ulonglong double2ulong(double);
	/*
	 * Convert a normalised double-precision float into our own
	 * little-endian format. We ignore issues of NaNs, infinities
	 * and subnormals (platform-specific).
	 */

extern double ulong2double(ulonglong);
	/*
	 * Convert from our little-endian format into a native double.
	 * See double2ulong() above for more details.
	 */

#endif
