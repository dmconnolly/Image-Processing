
/*
 * Name:
 *   pan_socket_io.cpp
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
 *   {add_changes_here}
 *
 * Future work:
 *   {add_suggestions_here}
 *
 * Bugs:
 *   {add_new_bugs_here}
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// For uint32_t (don't use cstdint which is different)
#include <stdint.h>

#include "pan_socket_io.h"
#include "pangu_endian.h"
#include "floating_point_structs.h"


// This is used to enable/disable debug printing.
int pan_socket_debug_print = 0;


  //=====================================================================//
 //                            unsigned char                            //
//=====================================================================//

int
pan_socket_size_uchar(unsigned char v)
	{ return 1; }

// FIXME: assumes sizeof(unsigned char)==1
int
pan_socket_read_uchar(SOCKET s, unsigned char *p)
	{ return pan_socket_read(s, (void *)p, sizeof(*p)); }

// FIXME: assumes sizeof(unsigned char)==1
char *
pan_socket_peek_uchar(char *p, unsigned char *v)
	{ return pan_socket_peek(p, (void *)&v, sizeof(v)); }

// FIXME: assumes sizeof(unsigned char)==1
int
pan_socket_write_uchar(SOCKET s, unsigned char v)
	{ return pan_socket_write(s, (void *)&v, sizeof(v)); }

// FIXME: assumes sizeof(unsigned char)==1
char *
pan_socket_poke_uchar(char *p, unsigned char v)
	{ return pan_socket_poke(p, (void *)&v, sizeof(v)); }


  //=====================================================================//
 //                                 char                                //
//=====================================================================//

int
pan_socket_size_char(char v)
	{ return 1; }

int
pan_socket_read_char(SOCKET s, char *p)
	{ return pan_socket_read_uchar(s, (unsigned char *)p); }

char *
pan_socket_peek_char(char *p, char *v)
	{ return pan_socket_peek(p, (void *)&v, sizeof(v)); }

int
pan_socket_write_char(SOCKET s, char v)
	{ return pan_socket_write(s, (void *)&v, sizeof(v)); }

char *
pan_socket_poke_char(char *p, char v)
	{ return pan_socket_poke(p, (void *)&v, sizeof(v)); }


  //=====================================================================//
 //                                 bool                                //
//=====================================================================//

int
pan_socket_size_bool(char v)
	{ return 1; }

// FIXME: assumes sizeof(unsigned char)==1
int
pan_socket_read_bool(SOCKET s, char *p)
{
	unsigned char c;
	int status = pan_socket_read_uchar(s, &c);
	*p = c ? 1 : 0;
	return status;
}

// FIXME: assumes sizeof(unsigned char)==1
char *
pan_socket_peek_bool(char *s, char *p)
{
	unsigned char c;
	s = pan_socket_peek_uchar(s, &c);
	*p = c ? 1 : 0;
	return s;
}

// FIXME: assumes sizeof(unsigned char)==1
int
pan_socket_write_bool(SOCKET s, char v)
{
	unsigned char c = v ? 1 : 0;
	return pan_socket_write_uchar(s, c);
}

// FIXME: assumes sizeof(unsigned char)==1
char *
pan_socket_poke_bool(char *p, char v)
{
	unsigned char c = v ? 1 : 0;
	return pan_socket_poke_uchar(p, c);
}

  //=====================================================================//
 //                            unsigned short                           //
//=====================================================================//

int
pan_socket_size_ushort(unsigned short v)
	{ return 2; }


// FIXME: assumes sizeof(unsigned short)==2
int
pan_socket_read_ushort(SOCKET s, unsigned short *p)
{
	unsigned short tmp = 0;
	int status = pan_socket_read(s, (void *)&tmp, sizeof(tmp));
	if (status == sizeof(tmp)) *p = ntohs(tmp);
	return status;
}

// FIXME: assumes sizeof(unsigned short)==2
char *
pan_socket_peek_ushort(char *s, unsigned short *p)
{
	unsigned short tmp = 0;
	s = pan_socket_peek(s, (void *)&tmp, sizeof(tmp));
	*p = ntohs(tmp);
	return s;
}

// FIXME: assumes sizeof(unsigned short)==2
int
pan_socket_write_ushort(SOCKET s, unsigned short v)
{
	unsigned short tmp = htons(v);
	return pan_socket_write(s, (void *)&tmp, sizeof(tmp));
}

// FIXME: assumes sizeof(unsigned short)==2
char *
pan_socket_poke_ushort(char *p, unsigned short v)
{
	unsigned short tmp = htons(v);
	return pan_socket_poke(p, (void *)&tmp, sizeof(tmp));
}


  //=====================================================================//
 //                                short                                //
//=====================================================================//

int
pan_socket_size_short(short v)
	{ return 2; }

int
pan_socket_read_short(SOCKET s, short *p)
	{ return pan_socket_read_ushort(s, (unsigned short *)p); }

char *
pan_socket_peek_short(char *s, short *p)
	{ return pan_socket_peek_ushort(s, (unsigned short *)p); }

int
pan_socket_write_short(SOCKET s, short v)
	{ return pan_socket_write_ushort(s, (unsigned short)v); }

char *
pan_socket_poke_short(char *p, unsigned short v)
	{ return pan_socket_poke_ushort(p, (unsigned short)v); }


  //=====================================================================//
 //                            unsigned long                            //
//=====================================================================//

int
pan_socket_size_ulong(unsigned long v)
	{ return 4; }


int
pan_socket_read_ulong(SOCKET s, unsigned long *p)
{
	uint32_t tmp = 0;
	int status = pan_socket_read(s, (void *)&tmp, sizeof(tmp));
	if (status == sizeof(tmp)) *p = ntohl((unsigned long)tmp);
	return status;
}

char *
pan_socket_peek_ulong(char *s, unsigned long *p)
{
	uint32_t tmp = 0;
	s = pan_socket_peek(s, (void *)&tmp, sizeof(tmp));
	*p = ntohl((unsigned long)tmp);
	return s;
}

int
pan_socket_write_ulong(SOCKET s, unsigned long v)
{
	uint32_t tmp = (uint32_t)htonl(v);
	return pan_socket_write(s, (void *)&tmp, sizeof(tmp));
}

char *
pan_socket_poke_ulong(char *p, unsigned long v)
{
	uint32_t tmp = (uint32_t)htonl(v);
	return pan_socket_poke(p, (void *)&tmp, sizeof(tmp));
}


  //=====================================================================//
 //                                 long                                //
//=====================================================================//

int
pan_socket_size_long(long v)
	{ return 4; }

int
pan_socket_read_long(SOCKET s, long *p)
	{ return pan_socket_read_ulong(s, (unsigned long *)p); }

char *
pan_socket_peek_long(char *s, long *p)
	{ return pan_socket_peek_ulong(s, (unsigned long *)p); }

int
pan_socket_write_long(SOCKET s, long v)
	{ return pan_socket_write_ulong(s, (unsigned long)v); }

char *
pan_socket_poke_long(char *p, long v)
	{ return pan_socket_poke_ulong(p, (unsigned long)v); }


  //=====================================================================//
 //                                float                                //
//=====================================================================//

int
pan_socket_size_float(float v)
	{ return 4; }

int
pan_socket_read_float(SOCKET s, float *f)
{
	unsigned long tmp = 0;
	int status = pan_socket_read_ulong(s, &tmp);
	*f = ulong2float(tmp);
	return status;
}

char *
pan_socket_peek_float(char *s, float *f)
{
	unsigned long tmp = 0;
	s = pan_socket_peek_ulong(s, &tmp);
	*f = ulong2float(tmp);
	return s;
}

int
pan_socket_write_float(SOCKET s, float v)
{
	unsigned long tmp = float2ulong(v);
	return pan_socket_write_ulong(s, tmp);
}

char *
pan_socket_poke_float(char *p, float v)
{
	unsigned long tmp = float2ulong(v);
	return pan_socket_poke_ulong(p, tmp);
}

  //=====================================================================//
 //                                double                               //
//=====================================================================//

int
pan_socket_size_double(double v)
	{ return 8; }

int
pan_socket_read_double(SOCKET s, double *f)
{
	// For some silly reason we decided to send the two 32-bit
	// words in little endian order even though the each word
	// is transmitted in big endian order. So the bytes of the
	// double are transmitted in order 3,2,1,0,7,6,5,4.
	unsigned long lo = 0, hi = 0;
	ulonglong tmp = 0;
	int status = pan_socket_read_ulong(s, &lo);
	status    += pan_socket_read_ulong(s, &hi);
	tmp = ((ulonglong)hi << 32) | (ulonglong)lo;
	if (status == 8) *f = ulong2double(tmp);
	return status;
}

char *
pan_socket_peek_double(char *s, double *f)
{
	unsigned long lo = 0, hi = 0;
	ulonglong tmp = 0;
	s = pan_socket_peek_ulong(s, &lo);
	s = pan_socket_peek_ulong(s, &hi);
	tmp = ((ulonglong)hi << 32) | (ulonglong)lo;
	*f = ulong2double(tmp);
	return s;
}

int
pan_socket_write_double(SOCKET s, double v)
{
	ulonglong tmp = double2ulong(v);
	unsigned long lo = (unsigned long)(tmp & 0xffffffff);
	unsigned long hi = (unsigned long)((tmp>>32) & 0xffffffff);
	int status    = pan_socket_write_ulong(s, lo);
	return status + pan_socket_write_ulong(s, hi);
}

char *
pan_socket_poke_double(char *p, double v)
{
	ulonglong tmp = double2ulong(v);
	unsigned long lo = (unsigned long)(tmp & 0xffffffff);
	unsigned long hi = (unsigned long)((tmp>>32) & 0xffffffff);
	p = pan_socket_poke_ulong(p, lo);
	return pan_socket_poke_ulong(p, hi);
}


  //=====================================================================//
 //                                string                               //
//=====================================================================//

int
pan_socket_size_string(char *v)
{
	/* We include the NUL terminator */
	unsigned short slen = strlen(v) + 1;

	/* We pad to even. */
	unsigned short xlen = slen + (slen & 1);

	/* Include the length field */
	return pan_socket_size_ushort(xlen) + xlen;
}

