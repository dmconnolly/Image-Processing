
/*
 * Name:
 *   pan_protocol_lib.cpp
 *
 * Purpose:
 *   C interface to the client end of the PANGU network protocol.
 *
 * Description:
 *   This is a sample implementation of the PANGU 1.x network protocol
 *   using ANSI C 89.
 *
 * Notes:
 *   This file has the suffix of .cpp to prevent the Visual Studio compiler
 *   from generating bad code resulting from linking C and C++ code. The
 *   file ought to be treated as pure C (although this has slipped a bit).
 *
 *   Keep ALL functions sorted by increasing client message number. Do
 *   not group related methods together e.g. double and single precision
 *   versions of the same underlying process. Users are expected to read
 *   PROTOCOL.TXT or the PANGU ICD to determine the name of the method
 *   that they want to use. Replace the capitalised words with all lower
 *   case words separated by _ and prefix with pan_protocol_. Thus the
 *   GetFrames(20) method becomes pan_protocol_get_frames().
 *
 *   The pan_protocol_* methods are monolithic: they send a command and
 *   decode the reply. If an error occurs the message from the server is
 *   printed and the program will exit. For clients that wish to manage
 *   their own error handling and/or do not want errors to terminate the
 *   program the pan_net_* methods are provided. These are used as the
 *   low-level interface so see the corresponding pan_protocol_* method
 *   to find out how to use them. The TX methods transmit a command to
 *   the server while the RX methods decode any reply.
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
#include <assert.h>

#include "pan_socket_io.h"
#include "pan_protocol_lib.h"


/* FIXME: move pan_net_* into new file pan_net_lib. */

/**************************************************************************
 * These are the new split TX/RX interface functions. We recommend that
 * users use these if they want to use their own error handling functionality.
 * Otherwise the pan_protocol_* methods below can be used.
 **************************************************************************/


/*
 * pan_net_safety_checks() ensures that fundamental data type sizes
 * are what we require. If they aren't then our assumptions in the low
 * level code will be violated and bad things will happen. Call this once
 * when your application begins or before it uses the PANGU client library
 * for the first time for safety.
 * Returns NULL if there are no errors else it returns error message.
 */
char *
pan_net_safety_checks(void)
{
	int s = 0;

	/* Declare buffer to store any error message. */
	static char err_buf[1024];
	char tmp_buf[1024];
	const int err_buf_size = sizeof(err_buf)/sizeof(*err_buf);

	/* Clear the buffer */
	int i = 0;
	for(i = 0; i < err_buf_size; i++) err_buf[i] = '\0';

	int count = 0;

	/*
	 * We don't support big endian machines at the moment, mainly due
	 * to pan_protocol_get_lidar_measurement().
	 */
#ifdef HW_BIG_ENDIAN
	(void)sprintf(tmp_buf, "Fatal: big endian machines not supported yet.\n");
	assert( strlen(tmp_buf) + count < err_buf_size - 1 );
	strcat( err_buf, tmp_buf );
	count += strlen(tmp_buf);
#endif

	/* We expect some data types to have a specific size. */
	if ((s=(int)sizeof(unsigned char)) != 1)
	{
		(void)sprintf(tmp_buf,
			"Fatal: unexpected sizeof(unsigned char)=%d\n",s);
		assert( strlen(tmp_buf) + count < err_buf_size - 1 );
		strcat( err_buf, tmp_buf );
		count += strlen(tmp_buf);
	}
	if ((s=(int)sizeof(unsigned short)) != 2)
	{
		(void)sprintf(tmp_buf,
			"Fatal: unexpected sizeof(unsigned short)=%d\n",s);
		assert( strlen(tmp_buf) + count < err_buf_size - 1 );
		strcat( err_buf, tmp_buf );
		count += strlen(tmp_buf);
	}
	if ((s=(int)sizeof(float)) != 4)
	{
		(void)sprintf(tmp_buf,
			"Fatal: unexpected sizeof(float)=%d\n",s);
		assert( strlen(tmp_buf) + count < err_buf_size - 1 );
		strcat( err_buf, tmp_buf );
		count += strlen(tmp_buf);
	}
	if ((s=(int)sizeof(double)) != 8)
	{
		(void)sprintf(tmp_buf,
			"Fatal: unexpected sizeof(double)=%d\n",s);
		assert( strlen(tmp_buf) + count < err_buf_size - 1 );
		strcat( err_buf, tmp_buf );
		count += strlen(tmp_buf);
	}

	/* We expect some data types to have a minimum size. */
	if ((s=(int)sizeof(unsigned long)) < 4)
	{
		(void)sprintf(tmp_buf,
			"Fatal: sizeof(unsigned long) too small (%d)\n",s);
		assert( strlen(tmp_buf) + count < err_buf_size - 1 );
		strcat( err_buf, tmp_buf );
		count += strlen(tmp_buf);
	}
	if ((s=(int)sizeof(ulonglong)) < 8)
	{
		(void)sprintf(tmp_buf,
			"Fatal: sizeof(ulonglong) too small (%d)\n",s);
		assert( strlen(tmp_buf) + count < err_buf_size - 1 );
		strcat( err_buf, tmp_buf );
		count += strlen(tmp_buf);
	}

	if (strlen(err_buf) == 0) return NULL;
	return err_buf;
}


/*
 * err = pan_net_want(s, t) reads the next message type from "s" and checks
 * to see if it matches the message type "t". If it doesn't and the message
 * type read is an error message then the error will be returned. Otherwise
 * an "unexpected message" error will be returned.
 */
char *
pan_net_want(SOCKET s, unsigned long want)
{
	/* Get the reply code from the server */
	unsigned long mcode;
	pan_socket_read_ulong(s, &mcode);

	/* If it is what we expect then return happy. */
	if (mcode == want) return NULL;

	/* Declare buffer to store any error message. */
	static char err_buf[1024];
	const int err_buf_size = sizeof(err_buf)/sizeof(*err_buf);

	/* Deal with any incoming error messages */
	if (mcode == MSG_ERROR)
	{
		/* Read the error message */
		long ecode;
		char *emsg = NULL;
		pan_socket_read_long(s, &ecode);
		pan_socket_read_string(s, &emsg);
		/* Write the start of the message to buffer */
		int count = 0;
		(void)sprintf(err_buf, "Error from server: %n", &count);
		/* Copy the error message to the buffer. Leaving space
		   for newline and terminator characters. */
		strncat(err_buf, emsg, err_buf_size - count - 2);
		/* Add newline at end. Terminator is added automatically. */
		strcat(err_buf, "\n");

		(void)free(emsg);
		return err_buf;
	}

	/* Unexpected message received */
	int count = 0;
	(void)sprintf(err_buf, "Error: received message type %ld when expecting message type %ld.\n%n", mcode, want, &count);
	return err_buf;
}


/*
 * err = pan_net_start_TX() starts a PANGU network protocol session.
 *
 * pan_net_start_RX() is not required.
 */
char *
pan_net_start_TX(SOCKET s)
{
	/* Send the version number of the protocol we wish to use (1.20) */
	unsigned long vno = 0x114;
	(void)pan_socket_write_ulong(s, vno);

	/* Want an Okay response */
	return pan_net_want(s, MSG_OKAY);
}


/*
 * pan_net_finish_TX() ends a PANGU network protocol session. The
 * remote server will probably close the connection afterwards.
 *
 * pan_net_finish_RX() is not required.
 *
 * IMPLEMENTS Goodbye (0)
 */
char *
pan_net_finish_TX(SOCKET s)
{
	(void)pan_socket_write_ulong(s, MSG_GOODBYE);
	return NULL;
}


/*
 * err = pan_net_get_image_TX(s) requests an image from the remote
 * server using the current camera settings.
 *
 * ptr = pan_net_get_image_RX(s, &size) returns the image. The memory returned
 * by the call is allocated by malloc() and may be released by free(). The size
 * field will be updated with the number of bytes in the result array.
 *
 * IMPLEMENTS GetImage (1)
 */
char *
pan_net_get_image_TX(SOCKET s)
{
	/* Send the empty GetImage message and want a MSG_IMAGE reply */
	(void)pan_socket_write_ulong(s, MSG_GET_IMAGE);
	return pan_net_want(s, MSG_IMAGE);
}
unsigned char *
pan_net_get_image_RX(SOCKET s, unsigned long *psize)
{
	long fsize;
	unsigned char *result;

	/* Read the size of the data in the message */
	(void)pan_socket_read_long(s, &fsize);
	if (psize) *psize = fsize;

	/*
	 * Allocate a buffer large enough for the result. We add one
	 * in case the size is zero because we need a valid pointer.
	 */
	result = (unsigned char *)malloc(fsize + 1);

	/* Read the data directly into the result buffer */
	(void)pan_socket_read(s, (void *)result, fsize);
	return result;
}


/*
 * err = pan_net_get_elevation_TX(s) requests the elevation of the
 * camera relative to the remote model.
 *
 * h = pan_net_get_elevation_RX(s, perr) stores the elevation in h.
 * If perr is not a null pointer then it will be set to zero if the
 * returned elevation is invalid and non-zero if it is valid. An
 * invalid elevation is returned when the camera is not directly
 * above any part of the remote model.
 *
 * IMPLEMENTS GetElevation (2)
 */
char *
pan_net_get_elevation_TX(SOCKET s)
{
	/* Send the empty GetElevation message and want a MSG_FLOAT reply */
	(void)pan_socket_write_ulong(s, MSG_GET_ELEVATION);
	return pan_net_want(s, MSG_FLOAT);
}
float
pan_net_get_elevation_RX(SOCKET s, char *perr)
{
	struct optional_float result;

	/* Read the value then the error flag */
	(void)pan_socket_read_float(s, &result.value);
	(void)pan_socket_read_bool(s, &result.valid);
	if (perr) *perr = result.valid ? (char)1 : (char)0;
	return result.value;
}


/*
 * err = pan_net_get_elevations_TX(s, n, pv) requests the camera elevations
 * relative to the remote model for each of the "n" camera positions "pv".
 *
 * pan_net_get_elevations_RX(s, rv, ev) writes the results into the array "rv".
 * The ith elevation rv[i] is only valid (the position pv[3*i] is over the
 * model) when ev[i] is non-zero.
 * Both rv and ev must point to an array of "n" elements.
 *
 * IMPLEMENTS GetElevations (3)
 */
char *
pan_net_get_elevations_TX(SOCKET s, unsigned long n, float *posv)
{
	unsigned long i;

	/*
	 * Send the GetElevations message with parameters "n", the number
	 * of camera positions provided and "n" floating-point 3D-positions.
	 *
	 * We're sending two ulongs and 3*n floats. Since the buffer size
	 * is variable we have to allocate it on the heap.
	 */
	const int bufsize = 4*2 + 3*n*4;
	char *buf = (char *)malloc(bufsize); assert(buf);
	int nbytes;

	/* Initialise the buffer. */
	char *p = buf;
	p = pan_socket_poke_ulong (p, MSG_GET_ELEVATIONS);
	p = pan_socket_poke_ulong (p, n);
	for (i = 0; i < n; i++)
	{
		/* Send each float individually (must be encoded) */
		p = pan_socket_poke_float (p, *posv++);
		p = pan_socket_poke_float (p, *posv++);
		p = pan_socket_poke_float (p, *posv++);
	}
	nbytes = p - buf;

	/* Write the buffer in one go. */
	assert(nbytes == bufsize);
	pan_socket_write(s, buf, nbytes);
	(void)free(buf); buf = 0;

	/* We want a MSG_FLOAT_ARRAY reply */
	return pan_net_want(s, MSG_FLOAT_ARRAY);
}
void
pan_net_get_elevations_RX(SOCKET s, float *resultv, char *errorv)
{
	unsigned long i, nelts;

	/* Read the number of elements in the array */
	(void)pan_socket_read_ulong(s, &nelts);

	/* Read the results into the user-supplied buffer */
	for (i = 0; i < nelts; i++)
	{
		(void)pan_socket_read_float(s, resultv++);
		(void)pan_socket_read_bool(s, errorv++);
	}
}


/*
 * err = pan_net_lookup_point_TX(s, x, y) requests the 3D position of the model
 * under the pixel at coordinates (x, y) where (0, 0) represents the bottom 
 * left corner of the image and (1, 1) the top-right corner.
 *
 * pan_net_lookup_point_RX(s, &px, &py, &pz, perr) returns the 3D position of
 * the model (px, py, pz). If perr is not a null pointer then it will be set
 * to zero if the returned point is invalid and non-zero if it is valid. An
 * invalid point occurs when the centre of the specified pixel does not
 * cover any part of the model.
 *
 * IMPLEMENTS LookupPoint (4)
 */
char *
pan_net_lookup_point_TX(SOCKET s, float x, float y)
{
	/*
	 * Send the LookupPoint message with parameters (x, y) and want
	 * a MSG_3D_POINT reply. We're sending one ulong and two floats.
	 */
	const int bufsize = 1*4 + 2*4;
	static char buf[bufsize];
	int nbytes;

	/* Initialise the buffer. */
	char *p = buf;
	p = pan_socket_poke_ulong (p, MSG_LOOKUP_POINT);
	p = pan_socket_poke_float(p, x);
	p = pan_socket_poke_float(p, y);
	nbytes = p - buf;

	/* Write the buffer in one go. */
	assert(nbytes == bufsize);
	pan_socket_write(s, buf, nbytes);
	return pan_net_want(s, MSG_3D_POINT);
}
void
pan_net_lookup_point_RX(SOCKET s, float *px, float *py, float *pz, char *perr)
{
	char valid;

	/* Read the value then the error flag */
	(void)pan_socket_read_float(s, px);
	(void)pan_socket_read_float(s, py);
	(void)pan_socket_read_float(s, pz);
	(void)pan_socket_read_bool(s, &valid);
	if (perr) *perr = valid ? (char)1 : (char)0;
}


/*
 * err = pan_net_lookup_points_TX(s, n, pv) requests the 3D positions of each
 * of the "n" pixels whose 2D positions are stored in "pv".
 *
 * pan_net_lookup_points_RX(s, rv, ev) writes the 3D positions of each of the
 * "n" pixels into the array "rv". The ith position rv[3*i] is only valid
 * if ev[i] is non-zero. The "rv" array must hold 3*n elements while the "ev"
 * array must hold "n" elements.
 *
 * IMPLEMENTS LookupPoints (5)
 */
char *
pan_net_lookup_points_TX(SOCKET s, unsigned long n, float *posv)
{
	unsigned long i;

	/*
	 * Send the LookupPoints message with parameters "n", the number
	 * of pixel positions provided and "n" floating-point 2D-positions.
	 *
	 * We're sending two ulongs and 2*n floats. Since the buffer size
	 * is variable we have to allocate it on the heap.
	 */
	const int bufsize = 4*2 + 2*n*4;
	char *buf = (char *)malloc(bufsize); assert(buf);
	int nbytes;

	/* Initialise the buffer. */
	char *p = buf;
	p = pan_socket_poke_ulong (p, MSG_LOOKUP_POINTS);
	p = pan_socket_poke_ulong(p, n);
	for (i = 0; i < n; i++)
	{
		/* Send each float individually (must be encoded) */
		p = pan_socket_poke_float(p, *posv++); /* x */
		p = pan_socket_poke_float(p, *posv++); /* y */
	}
	nbytes = p - buf;

	/* Write the buffer in one go. */
	assert(nbytes == bufsize);
	pan_socket_write(s, buf, nbytes);
	(void)free(buf); buf = 0;

	/* We want a MSG_3D_POINT_ARRAY reply */
	return pan_net_want(s, MSG_3D_POINT_ARRAY);
}
void
pan_net_lookup_points_RX(SOCKET s, float *resultv, char *errorv)
{
	unsigned long i, nelts;

	/* Read the number of elements in the array */
	(void)pan_socket_read_ulong(s, &nelts);

	/* Read the results into the user-supplied buffer */
	for (i = 0; i < nelts; i++)
	{
		(void)pan_socket_read_float(s, resultv++); /* x */
		(void)pan_socket_read_float(s, resultv++); /* y */
		(void)pan_socket_read_float(s, resultv++); /* z */
		(void)pan_socket_read_bool(s, errorv++);
	}
}


/*
 * err = pan_net_get_point_TX(s, dx, dy, dz) requests the 3D position of the
 * model visible along direction (dx, dy, dz).
 * 
 * pan_net_get_point_RX(s, &px, &py, &pz, perr) returns the 3D position
 * (px, py, pz) of the model. If perr is not a null pointer then it will be
 * set to zero if the returned point is invalid and non-zero if it is valid.
 * An invalid point occurs when no part of the model is visible along the
 * specified direction.
 *
 * IMPLEMENTS GetPoint (6)
 */
char *
pan_net_get_point_TX(SOCKET s, float dx, float dy, float dz)
{
	/*
	 * Send the GetPoint message with parameters (dx, dy, dz) and expect
	 * a MSG_3D_POINT reply. We're sending one ulong and three floats.
	 */
	const int bufsize = 1*4 + 3*4;
	static char buf[bufsize];
	int nbytes;

	/* Initialise the buffer. */
	char *p = buf;
	p = pan_socket_poke_ulong (p, MSG_GET_POINT);
	p = pan_socket_poke_float(p, dx);
	p = pan_socket_poke_float(p, dy);
	p = pan_socket_poke_float(p, dz);
	nbytes = p - buf;

	/* Write the buffer in one go. */
	assert(nbytes == bufsize);
	pan_socket_write(s, buf, nbytes);
	return pan_net_want(s, MSG_3D_POINT);
}
void
pan_net_get_point_RX(SOCKET s, float *px, float *py, float *pz, char *perr)
{
	char valid;

	/* Read the value then the error flag */
	(void)pan_socket_read_float(s, px);
	(void)pan_socket_read_float(s, py);
	(void)pan_socket_read_float(s, pz);
	(void)pan_socket_read_bool(s, &valid);
	if (perr) *perr = valid ? (char)1 : (char)0;
}


/*
 * err = pan_net_get_points_TX(s, n, pv) requests the 3D positions of the "n"
 * points on the model visible along the directions pv[3*i].
 *
 * pan_net_get_points_RX(s, rv, ev) writes the 3D positions into the array
 * rv[]. Each rv[3*i] position is only valid if ev[i] is non-zero. The "rv"
 * array must hold 3*n elements while the "ev" array must hold "n" elements.
 *
 * IMPLEMENTS GetPoints (7)
 */
char *
pan_net_get_points_TX(SOCKET s, unsigned long n, float *posv)
{
	unsigned long i;

	/*
	 * Send the GetPoints message with parameters "n", the number
	 * of camera positions provided and "n" floating-point 3D-positions.
	 *
	 * We're sending two ulongs and 3*n floats. Since the buffer size
	 * is variable we have to allocate it on the heap.
	 */
	const int bufsize = 4*2 + 3*n*4;
	char *buf = (char *)malloc(bufsize); assert(buf);
	int nbytes;

	/* Initialise the buffer. */
	char *p = buf;
	p = pan_socket_poke_ulong (p, MSG_GET_POINTS);
	p = pan_socket_poke_ulong(p, n);
	for (i = 0; i < n; i++)
	{
		/* Send each float individually (must be encoded) */
		p = pan_socket_poke_float(p, *posv++);
		p = pan_socket_poke_float(p, *posv++);
		p = pan_socket_poke_float(p, *posv++);
	}
	nbytes = p - buf;

	/* Write the buffer in one go. */
	assert(nbytes == bufsize);
	pan_socket_write(s, buf, nbytes);
	(void)free(buf); buf = 0;

	/* We want a MSG_3D_POINT_ARRAY reply */
	return pan_net_want(s, MSG_3D_POINT_ARRAY);
}
void
pan_net_get_points_RX(SOCKET s, float *resultv, char *errorv)
{
	unsigned long i, nelts;

	/* Read the number of elements in the array */
	(void)pan_socket_read_ulong(s, &nelts);

	/* Read the results into the user-supplied buffer */
	for (i = 0; i < nelts; i++)
	{
		(void)pan_socket_read_float(s, resultv++); /* x */
		(void)pan_socket_read_float(s, resultv++); /* y */
		(void)pan_socket_read_float(s, resultv++); /* z */
		(void)pan_socket_read_bool(s, errorv++);
	}
}


/*
 * err = pan_net_echo_TX(s, p, n) is used to pass the array of n bytes
 * starting at address p to the server.
 *
 * r = pan_net_echo_RX(s, &m) returns the reply data as the array r whose
 * length is written to m. The result array must be freed after use by calling
 * free(r).
 *
 * IMPLEMENTS Echo (8)
 */
char *
pan_net_echo_TX
(
	SOCKET s,
	void *src,
	unsigned long n
)
{
	/*
	 * Send the Echo message.
	 * We're sending two ulongs and n bytes. Since the buffer size
	 * is variable we have to allocate it on the heap.
	 */
	const int bufsize = 2*4 + n;
	char *buf = (char *)malloc(bufsize); assert(buf);
	int nbytes;

	/* Initialise the buffer. */
	char *p = buf;
	p = pan_socket_poke_ulong(p, MSG_ECHO);
	p = pan_socket_poke_ulong(p, n);
	p = pan_socket_poke(p, src, n);
	nbytes = p - buf;

	/* Write the buffer in one go. */
	assert(nbytes == bufsize);
	pan_socket_write(s, buf, nbytes);
	(void)free(buf); buf = 0;

	// Want an EchoReply
	return pan_net_want(s, MSG_ECHO_REPLY);
}
void *
pan_net_echo_RX
(
	SOCKET s,
	unsigned long *psize
)
{
	unsigned long fsize = 0;
	unsigned char *result = 0;

	/* Read the size of the data in the message */
	(void)pan_socket_read_ulong(s, &fsize);
	if (psize) *psize = fsize;

	/*
	 * Allocate a buffer large enough for the result. We add one
	 * in case the size is zero because we need a valid pointer.
	 */
	result = (unsigned char *)malloc(fsize + 1);

	/* Read the data directly into the result buffer */
	(void)pan_socket_read(s, (void *)result, fsize);
	return (void *)result;
}


/*
 * err = pan_net_get_range_image_TX(s, o, k) requests a range
 * image from the remote server using the current camera settings.
 * All range values will have "o" subtracted before being multiplied
 * by "k" to obtain a texture coordinate. The texture coordinate is
 * clamped to lie in the range [0,1] and then multiplied by W-1 where
 * W is the width of the range texture image to give a coordinate X.
 * The value of range texture image pixel (X, 0) is then used as the
 * pixel in the returned range image. The physical range is [o, o + 1/k].

 * ptr = pan_net_get_range_image_RX(s, &size) returns the range
 * image. The memory returned by the call is allocated by malloc()
 * and may be released by free(). The size field will be updated with
 * the number of bytes in the result array.
 *
 * IMPLEMENTS GetRangeImage (9)
 */
char *
pan_net_get_range_image_TX(SOCKET s, float offset, float scale)
{
	/*
	 * Send the GetRangeImage message, offset and scale.
	 * We're sending one ulong and two floats.
	 */
	const int bufsize = 1*4 + 2*4;
	static char buf[bufsize];
	int nbytes;

	/* Initialise the buffer. */
	char *p = buf;
	p = pan_socket_poke_ulong (p, MSG_GET_RANGE_IMAGE);
	p = pan_socket_poke_float(p, offset);
	p = pan_socket_poke_float(p, scale);
	nbytes = p - buf;

	/* Write the buffer in one go. */
	assert(nbytes == bufsize);
	pan_socket_write(s, buf, nbytes);

	/* Want a MSG_IMAGE reply */
	return pan_net_want(s, MSG_IMAGE);
}
unsigned char *
pan_net_get_range_image_RX(SOCKET s, unsigned long *psize)
{
	long fsize;
	unsigned char *result;

	/* Read the size of the data in the message */
	(void)pan_socket_read_long(s, &fsize);
	if (psize) *psize = fsize;

	/*
	 * Allocate a buffer large enough for the result. We add one
	 * in case the size is zero because we need a valid pointer.
	 */
	result = (unsigned char *)malloc(fsize + 1);

	/* Read the data directly into the result buffer */
	(void)pan_socket_read(s, (void *)result, fsize);
	return result;
}


/*
 * err = pan_net_get_range_texture_TX(s) requests the range texture
 * image from the remote server. This is a 1-D image whose
 * width can be used to determine the depth resolution of images from
 * pan_net_get_range_image_RX(). The pixel values can be used to
 * deduce physical ranges from pan_net_get_range_image_RX() ranges.
 *
 * ptr = pan_net_get_range_texture_RX(s, &size) returns the range
 * texture image. The memory returned by the call is allocated by
 * malloc() and may be released by free(). The size field will be
 * updated with the number of bytes in the result array.
 *
 * IMPLEMENTS GetRangeTexture (10)
 */
char *
pan_net_get_range_texture_TX(SOCKET s)
{
	/* Send empty GetRangeTexture message and want a MSG_IMAGE reply */
	(void)pan_socket_write_ulong(s, MSG_GET_RANGE_TEXTURE);
	return pan_net_want(s, MSG_IMAGE);
}
unsigned char *
pan_net_get_range_texture_RX(SOCKET s, unsigned long *psize)
{
	long fsize;
	unsigned char *result;

	/* Read the size of the data in the message */
	(void)pan_socket_read_long(s, &fsize);
	if (psize) *psize = fsize;

	/*
	 * Allocate a buffer large enough for the result. We add one
	 * in case the size is zero because we need a valid pointer.
	 */
	result = (unsigned char *)malloc(fsize + 1);

	/* Read the data directly into the result buffer */
	(void)pan_socket_read(s, (void *)result, fsize);
	return result;
}


/*
 * err = pan_net_get_viewpoint_by_degrees_s_TX(s, x, y, z, yw, pi, rl) is
 * used to set the camera/viewpoint position to (x, y, z) and yaw/pitch/roll
 * to (yw, pi, rl) and request an image from that position.
 *
 * ptr = pan_net_get_viewpoint_by_degrees_s_RX(s, &size) returns the image as
 * an array of unsigned chars. The memory returned by the call is allocated by
 * malloc() and may be released by free(). The size field will be updated with
 * the number of bytes in the result array. Angles are in degrees.
 *
 * IMPLEMENTS GetViewpointByDegreesS (11)
 */
char *
pan_net_get_viewpoint_by_degrees_s_TX(SOCKET s, float x, float y, float z, float yw, float pi, float rl)
{
	/*
	 * Send the GetViewpointByDegrees message with parameters (x, y, z)
	 * and (yw, pi, rl). We're sending one ulong and six floats.
	 */
	const int bufsize = 1*4 + 6*4;
	static char buf[bufsize];
	int nbytes;

	/* Initialise the buffer. */
	char *p = buf;
	p = pan_socket_poke_ulong (p, MSG_GET_VIEWPOINT_BY_DEGREES_S);
	p = pan_socket_poke_float(p, x);
	p = pan_socket_poke_float(p, y);
	p = pan_socket_poke_float(p, z);
	p = pan_socket_poke_float(p, yw);
	p = pan_socket_poke_float(p, pi);
	p = pan_socket_poke_float(p, rl);
	nbytes = p - buf;

	/* Write the buffer in one go. */
	assert(nbytes == bufsize);
	pan_socket_write(s, buf, nbytes);

	/* We want a MSG_IMAGE reply */
	return pan_net_want(s, MSG_IMAGE);
}
unsigned char *
pan_net_get_viewpoint_by_degrees_s_RX(SOCKET s, unsigned long *psize)
{
	long fsize;
	unsigned char *result;

	/* Read the size of the data in the message */
	(void)pan_socket_read_long(s, &fsize);
	if (psize) *psize = fsize;

	/*
	 * Allocate a buffer large enough for the result. We add one
	 * in case the size is zero because we need a valid pointer.
	 */
	result = (unsigned char *)malloc(fsize + 1);

	/* Read the data directly into the result buffer */
	(void)pan_socket_read(s, (void *)result, fsize);
	return result;
}


/*
 * err = pan_net_get_viewpoint_by_quaternion_s_TX(s,x,y,z,q0,q1,q2,q3)
 * is used to set the camera/viewpoint position to (x, y, z) and
 * attitude as defined by the quaternion [q0, q1, q2, q3] and request
 * an image from that position.
 *
 * ptr = pan_net_get_viewpoint_by_quaternion_s_RX(s, &size) is used to
 * receive the image. The memory returned by the call is allocated
 * by malloc() and may be released by free(). The size field will be
 * updated with the number of bytes in the result array.
 *
 * IMPLEMENTS GetViewpointByQuaternionS (12)
 */
char *
pan_net_get_viewpoint_by_quaternion_s_TX(SOCKET s, float x, float y, float z, float q0, float q1, float q2, float q3)
{
	/*
	 * Send the GetViewpointByQuaternion message with parameters (x, y, z)
	 * and (q0, q1, q2, q3). We're sending one ulong and seven floats.
	 */
	const int bufsize = 1*4 + 7*4;
	static char buf[bufsize];
	int nbytes;

	/* Initialise the buffer. */
	char *p = buf;
	p = pan_socket_poke_ulong (p, MSG_GET_VIEWPOINT_BY_QUATERNION_S);
	p = pan_socket_poke_float(p, x);
	p = pan_socket_poke_float(p, y);
	p = pan_socket_poke_float(p, z);
	p = pan_socket_poke_float(p, q0);
	p = pan_socket_poke_float(p, q1);
	p = pan_socket_poke_float(p, q2);
	p = pan_socket_poke_float(p, q3);
	nbytes = p - buf;

	/* Write the buffer in one go. */
	assert(nbytes == bufsize);
	pan_socket_write(s, buf, nbytes);

	/* We expect a MSG_IMAGE reply */
	return pan_net_want(s, MSG_IMAGE);
}
unsigned char *
pan_net_get_viewpoint_by_quaternion_s_RX(SOCKET s, unsigned long *psize)
{
	long fsize;
	unsigned char *result;

	/* Read the size of the data in the message */
	(void)pan_socket_read_long(s, &fsize);
	if (psize) *psize = fsize;

	/*
	 * Allocate a buffer large enough for the result. We add one
	 * in case the size is zero because we need a valid pointer.
	 */
	result = (unsigned char *)malloc(fsize + 1);

	/* Read the data directly into the result buffer */
	(void)pan_socket_read(s, (void *)result, fsize);
	return result;
}