int
pan_socket_read_string(SOCKET s, char **p)
{
	/* Get the data length. */
	unsigned short slen = 0;
	int status = pan_socket_read_ushort(s, &slen);
	if (status < 0) return status;
	if (!slen) return sizeof(slen);

	/* Read that many bytes of string data. */
	*p = (char *)malloc(slen*sizeof(char));
	status = pan_socket_read(s, (void *)(*p), slen);
	(*p)[slen - 1] = 0;
	if (status < 0) return status;
	return slen + sizeof(slen);
}

char *
pan_socket_peek_string(char *s, char **p)
{
	/* Get the data length. */
	unsigned short slen = 0;
	s = pan_socket_peek_ushort(s, &slen);
	if (!slen) return s;

	/* Read that many bytes of string data. */
	*p = (char *)malloc(slen*sizeof(char));
	s = pan_socket_peek(s, (void *)(*p), slen);
	(*p)[slen - 1] = 0;
	return s;
}

int
pan_socket_write_string(SOCKET s, char *v)
{
	int status;

	/* We include the NUL terminator */
	unsigned short slen = strlen(v) + 1;

	/* We pad to even. */
	unsigned short xlen = slen + (slen & 1);
	unsigned char pad = 0;

	/* Write the length. */
	status = pan_socket_write_ushort(s, xlen);
	if (status < 0) return status;

	/* Write the string and its terminator */
	status = pan_socket_write(s, v, slen);
	if (status < 0) return status;

	/* Write the pad byte if required. */
	if (xlen != slen) pan_socket_write_uchar(s, pad);

	/* Return the number of bytes written. */
	return slen + sizeof(slen);
}