/*
 * err = pan_net_get_lidar_pulse_result_TX(s, x,y,z, dx,dy,dz) is used to
 * request a LIDAR pulse from position (x, y, z) along direction
 * (dx, dy, dz).
 *
 * pan_net_get_lidar_pulse_result_RX(s, &r, &a) is used to obtain the
 * result. The range to the surface and the cosine of the incidence
 * angle will be written to r and a.
 *
 * IMPLEMENTS GetLidarPulseResult (13)
 */
char *
pan_net_get_lidar_pulse_result_TX(SOCKET s, float x, float y, float z, float dx, float dy, float dz)
{
	/*
	 * Send the GetLidarPulseResult message with parameters (x, y, z)
	 * and (dx, dy, dz). We're sending one ulong and six floats.
	 */
	const int bufsize = 1*4 + 6*4;
	static char buf[bufsize];
	int nbytes;

	/* Initialise the buffer. */
	char *p = buf;
	p = pan_socket_poke_ulong (p, MSG_GET_LIDAR_PULSE_RESULT);
	p = pan_socket_poke_float(p, x);
	p = pan_socket_poke_float(p, y);
	p = pan_socket_poke_float(p, z);
	p = pan_socket_poke_float(p, dx);
	p = pan_socket_poke_float(p, dy);
	p = pan_socket_poke_float(p, dz);
	nbytes = p - buf;

	/* Write the buffer in one go. */
	assert(nbytes == bufsize);
	pan_socket_write(s, buf, nbytes);

	/* We want a MSG_LIDAR_PULSE_RESULT reply */
	return pan_net_want(s, MSG_LIDAR_PULSE_RESULT);
}
void
pan_net_get_lidar_pulse_result_RX(SOCKET s, float *pr, float *pa)
{
	float v0, v1;

	/* Read the two floating point results */
	(void)pan_socket_read_float(s, &v0); /* range */
	(void)pan_socket_read_float(s, &v1); /* cos(angle) */

	/* Update the return value locations */
	if (pr) *pr = v0;
	if (pa) *pa = v1;
}


/*
 * Backwards compatibility. See GetLidarMeasurementS (34)
 *
 * pan_net_get_lidar_measurement_RX() automatically converts from PANGU
 * network floats to native floats before returning. However, note that
 * the server (incorrectly) sends data in native byte order (thus
 * little endian) rather than network order.
 *
 * GetLidarMeasurementS (34) receives the data in network order.
 *
 * IMPLEMENTS GetLidarMeasurement (14)
 */
char *
pan_net_get_lidar_measurement_TX(SOCKET s,
	float px, float py, float pz,
	float q0, float q1, float q2, float q3,
	float vx, float vy, float vz,
	float rx, float ry, float rz,
	float ax, float ay, float az,
	float sx, float sy, float sz,
	float jx, float jy, float jz,
	float tx, float ty, float tz)
{
	/*
	 * Send the GetLidarMeasurement message.
	 * We're sending one ulong and 25 floats.
	 */
	const int bufsize = 1*4 + 25*4;
	static char buf[bufsize];
	int nbytes;

	/* Initialise the buffer. */
	char *p = buf;
	p = pan_socket_poke_ulong(p, MSG_GET_LIDAR_MEASUREMENT);
	p = pan_socket_poke_float(p, px);
	p = pan_socket_poke_float(p, py);
	p = pan_socket_poke_float(p, pz);
	p = pan_socket_poke_float(p, q0);
	p = pan_socket_poke_float(p, q1);
	p = pan_socket_poke_float(p, q2);
	p = pan_socket_poke_float(p, q3);
	p = pan_socket_poke_float(p, vx);
	p = pan_socket_poke_float(p, vy);
	p = pan_socket_poke_float(p, vz);
	p = pan_socket_poke_float(p, rx);
	p = pan_socket_poke_float(p, ry);
	p = pan_socket_poke_float(p, rz);
	p = pan_socket_poke_float(p, ax);
	p = pan_socket_poke_float(p, ay);
	p = pan_socket_poke_float(p, az);
	p = pan_socket_poke_float(p, sx);
	p = pan_socket_poke_float(p, sy);
	p = pan_socket_poke_float(p, sz);
	p = pan_socket_poke_float(p, jx);
	p = pan_socket_poke_float(p, jy);
	p = pan_socket_poke_float(p, jz);
	p = pan_socket_poke_float(p, tx);
	p = pan_socket_poke_float(p, ty);
	p = pan_socket_poke_float(p, tz);
	nbytes = p - buf;

	/* Write the buffer in one go. */
	assert(nbytes == bufsize);
	pan_socket_write(s, buf, nbytes);

	/* We want a MSG_LIDAR_MEASUREMENT reply */
	return pan_net_want(s, MSG_LIDAR_MEASUREMENT);
}
float *
pan_net_get_lidar_measurement_RX(SOCKET s,
	float *pfx, float *pfy,
	unsigned long *pnx, unsigned long *pny,
	float *ptx, float *pty,
	unsigned long *pn, unsigned long *pm,
	unsigned long *pt, unsigned long *pfl,
	float *paz, float *pel, float *pth,
	float *pfaz, float *pfel,
	float *ptoff, float *ptaz0, float *ptel0)
{
	/* Return value */
	float *result, *dptr;
	char *sptr; /*unsigned long *sptr;*/

	/* Temporaries. */
	float f;
	unsigned long i, fsize, dsize, samplec, got;
	unsigned long nx, ny, n, m, t, fl;

	/* Count the number of LIDAR parameter fields we read */
	got = 0;

	/* Read and save the LIDAR emitter/detector parameters */
	(void)pan_socket_read_float(s, &f);  got++; if (pfx)  *pfx  = f;
	(void)pan_socket_read_float(s, &f);  got++; if (pfy)  *pfy  = f;
	(void)pan_socket_read_ulong(s, &nx); got++; if (pnx)  *pnx  = nx;
	(void)pan_socket_read_ulong(s, &ny); got++; if (pny)  *pny  = ny;
	(void)pan_socket_read_float(s, &f);  got++; if (ptx)  *ptx  = f;
	(void)pan_socket_read_float(s, &f);  got++; if (pty)  *pty  = f;
	(void)pan_socket_read_ulong(s, &n);  got++; if (pn)   *pn   = n;
	(void)pan_socket_read_ulong(s, &m);  got++; if (pm)   *pm   = m;
	(void)pan_socket_read_ulong(s, &t);  got++; if (pt)   *pt   = t;
	(void)pan_socket_read_ulong(s, &fl); got++; if (pfl)  *pfl  = fl;
	(void)pan_socket_read_float(s, &f);  got++; if (paz)  *paz  = f;
	(void)pan_socket_read_float(s, &f);  got++; if (pel)  *pel  = f;
	(void)pan_socket_read_float(s, &f);  got++; if (pth)  *pth  = f;
	(void)pan_socket_read_float(s, &f);  got++; /* wx */
	(void)pan_socket_read_float(s, &f);  got++; /* wy */
	(void)pan_socket_read_float(s, &f);  got++; if (pfaz) *pfaz = f;
	(void)pan_socket_read_float(s, &f);  got++; if (pfel) *pfel = f;
	(void)pan_socket_read_float(s, &f);  got++; if (ptoff) *ptoff = f;
	(void)pan_socket_read_float(s, &f);  got++; if (ptaz0) *ptaz0 = f;
	(void)pan_socket_read_float(s, &f);  got++; if (ptel0) *ptel0 = f;

	/* Read any padding words */
	for (; got < 32; got++) (void)pan_socket_read_ulong(s, &i);

	/* Read the number of bytes in the data portion of the packet */
	(void)pan_socket_read_ulong(s, &dsize);

	/* Compute the number of samples in the result arrays */
	samplec = 0;
	if (fl & (1UL<<0)) samplec += 2;
	if (fl & (1UL<<1)) samplec += 2;
	if (fl & (1UL<<2)) samplec += 2;
	if (fl & (1UL<<3)) samplec += 2;

	/* Allocate a buffer large enough for the result */
	fsize = samplec*(nx*n)*(ny*m)*sizeof(float);

	/*
	 * Allocate a buffer large enough for the result. We add one
	 * in case the size is zero because we need a valid pointer.
	 */
	result = (float *)malloc(fsize + 1);

	/* We expect fsize == dsize */
	if (dsize > fsize)
	{
		/* This is the number of padding/garbage bytes */
		unsigned long excess = dsize - fsize;
		unsigned char *trash = (unsigned char *)malloc(excess);

		/* Read what we expected and then the garbage */
		(void)pan_socket_read(s, (void *)result, fsize);
		(void)pan_socket_read(s, (void *)trash,  excess);

		/* Throw out the trash */
		(void)free(trash);
	}
	else
		(void)pan_socket_read(s, (void *)result, dsize);

	/*
	 * Convert the PANGU floats into native floats in situ. Note that
	 * a bug in the server means that the floating point data is sent
	 * in native byte order not network byte order. Since we only support
	 * little-endian hosts at the moment this means that the server can
	 * be regarded as sending data in little endian order. In the original
	 * code below we assumed that the client is also little endian. To
	 * solve the problem now we use the socket I/O ulong peek function
	 * which will apply an unwanted ntohl() on little endian machines.
	 * So we call htonl() to undo the unwanted conversion before calling
	 * ulong2float() to perform the PANGU/native format conversion. This
	 * will not work for big endian machines because ntohl() has no effect
	 * so the data will remain in little endian format. Since we don't
	 * support big endian machines yet this isn't a problem.
	 */
#if 0
	// FIXME: old code to be removed
	assert(sizeof(float) == sizeof(unsigned long));
	dptr = result;
	sptr = (unsigned long *)result;
	for (i = 0; i < fsize/sizeof(float); i++)
		*dptr++ = ulong2float(*sptr++);
#else
	// FIXME: new code to be kept.
	// FIXME: ought to replace htonl which a byte swap if on big endian
	dptr = result;
	sptr = (char *)result;
	for (i = 0; i < fsize/sizeof(float); i++)
	{
		unsigned long tmp = 0;
		sptr = pan_socket_peek_ulong(sptr, &tmp);
		*dptr++ = ulong2float(htonl(tmp));
	}
#endif

	/* Return the array of float pairs */
	return result;
}


/*
 * err = pan_net_get_radar_response_TX(s,fl,n,nr,ns,ox,oy,oz,vx,vy,vz,
 *       q0,q1,q1,q3,bw,rmd,smd,rbs,sbs)
 * is used to request a RADAR response for a beam emitted from the
 * point (ox,oy,oz) moving with velocity (vx,vy,vz) with axis (q0,q1,q2,q3)
 * and width bw degrees. The beam is sampled n times and the results are
 * integrated using a 2D histogram with nr range bins and ns speed bins. The
 * fl argument defines flags for the simulation with the meanings of each bit
 * defined as follows:
 *      bit  0: if set then zero align the left edge of the range histograms
 *      bit  1: if set then centre align the range histograms on rmd
 *      bit  2: if set then round range histogram width up to power of 10
 *      bit  3: if set then use fixed range histogram bin size rbs
 *      bit  4: if set then zero align the left edge of the speed histograms
 *      bit  5: if set then centre align the speed histograms on smd
 *      bit  6: if set then round speed histogram width up to power of 10
 *      bit  7: if set then use fixed speed histogram bin size sbs
 *      bit  8: if set then surface slope effects are ignored
 *      bit  9: if set then each sample is worth 1 not 1/n

 * v = pan_net_get_radar_response_TX(s,&st,&mxv,&totv,&offr,&offs,&bsr,&bss,
 *     &mnr,&mxr,&mns,&mxs,&nrb,&nsb, &nused)
 * is used to retrieve the RADAR response results.
 * The status of the response is returned in st with the maximum signal value
 * in mxv. The range associated with the left edge of the first histogram bin
 * is offr. The minimum and maximum ranges before clipping by the histogram
 * are mnr and mxr respectively; the minimum and maximum speeds are mns and
 * mxs respectively. The size of each bin is bsr for range and bss for speed.
 * The number of bins actually created is nrb and nsb. The 2D array of nrb*nsb
 * bins is returned as v consisting of nrb range values for the first speed
 * histogram, then the next nrb range values for the second speed histogram
 * etc. The sum of all elements of v is returned in totv. The number of samples
 * actually used in the histogram (the number that hit a target) is returned
 * in nused.
 *
 * IMPLEMENTS GetRadarResponse (15)
 */
char *
pan_net_get_radar_response_TX
(
	SOCKET s,
	unsigned long flags,
	unsigned long n,
	unsigned long nr, unsigned long ns,
	float ox, float oy, float oz,
	float vx, float vy, float vz,
	float q0, float q1, float q2, float q3,
	float bwidth,
	float rmid, float smid, // Ooops: we ought to have swapped these
	float rbs, float sbs   // two lines to match PROTOCOL.txt
)
{
	int i = 0;

	/*
	 * Send the GetRadarResponse message.
	 * We're sending five ulongs, fifteen floats and 13 ulong pads.
	 */
	const int bufsize = 5*4 + 15*4 + 13*4;
	static char buf[bufsize];
	int nbytes;

	/* Initialise the buffer. */
	char *p = buf;
	p = pan_socket_poke_ulong(p, MSG_GET_RADAR_RESPONSE);
	p = pan_socket_poke_ulong(p, flags);
	p = pan_socket_poke_ulong(p, n);
	p = pan_socket_poke_ulong(p, nr);
	p = pan_socket_poke_ulong(p, ns);
	p = pan_socket_poke_float(p, rbs);
	p = pan_socket_poke_float(p, sbs);
	p = pan_socket_poke_float(p, rmid);
	p = pan_socket_poke_float(p, smid);
	p = pan_socket_poke_float(p, ox);
	p = pan_socket_poke_float(p, oy);
	p = pan_socket_poke_float(p, oz);
	p = pan_socket_poke_float(p, vx);
	p = pan_socket_poke_float(p, vy);
	p = pan_socket_poke_float(p, vz);
	p = pan_socket_poke_float(p, q0);
	p = pan_socket_poke_float(p, q1);
	p = pan_socket_poke_float(p, q2);
	p = pan_socket_poke_float(p, q3);
	p = pan_socket_poke_float(p, bwidth);
	for (i = 0; i < 13; i++) p = pan_socket_poke_ulong(p, 0);
	nbytes = p - buf;

	/* Write the buffer in one go. */
	assert(nbytes == bufsize);
	pan_socket_write(s, buf, nbytes);

	/* We want a MSG_RADAR_RESPONSE reply */
	return pan_net_want(s, MSG_RADAR_RESPONSE);
}
float *
pan_net_get_radar_response_RX
(
	SOCKET s,
	unsigned long *pstat,
	float *pmaxv, float *ptotv,
	float *poffr, float *poffs,
	float *prbsize, float *psbsize,
	float *pminr, float *pmaxr,
	float *pmins, float *pmaxs,
	unsigned long *pnrelts, unsigned long *pnselts,
	unsigned long *pnused
)
{
	/* Return value */
	float *result;

	/* Temporaries */
	unsigned long i = 0;
	unsigned long nrbins = 0, nsbins = 0;
	unsigned long used = 0;
	unsigned long status = 0;
	float maxv = 0.0f, totv = 0.0f;
	float minr = 0.0f, maxr = 0.0f;
	float mins = 0.0f, maxs = 0.0f;
	float rbinsize = 0.0f, sbinsize = 0.0f;
	float offr = 0.0f, offs = 0.0f;
	unsigned long junk = 0;

	/* Read the response header */
	(void)pan_socket_read_ulong(s, &status);
	(void)pan_socket_read_float(s, &maxv);
	(void)pan_socket_read_float(s, &totv);
	(void)pan_socket_read_float(s, &offr);
	(void)pan_socket_read_float(s, &offs);
	(void)pan_socket_read_float(s, &rbinsize);
	(void)pan_socket_read_float(s, &sbinsize);
	(void)pan_socket_read_float(s, &minr);
	(void)pan_socket_read_float(s, &maxr);
	(void)pan_socket_read_float(s, &mins);
	(void)pan_socket_read_float(s, &maxs);
	(void)pan_socket_read_ulong(s, &used);
	(void)pan_socket_read_ulong(s, &nrbins);
	(void)pan_socket_read_ulong(s, &nsbins);
	for (i = 0; i < 10; i++) (void)pan_socket_read_ulong(s, &junk);

	/* Update the user return value fields. */
	if (pstat)   *pstat   = status;
	if (pmaxv)   *pmaxv   = maxv;
	if (ptotv)   *ptotv   = totv;
	if (poffr)   *poffr   = offr;
	if (poffs)   *poffs   = offs;
	if (prbsize) *prbsize = rbinsize;
	if (psbsize) *psbsize = sbinsize;
	if (pminr)   *pminr   = minr;
	if (pmaxr)   *pmaxr   = maxr;
	if (pmins)   *pmins   = mins;
	if (pmaxs)   *pmaxs   = maxs;
	if (pnrelts) *pnrelts = nrbins;
	if (pnselts) *pnselts = nsbins;
	if (pnused)  *pnused  = used;

	/*
	 * Allocate a buffer large enough for the result. We add one
	 * in case the size is zero because we need a valid pointer.
	 */
	result = (float *)malloc(nrbins*nsbins*sizeof(float) + 1);

	/* Read the results and return */
	for (i = 0; i < nrbins*nsbins; i++)
	{
		unsigned long tmp = 0;
		(void)pan_socket_read_ulong(s, &tmp);
		result[i] = ulong2float(tmp);
	}
	return result;
}


/*
 * err = pan_net_get_viewpoint_by_degrees_d_TX(s, x, y, z, yw, pi, rl) is
 * used to set the camera/viewpoint position to (x, y, z) and yaw/pitch/roll
 * to (yw, pi, rl) and request an image from that position. Angles are in
 * degrees.
 *
 * ptr = pan_net_get_viewpoint_by_degrees_d_RX(s, &size) is receive the image.
 * The memory returned by the call is allocated by malloc() and may be
 * released by free(). The size field will be updated with the number
 * of bytes in the result array.
 *
 * IMPLEMENTS GetViewpointByDegreesD (16)
 */
char *
pan_net_get_viewpoint_by_degrees_d_TX(SOCKET s, double x, double y, double z, double yw, double pi, double rl)
{
	/*
	 * Send the GetViewpointByDegrees message with parameters (x, y, z)
	 * and (yw, pi, rl). We're sending one ulong and six doubles.
	 */
	const int bufsize = 1*4 + 6*8;
	static char buf[bufsize];
	int nbytes;

	/* Initialise the buffer. */
	char *p = buf;
	p = pan_socket_poke_ulong (p, MSG_GET_VIEWPOINT_BY_DEGREES_D);
	p = pan_socket_poke_double(p, x);
	p = pan_socket_poke_double(p, y);
	p = pan_socket_poke_double(p, z);
	p = pan_socket_poke_double(p, yw);
	p = pan_socket_poke_double(p, pi);
	p = pan_socket_poke_double(p, rl);
	nbytes = p - buf;

	/* Write the buffer in one go. */
	assert(nbytes == bufsize);
	pan_socket_write(s, buf, nbytes);

	/* We want a MSG_IMAGE reply */
	return pan_net_want(s, MSG_IMAGE);
}
unsigned char *
pan_net_get_viewpoint_by_degrees_d_RX(SOCKET s, unsigned long *psize)
{
	long fsize;
	unsigned char *result;

	/* Read the size of the data in the message */
	(void)pan_socket_read_long(s, &fsize);
	if (psize) *psize = fsize;

	/*
	 * Allocate a buffer large enough for the result. We add one
	 * in case the size is zero because we need a valid pointer.
	 */
	result = (unsigned char *)malloc(fsize + 1);

	/* Read the data directly into the result buffer */
	(void)pan_socket_read(s, (void *)result, fsize);
	return result;
}


/*
 * err = pan_net_get_viewpoint_by_quaternion_d_TX(s, x, y, z, q0, q1, q2, q3)
 * is used to set the camera/viewpoint position to (x, y, z) and attitude as
 * defined by the quaternion [q0, q1, q2, q3] and request an image from that
 * position.
 * 
 * ptr = pan_net_get_viewpoint_by_quaternion_d_RX(s, &size) is used to receive
 * the image. The memory returned by the call is allocated by malloc() and may
 * be released by free(). The size field will be updated with the number of
 * bytes in the result array.
 *
 * IMPLEMENTS GetViewpointByQuaternionD (17)
 */
char *
pan_net_get_viewpoint_by_quaternion_d_TX(SOCKET s, double x, double y, double z, double q0, double q1, double q2, double q3)
{
	/*
	 * Send the GetViewpointByQuaternion message with parameters (x, y, z)
	 * and (q0, q1, q2, q3). We're sending one ulong and seven doubles.
	 */
	const int bufsize = 1*4 + 7*8;
	static char buf[bufsize];
	int nbytes;

	/* Initialise the buffer. */
	char *p = buf;
	p = pan_socket_poke_ulong (p, MSG_GET_VIEWPOINT_BY_QUATERNION_D);
	p = pan_socket_poke_double(p, x);
	p = pan_socket_poke_double(p, y);
	p = pan_socket_poke_double(p, z);
	p = pan_socket_poke_double(p, q0);
	p = pan_socket_poke_double(p, q1);
	p = pan_socket_poke_double(p, q2);
	p = pan_socket_poke_double(p, q3);
	nbytes = p - buf;

	/* Write the buffer in one go. */
	assert(nbytes == bufsize);
	pan_socket_write(s, buf, nbytes);

	/* We expect a MSG_IMAGE reply */
	return pan_net_want(s, MSG_IMAGE);
}
unsigned char *
pan_net_get_viewpoint_by_quaternion_d_RX(SOCKET s, unsigned long *psize)
{
	long fsize;
	unsigned char *result;

	/* Read the size of the data in the message */
	(void)pan_socket_read_long(s, &fsize);
	if (psize) *psize = fsize;

	/*
	 * Allocate a buffer large enough for the result. We add one
	 * in case the size is zero because we need a valid pointer.
	 */
	result = (unsigned char *)malloc(fsize + 1);

	/* Read the data directly into the result buffer */
	(void)pan_socket_read(s, (void *)result, fsize);
	return result;
}


/*
 * err = pan_net_get_joints_TX(s, i) requests joint data for each joint of 
 * object i.
 *
 * ptr = pan_net_get_joints_RX(s, n) receives the array of joint_data with
 * one entry for each joint. The location pointed to by n is filled with
 * the number of entries in the joint list. Each joint_data has: 
 *    id: The handle used to identify the joint
 *  name: Descriptive name for the joint
 *  type: 0 = general joint
 *        1 = revolute joint
 *        2 = prismatic joint
 *
 * If an invalid object is specified, an empty list is returned and *n = 0.
 * To free the joint list, first free each 'name' field.
 *
 * IMPLEMENTS GetJoints (18)
 */
char * 
pan_net_get_joints_TX(SOCKET s, unsigned long o)
{
	/*
	 * Send the GetJoints message.
	 * We're sending two ulongs.
	 */
	const int bufsize = 2*4;
	static char buf[bufsize];
	int nbytes;

	/* Initialise the buffer. */
	char *p = buf;
	p = pan_socket_poke_ulong(p, MSG_GET_JOINTS);
	p = pan_socket_poke_ulong(p, o);
	nbytes = p - buf;

	/* Write the buffer in one go. */
	assert(nbytes == bufsize);
	pan_socket_write(s, buf, nbytes);

	/* We want a MSG_JOINT_LIST reply */
	return pan_net_want(s, MSG_JOINT_LIST);
}
joint_data * 
pan_net_get_joints_RX(SOCKET s, unsigned long *n)
{
	unsigned long njoints;
	joint_data* jlist;
	joint_data* joint;
	unsigned long i;

	/* Read number of joints and allocate storage */
	pan_socket_read_ulong(s, &njoints);
	jlist = (joint_data*)malloc(njoints*sizeof(joint_data)+1);
	if (n) *n = njoints;

	/* Read the details of each joint */
	joint = jlist;
	for (i = 0; i < njoints; i++, joint++)
	{
		pan_socket_read_ulong(s, &(joint->id));
		pan_socket_read_string(s, &(joint->name));
		pan_socket_read_ulong(s, &(joint->type));
	}
	return jlist;
}


/*
 * err = pan_net_get_joint_config_TX(s,o,j) requests the configuration
 * of joint j of object o.
 *
 * pan_net_get_joint_config_RX(s,c) stores the configuration in c, which
 * should point to an array of 9 doubles.
 *
 * For general joints the configuration is 
 *    [sx,sy,sz,rx,ry,rz,tx,ty,tz]
 *    s = scale factor
 *    r = rotation (radians)
 *    t = translation (world units)
 *
 * For prismatic joints, the first element is the joint displacement,
 * other elements have no meaning.
 *
 * For revolute joints, the first element is the joint angle in
 * radians, other elements have no meaning.
 *
 * If an invalid joint or object is specified, each element of c is
 * set to zero.
 *
 * IMPLEMENTS GetJointConfig (19)
 */
char * 
pan_net_get_joint_config_TX
(
	SOCKET s,
	unsigned long obj,
	unsigned long joint
)
{
	/*
	 * Send a GetJointConfig message.
	 * We're sending three ulongs.
	 */
	const int bufsize = 3*4;
	static char buf[bufsize];
	int nbytes;

	/* Initialise the buffer. */
	char *p = buf;
	p = pan_socket_poke_ulong(p, MSG_GET_JOINT_CONFIG);
	p = pan_socket_poke_ulong(p, obj);
	p = pan_socket_poke_ulong(p, joint);
	nbytes = p - buf;

	/* Write the buffer in one go. */
	assert(nbytes == bufsize);
	pan_socket_write(s, buf, nbytes);

	/* We want an MSG_DOUBLE_ARRAY message */
	return pan_net_want(s, MSG_DOUBLE_ARRAY);
}
void 
pan_net_get_joint_config_RX
(
	SOCKET s,
	double *config
)
{
	unsigned long n;
	char ignored;
	unsigned long i;

	/* Read back the size of the array */
	pan_socket_read_ulong(s, &n);

	/* Protocol specifies that a joint has 9 parameters */
	assert(n == 9);
	
	/* Read back array elements */
	for (i = 0; i < 9; i++)
	{
		pan_socket_read_double(s, &(config[i]));
		pan_socket_read_bool(s, &ignored);
	}		
}


/*
 * err = pan_net_get_frames_TX(s,o) requests an array of frame_data,
 * with one entry for each frame of object o.
 *
 * ptr = pan_net_get_frames_RX(o,n) receives the array. The variable
 * pointed to by n is set to the number of frames.
 *
 * IMPLEMENTS GetFrames (20)
 */
char *
pan_net_get_frames_TX(SOCKET s, unsigned long obj)
{
	/*
	 * Send the GetFrames message.
	 * We're sending two ulongs.
	 */
	const int bufsize = 2*4;
	static char buf[bufsize];
	int nbytes;

	/* Initialise the buffer. */
	char *p = buf;
	p = pan_socket_poke_ulong(p, MSG_GET_FRAMES);
	p = pan_socket_poke_ulong(p, obj);
	nbytes = p - buf;

	/* Write the buffer in one go. */
	assert(nbytes == bufsize);
	pan_socket_write(s, buf, nbytes);

	/* We want a MSG_FRAME_LIST reply */
	return pan_net_want(s, MSG_FRAME_LIST);
}
frame_data *
pan_net_get_frames_RX(SOCKET s, unsigned long *n)
{
	unsigned long nframes;
	frame_data* flist;
	frame_data* frame;
	unsigned long i;

	/* Read number of frames and allocate storage */
	pan_socket_read_ulong(s, &nframes);
	flist = (frame_data*)malloc(nframes*sizeof(frame_data) + 1);
	if (n) *n = nframes;

	/* Read the frame details */
	frame = flist;
	for (i = 0; i < nframes; i++, frame++)
	{
		pan_socket_read_ulong(s, &(frame->id));
		pan_socket_read_string(s, &(frame->name));
	}
	return flist;
}


/*
 * err = pan_net_get_frame_TX(s,o,i) requests frame i of object o.
 *
 * pan_net_get_frame_RX(s,f) stores the frame in f, which should point
 * to an array of 12 doubles. 
 *
 * IMPLEMENTS GetFrame (21)
 */
char *
pan_net_get_frame_TX
(
	SOCKET s,
	unsigned long obj,
	unsigned long id
)
{
	/*
	 * Send the GetFrame message.
	 * We're sending three ulongs.
	 */
	const int bufsize = 3*4;
	static char buf[bufsize];
	int nbytes;

	/* Initialise the buffer. */
	char *p = buf;
	p = pan_socket_poke_ulong(p, MSG_GET_FRAME);
	p = pan_socket_poke_ulong(p, obj);
	p = pan_socket_poke_ulong(p, id);
	nbytes = p - buf;

	/* Write the buffer in one go. */
	assert(nbytes == bufsize);
	pan_socket_write(s, buf, nbytes);

	/* We want a MSG_DOUBLE_ARRAY reply */
	return pan_net_want(s, MSG_DOUBLE_ARRAY);
}
void
pan_net_get_frame_RX
(
	SOCKET s,
	double *data
)
{
	int i; 
	unsigned long n;
	char ignored;

	/* Read the frame size */
	pan_socket_read_ulong(s, &n);
	assert(n==12);

	/* Read the frame */
	for (i = 0; i < 12; i++)
	{
		pan_socket_read_double(s,&(data[i]));
		pan_socket_read_bool(s, &ignored);
	}	
}


/*
 * err = pan_net_get_frame_as_radians_TX(s,o,i) requests camera parameters
 * for frame i of object o.

 * pan_net_get_frame_as_radians_RX(s,v) stores the data in v, which
 * should point to an array of 6 doubles.
 *
 * IMPLEMENTS GetFrameAsRadians (22)
 */
char *
pan_net_get_frame_as_radians_TX
(
	SOCKET s,
	unsigned long obj,
	unsigned long id
)
{
	/*
	 * Send a GetFrameAsRadians message.
	 * We're sending three ulongs.
	 */
	const int bufsize = 3*4;
	static char buf[bufsize];
	int nbytes;

	/* Initialise the buffer. */
	char *p = buf;
	p = pan_socket_poke_ulong(p, MSG_GET_FRAME_AS_RADIANS);
	p = pan_socket_poke_ulong(p, obj);
	p = pan_socket_poke_ulong(p, id);
	nbytes = p - buf;

	/* Write the buffer in one go. */
	assert(nbytes == bufsize);
	pan_socket_write(s, buf, nbytes);

	/* We want a MSG_DOUBLE_ARRAY reply */
	return pan_net_want(s, MSG_DOUBLE_ARRAY);
}
void
pan_net_get_frame_as_radians_RX
(
	SOCKET s,
	double *data
)
{
	int i; 
	unsigned long n;
	char ignored;

	/* Read the frame size */
	pan_socket_read_ulong(s, &n);
	assert(n==6);

	/* read the frame */
	for (i = 0; i < 6; i++)
	{
		pan_socket_read_double(s,&(data[i]));
		pan_socket_read_bool(s, &ignored);
	}	
}