char *
pan_socket_poke_string(char *p, char *v)
{
	/* We include the NUL terminator */
	unsigned short slen = strlen(v) + 1;

	/* We pad to even. */
	unsigned short xlen = slen + (slen & 1);
	unsigned char pad = 0;

	/* Write the length. */
	p = pan_socket_poke_ushort(p, xlen);

	/* Write the string and its terminator */
	p = pan_socket_poke(p, v, slen);

	/* Write the pad byte if required. */
	if (xlen != slen) p = pan_socket_poke_uchar(p, pad);

	/* Return the address of the next byte to be written. */
	return p;
}

  //=====================================================================//
 //                            memory buffer                            //
//=====================================================================//

/*
 * pan_socket_read(s, p, n) reads "n" bytes from socket "s" and stores
 * them in the block of memory pointed to by "p".
 */
long
pan_socket_read(SOCKET s, void *dst, unsigned long n)
{
	long want = n;
	unsigned char *ptr = (unsigned char *)dst;
	while (want > 0)
	{
		long got = SOCKET_RECV(s, ptr, want, 0);
		if (got <= 0) return n - want;
		want -= got;
		ptr  += got;
	}

	// We print one byte per line so that the dumps are the same no
	// matter how messages are generated for reception.
	for (unsigned long i = 0; pan_socket_debug_print&&(i<n); ++i)
		(void)printf("pan_socket RX 0x%02x\n", 0xff & ((unsigned char *)dst)[i]);

	return n;
}