/*
 * err = pan_net_get_surface_elevation_TX(s, b, px, py) requests the
 * surface elevation at point (x,y) on the surface. If the value of b
 * is non-zero the elevation will include any boulder lying on the
 * surface at that point, if it is zero then only the underlying
 * surface will be used.
 *
 * v = pan_net_get_surface_elevation_RX(s, &e) receives the surface
 * elevation and returns it as v. If e is not null, then it will be set
 * to a non-zero value if the elevation is valid, otherwise it will be
 * set to zero. The elevation is valid if the line x=px, y=py intersects
 * the surface.
 *
 * IMPLEMENTS GetSurfaceElevation (23)
 */
char *
pan_net_get_surface_elevation_TX
(
	SOCKET s,
	unsigned char boulders,
	float x,
	float y
)
{
	/*
	 * Send the GetSurfaceElevation message.
	 * We're sending one ulong, one bool and two floats.
	 */
	const int bufsize = 1*4 + 1*1 + 2*4;
	static char buf[bufsize];
	int nbytes;

	/* Initialise the buffer. */
	char *p = buf;
	p = pan_socket_poke_ulong(p, MSG_GET_SURFACE_ELEVATION);
	p = pan_socket_poke_bool (p, boulders);
	p = pan_socket_poke_float(p, x);
	p = pan_socket_poke_float(p, y);
	nbytes = p - buf;

	/* Write the buffer in one go. */
	assert(nbytes == bufsize);
	pan_socket_write(s, buf, nbytes);

	/* Want a MSG_FLOAT reply */
	return pan_net_want(s, MSG_FLOAT);
}
float
pan_net_get_surface_elevation_RX
(
	SOCKET s,
	char *err
)
{
	struct optional_float result;

	/* Read the value then the error flag */
	(void)pan_socket_read_float(s, &result.value);
	(void)pan_socket_read_bool(s, &result.valid);
	if (err) *err = result.valid ? (char)1 : (char)0;
	return result.value;
}


/*
 * err = pan_net_get_surface_elevations_TX(s, b, n, pv) requests
 * the surface elevations at each of the "n" surface positions "pv".
 * If the value of b is non-zero then each elevation will include any
 * boulder lying on the surface at that point, if it is zero, then only
 * the underlying surface will be used.
 *
 * pan_net_get_surface_elevations_RX(s, rv, ev) receives the elevations
 * and writes them into the array "rv". The ith elevation rv[i] is only
 * valid (the position pv[2*i] is over the model) when ev[i] is
 * non-zero. Both rv and ev must be point to an array of "n" elements.
 *
 * IMPLEMENTS GetSurfaceElevations (24)
 */
char *
pan_net_get_surface_elevations_TX
(
	SOCKET s,
	unsigned char boulders,
	unsigned long n,
	float *posv
)
{
	unsigned long i;

	/*
	 * Send the GetSurfaceElevations message.
	 * 
	 * We're sending two ulongs, one bool and 2*n floats. Since the
	 * buffer size is variable we have to allocate it on the heap.
	 */
	const int bufsize = 2*4 + 1*1 + 2*n*4;
	char *buf = (char *)malloc(bufsize); assert(buf);
	int nbytes;

	/* Initialise the buffer. */
	char *p = buf;
	p = pan_socket_poke_ulong(p, MSG_GET_SURFACE_ELEVATIONS);
	p = pan_socket_poke_bool (p, boulders);
	p = pan_socket_poke_ulong(p, n);
	for (i = 0; i < n; i++)
	{
		p = pan_socket_poke_float(p, *posv++);
		p = pan_socket_poke_float(p, *posv++);
	}
	nbytes = p - buf;

	/* Write the buffer in one go. */
	assert(nbytes == bufsize);
	pan_socket_write(s, buf, nbytes);
	(void)free(buf); buf = 0;

	/* We want a MSG_FLOAT_ARRAY reply */
	return pan_net_want(s, MSG_FLOAT_ARRAY);
}
void
pan_net_get_surface_elevations_RX
(
	SOCKET s,
	float *resultv,
	char *errorv
)
{
	unsigned long i, nelts;

	/* Read the number of elements in the array */
	(void)pan_socket_read_ulong(s, &nelts);

	/* Read the results into the user-supplied buffer */
	for (i = 0; i < nelts; i++)
	{
		(void)pan_socket_read_float(s, resultv++);
		(void)pan_socket_read_bool(s, errorv++);
	}

}


/*
 * err = pan_net_get_surface_patch_TX(s, b, cx, cy, nx, ny, d, theta)
 * requests a DEM for a specified patch of the surface, centred at
 * (cx,cy) with nx samples horizontally and ny samples vertically,
 * with a distance d between sample points. The patch is rotated by
 * theta radians around (cx,cy) (anti-clockwise). This defines a
 * local coordinate system, with origin (cx,cy), whose axes are
 * rotated theta degrees to the world frame. If the  value of b is
 * non-zero then the elevation will include any boulder lying on the
 * surface at that point, if it is zero, then only the underlying
 * surface will be used.
 *
 * pan_net_get_surface_patch(s, rv, ev) receives the DEM. The DEM
 * is written to the array rv which must be of size (nx * ny). The
 * array is arranged by row, where each row is a line of increasing x
 * values (in the local frame). The rows are arranged in order of
 * increasing y value. rv[i] is a valid elevation value only if ev[i]
 * is non-zero.
 *
 * IMPLEMENTS GetSurfacePatch (25)
 */
char *
pan_net_get_surface_patch_TX
(
	SOCKET s,
	unsigned char boulders,
	float cx, float cy,
	unsigned long nx, unsigned long ny,
	float d,
	float theta
)
{
	/*
	 * Send the GetSurfacePatch message.
	 * We're sending three ulongs, one bool and four floats.
	 */
	const int bufsize = 3*4 + 1*1 + 4*4;
	static char buf[bufsize];
	int nbytes;

	/* Initialise the buffer. */
	char *p = buf;
	p = pan_socket_poke_ulong(p, MSG_GET_SURFACE_PATCH);
	p = pan_socket_poke_bool (p, boulders);
	p = pan_socket_poke_float(p, cx);
	p = pan_socket_poke_float(p, cy);
	p = pan_socket_poke_ulong(p, nx);
	p = pan_socket_poke_ulong(p, ny);
	p = pan_socket_poke_float(p, d);
	p = pan_socket_poke_float(p, theta);
	nbytes = p - buf;

	/* Write the buffer in one go. */
	assert(nbytes == bufsize);
	pan_socket_write(s, buf, nbytes);

	/* We want a MSG_FLOAT_ARRAY reply */
	return pan_net_want(s, MSG_FLOAT_ARRAY);
}
void
pan_net_get_surface_patch_RX
(
	SOCKET s,
	float *rv,
	char *ev
)
{
	unsigned long i, nelts;

	/* Read the number of elements in the array */
	pan_socket_read_ulong(s, &nelts);

	/* Read the results into the user-supplied buffer */
	for (i = 0; i < nelts; i++)
	{
		pan_socket_read_float(s, rv++);
		pan_socket_read_bool(s, ev++);
	}
}


/*
 * err = pan_net_get_viewpoint_by_radians_TX(s, x, y, z, yw, pi, rl) is
 * used to set the camera/viewpoint position to (x, y, z) and yaw/pitch/roll
 * to (yw, pi, rl) and request an image from that position. Angles are in
 * radians.
 *
 * ptr = pan_net_get_viewpoint_by_radians_RX(s, &size) is used to receive
 * the image. The memory returned by the call is allocated by malloc()
 * and may be released by free(). The size field will be updated with the
 * number of bytes in the result array.
 *
 * IMPLEMENTS GetViewpointByRadians (26)
 */
char *
pan_net_get_viewpoint_by_radians_TX(SOCKET s, double x, double y, double z, double yw, double pi, double rl)
{
	/*
	 * Send the GetViewpointByRadians message with parameters (x, y, z)
	 * and (yw, pi, rl). We're sending one ulong and six doubles.
	 */
	const int bufsize = 1*4 + 6*8;
	static char buf[bufsize];
	int nbytes;

	/* Initialise the buffer. */
	char *p = buf;
	p = pan_socket_poke_ulong (p, MSG_GET_VIEWPOINT_BY_RADIANS);
	p = pan_socket_poke_double(p, x);
	p = pan_socket_poke_double(p, y);
	p = pan_socket_poke_double(p, z);
	p = pan_socket_poke_double(p, yw);
	p = pan_socket_poke_double(p, pi);
	p = pan_socket_poke_double(p, rl);
	nbytes = p - buf;

	/* Write the buffer in one go. */
	assert(nbytes == bufsize);
	pan_socket_write(s, buf, nbytes);

	/* We want a MSG_IMAGE reply */
	return pan_net_want(s, MSG_IMAGE);
}
unsigned char *
pan_net_get_viewpoint_by_radians_RX(SOCKET s, unsigned long *psize)
{
	long fsize;
	unsigned char *result;

	/* Read the size of the data in the message */
	(void)pan_socket_read_long(s, &fsize);
	if (psize) *psize = fsize;

	/*
	 * Allocate a buffer large enough for the result. We add one
	 * in case the size is zero because we need a valid pointer.
	 */
	result = (unsigned char *)malloc(fsize + 1);

	/* Read the data directly into the result buffer */
	(void)pan_socket_read(s, (void *)result, fsize);
	return result;
}


/*
 * pan_net_quit_TX(s) causes the server program to quit. Note that this
 * is NOT the same as the GoodBye message sent by pan_net_finish_TX()
 * which closes a client/server connection.
 *
 * pan_net_quit_RX() is not required.
 *
 * IMPLEMENTS Quit (27)
 */
char *
pan_net_quit_TX(SOCKET s)
{
	/* Send the Quit message */
	pan_socket_write_ulong(s, MSG_QUIT);
	return pan_net_want(s, MSG_OKAY);
}


/*
 * err = pan_net_get_viewpoint_by_frame_TX(s, oid, fid) requests an
 * image from the frame fid within dynamic object oid.
 *
 * ptr = pan_net_get_viewpoint_by_frame_RX(s, &size) receives the image.
 * from the frame fid within dynamic object oid. Returns zero if
 * an image could not be obtained from the server. Returns an empty (0 byte)
 * image if the specified frame does not exist. The memory returned by the
 * call is allocated by malloc() and may be released by free(). The size
 * field will be updated with the number of bytes in the result array.
 *
 * IMPLEMENTS GetViewpointByFrame (28)
 */
char *
pan_net_get_viewpoint_by_frame_TX
(
	SOCKET s,
	unsigned long oid,
	unsigned long fid
)
{
	/*
	 * Send the GetViewpointByFrame message with parameters (oid,fid).
	 * We're sending three ulongs.
	 */
	const int bufsize = 1*4 + 2*4;
	static char buf[bufsize];
	int nbytes;

	/* Initialise the buffer. */
	char *p = buf;
	p = pan_socket_poke_ulong(p, MSG_GET_VIEWPOINT_BY_FRAME);
	p = pan_socket_poke_ulong(p, oid);
	p = pan_socket_poke_ulong(p, fid);
	nbytes = p - buf;

	/* Write the buffer in one go. */
	assert(nbytes == bufsize);
	pan_socket_write(s, buf, nbytes);

	/* Want a MSG_IMAGE reply */
	return pan_net_want(s, MSG_IMAGE);
}
unsigned char *
pan_net_get_viewpoint_by_frame_RX
(
	SOCKET s,
	unsigned long *psize
)
{
	long fsize;
	unsigned char *result;

	/* Read the size of the data in the message */
	(void)pan_socket_read_long(s, &fsize);
	if (psize) *psize = fsize;

	/*
	 * Allocate a buffer large enough for the result. We add one
	 * in case the size is zero because we need a valid pointer.
	 */
	result = (unsigned char *)malloc(fsize + 1);

	/* Read the data directly into the result buffer */
	(void)pan_socket_read(s, (void *)result, fsize);
	return result;
}


/*
 * err = pan_net_get_camera_properties_TX(s,c) is used to obtain the
 * properties of camera c.
 *
 * v = pan_net_get_camera_properties_RX(s,&w,&h,&hf,&vf,&px,&py,&pz,&q0,&q1,&q2,&3)
 * is used to receive the results. The variables w and h are updated
 * with the image dimensions, hf and vf with the horizontal and
 * vertical field of view angles (in radians), the (px,py,pz) variables with
 * the camera position and the (q0,q1,q2,q3) variables with the camera
 * attitude quaternion (q0 is the scalar term). The return value v is 0 if
 * the camera is invalid and positive if valid. A negative value indicates
 * that temporary storage allocation failed.
 *
 * IMPLEMENTS GetCameraProperties (29)
 */
char *
pan_net_get_camera_properties_TX
(
	SOCKET s,
	unsigned long cid
)
{
	int nbytes;

	/*
	 * Send a GetCameraProperties message.
	 * We're sending two ulongs.
	 */
	const int bufsize = 1*4 + 1*4;
	static char buf[bufsize];

	/* Initialise the buffer. */
	char *p = buf;
	p = pan_socket_poke_ulong(p, MSG_GET_CAMERA_PROPERTIES);
	p = pan_socket_poke_ulong(p, cid);
	nbytes = p - buf;

	/* Write the buffer in one go. */
	assert(nbytes == bufsize);
	pan_socket_write(s, buf, nbytes);

	/* We want a MSG_CAMERA_PROPERTIES reply */
	return pan_net_want(s, MSG_CAMERA_PROPERTIES);
}
int
pan_net_get_camera_properties_RX
(
	SOCKET s,
	unsigned long *pwidth,
	unsigned long *pheight,
	double *phfov,
	double *pvfov,
	double *px,
	double *py,
	double *pz,
	double *pq0,
	double *pq1,
	double *pq2,
	double *pq3
)
{
	unsigned long n;
	unsigned long want;
	unsigned long got;
	char ignored;
	char *reply;
	char *ptr;

	/* Read the number of bytes in the reply */
	pan_socket_read_ulong(s, &n);

	/* If the size is 0 then the camera ID was invalid */
	if (!n) return 0;

	/* Allocate a buffer of the size we want and reset to 0. */
	want = 2*4 + 2*8 + 3*8 + 4*8;
	if (!(reply = (char *)malloc(want))) return -1;
	(void)memset(reply, 0, want);

	/* Try to fill the reply buffer from the stream. */
	got = n > want ? want : n;
	(void)pan_socket_read(s, reply, got);

	/* Read any bytes we don't expect. */
	for (; got < n; ++got) (void)pan_socket_read_char(s, &ignored);

	/* Pointer into the reply buffer */
	ptr = reply;

	/* Read all the fields required */
	if (pwidth) pan_socket_peek_ulong(ptr, pwidth);
	ptr += 4;
	/**/
	if (pheight) pan_socket_peek_ulong(ptr, pheight);
	ptr += 4;
	/**/
	if (phfov) pan_socket_peek_double(ptr, phfov);
	ptr += 8;
	/**/
	if (pvfov) pan_socket_peek_double(ptr, pvfov);
	ptr += 8;
	/**/
	if (px) pan_socket_peek_double(ptr, px);
	ptr += 8;
	/**/
	if (py) pan_socket_peek_double(ptr, py);
	ptr += 8;
	/**/
	if (pz) pan_socket_peek_double(ptr, pz);
	ptr += 8;
	/**/
	if (pq0) pan_socket_peek_double(ptr, pq0);
	ptr += 8;
	/**/
	if (pq1) pan_socket_peek_double(ptr, pq1);
	ptr += 8;
	/**/
	if (pq2) pan_socket_peek_double(ptr, pq2);
	ptr += 8;
	/**/
	if (pq3) pan_socket_peek_double(ptr, pq3);
	ptr += 8;

	/* Safety check */
	assert((ptr - reply) == (long)want);

	/* Release the temporary buffer */
	(void)free(reply); reply = 0;

	/* Success */
	return 1;
}


/*
 * err = pan_net_get_viewpoint_by_camera_TX(s, cid) requests an image
 * from camera cid.
 *
 * ptr = pan_net_get_viewpoint_by_camera_RX(s, &size) receives the
 * image. Returns an empty (0 byte) image if the specified camera does
 * not exist. The memory returned by the call is allocated by malloc()
 * and may be released by free(). The size field will be updated with
 * the number of bytes in the result array.
 *
 * IMPLEMENTS GetViewpointByCamera (30)
 */
char *
pan_net_get_viewpoint_by_camera_TX
(
	SOCKET s,
	unsigned long cid
)
{
	/*
	 * Send the GetViewpointByCamera message with parameter cid.
	 * We're sending two ulongs.
	 */
	const int bufsize = 1*4 + 1*4;
	static char buf[bufsize];
	int nbytes;

	/* Initialise the buffer. */
	char *p = buf;
	p = pan_socket_poke_ulong(p, MSG_GET_VIEWPOINT_BY_CAMERA);
	p = pan_socket_poke_ulong(p, cid);
	nbytes = p - buf;

	/* Write the buffer in one go. */
	assert(nbytes == bufsize);
	pan_socket_write(s, buf, nbytes);

	/* Want a MSG_IMAGE reply */
	return pan_net_want(s, MSG_IMAGE);
}
unsigned char *
pan_net_get_viewpoint_by_camera_RX
(
	SOCKET s,
	unsigned long *psize
)
{
	long fsize;
	unsigned char *result;

	/* Read the size of the data in the message */
	(void)pan_socket_read_long(s, &fsize);
	if (psize) *psize = fsize;

	/*
	 * Allocate a buffer large enough for the result. We add one
	 * in case the size is zero because we need a valid pointer.
	 */
	result = (unsigned char *)malloc(fsize + 1);

	/* Read the data directly into the result buffer */
	(void)pan_socket_read(s, (void *)result, fsize);
	return result;
}


/*
 * err = pan_net_get_view_as_dem_TX(s, cid, b, nx, ny, dx, dy, rd)
 * requests a DEM based on the view from camera cid. cid must currently
 * be 1, for the remote camera. If b is non-zero then boulders will be
 * included in the height. The requested DEM will have nx samples
 * horizontally and ny samples vertically. Horizontally there will be
 * dx units between samples, and vertically there will be dy units
 * between samples. The heights of the DEM will be relative to the
 * relative distance, rd, from the camera.
 *
 * pan_net_get_view_as_dem_RX(s, rv, ev) receives the DEM. The DEM
 * is written to the array rv which must be of size (nx * ny). The
 * array is arranged by row, where each row is a line of increasing x
 * values (in the local frame). The rows are arranged in order of
 * decreasing y value. rv[i] is a valid elevation value only if ev[i]
 * is non-zero.
 *
 * IMPLEMENTS GetViewAsDEM (31)
 */
char *
pan_net_get_view_as_dem_TX
(
	SOCKET s,
	unsigned long cid,
	unsigned char boulders,
	unsigned long nx, unsigned long ny,
	float dx, float dy,
	float rd
)
{
	/*
	 * Send the GetViewAsDEM message.
	 * We're sending four ulongs, one bool and three floats.
	 */
	const int bufsize = 3*4 + 1*1 + 4*4;
	static char buf[bufsize];
	int nbytes;

	/* Initialise the buffer. */
	char *p = buf;
	p = pan_socket_poke_ulong(p, MSG_GET_VIEW_AS_DEM);
	p = pan_socket_poke_ulong(p, cid);
	p = pan_socket_poke_bool (p, boulders);
	p = pan_socket_poke_ulong(p, nx);
	p = pan_socket_poke_ulong(p, ny);
	p = pan_socket_poke_float(p, dx);
	p = pan_socket_poke_float(p, dy);
	p = pan_socket_poke_float(p, rd);
	nbytes = p - buf;

	/* Write the buffer in one go. */
	assert(nbytes == bufsize);
	pan_socket_write(s, buf, nbytes);

	/* We want a MSG_FLOAT_ARRAY reply */
	return pan_net_want(s, MSG_FLOAT_ARRAY);
}
void
pan_net_get_view_as_dem_RX
(
	SOCKET s,
	float *rv,
	char *ev
)
{
	unsigned long i, nelts;

	/* Read the number of elements in the array */
	pan_socket_read_ulong(s, &nelts);

	/* Read the results into the user-supplied buffer */
	for (i = 0; i < nelts; i++)
	{
		pan_socket_read_float(s, rv++);
		pan_socket_read_bool(s, ev++);
	}
}



/*
 * err = pan_net_get_lidar_measurement_d_TX(
 *        s, px,py,pz, q0,q1,q2,q3, vx,vy,vz, rx,ry,rx,
 *        ax,ay,az, sx,sy,sz, jx,jy,jz, tx,ty,tz) is
 * used to request a LIDAR scan from position (px,py,pz) with attitude
 * quaternion (q0,q1,q2,q3), linear velocity (vx,vy,vz) and angular velocity
 * (rx,ry,rz), linear acceleration (ax,ay,az), angular acceleration (sx,sy,sz),
 * linear jerk (jx,jy,jz) and angular jerk (tx,ty,tz).

 * v = pan_net_get_lidar_measurement_d_RX(
 *        s, &fx,&fy, &nx,&ny, &tx,&ty, &n,&m, &t, &fl, &az,&el, &th,
 *        &faz, &fel, &toff, &taz0, &tel0) is
 * used to receive the results. The return value v is a pointer to an
 * array of scan results while the LIDAR emitter/detector settings are
 * written to fx etc. If the scan has W by H beams with sub-sampling
 * factors N and M and B blocks of results then "v" will contain 
 * 2*B*(W*N)*(H*M) floating point values representing the 2D scan results at
 * the sub-sampling resolution NxM. The number of blocks B depends on the
 * results requested in the LIDAR emitter/detector settings flag field.
 * This function automatically converts from PANGU network floats to native
 * floats before returning.
 *
 * This function receives the float data in network order. As opposed to
 * GetLidarMeasurement (14) which receives it in host order. So this is the
 * correct function to use.
 *
 * IMPLEMENTS GetLidarMeasurementD (32)
 */
char *
pan_net_get_lidar_measurement_d_TX(SOCKET s,
	double px, double py, double pz,
	double q0, double q1, double q2, double q3,
	double vx, double vy, double vz,
	double rx, double ry, double rz,
	double ax, double ay, double az,
	double sx, double sy, double sz,
	double jx, double jy, double jz,
	double tx, double ty, double tz)
{
	/*
	 * Send the GetLidarMeasurement message.
	 * We're sending one ulong and 25 doubles.
	 */
	const int bufsize = 1*4 + 25*8;
	static char buf[bufsize];
	int nbytes;

	/* Initialise the buffer. */
	char *p = buf;
	p = pan_socket_poke_ulong(p, MSG_GET_LIDAR_MEASUREMENT_D);
	p = pan_socket_poke_double(p, px);
	p = pan_socket_poke_double(p, py);
	p = pan_socket_poke_double(p, pz);
	p = pan_socket_poke_double(p, q0);
	p = pan_socket_poke_double(p, q1);
	p = pan_socket_poke_double(p, q2);
	p = pan_socket_poke_double(p, q3);
	p = pan_socket_poke_double(p, vx);
	p = pan_socket_poke_double(p, vy);
	p = pan_socket_poke_double(p, vz);
	p = pan_socket_poke_double(p, rx);
	p = pan_socket_poke_double(p, ry);
	p = pan_socket_poke_double(p, rz);
	p = pan_socket_poke_double(p, ax);
	p = pan_socket_poke_double(p, ay);
	p = pan_socket_poke_double(p, az);
	p = pan_socket_poke_double(p, sx);
	p = pan_socket_poke_double(p, sy);
	p = pan_socket_poke_double(p, sz);
	p = pan_socket_poke_double(p, jx);
	p = pan_socket_poke_double(p, jy);
	p = pan_socket_poke_double(p, jz);
	p = pan_socket_poke_double(p, tx);
	p = pan_socket_poke_double(p, ty);
	p = pan_socket_poke_double(p, tz);
	nbytes = p - buf;

	/* Write the buffer in one go. */
	assert(nbytes == bufsize);
	pan_socket_write(s, buf, nbytes);

	/* We want a MSG_LIDAR_MEASUREMENT reply */
	return pan_net_want(s, MSG_LIDAR_MEASUREMENT);
}
float *
pan_net_get_lidar_measurement_d_RX(SOCKET s,
	float *pfx, float *pfy,
	unsigned long *pnx, unsigned long *pny,
	float *ptx, float *pty,
	unsigned long *pn, unsigned long *pm,
	unsigned long *pt, unsigned long *pfl,
	float *paz, float *pel, float *pth,
	float *pfaz, float *pfel,
	float *ptoff, float *ptaz0, float *ptel0)
{
	/* Return value */
	float *result, *dptr;
	char *sptr; /*unsigned long *sptr;*/

	/* Temporaries. */
	float f;
	unsigned long i, fsize, dsize, samplec, got;
	unsigned long nx, ny, n, m, t, fl;

	/* Count the number of LIDAR parameter fields we read */
	got = 0;

	/* Read and save the LIDAR emitter/detector parameters */
	(void)pan_socket_read_float(s, &f);  got++; if (pfx)  *pfx  = f;
	(void)pan_socket_read_float(s, &f);  got++; if (pfy)  *pfy  = f;
	(void)pan_socket_read_ulong(s, &nx); got++; if (pnx)  *pnx  = nx;
	(void)pan_socket_read_ulong(s, &ny); got++; if (pny)  *pny  = ny;
	(void)pan_socket_read_float(s, &f);  got++; if (ptx)  *ptx  = f;
	(void)pan_socket_read_float(s, &f);  got++; if (pty)  *pty  = f;
	(void)pan_socket_read_ulong(s, &n);  got++; if (pn)   *pn   = n;
	(void)pan_socket_read_ulong(s, &m);  got++; if (pm)   *pm   = m;
	(void)pan_socket_read_ulong(s, &t);  got++; if (pt)   *pt   = t;
	(void)pan_socket_read_ulong(s, &fl); got++; if (pfl)  *pfl  = fl;
	(void)pan_socket_read_float(s, &f);  got++; if (paz)  *paz  = f;
	(void)pan_socket_read_float(s, &f);  got++; if (pel)  *pel  = f;
	(void)pan_socket_read_float(s, &f);  got++; if (pth)  *pth  = f;
	(void)pan_socket_read_float(s, &f);  got++; /* wx */
	(void)pan_socket_read_float(s, &f);  got++; /* wy */
	(void)pan_socket_read_float(s, &f);  got++; if (pfaz) *pfaz = f;
	(void)pan_socket_read_float(s, &f);  got++; if (pfel) *pfel = f;
	(void)pan_socket_read_float(s, &f);  got++; if (ptoff) *ptoff = f;
	(void)pan_socket_read_float(s, &f);  got++; if (ptaz0) *ptaz0 = f;
	(void)pan_socket_read_float(s, &f);  got++; if (ptel0) *ptel0 = f;

	/* Read any padding words */
	for (; got < 32; got++) (void)pan_socket_read_ulong(s, &i);

	/* Read the number of bytes in the data portion of the packet */
	(void)pan_socket_read_ulong(s, &dsize);

	/* Compute the number of samples in the result arrays */
	samplec = 0;
	if (fl & (1UL<<0)) samplec += 2;
	if (fl & (1UL<<1)) samplec += 2;
	if (fl & (1UL<<2)) samplec += 2;
	if (fl & (1UL<<3)) samplec += 2;

	/* Allocate a buffer large enough for the result */
	fsize = samplec*(nx*n)*(ny*m)*sizeof(float);

	/*
	 * Allocate a buffer large enough for the result. We add one
	 * in case the size is zero because we need a valid pointer.
	 */
	result = (float *)malloc(fsize + 1);

	/* We expect fsize == dsize */
	if (dsize > fsize)
	{
		/* This is the number of padding/garbage bytes */
		unsigned long excess = dsize - fsize;
		unsigned char *trash = (unsigned char *)malloc(excess);

		/* Read what we expected and then the garbage */
		(void)pan_socket_read(s, (void *)result, fsize);
		(void)pan_socket_read(s, (void *)trash,  excess);

		/* Throw out the trash */
		(void)free(trash);
	}
	else
		(void)pan_socket_read(s, (void *)result, dsize);

	/* 
	 * Convert the PANGU floats into native floats in situ. 
	 * Note that peek internally converts from network to host format
	 */
	dptr = result;
	sptr = (char *)result;
	for (i = 0; i < fsize/sizeof(float); i++)
	{
		float tmp = 0;
		sptr = pan_socket_peek_float(sptr, &tmp);
		*dptr++ = tmp;
	}

	/* Return the array of floats */
	return result;
}


/*
 * err = pan_net_get_time_tag_TX(s) requests the time that the last
 * image was requested at.

 * t = pan_net_get_time_tag_RX(s, perr) receives the time, t, that the
 * last image was requested at. The time is measured in microseconds
 * since 00:00:00 UCT 01 January 1970. If perr is not a null pointer
 * then it will be set to zero if the returned timetag is invalid
 * and non-zero if it is valid.
 *
 * IMPLEMENTS GetTimeTag (33)
 */
char *
pan_net_get_time_tag_TX(SOCKET s)
{
	/* Send the empty GetTimeTag message and want a MSG_DOUBLE reply */
	(void)pan_socket_write_ulong(s, MSG_GET_TIME_TAG);
	return pan_net_want(s, MSG_DOUBLE);
}
double
pan_net_get_time_tag_RX(SOCKET s, char *perr)
{
	struct optional_double result;

	/* Read the value then the error flag */
	(void)pan_socket_read_double(s, &result.value);
	(void)pan_socket_read_bool(s, &result.valid);
	if (perr) *perr = result.valid ? (char)1 : (char)0;
	return result.value;
}


/*
 * err = pan_net_get_lidar_measurement_s_TX(
 *        s, px,py,pz, q0,q1,q2,q3, vx,vy,vz, rx,ry,rx,
 *        ax,ay,az, sx,sy,sz, jx,jy,jz, tx,ty,tz) is
 * used to request a LIDAR scan from position (px,py,pz) with attitude
 * quaternion (q0,q1,q2,q3), linear velocity (vx,vy,vz) and angular velocity
 * (rx,ry,rz), linear acceleration (ax,ay,az), angular acceleration (sx,sy,sz),
 * linear jerk (jx,jy,jz) and angular jerk (tx,ty,tz).

 * v = pan_net_get_lidar_measurement_s_RX(
 *        s, &fx,&fy, &nx,&ny, &tx,&ty, &n,&m, &t, &fl, &az,&el, &th,
 *        &faz, &fel, &toff, &taz0, &tel0) is
 * used to receive the results. The return value v is a pointer to an
 * array of scan results while the LIDAR emitter/detector settings are
 * written to fx etc. If the scan has W by H beams with sub-sampling
 * factors N and M and B blocks of results then "v" will contain 
 * 2*B*(W*N)*(H*M) floating point values representing the 2D scan results at
 * the sub-sampling resolution NxM. The number of blocks B depends on the
 * results requested in the LIDAR emitter/detector settings flag field.
 * This function automatically converts from PANGU network floats to native
 * floats before returning.
 *
 * This function receives the float data in network order. As opposed to
 * GetLidarMeasurement (14) which receives it in host order. So this is the
 * correct function to use.
 *
 * IMPLEMENTS GetLidarMeasurementS (34)
 */
char *
pan_net_get_lidar_measurement_s_TX(SOCKET s,
	float px, float py, float pz,
	float q0, float q1, float q2, float q3,
	float vx, float vy, float vz,
	float rx, float ry, float rz,
	float ax, float ay, float az,
	float sx, float sy, float sz,
	float jx, float jy, float jz,
	float tx, float ty, float tz)
{
	/*
	 * Send the GetLidarMeasurement message.
	 * We're sending one ulong and 25 floats.
	 */
	const int bufsize = 1*4 + 25*4;
	static char buf[bufsize];
	int nbytes;

	/* Initialise the buffer. */
	char *p = buf;
	p = pan_socket_poke_ulong(p, MSG_GET_LIDAR_MEASUREMENT_S);
	p = pan_socket_poke_float(p, px);
	p = pan_socket_poke_float(p, py);
	p = pan_socket_poke_float(p, pz);
	p = pan_socket_poke_float(p, q0);
	p = pan_socket_poke_float(p, q1);
	p = pan_socket_poke_float(p, q2);
	p = pan_socket_poke_float(p, q3);
	p = pan_socket_poke_float(p, vx);
	p = pan_socket_poke_float(p, vy);
	p = pan_socket_poke_float(p, vz);
	p = pan_socket_poke_float(p, rx);
	p = pan_socket_poke_float(p, ry);
	p = pan_socket_poke_float(p, rz);
	p = pan_socket_poke_float(p, ax);
	p = pan_socket_poke_float(p, ay);
	p = pan_socket_poke_float(p, az);
	p = pan_socket_poke_float(p, sx);
	p = pan_socket_poke_float(p, sy);
	p = pan_socket_poke_float(p, sz);
	p = pan_socket_poke_float(p, jx);
	p = pan_socket_poke_float(p, jy);
	p = pan_socket_poke_float(p, jz);
	p = pan_socket_poke_float(p, tx);
	p = pan_socket_poke_float(p, ty);
	p = pan_socket_poke_float(p, tz);
	nbytes = p - buf;

	/* Write the buffer in one go. */
	assert(nbytes == bufsize);
	pan_socket_write(s, buf, nbytes);

	/* We want a MSG_LIDAR_MEASUREMENT reply */
	return pan_net_want(s, MSG_LIDAR_MEASUREMENT);
}
float *
pan_net_get_lidar_measurement_s_RX(SOCKET s,
	float *pfx, float *pfy,
	unsigned long *pnx, unsigned long *pny,
	float *ptx, float *pty,
	unsigned long *pn, unsigned long *pm,
	unsigned long *pt, unsigned long *pfl,
	float *paz, float *pel, float *pth,
	float *pfaz, float *pfel,
	float *ptoff, float *ptaz0, float *ptel0)
{
	/* Return value */
	float *result, *dptr;
	char *sptr; /*unsigned long *sptr;*/

	/* Temporaries. */
	float f;
	unsigned long i, fsize, dsize, samplec, got;
	unsigned long nx, ny, n, m, t, fl;

	/* Count the number of LIDAR parameter fields we read */
	got = 0;

	/* Read and save the LIDAR emitter/detector parameters */
	(void)pan_socket_read_float(s, &f);  got++; if (pfx)  *pfx  = f;
	(void)pan_socket_read_float(s, &f);  got++; if (pfy)  *pfy  = f;
	(void)pan_socket_read_ulong(s, &nx); got++; if (pnx)  *pnx  = nx;
	(void)pan_socket_read_ulong(s, &ny); got++; if (pny)  *pny  = ny;
	(void)pan_socket_read_float(s, &f);  got++; if (ptx)  *ptx  = f;
	(void)pan_socket_read_float(s, &f);  got++; if (pty)  *pty  = f;
	(void)pan_socket_read_ulong(s, &n);  got++; if (pn)   *pn   = n;
	(void)pan_socket_read_ulong(s, &m);  got++; if (pm)   *pm   = m;
	(void)pan_socket_read_ulong(s, &t);  got++; if (pt)   *pt   = t;
	(void)pan_socket_read_ulong(s, &fl); got++; if (pfl)  *pfl  = fl;
	(void)pan_socket_read_float(s, &f);  got++; if (paz)  *paz  = f;
	(void)pan_socket_read_float(s, &f);  got++; if (pel)  *pel  = f;
	(void)pan_socket_read_float(s, &f);  got++; if (pth)  *pth  = f;
	(void)pan_socket_read_float(s, &f);  got++; /* wx */
	(void)pan_socket_read_float(s, &f);  got++; /* wy */
	(void)pan_socket_read_float(s, &f);  got++; if (pfaz) *pfaz = f;
	(void)pan_socket_read_float(s, &f);  got++; if (pfel) *pfel = f;
	(void)pan_socket_read_float(s, &f);  got++; if (ptoff) *ptoff = f;
	(void)pan_socket_read_float(s, &f);  got++; if (ptaz0) *ptaz0 = f;
	(void)pan_socket_read_float(s, &f);  got++; if (ptel0) *ptel0 = f;

	/* Read any padding words */
	for (; got < 32; got++) (void)pan_socket_read_ulong(s, &i);

	/* Read the number of bytes in the data portion of the packet */
	(void)pan_socket_read_ulong(s, &dsize);

	/* Compute the number of samples in the result arrays */
	samplec = 0;
	if (fl & (1UL<<0)) samplec += 2;
	if (fl & (1UL<<1)) samplec += 2;
	if (fl & (1UL<<2)) samplec += 2;
	if (fl & (1UL<<3)) samplec += 2;

	/* Allocate a buffer large enough for the result */
	fsize = samplec*(nx*n)*(ny*m)*sizeof(float);

	/*
	 * Allocate a buffer large enough for the result. We add one
	 * in case the size is zero because we need a valid pointer.
	 */
	result = (float *)malloc(fsize + 1);

	/* We expect fsize == dsize */
	if (dsize > fsize)
	{
		/* This is the number of padding/garbage bytes */
		unsigned long excess = dsize - fsize;
		unsigned char *trash = (unsigned char *)malloc(excess);

		/* Read what we expected and then the garbage */
		(void)pan_socket_read(s, (void *)result, fsize);
		(void)pan_socket_read(s, (void *)trash,  excess);

		/* Throw out the trash */
		(void)free(trash);
	}
	else
		(void)pan_socket_read(s, (void *)result, dsize);

	/* 
	 * Convert the PANGU floats into native floats in situ. 
	 * Note that peek internally converts from network to host format
	 */
	dptr = result;
	sptr = (char *)result;
	for (i = 0; i < fsize/sizeof(float); i++)
	{
		float tmp = 0;
		sptr = pan_socket_peek_float(sptr, &tmp);
		*dptr++ = tmp;
	}

	/* Return the array of float pairs */
	return result;
}


/*
 * err = pan_net_get_lidar_snapshot_TX(
 *        s, cid, px,py,pz, q0,q1,q2,q3) is
 * is used to request a LIDAR scan from position (px,py,pz) with
 * attitude quaternion (q0,q1,q2,q3).
 *
 * v = pan_net_get_lidar_snapshot_RX(s,&w,&h) returns a pointer to an 
 * array of floats representing a raw image with top left origin. w and
 * h are unsigned longs providing the width and height of the image.
 * Each pixel of the image has three floats: the first is the range
 * (i.e. the distance between the surface and the lidar scanner); the
 * second is the cosine of the angle between the surface normal and scan
 * direction of that pixel; and the third is a hit/miss flag where 0
 * represents a miss and 1 represents a hit. So v points to w*h*3 floats.
 * This function automatically converts from MSB_REAL_32 floats to
 * native floats before returning.
 *
 * IMPLEMENTS GetLidarSnapshot (35)
 */
char *
pan_net_get_lidar_snapshot_TX(SOCKET s, unsigned long cid,
		double px, double py, double pz,
		double q0, double q1, double q2, double q3)
{
	/*
	 * Send the GetLidarSnapshot message.
	 * We're sending 2 ulongs and 7 doubles.
	 */
	const int bufsize = 2*4 + 7*8;
	static char buf[bufsize];
	int nbytes;

	/* Initialise the buffer. */
	char *p = buf;
	p = pan_socket_poke_ulong(p, MSG_GET_LIDAR_SNAPSHOT);
	p = pan_socket_poke_ulong(p, cid);
	p = pan_socket_poke_double(p, px);
	p = pan_socket_poke_double(p, py);
	p = pan_socket_poke_double(p, pz);
	p = pan_socket_poke_double(p, q0);
	p = pan_socket_poke_double(p, q1);
	p = pan_socket_poke_double(p, q2);
	p = pan_socket_poke_double(p, q3);
	nbytes = p - buf;

	/* Write the buffer in one go. */
	assert(nbytes == bufsize);
	pan_socket_write(s, buf, nbytes);

	/* We want a MSG_RAW_IMAGE reply */
	return pan_net_want(s, MSG_RAW_IMAGE);
}
float *
pan_net_get_lidar_snapshot_RX(SOCKET s,
		unsigned long * width, unsigned long * height)
{
	/* Return value */
	float *result;
	uint32_t *dptr;

	/* Temporaries. */
	unsigned long i, fsize, dsize;
	unsigned long t, r, w, h;

	/* Read and save the raw image parameters */
	(void)pan_socket_read_ulong(s, &t);  /* Image type must be 0 */
	(void)pan_socket_read_ulong(s, &r);  /* Reserved value */
	(void)pan_socket_read_ulong(s, &w); if (width) *width = w;
	(void)pan_socket_read_ulong(s, &h); if (height) *height = h;

	/* Read the number of bytes in the data portion of the packet */
	(void)pan_socket_read_ulong(s, &dsize);

	/* Allocate a buffer large enough for the result */
	fsize = w*h*3*sizeof(float);

	/*
	 * Allocate a buffer large enough for the result. We add one
	 * in case the size is zero because we need a valid pointer.
	 */
	result = (float *)malloc(fsize + 1);

	/* We expect fsize == dsize */
	if (dsize > fsize)
	{
		/* This is the number of padding/garbage bytes */
		unsigned long excess = dsize - fsize;
		unsigned char *trash = (unsigned char *)malloc(excess);

		/* Read what we expected and then the garbage */
		(void)pan_socket_read(s, (void *)result, fsize);
		(void)pan_socket_read(s, (void *)trash,  excess);

		/* Throw out the trash */
		(void)free(trash);
	}
	else
		(void)pan_socket_read(s, (void *)result, dsize);

	/* 
	 * The PANGU server encodes the floating point values in MSB_REAL_32
	 * format which happens to be network byte order. So a simple way to
	 * convert floats to the native format is to use ntohl on 32-bit words. 
	 */
	dptr = (uint32_t*)result;
	for (i = 0; i < fsize/sizeof(float); i++)
	{
		uint32_t tmp = *dptr;
		*dptr++ = ntohl(tmp);
	}

	/* Return the array of floats */
	return result;
}


/*
 * pan_net_set_viewpoint_by_degrees_s_TX(s, x, y, z, yw, pi, rl) is used to
 * set the camera/viewpoint position to (x, y, z) and attitude yaw/pitch/roll
 * to (yw, pi, rl) degrees.
 *
 * pan_net_set_viewpoint_by_degrees_s_RX() is not required.
 *
 * IMPLEMENTS SetViewpointByDegreesS (256)
 */
char *
pan_net_set_viewpoint_by_degrees_s_TX(SOCKET s, float x, float y, float z, float yw, float pi, float rl)
{
	/*
	 * Send the SetViewpointByDegrees message with parameters (x, y, z)
	 * and (yw, pi, rl). We're sending one ulong and six floats.
	 */
	const int bufsize = 1*4 + 6*4;
	static char buf[bufsize];
	int nbytes;

	/* Initialise the buffer. */
	char *p = buf;
	p = pan_socket_poke_ulong (p, MSG_SET_VIEWPOINT_BY_DEGREES_S);
	p = pan_socket_poke_float(p, x);
	p = pan_socket_poke_float(p, y);
	p = pan_socket_poke_float(p, z);
	p = pan_socket_poke_float(p, yw);
	p = pan_socket_poke_float(p, pi);
	p = pan_socket_poke_float(p, rl);
	nbytes = p - buf;

	/* Write the buffer in one go. */
	assert(nbytes == bufsize);
	pan_socket_write(s, buf, nbytes);

	/* We want a MSG_OKAY reply */
	return pan_net_want(s, MSG_OKAY);
}


/*
 * pan_net_set_viewpoint_by_quaternion_s_TX(s, x, y, z, q0, q1, q2, q3) is
 * used to set the camera/viewpoint position to (x, y, z) and attitude as
 * defined by the quaternion [q0, q1, q2, q3].
 *
 * pan_net_set_viewpoint_by_quaternion_s_RX() is not required.
 *
 * IMPLEMENTS SetViewpointByQuaternionS (257)
 */
char *
pan_net_set_viewpoint_by_quaternion_s_TX(SOCKET s, float x, float y, float z, float q0, float q1, float q2, float q3)
{
	/*
	 * Send the SetViewpointByQuaternion message with parameters (x, y, z)
	 * and (q0, q1, q2, q3). We're sending one ulong and seven floats.
	 */
	const int bufsize = 1*4 + 7*4;
	static char buf[bufsize];
	int nbytes;

	/* Initialise the buffer. */
	char *p = buf;
	p = pan_socket_poke_ulong (p, MSG_SET_VIEWPOINT_BY_QUATERNION_S);
	p = pan_socket_poke_float(p, x);
	p = pan_socket_poke_float(p, y);
	p = pan_socket_poke_float(p, z);
	p = pan_socket_poke_float(p, q0);
	p = pan_socket_poke_float(p, q1);
	p = pan_socket_poke_float(p, q2);
	p = pan_socket_poke_float(p, q3);
	nbytes = p - buf;

	/* Write the buffer in one go. */
	assert(nbytes == bufsize);
	pan_socket_write(s, buf, nbytes);

	/* We want a MSG_OKAY reply */
	return pan_net_want(s, MSG_OKAY);
}


/*
 * pan_net_set_ambient_light_TX(s, r, g, b) is used to set the colour and
 * intensity of ambient light in the red, green and blue channels to (r, g, b).
 *
 * pan_net_set_ambient_light_RX() is not required.
 *
 * IMPLEMENTS SetAmbientLight (258)
 */
char *
pan_net_set_ambient_light_TX(SOCKET s, float r, float g, float b)
{
	/*
	 * Send the SetAmbientLight message with parameters (r, g, b).
	 * We're sending one ulong and three floats.
	 */
	const int bufsize = 1*4 + 3*4;
	static char buf[bufsize];
	int nbytes;

	/* Initialise the buffer. */
	char *p = buf;
	p = pan_socket_poke_ulong (p, MSG_SET_AMBIENT_LIGHT);
	p = pan_socket_poke_float(p, r);
	p = pan_socket_poke_float(p, g);
	p = pan_socket_poke_float(p, b);
	nbytes = p - buf;

	/* Write the buffer in one go. */
	assert(nbytes == bufsize);
	pan_socket_write(s, buf, nbytes);

	/* We want a MSG_OKAY reply */
	return pan_net_want(s, MSG_OKAY);
}


/*
 * pan_net_set_sun_colour_TX(s, r, g, b) is used to set the colour and
 * intensity of the Sun in the red, green and blue channels to (r, g, b).
 *
 * pan_net_set_sun_colour_RX() is not required.
 *
 * IMPLEMENTS SetSunColour (259)
 */
char *
pan_net_set_sun_colour_TX(SOCKET s, float r, float g, float b)
{
	/*
	 * Send the SetSunColour message with parameters (r, g, b).
	 * We're sending one ulong and three floats.
	 */
	const int bufsize = 1*4 + 3*4;
	static char buf[bufsize];
	int nbytes;

	/* Initialise the buffer. */
	char *p = buf;
	p = pan_socket_poke_ulong (p, MSG_SET_SUN_COLOUR);
	p = pan_socket_poke_float(p, r);
	p = pan_socket_poke_float(p, g);
	p = pan_socket_poke_float(p, b);
	nbytes = p - buf;

	/* Write the buffer in one go. */
	assert(nbytes == bufsize);
	pan_socket_write(s, buf, nbytes);

	/* We want a MSG_OKAY reply */
	return pan_net_want(s, MSG_OKAY);
}


/*
 * pan_net_set_sky_type_TX(s, t) is used to set the sky background type to
 * "t" where "t" may hold one of the following values:
 *    0 : no sky (black)
 *    1 : fake sky (sphere-mapped texture ignoring camera roll)
 *    2 : raw sky (single-pixel stars of varying intensity)
 *    3 : red sky (maximum red, no green, no blue)
 *    4 : green sky (no red, maximum green, no blue)
 *    5 : blue sky (no red, no green, maximum blue)
 *    6 : white sky (maximum red, green, blue)
 *    7 : PSF stars (stars of varying intensity modulated by a PSF)
 *    8 : RGB sky (user defined red, green, blue values)
 *    9 : CIE sky (user defined CIE xyY values converted into RGB)
 *
 * pan_net_set_sky_type_RX() is not required.
 *
 * IMPLEMENTS SetSkyType (260)
 */
char *
pan_net_set_sky_type_TX(SOCKET s, unsigned long t)
{
	/*
	 * Send the SetSkyType message with parameter t.
	 * We're sending two ulongs.
	 */
	const int bufsize = 2*4;
	static char buf[bufsize];
	int nbytes;

	/* Initialise the buffer. */
	char *p = buf;
	p = pan_socket_poke_ulong (p, MSG_SET_SKY_TYPE);
	p = pan_socket_poke_ulong(p, t);
	nbytes = p - buf;

	/* Write the buffer in one go. */
	assert(nbytes == bufsize);
	pan_socket_write(s, buf, nbytes);

	/* We want a MSG_OKAY reply */
	return pan_net_want(s, MSG_OKAY);
}


/*
 * pan_net_set_field_of_view_by_degrees_TX(s, f) is used to set the angular
 * field of view width to "f" degrees. The angular field of view height is
 * determined from the width and the pixel aspect ratio.
 *
 * pan_net_set_field_of_view_by_degrees_RX() is not required.
 *
 * IMPLEMENTS SetFieldOfViewByDegrees (261)
 */
char *
pan_net_set_field_of_view_by_degrees_TX(SOCKET s, float f)
{
	/*
	 * Send the SetFieldOfViewByDegrees message with parameter f.
	 * We're sending one ulong and one float.
	 */
	const int bufsize = 1*4 + 1*4;
	static char buf[bufsize];
	int nbytes;

	/* Initialise the buffer. */
	char *p = buf;
	p = pan_socket_poke_ulong (p, MSG_SET_FIELD_OF_VIEW_BY_DEGREES);
	p = pan_socket_poke_float(p, f);
	nbytes = p - buf;

	/* Write the buffer in one go. */
	assert(nbytes == bufsize);
	pan_socket_write(s, buf, nbytes);

	/* We want a MSG_OKAY reply */
	return pan_net_want(s, MSG_OKAY);
}


/*
 * pan_net_set_aspect_ratio_TX(s, r) is used to set the angular aspect
 * ratio of each pixel. This is used in conjunction with the angular field
 * of view width to determine the angular field of view height.
 *
 * pan_net_set_aspect_ratio_RX() is not required.
 *
 * IMPLEMENTS SetAspectRatio (262)
 */
char *
pan_net_set_aspect_ratio_TX(SOCKET s, float r)
{
	/*
	 * Send the SetAspectRatio message with parameter r.
	 * We're sending one ulong and one float.
	 */
	const int bufsize = 1*4 + 1*4;
	static char buf[bufsize];
	int nbytes;

	/* Initialise the buffer. */
	char *p = buf;
	p = pan_socket_poke_ulong (p, MSG_SET_ASPECT_RATIO);
	p = pan_socket_poke_float(p, r);
	nbytes = p - buf;

	/* Write the buffer in one go. */
	assert(nbytes == bufsize);
	pan_socket_write(s, buf, nbytes);

	/* We want a MSG_OKAY reply */
	return pan_net_want(s, MSG_OKAY);
}


/*
 * pan_net_set_boulder_view_TX(s, t, tex) is used to set the boulder
 * rendering method to "t" and to enable or disable boulder texturing via
 * "tex".
 *
 * pan_net_set_boulder_view_RX() is not required.
 *
 * IMPLEMENTS SetBoulderView (263)
 */
char *
pan_net_set_boulder_view_TX(SOCKET s, unsigned long type, int texture)
{
	/*
	 * Send the SetBoulderView message with parameters (type, texture).
	 * We're sending two ulongs and one bool.
	 */
	const int bufsize = 2*4 + 1*1;
	static char buf[bufsize];
	int nbytes;

	/* Initialise the buffer. */
	char *p = buf;
	p = pan_socket_poke_ulong (p, MSG_SET_BOULDER_VIEW);
	p = pan_socket_poke_ulong (p, type);
	p = pan_socket_poke_bool(p, (char)texture);
	nbytes = p - buf;

	/* Write the buffer in one go. */
	assert(nbytes == bufsize);
	pan_socket_write(s, buf, nbytes);

	/* We want a MSG_OKAY reply */
	return pan_net_want(s, MSG_OKAY);
}


/*
 * pan_net_set_surface_view_TX(s, t, tex, det) is used to set the surface
 * rendering method to "t" and to enable or disable surface texturing and
 * surface detailing.
 *
 * pan_net_set_surface_view_RX() is not required.
 *
 * IMPLEMENTS SetSurfaceView (264)
 */
char *
pan_net_set_surface_view_TX(SOCKET s, unsigned long type, int tex, int det)
{
	/*
	 * Send the SetSurfaceView message with parameters (type, tex, det).
	 * We're sending two ulongs and two bools.
	 */
	const int bufsize = 2*4 + 2*1;
	static char buf[bufsize];
	int nbytes;

	/* Initialise the buffer. */
	char *p = buf;
	p = pan_socket_poke_ulong (p, MSG_SET_SURFACE_VIEW);
	p = pan_socket_poke_ulong (p, type);
	p = pan_socket_poke_bool(p, (char)tex);
	p = pan_socket_poke_bool(p, (char)det);
	nbytes = p - buf;

	/* Write the buffer in one go. */
	assert(nbytes == bufsize);
	pan_socket_write(s, buf, nbytes);

	/* We want a MSG_OKAY reply */
	return pan_net_want(s, MSG_OKAY);
}


/*
 * pan_net_set_lidar_parameters_TX(s, fx,fy, nx,ny, tx,ty, n, m, t, fl, az,el,
 *    th, wx, wy, faz, fel, toff, taz0, tel0) is used to configure the LIDAR
 *    emitter/detector with field of view (fx, fy) degrees horizontally and
 *    vertically, an emitter or detector grid of nx by ny beams, and subsampling
 *    of n by m samples. The scan type is "t" where:
 *      0: TV scan (left-to-right, top-to-bottom)
 *      1: mode 01 (LiGNC project zig-zag scan, azi mirror before ele)
 *      2: mode 02 (LiGNC project zig-zag scan, ele mirror before azi)
 *      3: mode 01 (ILT project 1D sinusoidal scan, azi mirror before ele)
 *      4: mode 02 (ILT project 1D sinusoidal scan, ele mirror before azi)
 *      5: mode 01 (LAPS project zig-zag scan, azi mirror before ele)
 *      6: mode 02 (LAPS project zig-zag scan, ele mirror before azi)
 *      7: General scan (Uses samples given via pan_protocol_set_lidar_scan())
 * The centre of the scan has LIDAR azimuth/elevation (az,el) and the
 * beam half-angle is th degrees. The LIDAR flags fl is a bit vector:
 *      bit  0: return range/slope results if set
 *      bit  1: return azimuth/elevation values if set
 *      bit  2: return corner cube range/slope results if set
 *      bit  3: return time-of-pulse-emission values
 *      bit 16: if set the azimuth scan starts with decreasing azimuth
 *      bit 17: if set the elevation scan starts with decreasing elevation
 *              and the results are returned in that order (bottom to top)
 *      bit 18: if set (normally used instead of bit 17) the results are
 *              returned in top-to-bottom order but the pulses are emittted
 *              in bottom-to-top order
 * All other bits must be clear.
 * Each detector pixel is wx by wy degrees in size. Note that when
 * n and/or m are greater than 1 these are the sizes of the subsample
 * pixels not the full pixel.
 * For the ILT scans (type 3 and 4) the faz parameter defines the azimuth
 *    sinusoial scanning frequency.
 * For the LAPS scans (type 5 and 6) the faz parameter defines the x-axis
 *    zig-zag scanning frequency and the fel parameter defines the y-axis
 *    zig-zag scanning frequency. The (taz0, tel0) parameters define the
 *    offset of the zig-zag scanning patterns.
 *
 * pan_net_set_lidar_parameters_RX() is not required.
 *
 * IMPLEMENTS SetLidarParameters (265)
 */
char *
pan_net_set_lidar_parameters_TX(SOCKET s,
	float fx, float fy,
	unsigned long nx, unsigned long ny,
	float tx, float ty,
	unsigned long n, unsigned long m,
	unsigned long t, unsigned long fl,
	float az, float el, float th,
	float wx, float wy,
	float faz, float fel,
	float toff, float taz0, float tel0)
{
	int wrote = 0;
	unsigned long i, zero = 0;

	/*
	 * Send the SetLidarParameters message.
	 * We're sending one ulong and 32 words (ulongs and floats).
	 */
	const int bufsize = 1*4 + 32*4;
	static char buf[bufsize];
	int nbytes;

	/* Initialise the buffer. */
	char *p = buf;
	p = pan_socket_poke_ulong(p, MSG_SET_LIDAR_PARAMETERS);
	p = pan_socket_poke_float(p, fx); wrote++;
	p = pan_socket_poke_float(p, fy); wrote++;
	p = pan_socket_poke_ulong(p, nx); wrote++;
	p = pan_socket_poke_ulong(p, ny); wrote++;
	p = pan_socket_poke_float(p, tx); wrote++;
	p = pan_socket_poke_float(p, ty); wrote++;
	p = pan_socket_poke_ulong(p, n); wrote++;
	p = pan_socket_poke_ulong(p, m); wrote++;
	p = pan_socket_poke_ulong(p, t); wrote++;
	p = pan_socket_poke_ulong(p, fl); wrote++;
	p = pan_socket_poke_float(p, az); wrote++;
	p = pan_socket_poke_float(p, el); wrote++;
	p = pan_socket_poke_float(p, th); wrote++;
	p = pan_socket_poke_float(p, wx); wrote++;
	p = pan_socket_poke_float(p, wy); wrote++;
	p = pan_socket_poke_float(p, faz); wrote++;
	p = pan_socket_poke_float(p, fel); wrote++;
	p = pan_socket_poke_float(p, toff); wrote++;
	p = pan_socket_poke_float(p, taz0); wrote++;
	p = pan_socket_poke_float(p, tel0); wrote++;

	/* Padding words */
	for (i = wrote; i < 32; i++) p = pan_socket_poke_ulong(p, zero);
	nbytes = p - buf;

	/* Write the buffer in one go. */
	assert(nbytes == bufsize);
	pan_socket_write(s, buf, nbytes);

	/* We want a MSG_OKAY reply */
	return pan_net_want(s, MSG_OKAY);
}


/*
 * pan_net_set_corner_cubes_s_TX(s, n, f, p) is used to pass an
 * array of "n" corner cube records starting at address "p" using the
 * array format "f". Currently "f" must be set to zero and "p" must
 * consist of "n" consecutive blocks of corner cube records. Each
 * block contains seven floating point values (px,py,pz,nx,ny,nz,r)
 * where (px,py,pz) is the position of the cube, (nx,ny,nz) its face
 * normal and r its effective radius.
 *
 * Note that corner cubes are only used when the LIDAR parameters
 * flags field has bit 2 set.
 *
 * pan_net_set_corner_cubes_s_RX() is not required.
 *
 * IMPLEMENTS SetCornerCubesS (266)
 */
char *
pan_net_set_corner_cubes_s_TX
(
	SOCKET s,
	unsigned long n,
	unsigned long fmt,
	float *pcc
)
{
	unsigned long i, siz;

	/* In this implementation we require fmt==0 */
	assert(!fmt);

	/* Compute the size of the data section */
	siz = 7*n*sizeof(float);

	/*
	 * Send the SetCornerCubes message.
	 * We're sending four ulongs and 7*n floats. Since the buffer size
	 * is variable we have to allocate it on the heap.
	 */
	const int bufsize = 4*4 + 7*n*4;
	char *buf = (char *)malloc(bufsize); assert(buf);
	int nbytes;

	/* Initialise the buffer. */
	char *p = buf;
	p = pan_socket_poke_ulong(p, MSG_SET_CORNER_CUBES_S);
	p = pan_socket_poke_ulong(p, n);
	p = pan_socket_poke_ulong(p, fmt);
	p = pan_socket_poke_ulong(p, siz);

	/* Send each corner cube record: seven floats */
	for (i = 0; i < 7*n; i++) p = pan_socket_poke_float(p, *pcc++);
	nbytes = p - buf;

	/* Write the buffer in one go. */
	assert(nbytes == bufsize);
	pan_socket_write(s, buf, nbytes);
	(void)free(buf); buf = 0;

	/* Want an Okay response */
	return pan_net_want(s, MSG_OKAY);
}