/*
 * pan_socket_write(s, p, n) reads "n" bytes from the block of memory pointed
 * to by "p" and writes them to the socket "s".
 */
int
pan_socket_write(SOCKET s, void *src, unsigned long n)
{
	// We print one byte per line so that the dumps are the same no
	// matter how messages are generated for transmission.
	for (unsigned long i = 0; pan_socket_debug_print&&(i<n); ++i)
		(void)printf("pan_socket TX 0x%02x\n", 0xff & ((unsigned char *)src)[i]);

	// Transmit as many bytes at a time.
	long have = n;
	unsigned char *ptr = (unsigned char *)src;
	while (have > 0)
	{
		long sent = SOCKET_SEND(s, ptr, have, 0);
		if (sent <= 0) return n - have;
		have -= sent;
		ptr  += sent;
	}
	return n;
}


/*
 * pan_socket_peek(p, d, n) reads "n" bytes from the block of memory pointed
 * to by "p" and writes them to the memory block at "d" and returns a pointer
 * to the location after the last byte read i.e. p+n.
 */
char *
pan_socket_peek(char *src, void *dst, unsigned long n)
{
	(void)memcpy(dst, src, n);
	return src + n;
}


/*
 * pan_socket_poke(d, p, n) reads "n" bytes from the block of memory pointed
 * to by "p" and writes them to the memory block at "d" and returns a pointer
 * to the location after the last byte written i.e. d+n.
 */
char *
pan_socket_poke(char *dst, void *src, unsigned long n)
{
	(void)memcpy(dst, src, n);
	return dst + n;
}


  //=====================================================================//
 //                PANGU floating point encoding/decoding               //
//=====================================================================//

/*
 * Convert a normalised single-precision float into our own little-endian
 * format. Most systems store floating-point values in the same way (modulo
 * endianness and representation of NaNs): 1 bit sign, 8 bits of exponent
 * with 127 bias added and 24 bits of fraction. Normalised numbers always
 * have the most significant bit of the fraction set so it can be ignored
 * so fractions occupy 23 bits. We ignore issues of infinities, NaNs and
 * subnormals which are platform-specific.
 */