/*
 * pan_net_set_corner_cube_attitude_TX(
 *        s, q0,q1,q2,q3, rx,ry,rx, ax,ay,az, jx,jy,jz) is used
 * to define the attitude (q0,q1,q2,q3), angular velocity (rx,ry,rz),
 * angular acceleration (ax,ay,az) and angular jerk (jx,jy,jz) of the
 * corner cube lattice at the centre of all future LIDAR frames.
 *
 * pan_net_set_corner_cube_attitude_RX() is not required.
 *
 * IMPLEMENTS SetCornerCubeAttitude (267)
 */
char *
pan_net_set_corner_cube_attitude_TX
(
	SOCKET s,
	float q0, float q1, float q2, float q3,
	float rx, float ry, float rz,
	float ax, float ay, float az,
	float jx, float jy, float jz
)
{
	/*
	 * Send the SetCornerCubeAttitude message.
	 * We're sending one ulong and thirteen floats.
	 */
	const int bufsize = 1*4 + 13*4;
	static char buf[bufsize];
	int nbytes;

	/* Initialise the buffer. */
	char *p = buf;
	p = pan_socket_poke_ulong(p, MSG_SET_CORNER_CUBE_ATTITUDE);
	p = pan_socket_poke_float(p, q0);
	p = pan_socket_poke_float(p, q1);
	p = pan_socket_poke_float(p, q2);
	p = pan_socket_poke_float(p, q3);
	p = pan_socket_poke_float(p, rx);
	p = pan_socket_poke_float(p, ry);
	p = pan_socket_poke_float(p, rz);
	p = pan_socket_poke_float(p, ax);
	p = pan_socket_poke_float(p, ay);
	p = pan_socket_poke_float(p, az);
	p = pan_socket_poke_float(p, jx);
	p = pan_socket_poke_float(p, jy);
	p = pan_socket_poke_float(p, jz);
	nbytes = p - buf;

	/* Write the buffer in one go. */
	assert(nbytes == bufsize);
	pan_socket_write(s, buf, nbytes);

	/* We want a MSG_OKAY reply */
	return pan_net_want(s, MSG_OKAY);
}


/*
 * pan_net_set_viewpoint_by_degrees_d_TX(s, x, y, z, yw, pi, rl) is used to
 * set the camera/viewpoint position to (x, y, z) and attitude yaw/pitch/roll
 * to (yw, pi, rl) degrees.
 *
 * pan_net_set_viewpoint_by_degrees_d_RX() is not required.
 *
 * IMPLEMENTS SetViewpointByDegreesD (268)
 */
char *
pan_net_set_viewpoint_by_degrees_d_TX(SOCKET s, double x, double y, double z, double yw, double pi, double rl)
{
	/*
	 * Send the SetViewpointByDegrees message with parameters (x, y, z)
	 * and (yw, pi, rl). We're sending one ulong and six doubles.
	 */
	const int bufsize = 1*4 + 6*8;
	static char buf[bufsize];
	int nbytes;

	/* Initialise the buffer. */
	char *p = buf;
	p = pan_socket_poke_ulong (p, MSG_SET_VIEWPOINT_BY_DEGREES_D);
	p = pan_socket_poke_double(p, x);
	p = pan_socket_poke_double(p, y);
	p = pan_socket_poke_double(p, z);
	p = pan_socket_poke_double(p, yw);
	p = pan_socket_poke_double(p, pi);
	p = pan_socket_poke_double(p, rl);
	nbytes = p - buf;

	/* Write the buffer in one go. */
	assert(nbytes == bufsize);
	pan_socket_write(s, buf, nbytes);

	/* We want a MSG_OKAY reply */
	return pan_net_want(s, MSG_OKAY);
}


/*
 * pan_net_set_viewpoint_by_quaternion_d(s, x, y, z, q0, q1, q2, q3) is
 * used to set the camera/viewpoint position to (x, y, z) and attitude as
 * defined by the quaternion [q0, q1, q2, q3].
 *
 * pan_net_set_viewpoint_by_quaternion_d_RX() is not required.
 *
 * IMPLEMENTS SetViewpointByQuaternionD (269)
 */
char *
pan_net_set_viewpoint_by_quaternion_d_TX(SOCKET s, double x, double y, double z, double q0, double q1, double q2, double q3)
{
	/*
	 * Send the SetViewpointByQuaternion message with parameters (x, y, z)
	 * and (q0, q1, q2, q3). We're sending one ulong and seven doubles.
	 */
	const int bufsize = 1*4 + 7*8;
	static char buf[bufsize];
	int nbytes;

	/* Initialise the buffer. */
	char *p = buf;
	p = pan_socket_poke_ulong (p, MSG_SET_VIEWPOINT_BY_QUATERNION_D);
	p = pan_socket_poke_double(p, x);
	p = pan_socket_poke_double(p, y);
	p = pan_socket_poke_double(p, z);
	p = pan_socket_poke_double(p, q0);
	p = pan_socket_poke_double(p, q1);
	p = pan_socket_poke_double(p, q2);
	p = pan_socket_poke_double(p, q3);
	nbytes = p - buf;

	/* Write the buffer in one go. */
	assert(nbytes == bufsize);
	pan_socket_write(s, buf, nbytes);

	/* We want a MSG_OKAY reply */
	return pan_net_want(s, MSG_OKAY);
}


/*
 * pan_net_set_object_position_attitude_TX(s,i,x,y,z,q0,q1,q2,q3)
 * is used to set the position of object i to (x, y, z) and
 * attitude as defined by the quaternion [q0, q1, q2, q3].
 *
 * pan_net_set_object_position_attitude_RX() is not required.
 *
 * IMPLEMENTS SetObjectPositionAttitude (270)
 */
char *
pan_net_set_object_position_attitude_TX
(
	SOCKET s,
	unsigned long id,
	double x, double y, double z,
	double q0, double q1, double q2, double q3
)
{
	/*
	 * Send the SetObjectPositionAttitude message.
	 * We're sending two ulongs and seven doubles.
	 */
	const int bufsize = 2*4 + 7*8;
	static char buf[bufsize];
	int nbytes;

	/* Initialise the buffer. */
	char *p = buf;
	p = pan_socket_poke_ulong(p, MSG_SET_OBJECT_POSITION_ATTITUDE);
	p = pan_socket_poke_ulong(p, id);
	p = pan_socket_poke_double(p, x);
	p = pan_socket_poke_double(p, y);
	p = pan_socket_poke_double(p, z);
	p = pan_socket_poke_double(p, q0);
	p = pan_socket_poke_double(p, q1);
	p = pan_socket_poke_double(p, q2);
	p = pan_socket_poke_double(p, q3);
	nbytes = p - buf;

	/* Write the buffer in one go. */
	assert(nbytes == bufsize);
	pan_socket_write(s, buf, nbytes);

	/* We want a MSG_OKAY reply */
	return pan_net_want(s, MSG_OKAY);
}


/*
 * pan_net_set_sun_by_degrees_TX(r,a,e) is used to set the spherical polar
 * position of the Sun to (r,a,e) degrees.
 *
 * pan_net_set_sun_by_degrees_RX() is not required.
 *
 * IMPLEMENTS SetSunByDegrees (271)
 */
char *
pan_net_set_sun_by_degrees_TX(SOCKET s, double r, double a, double e)
{
	/*
	 * Send the SetSunByDegrees message.
	 * We're sending one ulong and three doubles.
	 */
	const int bufsize = 1*4 + 3*8;
	static char buf[bufsize];
	int nbytes;

	/* Initialise the buffer. */
	char *p = buf;
	p = pan_socket_poke_ulong(p, MSG_SET_SUN_BY_DEGREES);
	p = pan_socket_poke_double(p, r);
	p = pan_socket_poke_double(p, a);
	p = pan_socket_poke_double(p, e);
	nbytes = p - buf;

	/* Write the buffer in one go. */
	assert(nbytes == bufsize);
	pan_socket_write(s, buf, nbytes);

	/* We want a MSG_OKAY reply */
	return pan_net_want(s, MSG_OKAY);
}


/*
 * pan_net_set_joint_config_TX(s,o,j,c) sets the configuration of joint j of
 * object o to the values in array c.
 *
 * pan_net_set_joint_config_RX() is not required.
 *
 * IMPLEMENTS SetJointConfig (272)
 */
char *
pan_net_set_joint_config_TX
(
	SOCKET s,
	unsigned long obj,
	unsigned long joint,
	double config[9]
)
{
	int i;

	/*
	 * Send a SetJointConfig message.
	 * We're sending four ulongs, nine doubles and nine bools.
	 */
	const int bufsize = 4*4 + 9*8 + 9*1;
	static char buf[bufsize];
	int nbytes;

	/* Initialise the buffer. */
	char *p = buf;
	p = pan_socket_poke_ulong(p, MSG_SET_JOINT_CONFIG);
	p = pan_socket_poke_ulong(p, obj);
	p = pan_socket_poke_ulong(p, joint);
	p = pan_socket_poke_ulong(p, 9);
	for (i = 0; i < 9; i++)
	{
		p = pan_socket_poke_double(p, config[i]);
		p = pan_socket_poke_bool(p, 1);
	}
	nbytes = p - buf;

	/* Write the buffer in one go. */
	assert(nbytes == bufsize);
	pan_socket_write(s, buf, nbytes);

	/* We want a MSG_OKAY reply */
	return pan_net_want(s, MSG_OKAY);
}


/*
 * pan_net_set_star_quaternion_TX(s, q0, q1, q2, q3) is used to set
 * the attitude quaternion of the star sphere to [q0, q1, q2, q3].
 *
 * pan_net_set_star_quaternion_RX() is not required.
 *
 * IMPLEMENTS SetStarQuaternion (273)
 */
char *
pan_net_set_star_quaternion_TX
(
	SOCKET s,
	double q0, double q1, double q2, double q3
)
{
	/*
	 * Send the SetStarQuaternion message.
	 * We're sending one ulong and four doubles.
	 */
	const int bufsize = 1*4 + 4*8;
	static char buf[bufsize];
	int nbytes;

	/* Initialise the buffer. */
	char *p = buf;
	p = pan_socket_poke_ulong (p, MSG_SET_STAR_QUATERNION);
	p = pan_socket_poke_double(p, q0);
	p = pan_socket_poke_double(p, q1);
	p = pan_socket_poke_double(p, q2);
	p = pan_socket_poke_double(p, q3);
	nbytes = p - buf;

	/* Write the buffer in one go. */
	assert(nbytes == bufsize);
	pan_socket_write(s, buf, nbytes);

	/* We want a MSG_OKAY reply */
	return pan_net_want(s, MSG_OKAY);
}


/*
 * pan_net_set_star_magnitudes_TX(s, m) is used to set the reference
 * star magnitude to m.
 *
 * pan_net_set_star_magnitudes_RX() is not required.
 *
 * IMPLEMENTS SetStarMagnitudes (274)
 */
char *
pan_net_set_star_magnitudes_TX(SOCKET s, double m)
{
	/*
	 * Send the SetStarMagnitudes message.
	 * We're sending one ulong and one double.
	 */
	const int bufsize = 1*4 + 1*8;
	static char buf[bufsize];
	int nbytes;

	/* Initialise the buffer. */
	char *p = buf;
	p = pan_socket_poke_ulong (p, MSG_SET_STAR_MAGNITUDES);
	p = pan_socket_poke_double(p, m);
	nbytes = p - buf;

	/* Write the buffer in one go. */
	assert(nbytes == bufsize);
	pan_socket_write(s, buf, nbytes);

	/* We want a MSG_OKAY reply */
	return pan_net_want(s, MSG_OKAY);
}


/*
 * pan_net_set_secondary_by_degrees_TX(r,a,e) is used to set the spherical
 * polar position of the secondary to (r,a,e).
 *
 * pan_net_set_secondary_by_degrees_RX() is not required.
 *
 * IMPLEMENTS SetSecondaryByDegrees (275)
 */
char *
pan_net_set_secondary_by_degrees_TX(SOCKET s, double r, double a, double e)
{
	/*
	 * Send the SetSecondaryByDegrees message.
	 * We're sending one ulong and three doubles.
	 */
	const int bufsize = 1*4 + 3*8;
	static char buf[bufsize];
	int nbytes;

	/* Initialise the buffer. */
	char *p = buf;
	p = pan_socket_poke_ulong(p, MSG_SET_SECONDARY_BY_DEGREES);
	p = pan_socket_poke_double(p, r);
	p = pan_socket_poke_double(p, a);
	p = pan_socket_poke_double(p, e);
	nbytes = p - buf;

	/* Write the buffer in one go. */
	assert(nbytes == bufsize);
	pan_socket_write(s, buf, nbytes);

	/* We want a MSG_OKAY reply */
	return pan_net_want(s, MSG_OKAY);
}


/*
 * pan_net_set_global_time_TX(s, t) is used to set the current global
 * time value to t.
 *
 * pan_net_set_global_time_RX() is not required.
 *
 * IMPLEMENTS SetGlobalTime (276)
 */
char *
pan_net_set_global_time_TX(SOCKET s, double t)
{
	/*
	 * Send the SetStarMagnitudes message.
	 * We're sending one ulong and one double.
	 */
	const int bufsize = 1*4 + 1*8;
	static char buf[bufsize];
	int nbytes;

	/* Initialise the buffer. */
	char *p = buf;
	p = pan_socket_poke_ulong (p, MSG_SET_GLOBAL_TIME);
	p = pan_socket_poke_double(p, t);
	nbytes = p - buf;

	/* Write the buffer in one go. */
	assert(nbytes == bufsize);
	pan_socket_write(s, buf, nbytes);

	/* We want a MSG_OKAY reply */
	return pan_net_want(s, MSG_OKAY);
}


/*
 * pan_net_set_object_view_TX(obj, type) sets the rendering method of
 * dynamic object "obj" to type "type"
 *
 * pan_net_set_object_view_RX() is not required.
 *
 * IMPLEMENTS SetObjectView (277)
 */
char * 
pan_net_set_object_view_TX(SOCKET s, unsigned long id, unsigned long type)
{
	/*
	 * Send the SetObjectView message with parameters (id, type).
	 * We're sending three ulongs.
	 */
	const int bufsize = 3*4;
	static char buf[bufsize];
	int nbytes;

	/* Initialise the buffer. */
	char *p = buf;
	p = pan_socket_poke_ulong(p, MSG_SET_OBJECT_VIEW);
	p = pan_socket_poke_ulong(p, id);
	p = pan_socket_poke_ulong(p, type);
	nbytes = p - buf;

	/* Write the buffer in one go. */
	assert(nbytes == bufsize);
	pan_socket_write(s, buf, nbytes);

	/* We want a MSG_OKAY reply */
	return pan_net_want(s, MSG_OKAY);
}


/*
 * pan_net_set_viewpoint_by_radians_TX(s, x, y, z, yw, pi, rl) is used to
 * set the camera/viewpoint position to (x, y, z) and attitude yaw/pitch/roll
 * to (yw, pi, rl) radians.
 *
 * pan_net_set_viewpoint_by_radians_RX() is not required.
 *
 * IMPLEMENTS SetViewpointByRadians (278)
 */
char *
pan_net_set_viewpoint_by_radians_TX(SOCKET s, double x, double y, double z, double yw, double pi, double rl)
{
	/*
	 * Send the SetViewpointByRadians message with parameters (x, y, z)
	 * and (yw, pi, rl). We're sending one ulong and six doubles.
	 */
	const int bufsize = 1*4 + 6*8;
	static char buf[bufsize];
	int nbytes;

	/* Initialise the buffer. */
	char *p = buf;
	p = pan_socket_poke_ulong (p, MSG_SET_VIEWPOINT_BY_RADIANS);
	p = pan_socket_poke_double(p, x);
	p = pan_socket_poke_double(p, y);
	p = pan_socket_poke_double(p, z);
	p = pan_socket_poke_double(p, yw);
	p = pan_socket_poke_double(p, pi);
	p = pan_socket_poke_double(p, rl);
	nbytes = p - buf;

	/* Write the buffer in one go. */
	assert(nbytes == bufsize);
	pan_socket_write(s, buf, nbytes);

	/* We want a MSG_OKAY reply */
	return pan_net_want(s, MSG_OKAY);
}


/*
 * pan_net_set_field_of_view_by_radians_TX(s, f) is used to set the angular
 * field of view width to "f" radians. The angular field of view height is
 * determined from the width and the pixel aspect ratio.
 *
 * pan_net_set_field_of_view_by_radians_RX() is not required.
 *
 * IMPLEMENTS SetFieldOfViewByRadians (279)
 */
char *
pan_net_set_field_of_view_by_radians_TX(SOCKET s, float f)
{
	/*
	 * Send the SetFieldOfViewByRadians message with parameter f.
	 * We're sending one ulong and one float.
	 */
	const int bufsize = 1*4 + 1*4;
	static char buf[bufsize];
	int nbytes;

	/* Initialise the buffer. */
	char *p = buf;
	p = pan_socket_poke_ulong (p, MSG_SET_FIELD_OF_VIEW_BY_RADIANS);
	p = pan_socket_poke_float(p, f);
	nbytes = p - buf;

	/* Write the buffer in one go. */
	assert(nbytes == bufsize);
	pan_socket_write(s, buf, nbytes);

	/* We want a MSG_OKAY reply */
	return pan_net_want(s, MSG_OKAY);
}


/*
 * pan_net_set_sun_by_radians_TX(r,a,e) is used to set the spherical polar
 * position of the Sun to (r,a,e) radians.
 *
 * pan_net_set_sun_by_radians_RX() is not required.
 *
 * IMPLEMENTS SetSunByRadians (280)
 */
char *
pan_net_set_sun_by_radians_TX(SOCKET s, double r, double a, double e)
{
	/*
	 * Send the SetSunByRadians message.
	 * We're sending one ulong and three doubles.
	 */
	const int bufsize = 1*4 + 3*8;
	static char buf[bufsize];
	int nbytes;

	/* Initialise the buffer. */
	char *p = buf;
	p = pan_socket_poke_ulong(p, MSG_SET_SUN_BY_RADIANS);
	p = pan_socket_poke_double(p, r);
	p = pan_socket_poke_double(p, a);
	p = pan_socket_poke_double(p, e);
	nbytes = p - buf;

	/* Write the buffer in one go. */
	assert(nbytes == bufsize);
	pan_socket_write(s, buf, nbytes);

	/* We want a MSG_OKAY reply */
	return pan_net_want(s, MSG_OKAY);
}


/*
 * pan_net_set_secondary_by_radians_TX(r,a,e) is used to set the spherical
 * polar position of the secondary to (r,a,e).
 *
 * pan_net_set_secondary_by_radians_RX() is not required.
 *
 * IMPLEMENTS SetSecondaryByRadians (281)
 */
char *
pan_net_set_secondary_by_radians_TX(SOCKET s, double r, double a, double e)
{
	/*
	 * Send the SetSecondaryByRadians message.
	 * We're sending one ulong and three doubles.
	 */
	const int bufsize = 1*4 + 3*8;
	static char buf[bufsize];
	int nbytes;

	/* Initialise the buffer. */
	char *p = buf;
	p = pan_socket_poke_ulong(p, MSG_SET_SECONDARY_BY_RADIANS);
	p = pan_socket_poke_double(p, r);
	p = pan_socket_poke_double(p, a);
	p = pan_socket_poke_double(p, e);
	nbytes = p - buf;

	/* Write the buffer in one go. */
	assert(nbytes == bufsize);
	pan_socket_write(s, buf, nbytes);

	/* We want a MSG_OKAY reply */
	return pan_net_want(s, MSG_OKAY);
}


/*
 * pan_net_set_sky_rgb_TX(s, r, g, b) is used to set the RGB colour
 * for sky mode 8 to (r,g,b).
 *
 * pan_net_set_sky_rgb_RX() is not required.
 *
 * IMPLEMENTS SetSkyRGB (282)
 */
char *
pan_net_set_sky_rgb_TX(SOCKET s, float r, float g, float b)
{
	/*
	 * Send the SetSkyRGB message with parameters (r,g,b).
	 * We're sending one ulong and three floats.
	 */
	const int bufsize = 1*4 + 3*4;
	static char buf[bufsize];
	int nbytes;

	/* Initialise the buffer. */
	char *p = buf;
	p = pan_socket_poke_ulong(p, MSG_SET_SKY_RGB);
	p = pan_socket_poke_float(p, r);
	p = pan_socket_poke_float(p, g);
	p = pan_socket_poke_float(p, b);
	nbytes = p - buf;

	/* Write the buffer in one go. */
	assert(nbytes == bufsize);
	pan_socket_write(s, buf, nbytes);

	/* We want a MSG_OKAY reply */
	return pan_net_want(s, MSG_OKAY);
}


/*
 * pan_net_set_sky_cie_TX(s, x, y, Y) is used to set the CIE colour
 * for sky mode 9 to (x,y,Y).
 *
 * pan_net_set_sky_cie_RX() is not required.
 *
 * IMPLEMENTS SetSkyCIE (283)
 */
char *
pan_net_set_sky_cie_TX(SOCKET s, float x, float y, float Y)
{
	/*
	 * Send the SetSkyCIE message with parameters (x,y,Y).
	 * We're sending one ulong and three floats.
	 */
	const int bufsize = 1*4 + 3*4;
	static char buf[bufsize];
	int nbytes;

	/* Initialise the buffer. */
	char *p = buf;
	p = pan_socket_poke_ulong(p, MSG_SET_SKY_CIE);
	p = pan_socket_poke_float(p, x);
	p = pan_socket_poke_float(p, y);
	p = pan_socket_poke_float(p, Y);
	nbytes = p - buf;

	/* Write the buffer in one go. */
	assert(nbytes == bufsize);
	pan_socket_write(s, buf, nbytes);

	/* We want a MSG_OKAY reply */
	return pan_net_want(s, MSG_OKAY);
}


/*
 * pan_net_set_atmosphere_tau_TX(s, mr,mg,mb, rr,rg,rb) is used to
 * set the optical depth values (Tau). The (mr,mg,mb) value is the
 * Mie scattering depths (aerosols) while the (rr,rg,rb) value is the
 * Rayleigh scattering depths (gas).
 *
 * pan_net_set_atmosphere_tau_RX() is not required.
 *
 * IMPLEMENTS SetAtmosphereTau (284)
 */
char *
pan_net_set_atmosphere_tau_TX(SOCKET s, float mr, float mg, float mb, float rr, float rg, float rb)
{
	/*
	 * Send the SetAtmosphereTau message with parameters (mie,ray).
	 * We're sending one ulong and six floats.
	 */
	const int bufsize = 1*4 + 6*4;
	static char buf[bufsize];
	int nbytes;

	/* Initialise the buffer. */
	char *p = buf;
	p = pan_socket_poke_ulong(p, MSG_SET_ATMOSPHERE_TAU);
	p = pan_socket_poke_float(p, mr);
	p = pan_socket_poke_float(p, mg);
	p = pan_socket_poke_float(p, mb);
	p = pan_socket_poke_float(p, rr);
	p = pan_socket_poke_float(p, rg);
	p = pan_socket_poke_float(p, rb);
	nbytes = p - buf;

	/* Write the buffer in one go. */
	assert(nbytes == bufsize);
	pan_socket_write(s, buf, nbytes);

	/* We want a MSG_OKAY reply */
	return pan_net_want(s, MSG_OKAY);
}


/*
 * pan_net_set_global_fog_mode_TX(s, mode) is used to set the global
 * fog mode to mode.
 *
 * pan_net_set_global_fog_mode_RX() is not required.
 *
 * IMPLEMENTS SetGlobalFogMode (285)
 */
char *
pan_net_set_global_fog_mode_TX(SOCKET s, unsigned long mode)
{
	/*
	 * Send the SetGlobalFogMode message with parameter mode.
	 * We're sending two ulongs.
	 */
	const int bufsize = 1*4 + 1*4;
	static char buf[bufsize];
	int nbytes;

	/* Initialise the buffer. */
	char *p = buf;
	p = pan_socket_poke_ulong(p, MSG_SET_GLOBAL_FOG_MODE);
	p = pan_socket_poke_ulong(p, mode);
	nbytes = p - buf;

	/* Write the buffer in one go. */
	assert(nbytes == bufsize);
	pan_socket_write(s, buf, nbytes);

	/* We want a MSG_OKAY reply */
	return pan_net_want(s, MSG_OKAY);
}


/*
 * pan_net_set_global_fog_properties_TX(s,r,d,l0,l1) is used to set the
 * global fog properties. The r parameter defines the sky dome radius (the
 * effective limit of the atmosphere), the d parameter defines the density
 * (for exponential modes) and the (l0,l1) parameters define the start and
 * end distances of linear fog.
 *
 * pan_net_set_global_fog_properties_RX() is not required.
 *
 * IMPLEMENTS SetGlobalFogProperties (286)
 */
char *
pan_net_set_global_fog_properties_TX(SOCKET s, double radius, double density, double lin0, double lin1)
{
	/*
	 * Send the SetGlobalFogProperties message with parameters
	 * (radius,density,lin0,lin1).
	 * We're sending one ulong and four doubles.
	 */
	const int bufsize = 1*4 + 4*8;
	static char buf[bufsize];
	int nbytes;

	/* Initialise the buffer. */
	char *p = buf;
	p = pan_socket_poke_ulong (p, MSG_SET_GLOBAL_FOG_PROPERTIES);
	p = pan_socket_poke_double(p, radius);
	p = pan_socket_poke_double(p, density);
	p = pan_socket_poke_double(p, lin0);
	p = pan_socket_poke_double(p, lin1);
	nbytes = p - buf;

	/* Write the buffer in one go. */
	assert(nbytes == bufsize);
	pan_socket_write(s, buf, nbytes);

	/* We want a MSG_OKAY reply */
	return pan_net_want(s, MSG_OKAY);
}


/*
 * pan_net_set_atmosphere_mode_TX(s,sm,gm,am) is used to set the
 * atmosphere rendering modes. The sm parameter defines the sky mode,
 * the gm parameter defines the ground mode and the am parameter
 * defines the attenuation mode.
 *
 * pan_net_set_atmosphere_mode_RX() is not required.
 *
 * IMPLEMENTS SetAtmosphereMode (287)
 */
char *
pan_net_set_atmosphere_mode_TX(SOCKET s, unsigned long smode, unsigned long gmode, unsigned long amode)
{
	/*
	 * Send the SetAtmosphereMode message with parameters
	 * (smode, gmode, amode).
	 * We're sending four ulongs.
	 */
	const int bufsize = 1*4 + 3*4;
	static char buf[bufsize];
	int nbytes;

	/* Initialise the buffer. */
	char *p = buf;
	p = pan_socket_poke_ulong(p, MSG_SET_ATMOSPHERE_MODE);
	p = pan_socket_poke_ulong(p, smode);
	p = pan_socket_poke_ulong(p, gmode);
	p = pan_socket_poke_ulong(p, amode);
	nbytes = p - buf;

	/* Write the buffer in one go. */
	assert(nbytes == bufsize);
	pan_socket_write(s, buf, nbytes);

	/* We want a MSG_OKAY reply */
	return pan_net_want(s, MSG_OKAY);
}


/*
 * pan_net_select_camera_TX(s,c) is used to make camera c the
 * current camera for GetImage, LookupPoints etc.
 *
 * pan_net_select_camera_RX() is not required.
 *
 * IMPLEMENTS SelectCamera (288)
 */
char *
pan_net_select_camera_TX(SOCKET s, unsigned long cid)
{
	int nbytes;

	/*
	 * Send a SelectCamera message.
	 * We're sending two ulongs.
	 */
	const int bufsize = 1*4 + 1*4;
	static char buf[bufsize];

	/* Initialise the buffer. */
	char *p = buf;
	p = pan_socket_poke_ulong(p, MSG_SELECT_CAMERA);
	p = pan_socket_poke_ulong(p, cid);
	nbytes = p - buf;

	/* Write the buffer in one go. */
	assert(nbytes == bufsize);
	pan_socket_write(s, buf, nbytes);

	/* We want a MSG_OKAY reply */
	return pan_net_want(s, MSG_OKAY);
}


/*
 * pan_net_bind_light_to_camera_TX(s,0,c,e) is used to associated
 * the camera light with the camera whose ID is c. If e is non-zero
 * then the light will be on otherwise it will be off. The second
 * parameter must be zero (reserved for future use).
 *
 * pan_net_bind_light_to_camera_RX() is not required.
 *
 * IMPLEMENTS BindLightToCamera (289)
 */
char *
pan_net_bind_light_to_camera_TX(SOCKET s, unsigned long lid, unsigned long cid, unsigned char en)
{
	int nbytes;

	/*
	 * Send a BindLightToCamera message.
	 * We're sending three ulongs and a bool.
	 */
	const int bufsize = 1*4 + 1*4 + 1*4 + 1*1;
	static char buf[bufsize];

	/* Initialise the buffer. */
	char *p = buf;
	p = pan_socket_poke_ulong(p, MSG_BIND_LIGHT_TO_CAMERA);
	p = pan_socket_poke_ulong(p, lid);
	p = pan_socket_poke_ulong(p, cid);
	p = pan_socket_poke_bool (p, en);
	nbytes = p - buf;

	/* Write the buffer in one go. */
	assert(nbytes == bufsize);
	pan_socket_write(s, buf, nbytes);

	/* We want a MSG_OKAY reply */
	return pan_net_want(s, MSG_OKAY);
}


/*
 * pan_net_configure_light_by_degrees_TX(s,0,r,g,b,h,e) is used to
 * set the colour of the camera light to RGB(r,g,b), the half-angle
 * of the beam to h degrees and the exponent of the drop-off to e.
 * The second parameter must be zero (reserved for future use). The
 * value of h may be 180 or any value in the range [0,90] while the
 * value of e may be value in the range [0,128].
 *
 * pan_net_configure_light_by_degrees_RX() is not required.
 *
 * IMPLEMENTS ConfigureLightByDegrees (290)
 */
char *
pan_net_configure_light_by_degrees_TX(SOCKET s, unsigned long lid, double r, double g, double b, double h, double e)
{
	int nbytes;

	/*
	 * Send a ConfigureLightByDegrees message.
	 * We're sending 1+16 ulongs/floats.
	 */
	const int bufsize = 1*4 + 16*4;
	static char buf[bufsize];

	/* Initialise the buffer. */
	char *p = buf;
	p = pan_socket_poke_ulong(p, MSG_CONFIGURE_LIGHT_BY_DEGREES);
	p = pan_socket_poke_ulong(p, lid);
	p = pan_socket_poke_float(p, (float)r);
	p = pan_socket_poke_float(p, (float)g);
	p = pan_socket_poke_float(p, (float)b);
	p = pan_socket_poke_float(p, (float)h);
	p = pan_socket_poke_float(p, (float)e);
	/* Padding */
	for (int i = 0; i < 10; ++i) p = pan_socket_poke_float(p, 0.0f);
	nbytes = p - buf;

	/* Write the buffer in one go. */
	assert(nbytes == bufsize);
	pan_socket_write(s, buf, nbytes);

	/* We want a MSG_OKAY reply */
	return pan_net_want(s, MSG_OKAY);
}


/*
 * pan_net_configure_light_by_degrees_TX(s,0,r,g,b,h,e) is used to
 * set the colour of the camera light to RGB(r,g,b), the half-angle
 * of the beam to h radians and the exponent of the drop-off to e.
 * The second parameter must be zero (reserved for future use). The
 * value of h may be pi or any value in the range [0,pi/2] while the
 * value of e may be value in the range [0,128].
 *
 * pan_net_configure_light_by_degrees_RX() is not required.
 *
 * IMPLEMENTS ConfigureLightByRadians (291)
 */
char *
pan_net_configure_light_by_radians_TX(SOCKET s, unsigned long lid, double r, double g, double b, double h, double e)
{
	int nbytes;

	/*
	 * Send a ConfigureLightByRadians message.
	 * We're sending 1+16 ulongs/floats.
	 */
	const int bufsize = 1*4 + 16*4;
	static char buf[bufsize];

	/* Initialise the buffer. */
	char *p = buf;
	p = pan_socket_poke_ulong(p, MSG_CONFIGURE_LIGHT_BY_RADIANS);
	p = pan_socket_poke_ulong(p, lid);
	p = pan_socket_poke_float(p, (float)r);
	p = pan_socket_poke_float(p, (float)g);
	p = pan_socket_poke_float(p, (float)b);
	p = pan_socket_poke_float(p, (float)h);
	p = pan_socket_poke_float(p, (float)e);
	/* Padding */
	for (int i = 0; i < 10; ++i) p = pan_socket_poke_float(p, 0.0f);
	nbytes = p - buf;

	/* Write the buffer in one go. */
	assert(nbytes == bufsize);
	pan_socket_write(s, buf, nbytes);

	/* We want a MSG_OKAY reply */
	return pan_net_want(s, MSG_OKAY);
}


/*
 * pan_net_configure_light_by_degrees_TX(s,0,ox,oy,oz,dx,dy,dz) is
 * used to set the position of the camera light to (ox,oy,oz) and the
 * direction to (dx,dy,dz). These are coordinates in the frame of the
 * associated camera with x=right, y=down, z=forwards. The second
 * parameter must be zero (reserved for future use).
 *
 * pan_net_configure_light_by_degrees_RX() is not required.
 *
 * IMPLEMENTS SetLightPositionDirection (292)
 */
char *
pan_net_set_light_position_direction_TX(SOCKET s, unsigned long lid, double ox, double oy, double oz, double dx, double dy, double dz)
{
	int nbytes;

	/*
	 * Send a SetLightPositionDirection message.
	 * We're sending two ulongs and six doubles.
	 */
	const int bufsize = 1*4 + 1*4 + 6*8;
	static char buf[bufsize];

	/* Initialise the buffer. */
	char *p = buf;
	p = pan_socket_poke_ulong (p, MSG_SET_LIGHT_POSITION_DIRECTION);
	p = pan_socket_poke_ulong (p, lid);
	p = pan_socket_poke_double(p, ox);
	p = pan_socket_poke_double(p, oy);
	p = pan_socket_poke_double(p, oz);
	p = pan_socket_poke_double(p, dx);
	p = pan_socket_poke_double(p, dy);
	p = pan_socket_poke_double(p, dz);
	nbytes = p - buf;

	/* Write the buffer in one go. */
	assert(nbytes == bufsize);
	pan_socket_write(s, buf, nbytes);

	/* We want a MSG_OKAY reply */
	return pan_net_want(s, MSG_OKAY);
}


/*
 * pan_net_render_to_hold_buffer_TX(s,cid,bid) is used to render the
 * view of camera cid to the hold buffer. The bid parameter is unused
 * (reserved for future use) and must be set to 0.
 *
 * pan_net_render_to_hold_buffer_RX() is not required.
 *
 * IMPLEMENTS RenderToHoldBuffer (293)
 */
char *
pan_net_render_to_hold_buffer_TX
(
	SOCKET s,
	unsigned long cid,
	unsigned long bid
)
{
	int nbytes;

	/*
	 * Send a RenderToHoldBuffer message.
	 * We're sending three ulongs.
	 */
	const int bufsize = 1*4 + 2*4;
	static char buf[bufsize];

	/* Initialise the buffer. */
	char *p = buf;
	p = pan_socket_poke_ulong (p, MSG_RENDER_TO_HOLD_BUFFER);
	p = pan_socket_poke_ulong (p, cid);
	p = pan_socket_poke_ulong (p, bid);
	nbytes = p - buf;

	/* Write the buffer in one go. */
	assert(nbytes == bufsize);
	pan_socket_write(s, buf, nbytes);

	/* We want a MSG_OKAY reply */
	return pan_net_want(s, MSG_OKAY);
}


/*
 * pan_net_display_hold_buffer_TX(s,bid) is used to update the
 * display with the hold buffer contents. The bid parameter is unused
 * (reserved for future use) and must be set to 0.
 *
 * pan_net_display_hold_buffer_RX() is not required.
 *
 * IMPLEMENTS DisplayHoldBuffer (294)
 */
char *
pan_net_display_hold_buffer_TX
(
	SOCKET s,
	unsigned long bid
)
{
	int nbytes;

	/*
	 * Send a RenderToHoldBuffer message.
	 * We're sending two ulongs.
	 */
	const int bufsize = 1*4 + 1*4;
	static char buf[bufsize];

	/* Initialise the buffer. */
	char *p = buf;
	p = pan_socket_poke_ulong (p, MSG_DISPLAY_HOLD_BUFFER);
	p = pan_socket_poke_ulong (p, bid);
	nbytes = p - buf;

	/* Write the buffer in one go. */
	assert(nbytes == bufsize);
	pan_socket_write(s, buf, nbytes);

	/* We want a MSG_OKAY reply */
	return pan_net_want(s, MSG_OKAY);
}


/*
 * pan_net_set_corner_cubes_d_TX(s, n, f, p) is used to pass an
 * array of "n" corner cube records starting at address "p" using the
 * array format "f". Currently "f" must be set to zero and "p" must
 * consist of "n" consecutive blocks of corner cube records. Each
 * block contains seven double values (px,py,pz,nx,ny,nz,r)
 * where (px,py,pz) is the position of the cube, (nx,ny,nz) its face
 * normal and r its effective radius.
 *
 * Note that corner cubes are only used when the LIDAR parameters
 * flags field has bit 2 set.
 *
 * pan_net_set_corner_cubes_d_RX() is not required.
 *
 * IMPLEMENTS SetCornerCubesD (295)
 */
char *
pan_net_set_corner_cubes_d_TX
(
	SOCKET s,
	unsigned long n,
	unsigned long fmt,
	double *pcc
)
{
	unsigned long i, siz;

	/* In this implementation we require fmt==0 */
	assert(!fmt);

	/* Compute the size of the data section */
	siz = 7*n*sizeof(double);

	/*
	 * Send the SetCornerCubesD message.
	 * We're sending four ulongs and 7*n doubles. Since the buffer size
	 * is variable we have to allocate it on the heap.
	 */
	const int bufsize = 4*4 + 7*n*8;
	char *buf = (char *)malloc(bufsize); assert(buf);
	int nbytes;

	/* Initialise the buffer. */
	char *p = buf;
	p = pan_socket_poke_ulong(p, MSG_SET_CORNER_CUBES_D);
	p = pan_socket_poke_ulong(p, n);
	p = pan_socket_poke_ulong(p, fmt);
	p = pan_socket_poke_ulong(p, siz);

	/* Send each corner cube record: seven doubles */
	for (i = 0; i < 7*n; i++) p = pan_socket_poke_double(p, *pcc++);
	nbytes = p - buf;

	/* Write the buffer in one go. */
	assert(nbytes == bufsize);
	pan_socket_write(s, buf, nbytes);
	(void)free(buf); buf = 0;

	/* Want an Okay response */
	return pan_net_want(s, MSG_OKAY);
}


/*
 * pan_net_set_projection_mode_TX(s,cid,mode) is used to set the
 * projection mode for camera cid. If mode is 0 then perspective is used
 * else if mode is 1 then orthographic projection is used.
 *
 * pan_net_set_projection_mode_RX() is not required.
 *
 * IMPLEMENTS SetProjectionMode (296)
 */
char *
pan_net_set_projection_mode_TX
(
	SOCKET s,
	unsigned long cid,
	unsigned long mode
)
{
	int nbytes;

	/*
	 * Send a SetProjectionMode message.
	 * We're sending three ulongs.
	 */
	const int bufsize = 1*4 + 2*4;
	static char buf[bufsize];

	/* Initialise the buffer. */
	char *p = buf;
	p = pan_socket_poke_ulong (p, MSG_SET_PROJECTION_MODE);
	p = pan_socket_poke_ulong (p, cid);
	p = pan_socket_poke_ulong (p, mode);
	nbytes = p - buf;

	/* Write the buffer in one go. */
	assert(nbytes == bufsize);
	pan_socket_write(s, buf, nbytes);

	/* We want a MSG_OKAY reply */
	return pan_net_want(s, MSG_OKAY);
}


/*
 * pan_net_set_ortho_field_of_view_TX(s,cid,w,h) is used to set the
 * orthographic projection field of view for camera cid. The width, w, 
 * and height, h, should be given in meters.
 *
 * pan_net_set_ortho_field_of_view_RX() is not required.
 *
 * IMPLEMENTS SetOrthoFieldOfView (297)
 */
char *
pan_net_set_ortho_field_of_view_TX
(
	SOCKET s,
	unsigned long cid,
	double width,
	double height
)
{
	int nbytes;

	/*
	 * Send a SetOrthoFieldOfView message.
	 * We're sending two ulongs and two doubles.
	 */
	const int bufsize = 1*4 + 1*4 + 2*8;
	static char buf[bufsize];

	/* Initialise the buffer. */
	char *p = buf;
	p = pan_socket_poke_ulong (p, MSG_SET_ORTHO_FIELD_OF_VIEW);
	p = pan_socket_poke_ulong (p, cid);
	p = pan_socket_poke_double(p, width);
	p = pan_socket_poke_double(p, height);
	nbytes = p - buf;

	/* Write the buffer in one go. */
	assert(nbytes == bufsize);
	pan_socket_write(s, buf, nbytes);

	/* We want a MSG_OKAY reply */
	return pan_net_want(s, MSG_OKAY);
}


/*
 * pan_net_set_lidar_scan_TX(s, n, f, p) is used to pass an array of
 * "n" lidar scan samples starting at address "p" using the array
 * format "f". Currently "f" must be set to zero and "p" must consist
 * of "n" consecutive blocks of lidar scan samples. Each block contains
 * eight double values (t, q0,q1,q2,q3, s0,s1,s2) where t is the time
 * of the scan sample, (q0,q1,q2,q3) is the quaternion direction of the
 * scan sample, and (s0,s1,s2) are currently spare values that could be
 * used in the future.
 *
 * The lidar scan samples are used when the general scan type is used.
 *
 * pan_net_set_lidar_scan_RX() is not required.
 *
 * IMPLEMENTS SetLidarScan (298)
 */
char *
pan_net_set_lidar_scan_TX
(
	SOCKET s,
	unsigned long n,
	unsigned long fmt,
	double *pls         // pointer to lidar scan samples
)
{
	unsigned long i, siz;

	/* In this implementation we require fmt==0 */
	assert(!fmt);

	/* Compute the size of the data section */
	siz = 8*n*sizeof(double);

	/*
	 * Send the SetLidarScan message.
	 * We're sending four ulongs and 8*n doubles. Since the buffer size
	 * is variable we have to allocate it on the heap.
	 */
	const int bufsize = 4*4 + 8*n*8;
	char *buf = (char *)malloc(bufsize); assert(buf);
	int nbytes;

	/* Initialise the buffer. */
	char *p = buf;
	p = pan_socket_poke_ulong(p, MSG_SET_LIDAR_SCAN);
	p = pan_socket_poke_ulong(p, n);
	p = pan_socket_poke_ulong(p, fmt);
	p = pan_socket_poke_ulong(p, siz);

	/* Send each lidar scan sample: eight doubles */
	for (i = 0; i < 8*n; i++) p = pan_socket_poke_double(p, *pls++);
	nbytes = p - buf;

	/* Write the buffer in one go. */
	assert(nbytes == bufsize);
	pan_socket_write(s, buf, nbytes);
	(void)free(buf); buf = 0;

	/* Want an Okay response */
	return pan_net_want(s, MSG_OKAY);
}


/*
 * pan_net_set_camera_motion_TX(s,cid,vx,vy,vz,rx,ry,rz,ax,ay,az,sx,sy,sz,jx,jy,jz,tx,ty,tz)
 * is used to tell the server that camera cid has linear velocity (vx,vy,vz),
 * linear acceleration (ax,ay,az) and linear jerk (jx,jy,jz). The angular
 * velocity is (r0,r1,r2), angular acceleration (s0,s1,s2) and angular jerk
 * is (tx,ty,tz). The server will compute the position and attitude of the
 * camera using these parameters and equations of motion for each sub-frame
 * rendered for motion blur/rolling shutter images. The angular motion is
 * that of the axis of the attitude quaternion where +z is camera bore. The
 * magnitude of the axis is the rotation angle in radians.
 *
 * pan_net_set_camera_motion_RX() is not required.
 *
 * IMPLEMENTS SetCameraMotion (299)
 */
char *
pan_net_set_camera_motion_TX
(
	SOCKET s,
	unsigned long cid,
	double vx, double vy, double vz, /* linear velocity */
	double rx, double ry, double rz, /* angular velocity */
	double ax, double ay, double az, /* linear acceleration */
	double sx, double sy, double sz, /* angular acceleration */
	double jx, double jy, double jz, /* linear jerk */
	double tx, double ty, double tz  /* angular jerk */
)
{
	/* Send the SetCameraMotion message. We have message and camera ID
	 * followed by three orders of motion (velocity, acceleration, jerk)
	 * in two types (linear and angular) and each is a vector of three
	 * doubles (ijk).
	 */
	const int bufsize = (1+1)*4 + 3*2*3*8;
	static char buf[bufsize];

	/* Initialise the buffer. */
	char *p = buf;
	int nbytes;
	p = pan_socket_poke_ulong (p, MSG_SET_CAMERA_MOTION);
	p = pan_socket_poke_ulong (p, cid);
	p = pan_socket_poke_double(p, vx);
	p = pan_socket_poke_double(p, vy);
	p = pan_socket_poke_double(p, vz);
	p = pan_socket_poke_double(p, rx);
	p = pan_socket_poke_double(p, ry);
	p = pan_socket_poke_double(p, rz);
	p = pan_socket_poke_double(p, ax);
	p = pan_socket_poke_double(p, ay);
	p = pan_socket_poke_double(p, az);
	p = pan_socket_poke_double(p, sx);
	p = pan_socket_poke_double(p, sy);
	p = pan_socket_poke_double(p, sz);
	p = pan_socket_poke_double(p, jx);
	p = pan_socket_poke_double(p, jy);
	p = pan_socket_poke_double(p, jz);
	p = pan_socket_poke_double(p, tx);
	p = pan_socket_poke_double(p, ty);
	p = pan_socket_poke_double(p, tz);
	nbytes = p - buf;

	/* Write the buffer in one go. */
	assert(nbytes == bufsize);
	pan_socket_write(s, buf, nbytes);

	/* Want an Okay response */
	return pan_net_want(s, MSG_OKAY);
}


/**************************************************************************
 * These are the original monolithic interface functions. We recommend that
 * users use the pan_net methods above if they want to use their own error
 * handling functionality.
 **************************************************************************/


/*
 * pan_protocol_safety_checks() ensures that fundamental data type sizes
 * are what we require. If they aren't then our assumptions in the low
 * level code will be violated and bad things will happen. Call this once
 * when your application begins or before it uses the PANGU client library
 * for the first time for safety.
 */
void
pan_protocol_safety_checks(void)
{
	char *error = pan_net_safety_checks();
	if (!error) return;
	(void)fprintf(stderr, "%s", error);
	(void)exit(1);
}


/*
 * pan_protocol_expect(s, t) reads the next message type from "s" and checks
 * to see if it matches the message type "t". If it doesn't and the message
 * type read is an error message then the error will be reported. Otherwise
 * an "unexpected message" error will be reported. The program will terminate
 * after reporting an error.
 */
void
pan_protocol_expect(SOCKET s, unsigned long want)
{
	char *error = pan_net_want(s, want);
	if (!error) return;
	(void)fprintf(stderr, "%s", error);
	(void)exit(1);
}


/*
 * pan_protocol_start() starts a PANGU network protocol session.
 */
void
pan_protocol_start(SOCKET s)
{
	char *error = pan_net_start_TX(s);
	if (!error) return;
	(void)fprintf(stderr, "%s", error);
	(void)exit(1);
}


/*
 * pan_protocol_finish() ends a PANGU network protocol session. The
 * remote server will probably close the connection afterwards.
 *
 * IMPLEMENTS Goodbye (0)
 */
void
pan_protocol_finish(SOCKET s)
{
	(void)pan_net_finish_TX(s);
}


/*
 * ptr = pan_protocol_get_image(s, &size) requests an image from the remote
 * server using the current camera settings. Returns zero if an image
 * could not be obtained from the server. The memory returned by the
 * call is allocated by malloc() and may be released by free(). The size
 * field will be updated with the number of bytes in the result array.
 *
 * IMPLEMENTS GetImage (1)
 */
unsigned char *
pan_protocol_get_image(SOCKET s, unsigned long *psize)
{
	char *error = pan_net_get_image_TX(s);
	if (!error) return pan_net_get_image_RX(s, psize);
	(void)fprintf(stderr, "%s", error);
	(void)exit(1);
}


/*
 * h = pan_protocol_get_elevation(s, perr) returns the elevation of the
 * camera relative to the remote model. If perr is not a null pointer
 * then it will be set to zero if the returned elevation is invalid
 * and non-zero if it is valid. An invalid elevation is returned when
 * the camera is not directly above any part of the remote model.
 *
 * IMPLEMENTS GetElevation (2)
 */
float
pan_protocol_get_elevation(SOCKET s, char *perr)
{
	char *error = pan_net_get_elevation_TX(s);
	if (!error) return pan_net_get_elevation_RX(s, perr);
	(void)fprintf(stderr, "%s", error);
	(void)exit(1);
}


/*
 * pan_protocol_get_elevations(s, n, pv, rv, ev) computes the camera elevations
 * relative to the remote model for each of the "n" camera positions "pv"
 * and writes the results into the array "rv". The ith elevation rv[i] is
 * only valid (the position pv[3*i] is over the model) when ev[i] is non-zero.
 * Both rv and ev must be point to an array of "n" elements.
 *
 * IMPLEMENTS GetElevations (3)
 */
void
pan_protocol_get_elevations(SOCKET s, unsigned long n, float *posv, float *resultv, char *errorv)
{
	char *error = pan_net_get_elevations_TX(s, n, posv);
	if (!error) return pan_net_get_elevations_RX(s, resultv, errorv);
	(void)fprintf(stderr, "%s", error);
	(void)exit(1);
}


/*
 * h = pan_protocol_lookup_point(s, x, y, &px, &py, &pz, perr) returns the
 * 3D position of the model (px, py, pz) under the pixel at coordinates (x, y)
 * where (0, 0) represents the bottom left corner of the image and (1, 1) the
 * top-right corner. If perr is not a null pointer then it will be set to
 * zero if the returned point is invalid and non-zero if it is valid. An
 * invalid point occurs when the centre of the specified pixel does not
 * cover any part of the model.
 *
 * IMPLEMENTS LookupPoint (4)
 */
void
pan_protocol_lookup_point(SOCKET s, float x, float y, float *px, float *py, float *pz, char *perr)
{
	char *error = pan_net_lookup_point_TX(s,x,y);
	if (!error) return pan_net_lookup_point_RX(s,px,py,pz,perr);
	(void)fprintf(stderr, "%s", error);
	(void)exit(1);
}


/*
 * pan_protocol_lookup_points(s, n, pv, rv, ev) computes the 3D positions of
 * each of the "n" pixels whose 2D positions are stored in "pv" and writes
 * the 3D results into the array "rv". The ith position rv[3*i] is only valid
 * if ev[i] is non-zero. The "rv" array must hold 3*n elements while the "ev"
 * array must hold "n" elements.
 *
 * IMPLEMENTS LookupPoints (5)
 */
void
pan_protocol_lookup_points(SOCKET s, unsigned long n, float *posv, float *resultv, char *errorv)
{
	char *error = pan_net_lookup_points_TX(s,n,posv);
	if (!error) return pan_net_lookup_points_RX(s,resultv,errorv);
	(void)fprintf(stderr, "%s", error);
	(void)exit(1);
}


/*
 * pan_protocol_get_point(s, dx, dy, dz, &px, &py, &pz, perr) returns the 3D
 * position (px, py, pz) of the model visible along direction (dx, dy, dz).
 * If perr is not a null pointer then it will be set to zero if the returned
 * point is invalid and non-zero if it is valid. An invalid point occurs when
 * no part of the model is visible along the specified direction.
 *
 * IMPLEMENTS GetPoint (6)
 */
void
pan_protocol_get_point(SOCKET s, float dx, float dy, float dz, float *px, float *py, float *pz, char *perr)
{
	char *error = pan_net_get_point_TX(s,dx,dy,dz);
	if (!error) return pan_net_get_point_RX(s,px,py,pz,perr);
	(void)fprintf(stderr, "%s", error);
	(void)exit(1);
}


/*
 * pan_protocol_get_points(s, n, pv, rv, ev) computes the 3D positions of
 * the "n" points on the model visible along the directions pv[3*i] and
 * writes the results into the array rv[]. Each pv[3*i] position is only
 * valid if ev[i] is non-zero. The "rv" array must hold 3*n elements while
 * the "ev" array must hold "n" elements.
 *
 * IMPLEMENTS GetPoints (7)
 */
void
pan_protocol_get_points(SOCKET s, unsigned long n, float *posv, float *resultv, char *errorv)
{
	char *error = pan_net_get_points_TX(s,n,posv);
	if (!error) return pan_net_get_points_RX(s,resultv,errorv);
	(void)fprintf(stderr, "%s", error);
	(void)exit(1);
}


/*
 * r = pan_protocol_echo(s, p, n, &m) is used to pass the array of n bytes
 * starting at address p to the server. The reply data is returned as the
 * array r whose length is written to m. The result array must be freed
 * after use by calling free(r).
 *
 * IMPLEMENTS Echo (8)
 */
void *
pan_protocol_echo(SOCKET s, void *src, unsigned long n, unsigned long *psize)
{
	char *error = pan_net_echo_TX(s,src,n);
	if (!error) return pan_net_echo_RX(s,psize);
	(void)fprintf(stderr, "%s", error);
	(void)exit(1);
}


/*
 * ptr = pan_protocol_get_range_image(s, &size, o, k) requests a range
 * image from the remote server using the current camera settings. All
 * range values will have "o" subtracted before being multiplied by "k"
 * to obtain a texture coordinate. The texture coordinate is clamped to
 * lie in the range [0,1] and then multiplied by W-1 where W is the
 * width of the range texture image to give a coordinate X. The value
 * of range texture image pixel (X, 0) is then used as the pixel in the
 * returned range image. The physical range is [o, o + 1/k].
 *
 * This function returns zero if an image could not be obtained from
 * the server. The memory returned by the call is allocated by malloc()
 * and may be released by free(). The size field will be updated with
 * the number of bytes in the result array.
 *
 * IMPLEMENTS GetRangeImage (9)
 */
unsigned char *
pan_protocol_get_range_image(SOCKET s, unsigned long *psize,
	float offset, float scale)
{
	char *error = pan_net_get_range_image_TX(s,offset,scale);
	if (!error) return pan_net_get_range_image_RX(s, psize);
	(void)fprintf(stderr, "%s", error);
	(void)exit(1);
}


/*
 * ptr = pan_protocol_get_range_texture(s, &size) requests the range
 * texture image from the remote server. This is a 1-D image whose
 * width can be used to determine the depth resolution of images from
 * pan_protocol_get_range_image(). The pixel values can be used to
 * deduce physical ranges from pan_protocol_get_range_image() ranges.
 *
 * This function returns zero if an image could not be obtained from
 * the server. The memory returned by the call is allocated by malloc()
 * and may be released by free(). The size field will be updated with
 * the number of bytes in the result array.
 *
 * IMPLEMENTS GetRangeTexture (10)
 */
unsigned char *
pan_protocol_get_range_texture(SOCKET s, unsigned long *psize)
{
	char *error = pan_net_get_range_texture_TX(s);
	if (!error) return pan_net_get_range_texture_RX(s,psize);
	(void)fprintf(stderr, "%s", error);
	(void)exit(1);
}


/*
 * pan_protocol_get_viewpoint_by_degrees_s(s, x, y, z, yw, pi, rl, &size) is
 * used to set the camera/viewpoint position to (x, y, z) and yaw/pitch/roll
 * to (yw, pi, rl) and return an image from that position. Returns zero if a
 * image could not be obtained from the server. The memory returned by the
 * call is allocated by malloc() and may be released by free(). The size
 * field will be updated with the number of bytes in the result array. Angles
 * are in degrees.
 *
 * IMPLEMENTS GetViewpointByDegreesS (11)
 */
unsigned char *
pan_protocol_get_viewpoint_by_degrees_s(SOCKET s, float x, float y, float z, float yw, float pi, float rl, unsigned long *psize)
{
	char *error = pan_net_get_viewpoint_by_degrees_s_TX(s,x,y,z,yw,pi,rl);
	if (!error) return pan_net_get_viewpoint_by_degrees_s_RX(s,psize);
	(void)fprintf(stderr, "%s", error);
	(void)exit(1);
}


/*
 * Backwards compatibility.
 *
 * IMPLEMENTS GetViewpointByAngle (11)
 */
unsigned char *
pan_protocol_get_viewpoint_by_angle(SOCKET s, float x, float y, float z, float yw, float pi, float rl, unsigned long *psize)
	{ return pan_protocol_get_viewpoint_by_degrees_s(s, x, y, z, yw, pi, rl, psize); }


/*
 * Backwards compatibility.
 *
 * IMPLEMENTS GetViewpointByAngleS (11)
 */
unsigned char *
pan_protocol_get_viewpoint_by_angle_s(SOCKET s, float x, float y, float z, float yw, float pi, float rl, unsigned long *sz)
	{ return pan_protocol_get_viewpoint_by_degrees_s(s, x, y, z, yw, pi, rl,sz); }


/*
 * pan_protocol_get_viewpoint_by_quaternion_s(s, x, y, z, q0, q1, q2, q3, &size)
 * is used to set the camera/viewpoint position to (x, y, z) and attitude as
 * defined by the quaternion [q0, q1, q2, q3] and return an image from that
 * position. Returns zero if a image could not be obtained from the server.
 * The memory returned by the call is allocated by malloc() and may be
 * released by free(). The size field will be updated with the number of bytes
 * in the result array.
 *
 * IMPLEMENTS GetViewpointByQuaternionS (12)
 */
unsigned char *
pan_protocol_get_viewpoint_by_quaternion_s(SOCKET s, float x, float y, float z, float q0, float q1, float q2, float q3, unsigned long *psize)
{
	char *error = pan_net_get_viewpoint_by_quaternion_s_TX(s,x,y,z,q0,q1,q2,q3);
	if (!error) return pan_net_get_viewpoint_by_quaternion_s_RX(s,psize);
	(void)fprintf(stderr, "%s", error);
	(void)exit(1);
}


/*
 * Backwards compatibility.
 *
 * IMPLEMENTS GetViewpointByQuaternion (12)
 */
unsigned char *
pan_protocol_get_viewpoint_by_quaternion(SOCKET s, float x, float y, float z, float q0, float q1, float q2, float q3, unsigned long *psize)
	{ return pan_protocol_get_viewpoint_by_quaternion_s(s,x,y,z,q0,q1,q2,q3,psize); }


/*
 * pan_protocol_get_lidar_pulse_result(s, x,y,z, dx,dy,dz, &r, &a) is
 * used to obtain the result of a LIDAR pulse from position (x, y, z) along
 * direction (dx, dy, dz). The range to the surface and the cosine of the
 * incidence angle will be written to r and a on return.
 *
 * IMPLEMENTS GetLidarPulseResult (13)
 */
void
pan_protocol_get_lidar_pulse_result(SOCKET s, float x, float y, float z, float dx, float dy, float dz, float *pr, float *pa)
{
	char *error = pan_net_get_lidar_pulse_result_TX(s,x,y,z,dx,dy,dz);
	if (!error) return pan_net_get_lidar_pulse_result_RX(s,pr,pa);
	(void)fprintf(stderr, "%s", error);
	(void)exit(1);
}


/*
 * Backwards compatibility. See GetLidarMeasurementS (34)
 *
 * This function automatically converts from PANGU network floats to native
 * floats before returning. However, note that the server (incorrectly) sends
 * data in native byte order (thus little endian) rather than network order.
 *
 * GetLidarMeasurementS (34) receives the data in network order.
 *
 * IMPLEMENTS GetLidarMeasurement (14)
 */