unsigned long
float2ulong(float f)
{
	unsigned long sign, exponent, fraction;
#if defined(HW_BIG_ENDIAN)
	bigieee_float_struct data;
#else
	smallieee_float_struct data;
#endif
	/* Assume format compatible with ours modulo bitsex */
	data.whole = f;
	sign = data.parts.sign;
	fraction = data.parts.fraction;
	exponent = data.parts.exponent;

#ifdef BARF_ON_INF
	if (isinf(f)) *(int *)0 = 0;
#endif
#ifdef BARF_ON_NAN
	if (isnan(f)) *(int *)0 = 0;
#endif

	/* Convert into our format */
	return (((fraction << 1) | sign) << 8) | exponent;
}


/*
 * Convert from our little-endian format into a native float.
 * See float2ulong() above for more details.
 */
float
ulong2float(unsigned long uvalue)
{
#if defined(HW_BIG_ENDIAN)
	bigieee_float_struct data;
#else
	smallieee_float_struct data;
#endif

	/* Explode into components  */
	float result;
	unsigned long exponent, sign, fraction;
	exponent = uvalue & 0xff; uvalue >>= 8;
	sign = uvalue & 1; uvalue >>= 1;
	fraction = uvalue & 0x7fffff;

	/* Assume format compatible with ours modulo bitsex */
	data.parts.fraction = fraction;
	data.parts.exponent = exponent;

	/* Return the converted float */
	data.parts.sign = sign;
	result = data.whole;

#ifdef BARF_ON_INF
	if (isinf(result)) *(int *)0 = 0;
#endif
#ifdef BARF_ON_NAN
	if (isnan(result)) *(int *)0 = 0;
#endif

	return result;
}


/*
 * Convert a normalised double-precision float into our own little-endian
 * format. Most systems store floating-point values in the same way (modulo
 * endianness and representation of NaNs): 1 bit sign, 11 bits of exponent
 * with 1023 bias added and 53 bits of fraction. Normalised numbers always
 * have the most significant bit of the fraction set so it can be ignored
 * so fractions occupy 52 bits. We ignore issues of infinities, NaNs and
 * subnormals which are platform-specific.
 */
ulonglong
double2ulong(double f)
{
	ulonglong sign, exponent, flo, fhi, fraction;
#if defined(HW_BIG_ENDIAN)
	bigieee_double_struct data;
#else
	smallieee_double_struct data;
#endif
	/* Assume format compatible with ours modulo bitsex */
	data.whole = f;
	sign       = data.parts.sign;
	flo        = data.parts.frac_lo;
	fhi        = data.parts.frac_hi;
	fraction   = (fhi<<32)|flo;
	exponent   = data.parts.exponent;

#ifdef BARF_ON_INF
	if (isinf(f)) *(int *)0 = 0;
#endif
#ifdef BARF_ON_NAN
	if (isnan(f)) *(int *)0 = 0;
#endif

	/* Convert into our format */
	return (((fraction << 1) | sign) << 11) | exponent;
}


/*
 * Convert from our little-endian format into a native double.
 * See double2ulong() above for more details.
 */
double
ulong2double(ulonglong uvalue)
{
#if defined(HW_BIG_ENDIAN)
	bigieee_double_struct data;
#else
	smallieee_double_struct data;
#endif

	/* Explode into components  */
	double result;
	ulonglong exponent, sign, flo, fhi; //, fraction;
	exponent = uvalue & 0x7ff; uvalue >>= 11;
	sign = uvalue & 1; uvalue >>= 1;
	flo = uvalue & 0xffffffff; uvalue >>= 32;
	fhi = uvalue; // & (2**20-1)

	/* Assume format compatible with ours modulo bitsex */
	data.parts.frac_lo  = flo;
	data.parts.frac_hi  = fhi;
	data.parts.exponent = exponent;

	/* Return the converted float */
	data.parts.sign = sign;
	result = data.whole;

#ifdef BARF_ON_INF
	if (isinf(result)) *(int *)0 = 0;
#endif
#ifdef BARF_ON_NAN
	if (isnan(result)) *(int *)0 = 0;
#endif

	return result;
}