float *
pan_protocol_get_lidar_measurement(SOCKET s,
	float px, float py, float pz,
	float q0, float q1, float q2, float q3,
	float vx, float vy, float vz,
	float rx, float ry, float rz,
	float ax, float ay, float az,
	float sx, float sy, float sz,
	float jx, float jy, float jz,
	float tx, float ty, float tz,
	float *pfx, float *pfy,
	unsigned long *pnx, unsigned long *pny,
	float *ptx, float *pty,
	unsigned long *pn, unsigned long *pm,
	unsigned long *pt, unsigned long *pfl,
	float *paz, float *pel, float *pth,
	float *pfaz, float *pfel,
	float *ptoff, float *ptaz0, float *ptel0)
{
	char *error = pan_net_get_lidar_measurement_TX(s,px,py,pz,q0,q1,q2,q3,
			vx,vy,vz,rx,ry,rz,ax,ay,az,sx,sy,sz,jx,jy,jz,tx,ty,tz);
	if (!error) return pan_net_get_lidar_measurement_RX(s,pfx,pfy,pnx,pny,
			ptx,pty,pn,pm,pt,pfl,paz,pel,pth,pfaz,pfel,ptoff,ptaz0,ptel0);
	(void)fprintf(stderr, "%s", error);
	(void)exit(1);
}


/*
 * v = pan_protocol_get_radar_response(s,fl,n,nr,ns,ox,oy,oz,vx,vy,vz,
 *     q0,q1,q1,q3,bw,rmd,smd,rbs,sbs,
 *     &st,&mxv,&totv,&offr,&offs,&bsr,&bss,&mnr,&mxr,&mns,&mxs,&nrb,&nsb,
 *     &nused)
 * is used to retrieve the RADAR response for a beam emitted from the
 * point (ox,oy,oz) moving with velocity (vx,vy,vz) with axis (q0,q1,q2,q3)
 * and width bw degrees. The beam is sampled n times and the results are
 * integrated using a 2D histogram with nr range bins and ns speed bins. The
 * fl argument defines flags for the simulation with the meanings of each bit
 * defined as follows:
 *      bit  0: if set then zero align the left edge of the range histograms
 *      bit  1: if set then centre align the range histograms on rmd
 *      bit  2: if set then round range histogram width up to power of 10
 *      bit  3: if set then use fixed range histogram bin size rbs
 *      bit  4: if set then zero align the left edge of the speed histograms
 *      bit  5: if set then centre align the speed histograms on smd
 *      bit  6: if set then round speed histogram width up to power of 10
 *      bit  7: if set then use fixed speed histogram bin size sbs
 *      bit  8: if set then surface slope effects are ignored
 *      bit  9: if set then each sample is worth 1 not 1/n
 * The status of the response is returned in st with the maximum signal value
 * in mxv. The range associated with the left edge of the first histogram bin
 * is offr. The minimum and maximum ranges before clipping by the histogram
 * are minr and maxr respectively; the minimum and maximum speeds is mins and
 * maxs respectively. The size of each bin is bs. The number of bins actually
 * created is nrb and nsb. The 2D array of nrb*nsb bins is returned as v
 * consisting of nrb range values for the first speed histogram, then the next
 * nrb range values for the second speed histogram etc. The sum of all
 * elements of v is returned in totv. The number of samples actually used in
 * the histogram (the number that hit a target) is returned in nused.
 *
 * IMPLEMENTS GetRadarResponse (15)
 */
float *
pan_protocol_get_radar_response
(
	SOCKET s,
	unsigned long flags,
	unsigned long n,
	unsigned long nr, unsigned long ns,
	float ox, float oy, float oz,
	float vx, float vy, float vz,
	float q0, float q1, float q2, float q3,
	float bwidth,
	float rmid, float smid, // Ooops: we ought to have swapped these
	float rbs, float sbs,   // two lines to match PROTOCOL.txt
	unsigned long *pstat,
	float *pmaxv, float *ptotv,
	float *poffr, float *poffs,
	float *prbsize, float *psbsize,
	float *pminr, float *pmaxr,
	float *pmins, float *pmaxs,
	unsigned long *pnrelts, unsigned long *pnselts,
	unsigned long *pnused
)
{
	char *error = pan_net_get_radar_response_TX(s,flags,n,nr,ns,ox,oy,oz,
			vx,vy,vz,q0,q1,q2,q3,bwidth,rmid,smid,rbs,sbs);
	if (!error) return pan_net_get_radar_response_RX(s,pstat,pmaxv,ptotv,
			poffr,poffs,prbsize,psbsize,pminr,pmaxr,pmins,pmaxs,
			pnrelts,pnselts,pnused);
	(void)fprintf(stderr, "%s", error);
	(void)exit(1);
}


/*
 * pan_protocol_get_viewpoint_by_degrees_d(s, x, y, z, yw, pi, rl, &size) is
 * used to set the camera/viewpoint position to (x, y, z) and yaw/pitch/roll
 * to (yw, pi, rl) and return an image from that position. Returns zero if a
 * image could not be obtained from the server. The memory returned by the
 * call is allocated by malloc() and may be released by free(). The size
 * field will be updated with the number of bytes in the result array. Angles
 * are in degrees.
 *
 * IMPLEMENTS GetViewpointByDegreesD (16)
 */
unsigned char *
pan_protocol_get_viewpoint_by_degrees_d(SOCKET s, double x, double y, double z, double yw, double pi, double rl, unsigned long *psize)
{
	char *error = pan_net_get_viewpoint_by_degrees_d_TX(s,x,y,z,yw,pi,rl);
	if (!error) return pan_net_get_viewpoint_by_degrees_d_RX(s,psize);
	(void)fprintf(stderr, "%s", error);
	(void)exit(1);
}


/*
 * Backwards compatibility.
 *
 * IMPLEMENTS GetViewpointByAngleD (16)
 */
unsigned char *
pan_protocol_get_viewpoint_by_angle_d(SOCKET s, double x, double y, double z, double  yw, double pi, double rl, unsigned long *sz)
	{ return pan_protocol_get_viewpoint_by_degrees_d(s, x, y, z, yw, pi, rl, sz); }


/*
 * pan_protocol_get_viewpoint_by_quaternion_d(s, x, y, z, q0, q1, q2, q3, &size)
 * is used to set the camera/viewpoint position to (x, y, z) and attitude as
 * defined by the quaternion [q0, q1, q2, q3] and return an image from that
 * position. Returns zero if a image could not be obtained from the server.
 * The memory returned by the call is allocated by malloc() and may be
 * released by free(). The size field will be updated with the number of bytes
 * in the result array.
 *
 * IMPLEMENTS GetViewpointByQuaternionD (17)
 */
unsigned char *
pan_protocol_get_viewpoint_by_quaternion_d(SOCKET s, double x, double y, double z, double q0, double q1, double q2, double q3, unsigned long *psize)
{
	char *error = pan_net_get_viewpoint_by_quaternion_d_TX(s,x,y,z,q0,q1,q2,q3);
	if (!error) return pan_net_get_viewpoint_by_quaternion_d_RX(s,psize);
	(void)fprintf(stderr, "%s", error);
	(void)exit(1);
}


/*
 * pan_protocol_get_joints(s, i, n) gets an array of joint_data with one entry
 * for each joint of object i. The location pointed to by n is filled with
 * the number of entries in the joint list. 
 *    id: The handle used to identify the joint
 *  name: Descriptive name for the joint
 *  type: 0 = general joint
 *        1 = revolute joint
 *        2 = prismatic joint
 *
 * If an invalid object is specified, an empty list is returned and *n = 0.
 *
 * To free the joint list, first free each 'name' field. 
 *
 * IMPLEMENTS GetJoints (18)
 */
joint_data * 
pan_protocol_get_joints(SOCKET s, unsigned long o, unsigned long *n)
{
	char *error = pan_net_get_joints_TX(s, o);
	if (!error) return pan_net_get_joints_RX(s, n);
	(void)fprintf(stderr, "%s", error);
	(void)exit(1);
}


/*
 * pan_prototcol_get_joint_config(s, o, j, c) returns the configuration
 * of joint j of object o in c, which should point to an array of
 * 9 doubles.
 *
 * For general joints the configuration is 
 *    [sx,sy,sz,rx,ry,rz,tx,ty,tz]
 *    s = scale factor
 *    r = rotation (radians)
 *    t = translation (world units)
 *
 * For prismatic joints, the first element is the joint displacement,
 * other elements have no meaning.
 *
 * For revolute joints, the first element is the joint angle in
 * radians, other elements have no meaning.
 *
 * If an invalid joint or object is specified, each element of c is
 * set to zero.
 *
 * IMPLEMENTS GetJointConfig (19)
 */
void 
pan_protocol_get_joint_config
(
	SOCKET s,
	unsigned long obj,
	unsigned long joint,
	double *config
)
{
	char *error = pan_net_get_joint_config_TX(s, obj, joint);
	if (!error) return pan_net_get_joint_config_RX(s, config);
	(void)fprintf(stderr, "%s", error);
	(void)exit(1);
}


/*
 * pan_protocol_get_frames(s,o,n) returns an array of frame_data, with one
 * entry for each frame of object o; the variable pointed to by n is set
 * to the number of frames.
 *
 * IMPLEMENTS GetFrames (20)
 */
frame_data *
pan_protocol_get_frames(SOCKET s, unsigned long obj, unsigned long *n)
{
	char *error = pan_net_get_frames_TX(s, obj);
	if (!error) return pan_net_get_frames_RX(s, n);
	(void)fprintf(stderr, "%s", error);
	(void)exit(1);
}


/*
 * pan_protocol_get_frame(s,o,i,f) write frame i of object o to f, which
 * should point to an array of 12 doubles. 
 *
 * IMPLEMENTS GetFrame (21)
 */
void
pan_protocol_get_frame
(
	SOCKET s,
	unsigned long obj,
	unsigned long id,
	double *data
)
{
	char *error = pan_net_get_frame_TX(s,obj,id);
	if (!error) return pan_net_get_frame_RX(s, data);
	(void)fprintf(stderr, "%s", error);
	(void)exit(1);
}


/*
 * pan_protocol_get_frame_as_radians(s,o,i,v) writes camera parameters for frame
 * i of object o to v, which should point to an array of 6 doubles.
 *
 * IMPLEMENTS GetFrameAsRadians (22)
 */
void
pan_protocol_get_frame_as_radians
(
	SOCKET s,
	unsigned long obj,
	unsigned long id,
	double *data
)
{
	char *error = pan_net_get_frame_as_radians_TX(s,obj,id);
	if (!error) return pan_net_get_frame_as_radians_RX(s, data);
	(void)fprintf(stderr, "%s", error);
	(void)exit(1);
}


/*
 * Backwards compatibility.
 *
 * IMPLEMENTS GetFrameViewpointByAngle (22)
 */
void 
pan_protocol_get_frame_viewpoint_by_angle(SOCKET s, unsigned long o, unsigned long i, double* v)
	{ pan_protocol_get_frame_as_radians(s, o, i, v); }


/*
 * pan_protocol_get_surface_elevation(s, b, px, py, &e) computes the
 * surface elevation at point (x,y) on the surface and returns it.
 * If e is not null, then it will be set to a non-zero value if the
 * elevation is valid, otherwise it will be set to zero. The elevation
 * is valid if the line x=px, y=py intersects the surface. If the
 * value of b is non-zero the elevation will include any boulder
 * lying on the surface at that point, if it is zero then only the 
 * underlying surface will be used.
 *
 * IMPLEMENTS GetSurfaceElevation (23)
 */
float
pan_protocol_get_surface_elevation
(
	SOCKET s,
	unsigned char boulders,
	float x,
	float y,
	char *err
)
{
	char *error = pan_net_get_surface_elevation_TX(s,boulders,x,y);
	if (!error) return pan_net_get_surface_elevation_RX(s, err);
	(void)fprintf(stderr, "%s", error);
	(void)exit(1);
}


/*
 * pan_protocol_get_surface_elevations(s, b, n, pv, rv, ev) computes
 * the surface elevations at each of the "n" surface positions "pv"
 * and writes the results into the array "rv". The ith elevation
 * rv[i] is only valid (the position pv[2*i] is over the model) when
 * ev[i] is non-zero. Both rv and ev must be point to an array of
 * "n" elements. If the value of b is non-zero then the elevation
 * will include any boulder lying on the surface at that point, if
 * it is zero, then only the underlying surface will be used.
 *
 * IMPLEMENTS GetSurfaceElevations (24)
 */
void
pan_protocol_get_surface_elevations
(
	SOCKET s,
	unsigned char boulders,
	unsigned long n,
	float *posv,
	float *resultv,
	char *errorv
)
{
	char *error = pan_net_get_surface_elevations_TX(s,boulders,n,posv);
	if (!error) return pan_net_get_surface_elevations_RX(s, resultv, errorv);
	(void)fprintf(stderr, "%s", error);
	(void)exit(1);
}


/*
 * pan protocol_get_surface_patch(s, b, cx, cy, nx, ny, d, theta, rv, ev).
 * Computes a DEM for a specified patch of the surface, centred at
 * (cx,cy) with nx samples horizontally and ny samples vertically,
 * with a distance d between sample points. The patch is rotated by
 * theta radians around (cx,cy) (anti-clockwise). This defines a
 * local coordinate system, with origin (cx,cy), whose axes are
 * rotated theta degrees to the world frame. The DEM is written to
 * the array rv which must be of size (nx * ny). The array is arranged
 * by row, where each row is a line of increasing x values (in the
 * local frame). The rows are arranged in order of increasing y value.
 * rv[i] is a valid elevation value only if ev[i] is non-zero. If the 
 * value of b is non-zero then the elevation will include any boulder
 * lying on the surface at that point, if it is zero, then only the
 * underlying surface will be used.
 *
 * IMPLEMENTS GetSurfacePatch (25)
 */
void
pan_protocol_get_surface_patch
(
	SOCKET s,
	unsigned char boulders,
	float cx, float cy,
	unsigned long nx, unsigned long ny,
	float d,
	float theta,
	float *rv,
	char *ev
)
{
	char *error = pan_net_get_surface_patch_TX(s,boulders,cx,cy,nx,ny,d,theta);
	if (!error) return pan_net_get_surface_patch_RX(s,rv,ev);
	(void)fprintf(stderr, "%s", error);
	(void)exit(1);
}


/*
 * pan_protocol_get_viewpoint_by_radians(s, x, y, z, yw, pi, rl, &size) is
 * used to set the camera/viewpoint position to (x, y, z) and yaw/pitch/roll
 * to (yw, pi, rl) and return an image from that position. Returns zero if a
 * image could not be obtained from the server. The memory returned by the
 * call is allocated by malloc() and may be released by free(). The size field
 * will be updated with the number of bytes in the result array. Angles are
 * in radians.
 *
 * IMPLEMENTS GetViewpointByRadians (26)
 */
unsigned char *
pan_protocol_get_viewpoint_by_radians(SOCKET s, double x, double y, double z, double yw, double pi, double rl, unsigned long *psize)
{
	char *error = pan_net_get_viewpoint_by_radians_TX(s,x,y,z,yw,pi,rl);
	if (!error) return pan_net_get_viewpoint_by_radians_RX(s, psize);
	(void)fprintf(stderr, "%s", error);
	(void)exit(1);
}


/*
 * pan_protocol_quit(s) causes the server program to quit. Note that this
 * is NOT the same as the GoodBye message sent by pan_protocol_finish()
 * which closes a client/server connection.
 *
 * IMPLEMENTS Quit (27)
 */
void
pan_protocol_quit(SOCKET s)
{
	char *error = pan_net_quit_TX(s);
	if (!error) return;
	(void)fprintf(stderr, "%s", error);
	(void)exit(1);
}


/*
 * ptr = pan_protocol_get_viewpoint_by_frame(s, oid, fid, &size) requests
 * an image from the frame fid within dynamic object oid. Returns zero if
 * an image could not be obtained from the server. Returns an empty (0 byte)
 * image if the specified frame does not exist. The memory returned by the
 * call is allocated by malloc() and may be released by free(). The size
 * field will be updated with the number of bytes in the result array.
 *
 * IMPLEMENTS GetViewpointByFrame (28)
 */
unsigned char *
pan_protocol_get_viewpoint_by_frame
(
	SOCKET s,
	unsigned long oid,
	unsigned long fid,
	unsigned long *psize
)
{
	char *error = pan_net_get_viewpoint_by_frame_TX(s,oid,fid);
	if (!error) return pan_net_get_viewpoint_by_frame_RX(s, psize);
	(void)fprintf(stderr, "%s", error);
	(void)exit(1);
}


/*
 * v = pan_protocol_get_camera_properties(s,c,&w,&h,&hf,&vf,&px,&py,&pz,&q0,&q1,&q2,&3)
 * is used to obtain the properties of camera c. The variables w and h are
 * updated with the image dimensions, hf and vf with the horizontal and
 * vertical field of view angles (in radians), the (px,py,pz) variables with
 * the camera position and the (q0,q1,q2,q3) variables with the camera
 * attitude quaternion (q0 is the scalar term). The return value v is 0 if
 * the camera is invalid and positive if valid. A negative value indicates
 * that temporary storage allocation failed.
 *
 * IMPLEMENTS GetCameraProperties (29)
 */
int
pan_protocol_get_camera_properties
(
	SOCKET s,
	unsigned long cid,
	unsigned long *pwidth,
	unsigned long *pheight,
	double *phfov,
	double *pvfov,
	double *px,
	double *py,
	double *pz,
	double *pq0,
	double *pq1,
	double *pq2,
	double *pq3
)
{
	char *error = pan_net_get_camera_properties_TX(s,cid);
	if (!error) return pan_net_get_camera_properties_RX(s,pwidth,pheight,
				phfov,pvfov,px,py,pz,pq0,pq1,pq2,pq3);
	(void)fprintf(stderr, "%s", error);
	(void)exit(1);
}


/*
 * ptr = pan_protocol_get_viewpoint_by_camera(s, cid, &size)
 * requests an image from camera cid. Returns zero if an image could
 * not be obtained from the server. Returns an empty (0 byte) image
 * if the specified camera does not exist. The memory returned by
 * the call is allocated by malloc() and may be released by free().
 * The size field will be updated with the number of bytes in the
 * result array.
 *
 * IMPLEMENTS GetViewpointByCamera (30)
 */
unsigned char *pan_protocol_get_viewpoint_by_camera
(
	SOCKET s,
	unsigned long cid,
	unsigned long *psize
)
{
	char *error = pan_net_get_viewpoint_by_camera_TX(s,cid);
	if (!error) return pan_net_get_viewpoint_by_camera_RX(s, psize);
	(void)fprintf(stderr, "%s", error);
	(void)exit(1);
}


/*
 * pan_protocol_get_view_as_dem(s,cid,b,nx,ny,dx,dy,rd,rv,ev).
 * Requests a DEM based on the view from camera cid. cid must currently
 * be 1, for the remote camera. If b is non-zero then boulders will be
 * included in the height. The requested DEM will have nx samples
 * horizontally and ny samples vertically. Horizontally there will be
 * dx units between samples, and vertically there will be dy units
 * between samples. The heights of the DEM will be relative to the
 * relative distance, rd, from the camera.
 * The DEM is written to the array rv which must be of size (nx * ny).
 * The array is arranged by row, where each row is a line of increasing
 * x values (in the local frame). The rows are arranged in order of
 * decreasing y value. rv[i] is a valid elevation value only if ev[i]
 * is non-zero.
 *
 * IMPLEMENTS GetViewAsDEM (31)
 */
void
pan_protocol_get_view_as_dem
(
	SOCKET s,
	unsigned long cid,
	unsigned char boulders,
	unsigned long nx, unsigned long ny,
	float dx, float dy,
	float rd,
	float *rv,
	char *ev
)
{
	char *error = pan_net_get_view_as_dem_TX(s,cid,boulders,nx,ny,dx,dy,rd);
	if (!error) return pan_net_get_view_as_dem_RX(s,rv,ev);
	(void)fprintf(stderr, "%s", error);
	(void)exit(1);
}


/*
 * v = pan_protocol_get_lidar_measurement_d(
 *        s, px,py,pz, q0,q1,q2,q3, vx,vy,vz, rx,ry,rx,
 *        ax,ay,az, sx,sy,sz, jx,jy,jz, tx,ty,tz,  
 *        &fx,&fy, &nx,&ny, &tx,&ty, &n,&m, &t, &fl, &az,&el, &th, &faz, &fel,
 *        &toff, &taz0, &tel0) is
 * used to request a LIDAR scan from position (px,py,pz) with attitude
 * quaternion (q0,q1,q2,q3), linear velocity (vx,vy,vz) and angular velocity
 * (rx,ry,rz), linear acceleration (ax,ay,az), angular acceleration (sx,sy,sz),
 * linear jerk (jx,jy,jz) and angular jerk (tx,ty,tz). The return value v is a
 * pointer to an array of scan results while the LIDAR emitter/detector
 * settings are written to fx etc. If the scan has W by H beams with
 * sub-sampling factors N and M and B blocks of results then "v" will contain
 * 2*B*(W*N)*(H*M) floating point values representing the 2D scan results at
 * the sub-sampling resolution NxM. The number of blocks B depends on the
 * results requested in the LIDAR emitter/detector settings flag field.
 *
 * This function automatically converts from PANGU network floats to native
 * floats before returning.
 *
 * This function receives the float data in network order. As opposed to
 * GetLidarMeasurement (14) which receives it in host order. So this is the
 * correct function to use.
 *
 * IMPLEMENTS GetLidarMeasurementD (32)
 */
float *
pan_protocol_get_lidar_measurement_d(SOCKET s,
	double px, double py, double pz,
	double q0, double q1, double q2, double q3,
	double vx, double vy, double vz,
	double rx, double ry, double rz,
	double ax, double ay, double az,
	double sx, double sy, double sz,
	double jx, double jy, double jz,
	double tx, double ty, double tz,
	float *pfx, float *pfy,
	unsigned long *pnx, unsigned long *pny,
	float *ptx, float *pty,
	unsigned long *pn, unsigned long *pm,
	unsigned long *pt, unsigned long *pfl,
	float *paz, float *pel, float *pth,
	float *pfaz, float *pfel,
	float *ptoff, float *ptaz0, float *ptel0)
{
	char *error = pan_net_get_lidar_measurement_d_TX(s,px,py,pz,q0,q1,q2,q3,
			vx,vy,vz,rx,ry,rz,ax,ay,az,sx,sy,sz,jx,jy,jz,tx,ty,tz);
	if (!error) return pan_net_get_lidar_measurement_d_RX(s,pfx,pfy,pnx,pny,
			ptx,pty,pn,pm,pt,pfl,paz,pel,pth,pfaz,pfel,ptoff,ptaz0,ptel0);
	(void)fprintf(stderr, "%s", error);
	(void)exit(1);
}


/*
 * t = pan_protocol_get_time_tag(s, perr) returns the time, t, that the
 * last image was requested at. The time is measured in microseconds
 * since 00:00:00 UCT 01 January 1970. If perr is not a null pointer
 * then it will be set to zero if the returned timetag is invalid
 * and non-zero if it is valid.
 *
 * IMPLEMENTS GetTimeTag (33)
 */
double
pan_protocol_get_time_tag(SOCKET s, char *perr)
{
	char *error = pan_net_get_time_tag_TX(s);
	if (!error) return pan_net_get_time_tag_RX(s, perr);

	(void)fprintf(stderr, "%s", error);
	(void)exit(1);
}


/*
 * v = pan_protocol_get_lidar_measurement_s(
 *        s, px,py,pz, q0,q1,q2,q3, vx,vy,vz, rx,ry,rx,
 *        ax,ay,az, sx,sy,sz, jx,jy,jz, tx,ty,tz,  
 *        &fx,&fy, &nx,&ny, &tx,&ty, &n,&m, &t, &fl, &az,&el, &th, &faz, &fel,
 *        &toff, &taz0, &tel0) is
 * used to request a LIDAR scan from position (px,py,pz) with attitude
 * quaternion (q0,q1,q2,q3), linear velocity (vx,vy,vz) and angular velocity
 * (rx,ry,rz), linear acceleration (ax,ay,az), angular acceleration (sx,sy,sz),
 * linear jerk (jx,jy,jz) and angular jerk (tx,ty,tz). The return value v is a
 * pointer to an array of scan results while the LIDAR emitter/detector
 * settings are written to fx etc. If the scan has W by H beams with
 * sub-sampling factors N and M and B blocks of results then "v" will contain
 * 2*B*(W*N)*(H*M) floating point values representing the 2D scan results at
 * the sub-sampling resolution NxM. The number of blocks B depends on the
 * results requested in the LIDAR emitter/detector settings flag field.
 *
 * This function automatically converts from PANGU network floats to native
 * floats before returning.
 *
 * This function receives the float data in network order. As opposed to
 * GetLidarMeasurement (14) which receives it in host order. So this is the
 * correct function to use.
 *
 * IMPLEMENTS GetLidarMeasurementS (34)
 */
float *
pan_protocol_get_lidar_measurement_s(SOCKET s,
	float px, float py, float pz,
	float q0, float q1, float q2, float q3,
	float vx, float vy, float vz,
	float rx, float ry, float rz,
	float ax, float ay, float az,
	float sx, float sy, float sz,
	float jx, float jy, float jz,
	float tx, float ty, float tz,
	float *pfx, float *pfy,
	unsigned long *pnx, unsigned long *pny,
	float *ptx, float *pty,
	unsigned long *pn, unsigned long *pm,
	unsigned long *pt, unsigned long *pfl,
	float *paz, float *pel, float *pth,
	float *pfaz, float *pfel,
	float *ptoff, float *ptaz0, float *ptel0)
{
	char *error = pan_net_get_lidar_measurement_s_TX(s,px,py,pz,q0,q1,q2,q3,
			vx,vy,vz,rx,ry,rz,ax,ay,az,sx,sy,sz,jx,jy,jz,tx,ty,tz);
	if (!error) return pan_net_get_lidar_measurement_s_RX(s,pfx,pfy,pnx,pny,
			ptx,pty,pn,pm,pt,pfl,paz,pel,pth,pfaz,pfel,ptoff,ptaz0,ptel0);

	(void)fprintf(stderr, "%s", error);
	(void)exit(1);
}


/*
 * v = pan_protocol_get_lidar_snapshot(
 *        s, cid, px,py,pz, q0,q1,q2,q3, &w,&h) is
 * is used to request a LIDAR scan from position (px,py,pz) with
 * attitude quaternion (q0,q1,q2,q3).
 * The return value v is a pointer to an array of floats representing a
 * raw image with top left origin. w and h are unsigned longs providing
 * the width and height of the image.
 * Each pixel of the image has three floats: the first is the range
 * (i.e. the distance between the surface and the lidar scanner); the
 * second is the cosine of the angle between the surface normal and scan
 * direction of that pixel; and the third is a hit/miss flag where 0
 * represents a miss and 1 represents a hit. So v points to w*h*3 floats.
 * 
 * This function automatically converts from MSB_REAL_32 floats to
 * native floats before returning.
 *
 * IMPLEMENTS GetLidarSnapshot (35)
 */
float *
pan_protocol_get_lidar_snapshot(SOCKET s, unsigned long cid,
		double px, double py, double pz,
		double q0, double q1, double q2, double q3,
		unsigned long * width, unsigned long * height)
{
	char *error = pan_net_get_lidar_snapshot_TX(s,cid,px,py,pz,q0,q1,q2,q3);
	if (!error) return pan_net_get_lidar_snapshot_RX(s, width, height);
	(void)fprintf(stderr, "%s", error);
	(void)exit(1);
}


/*
 * pan_protocol_set_viewpoint_by_degrees_s(s, x, y, z, yw, pi, rl) is used to
 * set the camera/viewpoint position to (x, y, z) and attitude yaw/pitch/roll
 * to (yw, pi, rl) degrees.
 *
 * IMPLEMENTS SetViewpointByDegreesS (256)
 */
void
pan_protocol_set_viewpoint_by_degrees_s(SOCKET s, float x, float y, float z, float yw, float pi, float rl)
{
	char *error = pan_net_set_viewpoint_by_degrees_s_TX(s,x,y,z,yw,pi,rl);
	if (!error) return;
	(void)fprintf(stderr, "%s", error);
	(void)exit(1);
}


/*
 * Backwards compatibility.
 *
 * IMPLEMENTS SetViewpointByAngle (256)
 */
void
pan_protocol_set_viewpoint_by_angle(SOCKET s, float x, float y, float z, float yw, float pi, float rl)
	{ pan_protocol_set_viewpoint_by_degrees_s(s, x, y, z, yw, pi, rl); }


/*
 * Backwards compatibility.
 *
 * IMPLEMENTS SetViewpointByAngleS (256)
 */
void 
pan_protocol_set_viewpoint_by_angle_s(SOCKET s, float x , float y, float z, float yw, float pi, float rl)
	{ pan_protocol_set_viewpoint_by_degrees_s(s, x, y, z, yw, pi, rl); }


/*
 * pan_protocol_set_viewpoint_by_quaternion_s(s, x, y, z, q0, q1, q2, q3) is
 * used to set the camera/viewpoint position to (x, y, z) and attitude as
 * defined by the quaternion [q0, q1, q2, q3].
 *
 * IMPLEMENTS SetViewpointByQuaternionS (257)
 */
void
pan_protocol_set_viewpoint_by_quaternion_s(SOCKET s, float x, float y, float z, float q0, float q1, float q2, float q3)
{
	char *error = pan_net_set_viewpoint_by_quaternion_s_TX(s,x,y,z,q0,q1,q2,q3);
	if (!error) return;
	(void)fprintf(stderr, "%s", error);
	(void)exit(1);
}


/*
 * Backwards compatibility.
 *
 * IMPLEMENTS SetViewpointByQuaternion (257)
 */
void
pan_protocol_set_viewpoint_by_quaternion(SOCKET s, float x, float y, float z, float q0, float q1, float q2, float q3)
	{ pan_protocol_set_viewpoint_by_quaternion_s(s,x,y,z,q0,q1,q2,q3); }


/*
 * pan_protocol_set_ambient_light(s, r, g, b) is used to set the colour and
 * intensity of ambient light in the red, green and blue channels to (r, g, b).
 *
 * IMPLEMENTS SetAmbientLight (258)
 */
void
pan_protocol_set_ambient_light(SOCKET s, float r, float g, float b)
{
	char *error = pan_net_set_ambient_light_TX(s,r,g,b);
	if (!error) return;
	(void)fprintf(stderr, "%s", error);
	(void)exit(1);
}


/*
 * pan_protocol_set_sun_colour(s, r, g, b) is used to set the colour and
 * intensity of the Sun in the red, green and blue channels to (r, g, b).
 *
 * IMPLEMENTS SetSunColour (259)
 */
void
pan_protocol_set_sun_colour(SOCKET s, float r, float g, float b)
{
	char *error = pan_net_set_sun_colour_TX(s,r,g,b);
	if (!error) return;
	(void)fprintf(stderr, "%s", error);
	(void)exit(1);
}


/*
 * pan_protocol_set_sky_type(s, t) is used to set the sky background type to
 * "t" where "t" may hold one of the following values:
 *    0 : no sky (black)
 *    1 : fake sky (sphere-mapped texture ignoring camera roll)
 *    2 : raw sky (single-pixel stars of varying intensity)
 *    3 : red sky (maximum red, no green, no blue)
 *    4 : green sky (no red, maximum green, no blue)
 *    5 : blue sky (no red, no green, maximum blue)
 *    6 : white sky (maximum red, green, blue)
 *    7 : PSF stars (stars of varying intensity modulated by a PSF)
 *    8 : RGB sky (user defined red, green, blue values)
 *    9 : CIE sky (user defined CIE xyY values converted into RGB)
 *
 * IMPLEMENTS SetSkyType (260)
 */
void
pan_protocol_set_sky_type(SOCKET s, unsigned long t)
{
	char *error = pan_net_set_sky_type_TX(s,t);
	if (!error) return;
	(void)fprintf(stderr, "%s", error);
	(void)exit(1);
}


/*
 * pan_protocol_set_field_of_view_by_degrees(s, f) is used to set the angular
 * field of view width to "f" degrees. The angular field of view height is
 * determined from the width and the pixel aspect ratio.
 *
 * IMPLEMENTS SetFieldOfViewByDegrees (261)
 */
void
pan_protocol_set_field_of_view_by_degrees(SOCKET s, float f)
{
	char *error = pan_net_set_field_of_view_by_degrees_TX(s,f);
	if (!error) return;
	(void)fprintf(stderr, "%s", error);
	(void)exit(1);
}


/*
 * Backwards compatibility.
 *
 * IMPLEMENTS SetFieldOfView (261)
 */
void 
pan_protocol_set_field_of_view(SOCKET s, float f)
	{ pan_protocol_set_field_of_view_by_degrees(s, f); }


/*
 * pan_protocol_set_aspect_ratio(s, r) is used to set the angular aspect
 * ratio of each pixel. This is used in conjunction with the angular field
 * of view width to determine the angular field of view height.
 *
 * IMPLEMENTS SetAspectRatio (262)
 */
void
pan_protocol_set_aspect_ratio(SOCKET s, float r)
{
	char *error = pan_net_set_aspect_ratio_TX(s,r);
	if (!error) return;
	(void)fprintf(stderr, "%s", error);
	(void)exit(1);
}


/*
 * pan_protocol_set_boulder_view(s, t, tex) is used to set the boulder
 * rendering method to "t" and to enable or disable boulder texturing via
 * "tex".
 *
 * IMPLEMENTS SetBoulderView (263)
 */
void
pan_protocol_set_boulder_view(SOCKET s, unsigned long type, int texture)
{
	char *error = pan_net_set_boulder_view_TX(s,type,texture);
	if (!error) return;
	(void)fprintf(stderr, "%s", error);
	(void)exit(1);
}


/*
 * pan_protocol_set_surface_view(s, t, tex, det) is used to set the surface
 * rendering method to "t" and to enable or disable surface texturing and
 * surface detailing.
 *
 * IMPLEMENTS SetSurfaceView (264)
 */
void
pan_protocol_set_surface_view(SOCKET s, unsigned long type, int tex, int det)
{
	char *error = pan_net_set_surface_view_TX(s,type,tex,det);
	if (!error) return;
	(void)fprintf(stderr, "%s", error);
	(void)exit(1);
}


/*
 * pan_protocol_set_lidar_parameters(s, fx,fy, nx,ny, tx,ty, n, m, t, fl, az,el,
 *    th, wx, wy, faz, fel, toff, taz0, tel0) is used to configure the LIDAR
 *    emitter/detector with field of view (fx, fy) degrees horizontally and
 *    vertically, an emitter or detector grid of nx by ny beams, and subsampling
 *    of n by m samples. The scan type is "t" where:
 *      0: TV scan (left-to-right, top-to-bottom)
 *      1: mode 01 (LiGNC project zig-zag scan, azi mirror before ele)
 *      2: mode 02 (LiGNC project zig-zag scan, ele mirror before azi)
 *      3: mode 01 (ILT project 1D sinusoidal scan, azi mirror before ele)
 *      4: mode 02 (ILT project 1D sinusoidal scan, ele mirror before azi)
 *      5: mode 01 (LAPS project zig-zag scan, azi mirror before ele)
 *      6: mode 02 (LAPS project zig-zag scan, ele mirror before azi)
 *      7: General scan (Uses samples given via pan_protocol_set_lidar_scan())
 * The centre of the scan has LIDAR azimuth/elevation (az,el) and the
 * beam half-angle is th degrees. The LIDAR flags fl is a bit vector:
 *      bit  0: return range/slope results if set
 *      bit  1: return azimuth/elevation values if set
 *      bit  2: return corner cube range/slope results if set
 *      bit  3: return time-of-pulse-emission values
 *      bit 16: if set the azimuth scan starts with decreasing azimuth
 *      bit 17: if set the elevation scan starts with decreasing elevation
 *              and the results are returned in that order (bottom to top)
 *      bit 18: if set (normally used instead of bit 17) the results are
 *              returned in top-to-bottom order but the pulses are emittted
 *              in bottom-to-top order
 * All other bits must be clear.
 * Each detector pixel is wx by wy degrees in size. Note that when
 * n and/or m are greater than 1 these are the sizes of the subsample
 * pixels not the full pixel.
 * For the ILT scans (type 3 and 4) the faz parameter defines the azimuth
 *    sinusoial scanning frequency.
 * For the LAPS scans (type 5 and 6) the faz parameter defines the x-axis
 *    zig-zag scanning frequency and the fel parameter defines the y-axis
 *    zig-zag scanning frequency. The (taz0, tel0) parameters define the
 *    offset of the zig-zag scanning patterns.
 *
 * IMPLEMENTS SetLidarParameters (265)
 */
void
pan_protocol_set_lidar_parameters(SOCKET s,
	float fx, float fy,
	unsigned long nx, unsigned long ny,
	float tx, float ty,
	unsigned long n, unsigned long m,
	unsigned long t, unsigned long fl,
	float az, float el, float th,
	float wx, float wy,
	float faz, float fel,
	float toff, float taz0, float tel0)
{
	char *error = pan_net_set_lidar_parameters_TX(s,fx,fy,nx,ny,
			tx,ty,n,m,t,fl,az,el,th,wx,wy,faz,fel,toff,taz0,tel0);
	if (!error) return;
	(void)fprintf(stderr, "%s", error);
	(void)exit(1);
}


/*
 * pan_protocol_set_corner_cubes_s(s, n, f, p) is used to pass an
 * array of "n" corner cube records starting at address "p" using the
 * array format "f". Currently "f" must be set to zero and "p" must
 * consist of "n" consecutive blocks of corner cube records. Each
 * block contains seven floating point values (px,py,pz,nx,ny,nz,r)
 * where (px,py,pz) is the position of the cube, (nx,ny,nz) its face
 * normal and r its effective radius.
 *
 * Note that corner cubes are only used when the LIDAR parameters
 * flags field has bit 2 set.
 *
 * IMPLEMENTS SetCornerCubesS (266)
 */
void
pan_protocol_set_corner_cubes_s
(
	SOCKET s,
	unsigned long n,
	unsigned long fmt,
	float *pcc
)
{
	char *error = pan_net_set_corner_cubes_s_TX(s,n,fmt,pcc);
	if (!error) return;
	(void)fprintf(stderr, "%s", error);
	(void)exit(1);
}


/*
 * Backwards compatibility.
 *
 * IMPLEMENTS SetCornerCubesS (266)
 */
void
pan_protocol_set_corner_cubes
(
	SOCKET s,
	unsigned long n,
	unsigned long fmt,
	float *pcc
)
{
	pan_protocol_set_corner_cubes_s(s, n, fmt, pcc);
}


/*
 * pan_protocol_set_corner_cube_attitude(
 *        s, q0,q1,q2,q3, rx,ry,rx, ax,ay,az, jx,jy,jz) is used
 * to define the attitude (q0,q1,q2,q3), angular velocity (rx,ry,rz),
 * angular acceleration (ax,ay,az) and angular jerk (jx,jy,jz) of the
 * corner cube lattice at the centre of all future LIDAR frames.
 *
 * IMPLEMENTS SetCornerCubeAttitude (267)
 */
void
pan_protocol_set_corner_cube_attitude
(
	SOCKET s,
	float q0, float q1, float q2, float q3,
	float rx, float ry, float rz,
	float ax, float ay, float az,
	float jx, float jy, float jz
)
{
	char *error = pan_net_set_corner_cube_attitude_TX(s, q0,q1,q2,q3,
			rx,ry,rz, ax,ay,az, jx,jy,jz);
	if (!error) return;
	(void)fprintf(stderr, "%s", error);
	(void)exit(1);
}


/*
 * pan_protocol_set_viewpoint_by_degrees_d(s, x, y, z, yw, pi, rl) is used to
 * set the camera/viewpoint position to (x, y, z) and attitude yaw/pitch/roll
 * to (yw, pi, rl) degrees.
 *
 * IMPLEMENTS SetViewpointByDegreesD (268)
 */
void
pan_protocol_set_viewpoint_by_degrees_d(SOCKET s, double x, double y, double z, double yw, double pi, double rl)
{
	char *error = pan_net_set_viewpoint_by_degrees_d_TX(s,x,y,z,yw,pi,rl);
	if (!error) return;
	(void)fprintf(stderr, "%s", error);
	(void)exit(1);
}


/*
 * Backwards compatibility.
 *
 * IMPLEMENTS SetViewpointByAngleD (268)
 */
void 
pan_protocol_set_viewpoint_by_angle_d(SOCKET s, double x, double y, double z, double yw, double pi, double rl)
{
	pan_protocol_set_viewpoint_by_degrees_d(s, x, y, z, yw, pi, rl);
}


/*
 * pan_protocol_set_viewpoint_by_quaternion_d(s, x, y, z, q0, q1, q2, q3) is
 * used to set the camera/viewpoint position to (x, y, z) and attitude as
 * defined by the quaternion [q0, q1, q2, q3].
 *
 * IMPLEMENTS SetViewpointByQuaternionD (269)
 */
void
pan_protocol_set_viewpoint_by_quaternion_d(SOCKET s, double x, double y, double z, double q0, double q1, double q2, double q3)
{
	char *error = pan_net_set_viewpoint_by_quaternion_d_TX(s,x,y,z,q0,q1,q2,q3);
	if (!error) return;
	(void)fprintf(stderr, "%s", error);
	(void)exit(1);
}


/*
 * pan_protocol_set_object_position(i,s,x,y,z,q0,q1,q2,q3)
 * is used to set object i position to (x, y, z) and
 * attitude as defined by the quaternion [q0, q1, q2, q3].
 *
 * IMPLEMENTS SetObjectPositionAttitude (270)
 */
void
pan_protocol_set_object_position_attitude
(
	SOCKET s,
	unsigned long id,
	double x, double y, double z,
	double q0, double q1, double q2, double q3
)
{
	char *error = pan_net_set_object_position_attitude_TX(s,id,x,y,z,q0,q1,q2,q3);
	if (!error) return;
	(void)fprintf(stderr, "%s", error);
	(void)exit(1);
}


/*
 * Backwards compatibility.
 *
 * IMPLEMENTS SetObjectPositionAttitude (270)
 */
void
pan_protocol_set_object_position(SOCKET s, unsigned long id, double x, double y, double z, double q0, double q1, double q2, double q3)
{
	pan_protocol_set_object_position_attitude(s,id,x,y,z,q0,q1,q2,q3);
}


/*
 * pan_protocol_set_sun_by_degrees(r,a,e) is used to set the spherical polar
 * position of the Sun to (r,a,e) degrees.
 *
 * IMPLEMENTS SetSunByDegrees (271)
 */
void
pan_protocol_set_sun_by_degrees(SOCKET s, double r, double a, double e)
{
	char *error = pan_net_set_sun_by_degrees_TX(s,r,a,e);
	if (!error) return;
	(void)fprintf(stderr, "%s", error);
	(void)exit(1);
}


/*
 * Backwards compatibility.
 *
 * IMPLEMENTS SetSunPosition (271)
 */
void 
pan_protocol_set_sun_position(SOCKET s, double r, double a, double e)
	{ pan_protocol_set_sun_by_degrees(s, r, a, e); }


/*
 * pan_protocol_set_joint_config(o,j,c) sets the configuration of joint j of
 * object o to the values in array c.
 *
 * IMPLEMENTS SetJointConfig (272)
 */
void
pan_protocol_set_joint_config
(
	SOCKET s,
	unsigned long obj,
	unsigned long joint,
	double config[9]
)
{
	char *error = pan_net_set_joint_config_TX(s,obj,joint,config);
	if (!error) return;
	(void)fprintf(stderr, "%s", error);
	(void)exit(1);
}


/*
 * pan_protocol_set_star_quaternion(s, q0, q1, q2, q3) is used to set
 * the attitude quaternion of the star sphere to [q0, q1, q2, q3].
 *
 * IMPLEMENTS SetStarQuaternion (273)
 */
void
pan_protocol_set_star_quaternion
(
	SOCKET s,
	double q0, double q1, double q2, double q3
)
{
	char *error = pan_net_set_star_quaternion_TX(s,q0,q1,q2,q3);
	if (!error) return;
	(void)fprintf(stderr, "%s", error);
	(void)exit(1);
}


/*
 * pan_protocol_set_star_magnitudes(s, m) is used to set the reference
 * star magnitude to m.
 *
 * IMPLEMENTS SetStarMagnitudes (274)
 */
void
pan_protocol_set_star_magnitudes(SOCKET s, double m)
{
	char *error = pan_net_set_star_magnitudes_TX(s,m);
	if (!error) return;
	(void)fprintf(stderr, "%s", error);
	(void)exit(1);
}


/*
 * pan_protocol_set_secondary_by_degrees(r,a,e) is used to set the spherical
 * polar position of the secondary to (r,a,e).
 *
 * IMPLEMENTS SetSecondaryByDegrees (275)
 */
void
pan_protocol_set_secondary_by_degrees(SOCKET s, double r, double a, double e)
{
	char *error = pan_net_set_secondary_by_degrees_TX(s,r,a,e);
	if (!error) return;
	(void)fprintf(stderr, "%s", error);
	(void)exit(1);
}


/*
 * pan_protocol_set_global_time(s, t) is used to set the current global
 * time value to t.
 *
 * IMPLEMENTS SetGlobalTime (276)
 */
void
pan_protocol_set_global_time(SOCKET s, double t)
{
	char *error = pan_net_set_global_time_TX(s,t);
	if (!error) return;
	(void)fprintf(stderr, "%s", error);
	(void)exit(1);
}


/*
 * pan_protocol_set_object_view(obj, type) sets the rendering method of
 * dynamic object "obj" to type "type"
 *
 * IMPLEMENTS SetObjectView (277)
 */
void 
pan_protocol_set_object_view(SOCKET s, unsigned long id, unsigned long type)
{
	char *error = pan_net_set_object_view_TX(s,id,type);
	if (!error) return;
	(void)fprintf(stderr, "%s", error);
	(void)exit(1);
}


/*
 * pan_protocol_set_viewpoint_by_radians(s, x, y, z, yw, pi, rl) is used to
 * set the camera/viewpoint position to (x, y, z) and attitude yaw/pitch/roll
 * to (yw, pi, rl) radians.
 *
 * IMPLEMENTS SetViewpointByRadians (278)
 */
void
pan_protocol_set_viewpoint_by_radians(SOCKET s, double x, double y, double z, double yw, double pi, double rl)
{
	char *error = pan_net_set_viewpoint_by_radians_TX(s,x,y,z,yw,pi,rl);
	if (!error) return;
	(void)fprintf(stderr, "%s", error);
	(void)exit(1);
}


/*
 * pan_protocol_set_field_of_view_by_radians(s, f) is used to set the angular
 * field of view width to "f" radians. The angular field of view height is
 * determined from the width and the pixel aspect ratio.
 *
 * IMPLEMENTS SetFieldOfViewByRadians (279)
 */
void
pan_protocol_set_field_of_view_by_radians(SOCKET s, float f)
{
	char *error = pan_net_set_field_of_view_by_radians_TX(s,f);
	if (!error) return;
	(void)fprintf(stderr, "%s", error);
	(void)exit(1);
}


/*
 * pan_protocol_set_sun_by_radians(r,a,e) is used to set the spherical polar
 * position of the Sun to (r,a,e) radians.
 *
 * IMPLEMENTS SetSunByRadians (280)
 */
void
pan_protocol_set_sun_by_radians(SOCKET s, double r, double a, double e)
{
	char *error = pan_net_set_sun_by_radians_TX(s,r,a,e);
	if (!error) return;
	(void)fprintf(stderr, "%s", error);
	(void)exit(1);
}


/*
 * pan_protocol_set_secondary_by_radians(r,a,e) is used to set the spherical
 * polar position of the secondary to (r,a,e).
 *
 * IMPLEMENTS SetSecondaryByRadians (281)
 */
void
pan_protocol_set_secondary_by_radians(SOCKET s, double r, double a, double e)
{
	char *error = pan_net_set_secondary_by_radians_TX(s,r,a,e);
	if (!error) return;
	(void)fprintf(stderr, "%s", error);
	(void)exit(1);
}


/*
 * pan_protocol_set_sky_rgb(s, r, g, b) is used to set the RGB colour
 * for sky mode 8 to (r,g,b).
 *
 * IMPLEMENTS SetSkyRGB (282)
 */
void
pan_protocol_set_sky_rgb(SOCKET s, float r, float g, float b)
{
	char *error = pan_net_set_sky_rgb_TX(s,r,g,b);
	if (!error) return;
	(void)fprintf(stderr, "%s", error);
	(void)exit(1);
}


/*
 * pan_protocol_set_sky_cie(s, x, y, Y) is used to set the CIE colour
 * for sky mode 9 to (x,y,Y).
 *
 * IMPLEMENTS SetSkyCIE (283)
 */
void
pan_protocol_set_sky_cie(SOCKET s, float x, float y, float Y)
{
	char *error = pan_net_set_sky_cie_TX(s,x,y,Y);
	if (!error) return;
	(void)fprintf(stderr, "%s", error);
	(void)exit(1);
}


/*
 * pan_protocol_set_atmosphere_tau(s, mr,mg,mb, rr,rg,rb) is used to
 * set the optical depth values (Tau). The (mr,mg,mb) value is the
 * Mie scattering depths (aerosols) while the (rr,rg,rb) value is the
 * Rayleigh scattering depths (gas).
 *
 * IMPLEMENTS SetAtmosphereTau (284)
 */
void
pan_protocol_set_atmosphere_tau(SOCKET s, float mr, float mg, float mb, float rr, float rg, float rb)
{
	char *error = pan_net_set_atmosphere_tau_TX(s,mr,mg,mb,rr,rg,rb);
	if (!error) return;
	(void)fprintf(stderr, "%s", error);
	(void)exit(1);
}


/*
 * pan_protocol_set_global_fog_mode(s, mode) is used to set the global
 * fog mode to mode.
 *
 * IMPLEMENTS SetGlobalFogMode (285)
 */
void
pan_protocol_set_global_fog_mode(SOCKET s, unsigned long mode)
{
	char *error = pan_net_set_global_fog_mode_TX(s,mode);
	if (!error) return;
	(void)fprintf(stderr, "%s", error);
	(void)exit(1);
}


/*
 * pan_protocol_set_global_fog_properties(s,r,d,l0,l1) is used to set the
 * global fog properties. The r parameter defines the sky dome radius (the
 * effective limit of the atmosphere), the d parameter defines the density
 * (for exponential modes) and the (l0,l1) parameters define the start and
 * end distances of linear fog.
 *
 * IMPLEMENTS SetGlobalFogProperties (286)
 */
void
pan_protocol_set_global_fog_properties(SOCKET s, double radius, double density, double lin0, double lin1)
{
	char *error = pan_net_set_global_fog_properties_TX(s,radius,density,lin0,lin1);
	if (!error) return;
	(void)fprintf(stderr, "%s", error);
	(void)exit(1);
}


/*
 * pan_protocol_set_atmosphere_mode(s,sm,gm,am) is used to set the
 * atmosphere rendering modes. The sm parameter defines the sky mode,
 * the gm parameter defines the ground mode and the am parameter
 * defines the attenuation mode.
 *
 * IMPLEMENTS SetAtmosphereMode (287)
 */
void
pan_protocol_set_atmosphere_mode(SOCKET s, unsigned long smode, unsigned long gmode, unsigned long amode)
{
	char *error = pan_net_set_atmosphere_mode_TX(s,smode,gmode,amode);
	if (!error) return;
	(void)fprintf(stderr, "%s", error);
	(void)exit(1);
}


/*
 * pan_protocol_select_camera(s,c) is used to make camera c the
 * current camera for GetImage, LookupPoints etc.
 *
 * IMPLEMENTS SelectCamera (288)
 */
void
pan_protocol_select_camera(SOCKET s, unsigned long cid)
{
	char *error = pan_net_select_camera_TX(s,cid);
	if (!error) return;
	(void)fprintf(stderr, "%s", error);
	(void)exit(1);
}


/*
 * pan_protocol_bind_light_to_camera(s,0,c,e) is used to associated
 * the camera light with the camera whose ID is c. If e is non-zero
 * then the light will be on otherwise it will be off. The second
 * parameter must be zero (reserved for future use).
 *
 * IMPLEMENTS BindLightToCamera (289)
 */
void
pan_protocol_bind_light_to_camera(SOCKET s, unsigned long lid, unsigned long cid, unsigned char en)
{
	char *error = pan_net_bind_light_to_camera_TX(s,lid,cid,en);
	if (!error) return;
	(void)fprintf(stderr, "%s", error);
	(void)exit(1);
}


/*
 * pan_protocol_configure_light_by_degrees(s,0,r,g,b,h,e) is used to
 * set the colour of the camera light to RGB(r,g,b), the half-angle
 * of the beam to h degrees and the exponent of the drop-off to e.
 * The second parameter must be zero (reserved for future use). The
 * value of h may be 180 or any value in the range [0,90] while the
 * value of e may be value in the range [0,128].
 *
 * IMPLEMENTS ConfigureLightByDegrees (290)
 */
void
pan_protocol_configure_light_by_degrees(SOCKET s, unsigned long lid, double r, double g, double b, double h, double e)
{
	char *error = pan_net_configure_light_by_degrees_TX(s,lid,r,g,b,h,e);
	if (!error) return;
	(void)fprintf(stderr, "%s", error);
	(void)exit(1);
}


/*
 * pan_protocol_configure_light_by_degrees(s,0,r,g,b,h,e) is used to
 * set the colour of the camera light to RGB(r,g,b), the half-angle
 * of the beam to h radians and the exponent of the drop-off to e.
 * The second parameter must be zero (reserved for future use). The
 * value of h may be pi or any value in the range [0,pi/2] while the
 * value of e may be value in the range [0,128].
 *
 * IMPLEMENTS ConfigureLightByRadians (291)
 */
void
pan_protocol_configure_light_by_radians(SOCKET s, unsigned long lid, double r, double g, double b, double h, double e)
{
	char *error = pan_net_configure_light_by_radians_TX(s,lid,r,g,b,h,e);
	if (!error) return;
	(void)fprintf(stderr, "%s", error);
	(void)exit(1);
}


/*
 * pan_protocol_configure_light_by_degrees(s,0,ox,oy,oz,dx,dy,dz) is
 * used to set the position of the camera light to (ox,oy,oz) and the
 * direction to (dx,dy,dz). These are coordinates in the frame of the
 * associated camera woth x=right, y=down, z=forwards.
 *
 * IMPLEMENTS SetLightPositionDirection (292)
 */
void
pan_protocol_set_light_position_direction(SOCKET s, unsigned long lid, double ox, double oy, double oz, double dx, double dy, double dz)
{
	char *error = pan_net_set_light_position_direction_TX(s,lid,ox,oy,oz,dx,dy,dz);
	if (!error) return;
	(void)fprintf(stderr, "%s", error);
	(void)exit(1);
}


/*
 * pan_protocol_render_to_hold_buffer(s,cid,bid) is used to render the
 * view of camera cid to the hold buffer. The bid parameter is unused
 * (reserved for future use) and must be set to 0.
 *
 * IMPLEMENTS RenderToHoldBuffer (293)
 */
void
pan_protocol_render_to_hold_buffer
(
	SOCKET s,
	unsigned long cid,
	unsigned long bid
)
{
	char *error = pan_net_render_to_hold_buffer_TX(s, cid, bid);
	if (!error) return;
	(void)fprintf(stderr, "%s", error);
	(void)exit(1);
}


/*
 * pan_protocol_display_hold_buffer(s,bid) is used to update the
 * display with the hold buffer contents. The bid parameter is unused
 * (reserved for future use) and must be set to 0.
 *
 * IMPLEMENTS DisplayHoldBuffer (294)
 */
void
pan_protocol_display_hold_buffer
(
	SOCKET s,
	unsigned long bid
)
{
	char *error = pan_net_display_hold_buffer_TX(s, bid);
	if (!error) return;
	(void)fprintf(stderr, "%s", error);
	(void)exit(1);
}


/*
 * pan_protocol_set_corner_cubes_d(s, n, f, p) is used to pass an
 * array of "n" corner cube records starting at address "p" using the
 * array format "f". Currently "f" must be set to zero and "p" must
 * consist of "n" consecutive blocks of corner cube records. Each
 * block contains seven double values (px,py,pz,nx,ny,nz,r)
 * where (px,py,pz) is the position of the cube, (nx,ny,nz) its face
 * normal and r its effective radius.
 *
 * Note that corner cubes are only used when the LIDAR parameters
 * flags field has bit 2 set.
 *
 * IMPLEMENTS SetCornerCubesD (295)
 */
void
pan_protocol_set_corner_cubes_d
(
	SOCKET s,
	unsigned long n,
	unsigned long fmt,
	double *pcc
)
{
	char *error = pan_net_set_corner_cubes_d_TX(s, n, fmt, pcc);
	if (!error) return;
	(void)fprintf(stderr, "%s", error);
	(void)exit(1);
}


/*
 * pan_protocol_set_projection_mode(s,cid,mode) is used to set the
 * projection mode for camera cid. If mode is 0 then perspective is used
 * else if mode is 1 then orthographic projection is used.
 *
 * IMPLEMENTS SetProjectionMode (296)
 */
void
pan_protocol_set_projection_mode
(
	SOCKET s,
	unsigned long cid,
	unsigned long mode
)
{
	char *error = pan_net_set_projection_mode_TX(s, cid, mode);
	if (!error) return;
	(void)fprintf(stderr, "%s", error);
	(void)exit(1);
}


/*
 * pan_protocol_set_ortho_field_of_view(s,cid,w,h) is used to set the
 * orthographic projection field of view for camera cid. The width, w, 
 * and height, h, should be given in meters.
 *
 * IMPLEMENTS SetOrthoFieldOfView (297)
 */
void
pan_protocol_set_ortho_field_of_view
(
	SOCKET s,
	unsigned long cid,
	double width,
	double height
)
{
	char *error = pan_net_set_ortho_field_of_view_TX(s, cid, width, height);
	if (!error) return;
	(void)fprintf(stderr, "%s", error);
	(void)exit(1);
}


/*
 * pan_protocol_set_lidar_scan(s, n, f, p) is used to pass an array of
 * "n" lidar scan samples starting at address "p" using the array
 * format "f". Currently "f" must be set to zero and "p" must consist
 * of "n" consecutive blocks of lidar scan samples. Each block contains
 * eight double values (t, q0,q1,q2,q3, s0,s1,s2) where t is the time
 * of the scan sample, (q0,q1,q2,q3) is the quaternion direction of the
 * scan sample, and (s0,s1,s2) are currently spare values that could be
 * used in the future.
 *
 * The lidar scan samples are used when the general scan type is used.
 *
 * IMPLEMENTS SetLidarScan (298)
 */
void
pan_protocol_set_lidar_scan
(
	SOCKET s,
	unsigned long n,
	unsigned long fmt,
	double *pls         // pointer to lidar scan samples
)
{
	char *error = pan_net_set_lidar_scan_TX(s, n, fmt, pls);
	if (!error) return;
	(void)fprintf(stderr, "%s", error);
	(void)exit(1);
}


/*
 * pan_protocol_set_camera_motion(s,cid,vx,vy,vz,rx,ry,rz,ax,ay,az,sx,sy,sz,jx,jy,jz,tx,ty,tz)
 * is used to tell the server that camera cid has linear velocity (vx,vy,vz),
 * linear acceleration (ax,ay,az) and linear jerk (jx,jy,jz). The angular
 * velocity is (r0,r1,r2), angular acceleration (s0,s1,s2) and angular jerk
 * is (tx,ty,tz). The server will compute the position and attitude of the
 * camera using these parameters and equations of motion for each sub-frame
 * rendered for motion blur/rolling shutter images. The angular motion is
 * that of the axis of the attitude quaternion where +z is camera bore. The
 * magnitude of the axis is the rotation angle in radians.
 *
 * IMPLEMENTS SetCameraMotion (299)
 */
void
pan_protocol_set_camera_motion
(
	SOCKET s,
	unsigned long cid,
	double vx, double vy, double vz, /* linear velocity */
	double rx, double ry, double rz, /* angular velocity */
	double ax, double ay, double az, /* linear acceleration */
	double sx, double sy, double sz, /* angular acceleration */
	double jx, double jy, double jz, /* linear jerk */
	double tx, double ty, double tz  /* angular jerk */
)
{
	char *error = pan_net_set_camera_motion_TX
	(
		s, cid,
		vx,vy,vz,
		rx,ry,rz,
		ax,ay,az,
		sx,sy,sz,
		jx,jy,jz,
		tx,ty,tz
	);
	if (!error) return;
	(void)fprintf(stderr, "%s", error);
	(void)exit(1);
}

