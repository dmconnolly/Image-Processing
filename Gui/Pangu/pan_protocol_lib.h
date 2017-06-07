
/*
 * Name:
 *   pan_protocol_lib.h
 *
 * Purpose:
 *   Public C interface to the client end of the PANGU network protocol.
 *
 * Description:
 *
 * Notes:
 *   Keep ALL functions sorted by increasing client message number. Do
 *   not group related methods together e.g. double and single precision
 *   versions of the same underlying process. Users are expected to read
 *   PROTOCOL.TXT or the PANGU ICD to determine the name of the method
 *   that they want to use. Replace the capitalised words with all lower
 *   case words separated by _ and prefix with pan_protocol_. Thus the
 *   GetFrames(20) method becomes pan_protocol_get_frames().
 *
 * Language:
 *   ANSI C
 *
 * Author:
 *   Martin N Dunstan (mdunstan@computing.dundee.ac.uk)
 *   {add_authors_here}
 *
 * Copyright:
 *   (c) Space Technology Centre, University of Dundee, 2001-2016.
 *
 * History:
 *   22-Oct-2002 (mnd):
 *      Original version.
 *   {add_changes_here}
 *
 * Future work:
 *   {add_suggestions_here}
 *
 * Bugs:
 *   {add_new_bugs_here}
 */

#ifndef PAN_PROTOCOL_LIB_H_INCLUDED
#define PAN_PROTOCOL_LIB_H_INCLUDED

#include "pan_socket_io.h"


/* Client message numbers */
enum
{
	/* Version 1.00 command/request messages */
	MSG_GOODBYE				=   0,
	MSG_GET_IMAGE				=   1,
	MSG_GET_ELEVATION			=   2,
	MSG_GET_ELEVATIONS			=   3,
	MSG_LOOKUP_POINT			=   4,
	MSG_LOOKUP_POINTS			=   5,
	MSG_GET_POINT				=   6,
	MSG_GET_POINTS				=   7,
	MSG_ECHO				=   8,
	/* Version 1.01 */
	MSG_GET_RANGE_IMAGE			=   9,
	MSG_GET_RANGE_TEXTURE			=  10,
	/* Version 1.02 */
	MSG_GET_VIEWPOINT_BY_DEGREES_S		=  11,
	MSG_GET_VIEWPOINT_BY_QUATERNION_S	=  12,
	/* Version 1.03 */
	MSG_GET_LIDAR_PULSE_RESULT		=  13,
	MSG_GET_LIDAR_MEASUREMENT		=  14,
	/* Version 1.06 */
	MSG_GET_RADAR_RESPONSE			=  15,
	/* Version 1.08 */
	MSG_GET_VIEWPOINT_BY_DEGREES_D		=  16,
	MSG_GET_VIEWPOINT_BY_QUATERNION_D	=  17,
	/* Version 1.09 */
	MSG_GET_JOINTS				=  18,
	MSG_GET_JOINT_CONFIG			=  19,
	/* Version 1.10 */
	MSG_GET_FRAMES				=  20,
	MSG_GET_FRAME				=  21,
	MSG_GET_FRAME_AS_RADIANS		=  22,
	/* Version 1.11 */
	MSG_GET_SURFACE_ELEVATION		=  23,
	MSG_GET_SURFACE_ELEVATIONS		=  24,
	MSG_GET_SURFACE_PATCH			=  25,
	/* Version 1.15 */
	MSG_GET_VIEWPOINT_BY_RADIANS		=  26,
	/* Version 1.16 */
	MSG_QUIT				=  27,
	/* Version 1.17 */
	MSG_GET_VIEWPOINT_BY_FRAME		=  28,
	MSG_GET_CAMERA_PROPERTIES		=  29,
	MSG_GET_VIEWPOINT_BY_CAMERA		=  30,
	/* Version 1.20 */
	MSG_GET_VIEW_AS_DEM			=  31,
	MSG_GET_LIDAR_MEASUREMENT_D		=  32,
	MSG_GET_TIME_TAG			=  33,
	MSG_GET_LIDAR_MEASUREMENT_S		=  34,
	MSG_GET_LIDAR_SNAPSHOT			=  35,

	/* Version 1.00 configuration messages */
	MSG_SET_VIEWPOINT_BY_DEGREES_S		= 256,
	MSG_SET_VIEWPOINT_BY_QUATERNION_S	= 257,
	MSG_SET_AMBIENT_LIGHT			= 258,
	MSG_SET_SUN_COLOUR			= 259,
	MSG_SET_SKY_TYPE			= 260,
	MSG_SET_FIELD_OF_VIEW_BY_DEGREES	= 261,
	MSG_SET_ASPECT_RATIO			= 262,
	MSG_SET_BOULDER_VIEW			= 263,
	MSG_SET_SURFACE_VIEW			= 264,
	/* Version 1.03 */
	MSG_SET_LIDAR_PARAMETERS		= 265,
	/* Version 1.04 */
	MSG_SET_CORNER_CUBES_S			= 266,
	/* Version 1.05 */
	MSG_SET_CORNER_CUBE_ATTITUDE		= 267,
	/* Version 1.08 */
	MSG_SET_VIEWPOINT_BY_DEGREES_D		= 268,
	MSG_SET_VIEWPOINT_BY_QUATERNION_D	= 269,
	MSG_SET_OBJECT_POSITION_ATTITUDE	= 270,
	MSG_SET_SUN_BY_DEGREES			= 271,
	/* Version 1.09 */
	MSG_SET_JOINT_CONFIG			= 272,
	/* Version 1.12 */
	MSG_SET_STAR_QUATERNION			= 273,
	MSG_SET_STAR_MAGNITUDES			= 274,
	/* Version 1.13 */
	MSG_SET_SECONDARY_BY_DEGREES		= 275,
	/* Version 1.14 */
	MSG_SET_GLOBAL_TIME			= 276,
	/* Version 1.15 */
	MSG_SET_OBJECT_VIEW			= 277,
	MSG_SET_VIEWPOINT_BY_RADIANS		= 278,
	MSG_SET_FIELD_OF_VIEW_BY_RADIANS	= 279,
	MSG_SET_SUN_BY_RADIANS			= 280,
	MSG_SET_SECONDARY_BY_RADIANS		= 281,
	/* Version 1.17 */
	MSG_SET_SKY_RGB				= 282,
	MSG_SET_SKY_CIE				= 283,
	MSG_SET_ATMOSPHERE_TAU			= 284,
	MSG_SET_GLOBAL_FOG_MODE			= 285,
	MSG_SET_GLOBAL_FOG_PROPERTIES		= 286,
	MSG_SET_ATMOSPHERE_MODE			= 287,
	MSG_SELECT_CAMERA			= 288,
	/* Version 1.18 */
	MSG_BIND_LIGHT_TO_CAMERA		= 289,
	MSG_CONFIGURE_LIGHT_BY_DEGREES		= 290,
	MSG_CONFIGURE_LIGHT_BY_RADIANS		= 291,
	MSG_SET_LIGHT_POSITION_DIRECTION	= 292,
	/* Version 1.19 */
	MSG_RENDER_TO_HOLD_BUFFER		= 293,
	MSG_DISPLAY_HOLD_BUFFER			= 294,
	/* Version 1.20 */
	MSG_SET_CORNER_CUBES_D			= 295,
	MSG_SET_PROJECTION_MODE			= 296,
	MSG_SET_ORTHO_FIELD_OF_VIEW		= 297,
	MSG_SET_LIDAR_SCAN			= 298,
	MSG_SET_CAMERA_MOTION			= 299,

	MSG_CLIENT_LIMIT
};

/* Server message numbers */
enum
{
	MSG_OKAY			=  0,
	MSG_ERROR			=  1,
	MSG_IMAGE			=  2,
	MSG_FLOAT			=  3,
	MSG_FLOAT_ARRAY			=  4,
	MSG_3D_POINT			=  5,
	MSG_3D_POINT_ARRAY		=  6,
	MSG_MEMORY_BLOCK		=  7,
	MSG_ECHO_REPLY			=  8,
	MSG_LIDAR_PULSE_RESULT		=  9,
	MSG_LIDAR_MEASUREMENT		= 10,
	MSG_RADAR_RESPONSE		= 11,
	/* Version 1.09 */
	MSG_DOUBLE			= 12,
	MSG_DOUBLE_ARRAY		= 13,
	MSG_JOINT_LIST			= 14,
	/* Version 1.10 */
	MSG_FRAME_LIST			= 15,
	/* Version 1.17 */
	MSG_CAMERA_PROPERTIES		= 16,
	/* Version 1.20 */
	MSG_RAW_IMAGE			= 17,

	MSG_SERVER_LIMIT
};

/* Structure representing floating point values and a validity flag */
struct optional_float
{
	float	value;
	char	valid;
};

/* Structure representing double values and a validity flag */
struct optional_double
{
	double	value;
	char	valid;
};

struct joint_data
{
	unsigned long id;
	char* name;
	unsigned long type;
};

struct frame_data
{
	unsigned long id;
	char* name;
};


extern void pan_protocol_safety_checks(void);
	/*
	 * pan_protocol_safety_checks() ensures that fundamental data type
	 * sizes are what we require. If they aren't then our assumptions in
	 * the low level code will be violated and bad things will happen.
	 */

extern void pan_protocol_expect(SOCKET, unsigned long);
	/*
	 * pan_protocol_expect(s, t) reads the next message type from "s" and
	 * checks to see if it matches the message type "t". If it doesn't and
	 * the message type read is an error message then the error will be
	 * reported. Otherwise an "unexpected message" error will be reported.
	 * The program will terminate after reporting an error.
	 */

extern void pan_protocol_start(SOCKET);
	/*
	 * pan_protocol_start() starts a PANGU network protocol session.
	 */

extern void pan_protocol_finish(SOCKET);
	/*
	 * pan_protocol_finish() ends a PANGU network protocol session. The
	 * remote server will probably close the connection afterwards.
	 *
	 * IMPLEMENTS Goodbye (0)
	 */

extern unsigned char *pan_protocol_get_image(SOCKET, unsigned long *);
	/*
	 * ptr = pan_protocol_get_image(s, &size) requests an image from the
	 * remote server using the current camera settings. Returns zero if an
	 * image could not be obtained from the server. The memory returned by
	 * the call is allocated by malloc() and may be released by free().
	 * The size field will be updated with the number of bytes in the
	 * result array.
	 *
	 * IMPLEMENTS GetImage (1)
	 */

extern float pan_protocol_get_elevation(SOCKET, char *);
	/*
	 * h = pan_protocol_get_elevation(s, perr) returns the elevation of the
	 * camera relative to the remote model. If perr is not a null pointer
	 * then it will be set to zero if the returned elevation is invalid
	 * and non-zero if it is valid. An invalid elevation is returned when
	 * the camera is not directly above any part of the remote model.
	 *
	 * IMPLEMENTS GetElevation (2)
	 */

extern void pan_protocol_get_elevations(SOCKET, unsigned long, float *, float *, char *);
	/*
	 * pan_protocol_get_elevations(s, n, pv, rv, ev) computes the camera
	 * elevations relative to the remote model for each of the "n" camera
	 * positions "pv" and writes the results into the array "rv". The ith
	 * elevation rv[i] is only valid (the position pv[3*i] is over the
	 * model) when ev[i] is non-zero. Both rv and ev must be point to an
	 * array of "n" elements.
	 *
	 * IMPLEMENTS GetElevations (3)
	 */

extern void pan_protocol_lookup_point(SOCKET, float, float, float *, float *, float *, char *);
	/*
	 * h = pan_protocol_lookup_point(s, x, y, &px, &py, &pz, perr) returns
	 * the 3D position of the model (px, py, pz) under the pixel at
	 * coordinates (x, y) where (0, 0) represents the bottom left corner
	 * of the image and (1, 1) the top-right corner. If perr is not a null
	 * pointer then it will be set to zero if the returned point is invalid
	 * and non-zero if it is valid. An invalid point occurs when the centre
	 * of the specified pixel does not cover any part of the model.
	 *
	 * IMPLEMENTS LookupPoint (4)
	 */

extern void pan_protocol_lookup_points(SOCKET, unsigned long, float *, float *, char *);
	/*
	 * pan_protocol_lookup_points(s, n, pv, rv, ev) computes the 3D
	 * positions of each of the "n" pixels whose 2D positions are
	 * stored in "pv" and writes the 3D results into the array "rv".
	 * The ith position rv[3*i] is only valid if ev[i] is non-zero.
	 * The "rv" array must hold 3*n elements while the "ev" array must
	 * hold "n" elements.
	 *
	 * IMPLEMENTS LookupPoints (5)
	 */

extern void pan_protocol_get_point(SOCKET, float, float, float, float *, float *, float *, char *);
	/*
	 * pan_protocol_get_point(s, dx, dy, dz, &px, &py, &pz, perr) returns
	 * the 3D position (px, py, pz) of the model visible along direction
	 * (dx, dy, dz). If perr is not a null pointer then it will be set to
	 * zero if the returned point is invalid and non-zero if it is valid.
	 * An invalid point occurs when no part of the model is visible along
	 * the specified direction.
	 *
	 * IMPLEMENTS GetPoint (6)
	 */

extern void pan_protocol_get_points(SOCKET, unsigned long, float *, float *, char *);
	/*
	 * pan_protocol_get_points(s, n, pv, rv, ev) computes the 3D positions
	 * of the "n" points on the model visible along the directions pv[3*i]
	 * and writes the results into the array rv[]. Each pv[3*i] position
	 * is only valid if ev[i] is non-zero. The "rv" array must hold 3*n
	 * elements while the "ev" array must hold "n" elements.
	 *
	 * IMPLEMENTS GetPoints (7)
	 */

extern void *pan_protocol_echo(SOCKET, void *, unsigned long, unsigned long *);
	/*
	 * r = pan_protocol_echo(s, p, n, &m) is used to pass the array of n
	 * bytes starting at address p to the server. The reply data is
	 * returned as the array r whose length is written to m. The result
	 * array must be freed after use by calling free(r).
	 *
	 * IMPLEMENTS Echo (8)
	 */

extern unsigned char *pan_protocol_get_range_image(SOCKET, unsigned long *, float, float);
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

extern unsigned char *pan_protocol_get_range_texture(SOCKET, unsigned long *);
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

extern unsigned char *pan_protocol_get_viewpoint_by_degrees_s(SOCKET, float, float, float, float, float, float, unsigned long *);
	/*
	 * pan_protocol_get_viewpoint_by_degrees_s(s, x, y, z, yw, pi, rl, &size)
	 * is used to set the camera/viewpoint position to (x, y, z) and
	 * yaw/pitch/roll to (yw, pi, rl) and return an image from that
	 * position. Returns zero if a image could not be obtained from the
	 * server. The memory returned by the call is allocated by malloc()
	 * and may be released by free(). The size field will be updated with
	 * the number of bytes in the result array. Angles are in degrees.
	 *
	 * IMPLEMENTS GetViewpointByDegreesS (11)
	 */

extern unsigned char *pan_protocol_get_viewpoint_by_angle(SOCKET, float, float, float, float, float, float, unsigned long *);
	/*
	 * Backwards compatibility.
	 *
	 * IMPLEMENTS GetViewpointByAngle (11)
	 */

extern unsigned char *pan_protocol_get_viewpoint_by_angle_s(SOCKET, float, float, float, float, float, float, unsigned long);
	/*
	 * Backwards compatibility.
	 *
	 * IMPLEMENTS GetViewpointByAngleS (11)
	 */

extern unsigned char *pan_protocol_get_viewpoint_by_quaternion_s(SOCKET, float, float, float, float, float, float, float, unsigned long *);
	/*
	 * pan_protocol_get_viewpoint_by_quaternion_s(s,x,y,z,q0,q1,q2,q3,&size)
	 * is used to set the camera/viewpoint position to (x, y, z) and
	 * attitude as defined by the quaternion [q0, q1, q2, q3] and return
	 * an image from that position. Returns zero if a image could not be
	 * obtained from the server. The memory returned by the call is
	 * allocated by malloc() and may be released by free(). The size field
	 * will be updated with the number of bytes in the result array.
	 *
	 * IMPLEMENTS GetViewpointByQuaternionS (12)
	 */

extern unsigned char *pan_protocol_get_viewpoint_by_quaternion(SOCKET, float, float, float, float, float, float, float, unsigned long *);
	/*
	 * Backwards compatibility.
	 *
	 * IMPLEMENTS GetViewpointByQuaternion (12)
	 */

extern void pan_protocol_get_lidar_pulse_result(SOCKET, float, float, float, float, float, float, float *, float *);
	/*
	 * pan_protocol_get_lidar_pulse_result(s, x,y,z, dx,dy,dz, &r, &a) is
	 * used to obtain the result of a LIDAR pulse from position (x, y, z)
	 * along direction (dx, dy, dz). The range to the surface and the
	 * cosine of the incidence angle will be written to r and a on return.
	 *
	 * IMPLEMENTS GetLidarPulseResult (13)
	 */


extern float *pan_protocol_get_lidar_measurement(SOCKET, float,float,float, float,float,float,float, float,float,float, float,float,float, float,float,float, float,float,float, float,float,float, float,float,float, float *,float *, unsigned long *,unsigned long *, float *,float *, unsigned long *,unsigned long *, unsigned long *, unsigned long *, float *,float *, float *, float *, float *, float *, float *, float *);
	/*
	 * Backwards compatibility. See GetLidarMeasurementS (34)
	 *
	 * IMPLEMENTS GetLidarMeasurement (14)
	 */

extern float *pan_protocol_get_radar_response(SOCKET, unsigned long, unsigned long, unsigned long, unsigned long, float, float, float, float, float, float, float, float, float, float, float, float, float, float, float, unsigned long *, float *, float *, float *, float *, float *, float *, float *, float *, float *, float *, unsigned long *, unsigned long *, unsigned long *);
	/*
	 * v = pan_protocol_get_radar_response(s,fl,n,nr,ns,ox,oy,oz,vx,vy,vz,
	 *     q0,q1,q2,q3,bw,rmd,smd,rbs,sbs,
	 *     &st,&mxv,&totv,&offr,&offs,&bsr,&bss,&mnr,&mxr,&mns,&mxs,&nrb,
	 *     &nsb, &nused)
	 * is used to retrieve the RADAR response for a beam emitted from the
	 * point (ox,oy,oz) moving with velocity (vx,vy,vz) with axis
	 * (q0,q1,q2,q3) and width bw degrees. The beam is sampled n times and
	 * the results are integrated using a 2D histogram with nr range bins
	 * and ns speed bins. The fl argument defines flags for the simulation
	 * with the meanings of each bit defined as follows:
	 *      bit  0: if set then zero align the of the range histograms
	 *      bit  1: if set then centre align the range histograms on rmd
	 *      bit  2: if set then round range histogram width to power of 10
	 *      bit  3: if set then use fixed range histogram bin size rbs
	 *      bit  4: if set then zero align the speed histograms
	 *      bit  5: if set then centre align the speed histograms on smd
	 *      bit  6: if set then round speed histogram width to power of 10
	 *      bit  7: if set then use fixed speed histogram bin size sbs
	 *      bit  8: if set then surface slope effects are ignored
	 *      bit  9: if set then each sample is worth 1 not 1/n
	 * The status of the response is returned in st with the maximum signal
	 * value in mxv. The range associated with the left edge of the first
	 * histogram bin is offr. The minimum and maximum ranges before being
	 * clipping by the histogram are minr and maxr respectively; the
	 * minimum and maximum speeds is mins and maxs respectively. The size
	 * of each bin is bs. The number of bins actually created is nrb and
	 * nsb. The 2D array of nrb*nsb bins is returned as v consisting of
	 * nrb range values for the first speed histogram, then the next nrb
	 * range values for the second speed histogram etc. The sum of all
	 * elements of v is returned in totv. The number of samples actually
	 * used in the histogram (the number that hit a target) is returned in
	 * nused.
	 *
	 * IMPLEMENTS GetRadarResponse (15)
	 */

extern unsigned char *pan_protocol_get_viewpoint_by_degrees_d(SOCKET, double, double, double, double, double, double, unsigned long *);
	/*
	 * pan_protocol_get_viewpoint_by_degrees_d(s, x, y, z, yw, pi, rl, &size)
	 * is used to set the camera/viewpoint position to (x, y, z) and
	 * yaw/pitch/roll to (yw, pi, rl) and return an image from that
	 * position. Returns zero if a image could not be obtained from the
	 * server. The memory returned by the call is allocated by malloc()
	 * and may be released by free(). The size field will be updated with
	 * the number of bytes in the result array. Angles are in degrees.
	 *
	 * IMPLEMENTS GetViewpointByDegreesD (16)
	 */

extern unsigned char *pan_protocol_get_viewpoint_by_angle_d(SOCKET, double, double, double, double, double, double, unsigned long *);
	/*
	 * Backwards compatibility.
	 *
	 * IMPLEMENTS GetViewpointByAngleD (16)
	 */

extern unsigned char *pan_protocol_get_viewpoint_by_quaternion_d(SOCKET, double, double, double, double, double, double, double, unsigned long *);
	/*
	 * pan_protocol_get_viewpoint_by_quaternion_d(s,x,y,z,q0,q1,q2,q3,&size)
	 * is used to set the camera/viewpoint position to (x, y, z) and
	 * attitude as defined by the quaternion [q0, q1, q2, q3] and return
	 * an image from that position. Returns zero if a image could not be
	 * obtained from the server. The memory returned by the call is
	 * allocated by malloc() and may be released by free(). The size field
	 * will be updated with the number of bytes in the result array.
	 *
	 * IMPLEMENTS GetViewpointByQuaternionD (17)
	 */

extern joint_data* pan_protocol_get_joints(SOCKET, unsigned long, unsigned long*);
	/*
	 * pan_protocol_get_joints(i, n) gets an array of joint_data
	 * with one entry for each joint of object i. The location pointed
	 * to by n is filled with the number of entries in the joint list. 
	 *      id:   The handle used to identify the joint
	 *      name: Descriptive name for the joint
	 *  type: 0 = general joint
	 *        1 = revolute joint
	 *        2 = prismatic joint
	 *
	 * If an invalid object is specified, an empty list is
	 * returned and n = 0
	 *
	 * To free the joint list, first free each 'name' field. 
	 *
	 * IMPLEMENTS GetJoints (18)
	 */

extern void pan_protocol_get_joint_config(SOCKET, unsigned long, unsigned long, double*);
	/*
	 * pan_prototcol_get_joint_config(o,j, c) returns the configuration
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

extern frame_data* pan_protocol_get_frames(SOCKET, unsigned long, unsigned long *);
	/*
	 * pan_protocol_get_frames(o,n) returns an array of frame_data, with one
	 * entry for each frame of object o. the variable pointed to by n is 
	 * set to the number of frames.
	 *
	 *	Each entry has the following fields:
	 *		id: The handle used to identify the frame
	 *	  name: A descriptive label for the frame.
	 *
	 * If an invalid object is specified, an empty list is
	 * returned and n = 0
	 *
	 * To free the joint list, first free each 'name' field. 
	 *
	 *
	 * IMPLEMENTS GetFrames (20)
	 */

extern void pan_protocol_get_frame(SOCKET, unsigned long, unsigned long, double*);
	/*
	 * pan_protocol_get_frame(o,i,f) write frame i of object o to f, which
	 * should point to an array of 12 doubles. The frame is returned as
	 * follows
	 *	[fi, fj, fk, ui, uj, uk, ri, rj, rk, x, y, z].
	 * Where f is the forward vector, u is up and r is right, (x,y,z) is
	 * the position of the frame origin. 
	 *
	 * If an invalid object or frame is specified, then each element of f
	 * is set to 0.
	 *
	 * IMPLEMENTS GetFrame (21)
	 */

extern void pan_protocol_get_frame_as_radians(SOCKET, unsigned long, unsigned long, double*);
	/*
	 * pan_protocol_get_frame_as_radians(o,i,v) writes camera parameters
	 * for frame i of object o to v, which should point to an array of
	 * 6 doubles. The viewpoint is returned as follows:
	 *	[yaw,pitch,roll,x,y,z]
	 * This corresponds to a camera located at the origin of the frame
	 * and aligned with it (looking along the forward vector etc). These
	 * can be used directly with GetViewPointByRadians.
	 *
	 * If an invalid object or frame is specified, then each element of v
	 * is set to 0.
	 *
	 * IMPLEMENTS GetFrameAsRadians (22)
	 */

extern void pan_protocol_get_frame_viewpoint_by_angle(SOCKET, unsigned long, unsigned long, double*);
	/*
	 * Backwards compatibility.
	 *
	 * IMPLEMENTS GetFrameViewpointByAngle (22)
	 */

extern float pan_protocol_get_surface_elevation(SOCKET, unsigned char, float, float, char*);
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

extern void pan_protocol_get_surface_elevations(SOCKET, unsigned char, unsigned long, float *, float*, char*);
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


extern void pan_protocol_get_surface_patch(SOCKET, unsigned char, float, float, unsigned long, unsigned long, float, float, float*, char*);
	/*
	 * pan protocol_get_surface_patch(s,b,cx,cy,nx,ny,d,theta,rv,ev).
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

extern unsigned char *pan_protocol_get_viewpoint_by_radians(SOCKET, double, double, double, double, double, double, unsigned long *);
	/*
	 * pan_protocol_get_viewpoint_by_radians(s, x, y, z, yw, pi, rl, &size)
	 * is used to set the camera/viewpoint position to (x, y, z) and
	 * yaw/pitch/roll to (yw, pi, rl) and return an image from that
	 * position. Returns zero if a image could not be obtained from the
	 * server. The memory returned by the call is allocated by malloc()
	 * and may be released by free(). The size field will be updated with
	 * the number of bytes in the result array. Angles are in radians.
	 *
	 * IMPLEMENTS GetViewpointByRadians (26)
	 */

extern void pan_protocol_quit(SOCKET);
	/*
	 * pan_protocol_quit(s), will cause the server to quit.
	 *
	 * IMPLEMENTS Quit (27)
	 */

extern unsigned char *pan_protocol_get_viewpoint_by_frame(SOCKET, unsigned long, unsigned long, unsigned long *);
	/*
	 * ptr = pan_protocol_get_viewpoint_by_frame(s, oid, fid, &size)
	 * requests an image from the frame fid within dynamic object oid.
	 * Returns zero if an image could not be obtained from the server.
	 * Returns an empty (0 byte) image if the specified frame does not
	 * exist. The memory returned by the call is allocated by malloc()
	 * and may be released by free(). The size field will be updated
	 * with the number of bytes in the result array.
	 *
	 * IMPLEMENTS GetViewpointByFrame (28)
	 */

extern int pan_protocol_get_camera_properties(SOCKET, unsigned long, unsigned long *, unsigned long *, double *, double *, double *, double *, double *, double *, double *, double *, double *);
	/*
	 * v=pan_protocol_get_camera_properties(s,c,&w,&h,&hf,&vf,&px,&py,&pz,&q0,&q1,&q2,&3)
	 * is used to obtain the properties of camera c. The variables w and h
	 * are updated with the image dimensions, hf and vf with the horizontal
	 * and vertical field of view angles (in radians), the (px,py,pz)
	 * variables with the camera position and the (q0,q1,q2,q3) variables
	 * with the camera attitude quaternion (q0 is the scalar term). The
	 * return value v is 0 if the camera is invalid and non-zero if valid.
	 * A negative value indicates that temporary storage allocation failed.
	 *
	 * IMPLEMENTS GetCameraProperties (29)
	 */

extern unsigned char *pan_protocol_get_viewpoint_by_camera(SOCKET, unsigned long, unsigned long *);
	/*
	 * ptr = pan_protocol_get_viewpoint_by_camera(s, cid, &size)
	 * requests an image from camera cid. Returns zero if an image could
	 * not be obtained from the server. Returns an empty (0 byte) image
	 * if the specified camera does not exist. The memory returned by
	 * the call is allocated by malloc() and may be released by free().
	 *The size field will be updated with the number of bytes in the
	 * result array.
	 *
	 * IMPLEMENTS GetViewpointByCamera (30)
	 */


extern void pan_protocol_get_view_as_dem(SOCKET, unsigned long, unsigned char, unsigned long, unsigned long, float, float, float, float*, char*);
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


extern float *pan_protocol_get_lidar_measurement_d(SOCKET, double,double,double, double,double,double,double, double,double,double, double,double,double, double,double,double, double,double,double, double,double,double, double,double,double, float *,float *, unsigned long *,unsigned long *, float *,float *, unsigned long *,unsigned long *, unsigned long *, unsigned long *, float *,float *, float *, float *, float *, float *, float *, float *);
	/*
	 * v = pan_protocol_get_lidar_measurement_d(
	 *        s, px,py,pz, q0,q1,q2,q3, vx,vy,vz, rx,ry,rx,
	 *        ax,ay,az, sx,sy,sz, jx,jy,jz, tx,ty,tz,  
	 *        &fx,&fy, &nx,&ny, &tx,&ty, &n,&m, &t, &fl, &az,&el, &th,
	 *        &faz,&fel, &toff, &taz0, &tel0) is
	 * is used to request a LIDAR scan from position (px,py,pz) with
	 * attitude quaternion (q0,q1,q2,q3), linear velocity (vx,vy,vz) and
	 * angular velocity (rx,ry,rz), linear acceleration (ax,ay,az), angular
	 * acceleration (sx,sy,sz), linear jerk (jx,jy,jz) and angular jerk
	 * (tx,ty,tz). The return value v is a pointer to an array of scan
	 * results while the LIDAR emitter/detector settings are written to fx
	 * etc. If the scan has W by H beams with sub-sampling factors N and M
	 * and B blocks of results then "v" will contain 2*B*(W*N)*(H*M)
	 * floating point values representing the 2D scan results at the sub-
	 * sampling resolution NxM. The number of blocks B depends on the
	 * results requested in the LIDAR emitter/detector settings flag field.
	 *
	 * This function automatically converts from PANGU network floats to
	 * native floats before returning.
	 *
	 * IMPLEMENTS GetLidarMeasurementD (32)
	 */

extern double pan_protocol_get_time_tag(SOCKET, char *);
	/*
	 * t = pan_protocol_get_time_tag(s, perr) returns the time, t, that the
	 * last image was requested at. The time is measured in microseconds
	 * since 00:00:00 UCT 01 January 1970. If perr is not a null pointer
	 * then it will be set to zero if the returned timetag is invalid
	 * and non-zero if it is valid.
	 *
	 * IMPLEMENTS GetTimeTag (33)
	 */

extern float *pan_protocol_get_lidar_measurement_s(SOCKET, float,float,float, float,float,float,float, float,float,float, float,float,float, float,float,float, float,float,float, float,float,float, float,float,float, float *,float *, unsigned long *,unsigned long *, float *,float *, unsigned long *,unsigned long *, unsigned long *, unsigned long *, float *,float *, float *, float *, float *, float *, float *, float *);
	/*
	 * v = pan_protocol_get_lidar_measurement_s(
	 *        s, px,py,pz, q0,q1,q2,q3, vx,vy,vz, rx,ry,rx,
	 *        ax,ay,az, sx,sy,sz, jx,jy,jz, tx,ty,tz,  
	 *        &fx,&fy, &nx,&ny, &tx,&ty, &n,&m, &t, &fl, &az,&el, &th,
	 *        &faz,&fel, &toff, &taz0, &tel0) is
	 * is used to request a LIDAR scan from position (px,py,pz) with
	 * attitude quaternion (q0,q1,q2,q3), linear velocity (vx,vy,vz) and
	 * angular velocity (rx,ry,rz), linear acceleration (ax,ay,az), angular
	 * acceleration (sx,sy,sz), linear jerk (jx,jy,jz) and angular jerk
	 * (tx,ty,tz). The return value v is a pointer to an array of scan
	 * results while the LIDAR emitter/detector settings are written to fx
	 * etc. If the scan has W by H beams with sub-sampling factors N and M
	 * and B blocks of results then "v" will contain 2*B*(W*N)*(H*M)
	 * floating point values representing the 2D scan results at the sub-
	 * sampling resolution NxM. The number of blocks B depends on the
	 * results requested in the LIDAR emitter/detector settings flag field.
	 *
	 * This function automatically converts from PANGU network floats to
	 * native floats before returning.
	 *
	 * IMPLEMENTS GetLidarMeasurementS (34)
	 */

extern float *pan_protocol_get_lidar_snapshot(SOCKET, unsigned long, double, double, double, double, double, double, double, unsigned long *, unsigned long *);
	/*
	 * v = pan_protocol_get_lidar_snapshot(
	 *        s, cid, px,py,pz, q0,q1,q2,q3, &t,&r, &w,&h) is
	 * is used to request a LIDAR scan from position (px,py,pz) with
	 * attitude quaternion (q0,q1,q2,q3).
	 * The return value v is a pointer to an array of floats representing a
	 * raw image with top left origin. w and h are unsigned longs providing
	 * the width and height of the image. Each pixel of the image has three
	 * floats: the first is the range (i.e. the distance between the 
	 * surface and the lidar scanner); the second is the angle between the
	 * surface normal and scan direction of that pixel; and the third is a
	 * hit/miss flag where 0 represents a miss and 1 represents a hit.
	 * So v points to w*h*3 floats.
	 * 
	 * This function automatically converts from MSB_REAL_32 floats to
	 * native floats before returning.
	 *
	 * IMPLEMENTS GetLidarSnapshot (35)
	 */

extern void pan_protocol_set_viewpoint_by_degrees_s(SOCKET, float, float, float, float, float, float);
	/*
	 * pan_protocol_set_viewpoint_by_degrees_s(s, x, y, z, yw, pi, rl)
	 * is used to set the camera/viewpoint position to (x, y, z) and
	 * attitude yaw/pitch/roll to (yw, pi, rl). Angles in degrees.
	 *
	 * IMPLEMENTS SetViewpointByDegreesS (256)
	 */

extern void pan_protocol_set_viewpoint_by_angle(SOCKET, float, float, float, float, float, float);
	/*
	 * Backwards compatibility.
	 *
	 * IMPLEMENTS SetViewpointByAngle (256)
	 */

extern void pan_protocol_set_viewpoint_by_angle_s(SOCKET, float, float, float, float, float, float rl);
	/*
	 * Backwards compatibility.
	 *
	 * IMPLEMENTS SetViewpointByAngleS (256)
	 */

extern void pan_protocol_set_viewpoint_by_quaternion_s(SOCKET, float, float, float, float, float, float, float);
	/*
	 * pan_protocol_set_viewpoint_by_quaternion_s(s,x,y,z,q0,q1,q2,q3)
	 * is used to set the camera/viewpoint position to (x, y, z) and
	 * attitude as defined by the quaternion [q0, q1, q2, q3].
	 *
	 * IMPLEMENTS SetViewpointByQuaternionS (257)
	 */

extern void pan_protocol_set_viewpoint_by_quaternion(SOCKET, float, float, float, float, float, float, float);
	/*
	 * Backwards compatibility.
	 *
	 * IMPLEMENTS SetViewpointByQuaternion (257)
	 */

extern void pan_protocol_set_ambient_light(SOCKET, float, float, float);
	/*
	 * pan_protocol_set_ambient_light(s, r, g, b) is used to set the colour
	 * and intensity of ambient light in the red, green and blue channels
	 * to (r, g, b).
	 *
	 * IMPLEMENTS SetAmbientLight (258)
	 */

extern void pan_protocol_set_sun_colour(SOCKET, float, float, float);
	/*
	 * pan_protocol_set_sun_colour(s, r, g, b) is used to set the colour
	 * and intensity of the Sun in the red, green and blue channels to
	 * (r, g, b).
	 *
	 * IMPLEMENTS SetSunColour (259)
	 */

extern void pan_protocol_set_sky_type(SOCKET, unsigned long);
	/*
	 * pan_protocol_set_sky_type(s, t) is used to set the sky background
	 * type to "t" where "t" may hold one of the following values:
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

extern void pan_protocol_set_field_of_view_by_degrees(SOCKET, float);
	/*
	 * pan_protocol_set_field_of_view_by_degrees(s, f) is used to set
	 * the angular field of view width to "f" degrees. The angular field
	 * of view height is determined from the width and the pixel aspect
	 * ratio.
	 *
	 * IMPLEMENTS SetFieldOfViewByDegrees (261)
	 */

extern void pan_protocol_set_field_of_view(SOCKET, float);
	/*
	 * Backwards compatibility.
	 *
	 * IMPLEMENTS SetFieldOfView (261)
	 */

extern void pan_protocol_set_aspect_ratio(SOCKET, float);
	/*
	 * pan_protocol_set_aspect_ratio(s, r) is used to set the angular
	 * aspect ratio of each pixel. This is used in conjunction with the
	 * angular field of view width to determine the angular field of view
	 * height.
	 *
	 * IMPLEMENTS SetAspectRatio (262)
	 */

extern void pan_protocol_set_boulder_view(SOCKET, unsigned long, int);
	/*
	 * pan_protocol_set_boulder_view(s, t, tex) is used to set the boulder
	 * rendering method to "t" and to enable or disable boulder texturing
	 * via "tex".
	 *
	 * IMPLEMENTS SetBoulderView (263)
	 */

extern void pan_protocol_set_surface_view(SOCKET, unsigned long, int, int);
	/*
	 * pan_protocol_set_surface_view(s, t, tex, det) is used to set the
	 * surface rendering method to "t" and to enable or disable surface
	 * texturing and surface detailing.
	 *
	 * IMPLEMENTS SetSurfaceView (264)
	 */

extern void pan_protocol_set_lidar_parameters(SOCKET, float, float, unsigned long, unsigned long, float, float, unsigned long, unsigned long, unsigned long, unsigned long, float, float, float, float, float, float, float, float, float, float);
	/*
	 * pan_protocol_set_lidar_parameters(s, fx,fy, nx,ny, tx,ty, n, m, t,
	 *      fl, az,el, th, wx, wy, faz, fel, toff, taz0, tel0)
	 * is used to configure the LIDAR emitter/detector with field of view
	 * (fx, fy) degrees horizontally and vertically, an emitter or detector
	 * grid of nx by ny beams, and subsampling of n by m samples. The scan
	 * type is "t" where:
	 *      0: TV scan (left-to-right, top-to-bottom)
	 *      1: mode 01 (LiGNC project zig-zag scan, azi mirror before ele)
	 *      2: mode 02 (LiGNC project zig-zag scan, ele mirror before azi)
	 *      3: mode 01 (ILT 1D sinusoidal scan, azi mirror before ele)
	 *      4: mode 02 (ILT 1D sinusoidal scan, ele mirror before azi)
	 *      5: mode 01 (LAPS zig-zag scan, azi mirror before ele)
	 *      6: mode 02 (LAPS zig-zag scan, ele mirror before azi)
	 *      7: General scan (Uses samples given via pan_protocol_set_lidar_scan())
	 * The centre of the scan has LIDAR azimuth/elevation (az,el) and the
	 * beam half-angle is th degrees. The LIDAR flags fl is a bit vector:
	 *      bit  0: return range/slope results if set
	 *      bit  1: return azimuth/elevation values if set
	 *      bit  2: return corner cube range/slope results if set
	 *      bit  3: return time-of-pulse-emission values
	 *      bit 16: if set the azimuth scan starts with decreasing azimuth
	 *      bit 17: if set the elevation scan starts with decreasing
	 *              elevation and the results are returned in that order
	 *              (bottom to top)
	 *      bit 18: if set (normally used instead of bit 17) the results
	 *              are returned in top-to-bottom order but the pulses are
	 *              emittted in bottom-to-top order
	 * All other bits must be clear.
	 * Each detector pixel is wx by wy degrees in size. Note that when
	 *    n and/or m are greater than 1 these are the sizes of the
	 *    subsample pixels not the full pixel.
	 * For the ILT scans (type 3 and 4) the faz parameter defines the
	 *    azimuth sinusoial scanning frequency.
	 * For the LAPS scans (type 5 and 6) the faz parameter defines the
	 *    x-axis zig-zag scanning frequency and the fel parameter defines
	 *    the y-axis zig-zag scanning frequency. The (taz0, tel0)
	 *    parameters define the offset of the zig-zag scanning patterns.
	 *
	 * IMPLEMENTS SetLidarParameters (265)
	 */

extern void pan_protocol_set_corner_cubes_s(SOCKET, unsigned long, unsigned long, float *);
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

extern void pan_protocol_set_corner_cubes(SOCKET, unsigned long, unsigned long, float *);
	/*
	 * Backwards compatibility.
	 *
	 * IMPLEMENTS SetCornerCubes (266)
	 */

extern void pan_protocol_set_corner_cube_attitude(SOCKET, float, float, float, float, float, float, float, float, float, float, float, float, float);
	/*
	 * pan_protocol_set_corner_cube_attitude(
	 *        s, q0,q1,q2,q3, rx,ry,rx, ax,ay,az, jx,jy,jz) is used
	 * to define the attitude (q0,q1,q2,q3), angular velocity (rx,ry,rz),
	 * angular acceleration (ax,ay,az) and angular jerk (jx,jy,jz) of the
	 * corner cube lattice at the centre of all future LIDAR frames.
	 *
	 * IMPLEMENTS SetCornerCubeAttitude (267)
	 */

extern void pan_protocol_set_viewpoint_by_degrees_d(SOCKET, double, double, double, double, double, double);
	/*
	 * pan_protocol_set_viewpoint_by_degrees_d(s, x, y, z, yw, pi, rl)
	 * is used to set the camera/viewpoint position to (x, y, z) and
	 * attitude yaw/pitch/roll to (yw, pi, rl). Angles in degrees.
	 *
	 * IMPLEMENTS SetViewpointByDegreesD (268)
	 */

extern void pan_protocol_set_viewpoint_by_angle_d(SOCKET, double, double, double, double, double, double);
	/*
	 * Backwards compatibility.
	 *
	 * IMPLEMENTS SetViewpointByAngleD (268)
	 */

extern void pan_protocol_set_viewpoint_by_quaternion_d(SOCKET, double, double, double, double, double, double, double);
	/*
	 * pan_protocol_set_viewpoint_by_quaternion_d(s,x,y,z,q0,q1,q2,q3)
	 * is used to set the camera/viewpoint position to (x, y, z) and
	 * attitude as defined by the quaternion [q0, q1, q2, q3].
	 *
	 * IMPLEMENTS SetViewpointByQuaternionD (269)
	 */

extern void pan_protocol_set_object_position_attitude(SOCKET, unsigned long, double, double, double, double, double, double, double);
	/*
	 * pan_protocol_set_object_position(i,s,x,y,z,q0,q1,q2,q3)
	 * is used to set object i position to (x, y, z) and
	 * attitude as defined by the quaternion [q0, q1, q2, q3].
	 *
	 * IMPLEMENTS SetObjectPositionAttitude (270)
	 */

extern void pan_protocol_set_object_position(SOCKET, unsigned long, double, double, double, double, double, double, double);
	/*
	 * Backwards compatibility.
	 *
	 * IMPLEMENTS SetObjectPositionAttitude (270)
	 */

extern void pan_protocol_set_sun_by_degrees(SOCKET, double, double, double);
	/*
	 * pan_protocol_set_sun_by_degrees(r,a,e) is used to set the spherical
	 * polar position of the Sun to (r,a,e) degrees.
	 *
	 * IMPLEMENTS SetSunByDegrees (271)
	 */

extern void pan_protocol_set_sun_position(SOCKET, double, double, double);
	/*
	 * Backwards compatibility.
	 *
	 * IMPLEMENTS SetSunPosition (271)
	 */

extern void pan_protocol_set_joint_config(SOCKET, unsigned long, unsigned long, double[9]);
	/*
	 * pan_protocol_set_joint_config(o,j,c) sets the configuration of
	 * joint j of object o to the values in array c.
	 *
	 * c must always be of length 9. Meanings are as described for
	 * pan_protocol_get_joint_config(). Attempts to configure non-existing
	 * joints/angles are ignored.
	 *
	 * IMPLEMENTS SetJointConfig (272)
	 */

extern void pan_protocol_set_star_quaternion(SOCKET, double, double, double, double);
	/*
	 * pan_protocol_set_star_quaternion(s, q0, q1, q2, q3) is used to set
	 * the attitude quaternion of the star sphere to [q0, q1, q2, q3].
	 *
	 * IMPLEMENTS SetStarQuaternion (273)
	 */

extern void pan_protocol_set_star_magnitudes(SOCKET, double);
	/*
	 * pan_protocol_set_star_magnitudes(s, m) is used to set the reference
	 * star magnitude to m.
	 *
	 * IMPLEMENTS SetStarMagnitudes (274)
	 */

extern void pan_protocol_set_secondary_by_degrees(SOCKET, double, double, double);
	/*
	 * pan_protocol_set_secondary_by_degrees(r,a,e) is used to set the
	 * spherical polar position of the Secondary light source to (r,a,e)
	 * degrees.
	 *
	 * IMPLEMENTS SetSecondaryByDegrees (275)
	 */

extern void pan_protocol_set_global_time(SOCKET s, double t);
	/*
	 * pan_protocol_set_global_time(s, t) is used to set the current
	 * global time value to t.
	 *
	 * IMPLEMENTS SetGlobalTime (276)
	 */

extern void pan_protocol_set_object_view(SOCKET, unsigned long, unsigned long);
	/*
	 * pan_protocol_set_object_view(obj, t) is used to set the rendering
	 * method of dynamic object "obj" to type "t"
	 *
	 * IMPLEMENTS SetObjectView (277)
	 */

extern void pan_protocol_set_viewpoint_by_radians(SOCKET, double, double, double, double, double, double);
	/*
	 * pan_protocol_set_viewpoint_by_radians(s, x, y, z, yw, pi, rl) is used
	 * to set the camera/viewpoint position to (x, y, z) and attitude
	 * yaw/pitch/roll to (yw, pi, rl). Angles in radians.
	 *
	 * IMPLEMENTS SetViewpointByRadians (278)
	 */

extern void pan_protocol_set_field_of_view_by_radians(SOCKET, float);
	/*
	 * pan_protocol_set_field_of_view_by_radians(s, f) is used to set the
	 * angular field of view width to "f" radians. The angular field of
	 * view height is determined from the width and the pixel aspect ratio.
	 *
	 * IMPLEMENTS SetFieldOfViewByRadians (279)
	 */

extern void pan_protocol_set_sun_by_radians(SOCKET, double, double, double);
	/*
	 * pan_protocol_set_sun_by_radians(r,a,e) is used to set the spherical
	 * polar position of the Sun to (r,a,e) radians.
	 *
	 * IMPLEMENTS SetSunByRadians (280)
	 */

extern void pan_protocol_set_secondary_by_radians(SOCKET, double, double, double);
	/*
	 * pan_protocol_set_secondary_by_radians(r,a,e) is used to set the
	 * spherical polar position of the Secondary light source to (r,a,e)
	 * radians.
	 *
	 * IMPLEMENTS SetSecondaryByRadians (281)
	 */

extern void pan_protocol_set_sky_rgb(SOCKET, float, float, float);
	/*
	 * pan_protocol_set_sky_rgb(s, r, g, b) is used to set the RGB colour
	 * for sky mode 8 to (r,g,b).
	 *
	 * IMPLEMENTS SetSkyRGB (282)
	 */

extern void pan_protocol_set_sky_cie(SOCKET, float, float, float);
	/*
	 * pan_protocol_set_sky_cie(s, x, y, Y) is used to set the CIE colour
	 * for sky mode 9 to (x,y,Y).
	 *
	 * IMPLEMENTS SetSkyCIE (283)
	 */

extern void pan_protocol_set_atmosphere_tau(SOCKET, float, float, float, float, float, float);
	/*
	 * pan_protocol_set_atmosphere_tau(s, mr,mg,mb, rr,rg,rb) is used to
	 * set the optical depth values (Tau). The (mr,mg,mb) value is the
	 * Mie scattering depths (aerosols) while the (rr,rg,rb) value is the
	 * Rayleigh scattering depths (gas).
	 *
	 * IMPLEMENTS SetAtmosphereTau (284)
	 */

extern void pan_protocol_set_global_fog_mode(SOCKET, unsigned long);
	/*
	 * pan_protocol_set_global_fog_mode(s, mode) is used to set the global
	 * fog mode to mode.
	 *
	 * IMPLEMENTS SetGlobalFogMode (285)
	 */

extern void pan_protocol_set_global_fog_properties(SOCKET, double, double, double, double);
	/*
	 * pan_protocol_set_global_fog_properties(s,r,d,l0,l1) is used to set
	 * the global fog properties. The r parameter defines the sky dome
	 * radius (the effective limit of the atmosphere), the d parameter
	 * defines the density (for exponential modes) and the (l0,l1)
	 * parameters define the start and end distances of linear fog.
	 *
	 * IMPLEMENTS SetGlobalFogProperties (286)
	 */

extern void pan_protocol_set_atmosphere_mode(SOCKET, unsigned long, unsigned long, unsigned long);
	/*
	 * pan_protocol_set_atmosphere_mode(s,sm,gm,am) is used to set the
	 * atmosphere rendering modes. The sm parameter defines the sky mode,
	 * the gm parameter defines the ground mode and the am parameter
	 * defines the attenuation mode.
	 *
	 * IMPLEMENTS SetAtmosphereMode (287)
	 */

extern void pan_protocol_select_camera(SOCKET, unsigned long);
	/*
	 * pan_protocol_select_camera(s,c) is used to make camera c the
	 * current camera for GetImage, LookupPoints etc.
	 *
	 * IMPLEMENTS SelectCamera (288)
	 */

extern void pan_protocol_bind_light_to_camera(SOCKET, unsigned long, unsigned long, unsigned char);
	/*
	 * pan_protocol_bind_light_to_camera(s,0,c,e) is used to associated
	 * the camera light with the camera whose ID is c. If e is non-zero
	 * then the light will be on otherwise it will be off. The second
	 * parameter must be zero (reserved for future use).
	 *
	 * IMPLEMENTS BindLightToCamera (289)
	 */

extern void pan_protocol_configure_light_by_degrees(SOCKET, unsigned long, double, double, double, double, double);
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

extern void pan_protocol_configure_light_by_radians(SOCKET, unsigned long, double, double, double, double, double);
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

extern void pan_protocol_set_light_position_direction(SOCKET, unsigned long, double, double, double, double, double, double);
	/*
	 * pan_protocol_configure_light_by_degrees(s,0,ox,oy,oz,dx,dy,dz) is
	 * used to set the position of the camera light to (ox,oy,oz) and the
	 * direction to (dx,dy,dz). These are coordinates in the frame of the
	 * associated camera woth x=right, y=down, z=forwards.
	 *
	 * IMPLEMENTS SetLightPositionDirection (292)
	 */

extern void pan_protocol_render_to_hold_buffer(SOCKET, unsigned long, unsigned long);
	/*
	 * pan_protocol_render_to_hold_buffer(s,cid,bid) is used to render the
	 * view of camera cid to the hold buffer. The bid parameter is unused
	 * (reserved for future use) and must be set to 0.
	 *
	 * IMPLEMENTS RenderToHoldBuffer (293)
	 */

extern void pan_protocol_display_hold_buffer(SOCKET, unsigned long);
	/*
	 * pan_protocol_display_hold_buffer(s,bid) is used to update the
	 * display with the hold buffer contents. The bid parameter is unused
	 * (reserved for future use) and must be set to 0.
	 *
	 * IMPLEMENTS DisplayHoldBuffer (294)
	 */

extern void pan_protocol_set_corner_cubes_d(SOCKET, unsigned long, unsigned long, double *);
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

extern void pan_protocol_set_projection_mode(SOCKET, unsigned long, unsigned long);
	/*
	 * pan_protocol_set_projection_mode(s,cid,mode) is used to set the
	 * projection mode for camera cid. If mode is 0 then perspective is used
	 * else if mode is 1 then orthographic projection is used.
	 *
	 * IMPLEMENTS SetProjectionMode (296)
	 */

extern void pan_protocol_set_ortho_field_of_view(SOCKET, unsigned long, double, double);
	/*
	 * pan_protocol_set_ortho_field_of_view(s,cid,w,h) is used to set the
	 * orthographic projection field of view for camera cid. The width, w, 
	 * and height, h, should be given in meters.
	 *
	 * IMPLEMENTS SetOrthoFieldOfView (297)
	 */

extern void pan_protocol_set_lidar_scan(SOCKET, unsigned long, unsigned long, double *);
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


extern void pan_protocol_set_camera_motion(SOCKET, unsigned long, double, double, double, double, double, double, double, double, double, double, double, double, double, double, double, double, double, double);
	/*
	 * pan_protocol_set_camera_motion(s,cid,vx,vy,vz,rx,ry,rz,ax,ay,az,sx,sy,sz,jx,jy,jz,tx,ty,tz)
	 * is used to tell the server that camera cid has linear velocity
	 * (vx,vy,vz), linear acceleration (ax,ay,az) and linear jerk
	 * (jx,jy,jz). The angular velocity is (r0,r1,r2), angular
	 * acceleration (s0,s1,s2) and angular jerk is (tx,ty,tz). The server
	 * will compute the position and attitude of the camera using these
	 * parameters and equations of motion for each sub-frame rendered for
	 * motion blur/rolling shutter images. The angular motion is that of
	 * the axis of the attitude quaternion where +z is camera bore. The
	 * magnitude of the axis is the rotation angle in radians.
	 *
	 * IMPLEMENTS SetCameraMotion (299)
	 */


/* Below are the the pan_net_* functions; most of which are split between
   a TX (transmission) function and an RX (reception) function. */

extern char *pan_net_safety_checks(void);

extern char *pan_net_want(SOCKET, unsigned long);

extern char *pan_net_start_TX(SOCKET);
/*           pan_net_start_RX() is not required.  */

extern char *pan_net_finish_TX(SOCKET);
/*           pan_net_finish_RX() is not required.  */

extern char *         pan_net_get_image_TX(SOCKET, unsigned long *);
extern unsigned char *pan_net_get_image_RX(SOCKET, unsigned long *);

extern char *pan_net_get_elevation_TX(SOCKET);
extern float pan_net_get_elevation_RX(SOCKET, char *);

extern char *pan_net_get_elevations_TX(SOCKET, unsigned long, float *);
extern void  pan_net_get_elevations_RX(SOCKET, float *, char *);

extern char *pan_net_lookup_point_TX(SOCKET, float, float);
extern void  pan_net_lookup_point_RX(SOCKET, float *, float *, float *, char *);

extern char *pan_net_lookup_points_TX(SOCKET, unsigned long, float *);
extern void  pan_net_lookup_points_RX(SOCKET, float *, char *);

extern char *pan_net_get_point_TX(SOCKET, float, float, float);
extern void  pan_net_get_point_RX(SOCKET, float *, float *, float *, char *);

extern char *pan_net_get_points_TX(SOCKET, unsigned long, float *);
extern void  pan_net_get_points_RX(SOCKET, float *, char *);

extern char *pan_net_echo_TX(SOCKET, void *, unsigned long);
extern void *pan_net_echo_RX(SOCKET, unsigned long *);

extern char *         pan_net_get_range_image_TX(SOCKET, float, float);
extern unsigned char *pan_net_get_range_image_RX(SOCKET, unsigned long *);

extern char *         pan_net_get_range_texture_TX(SOCKET);
extern unsigned char *pan_net_get_range_texture_RX(SOCKET, unsigned long *);

extern char *         pan_net_get_viewpoint_by_degrees_s_TX(SOCKET, float, float, float, float, float, float);
extern unsigned char *pan_net_get_viewpoint_by_degrees_s_RX(SOCKET, unsigned long *);

extern char *         pan_net_get_viewpoint_by_quaternion_s_TX(SOCKET, float, float, float, float, float, float, float);
extern unsigned char *pan_net_get_viewpoint_by_quaternion_s_RX(SOCKET, unsigned long *);

extern char *pan_net_get_lidar_pulse_result_TX(SOCKET, float, float, float, float, float, float);
extern void  pan_net_get_lidar_pulse_result_RX(SOCKET, float *, float *);

extern char * pan_net_get_lidar_measurement_TX(SOCKET, float,float,float, float,float,float,float, float,float,float, float,float,float, float,float,float, float,float,float, float,float,float, float,float,float);
extern float *pan_net_get_lidar_measurement_RX(SOCKET, float *,float *, unsigned long *,unsigned long *, float *,float *, unsigned long *,unsigned long *, unsigned long *, unsigned long *, float *,float *, float *, float *, float *, float *, float *, float *);

extern char * pan_net_get_radar_response_TX(SOCKET, unsigned long, unsigned long, unsigned long, unsigned long, float, float, float, float, float, float, float, float, float, float, float, float, float, float, float);
extern float *pan_net_get_radar_response_RX(SOCKET, unsigned long *, float *, float *, float *, float *, float *, float *, float *, float *, float *, float *, unsigned long *, unsigned long *, unsigned long *);

extern char *         pan_net_get_viewpoint_by_degrees_d_TX(SOCKET, double, double, double, double, double, double);
extern unsigned char *pan_net_get_viewpoint_by_degrees_d_RX(SOCKET, unsigned long *);

extern char *         pan_net_get_viewpoint_by_quaternion_d_TX(SOCKET, double, double, double, double, double, double, double);
extern unsigned char *pan_net_get_viewpoint_by_quaternion_d_RX(SOCKET, unsigned long *);

extern char *      pan_net_get_joints_TX(SOCKET, unsigned long);
extern joint_data* pan_net_get_joints_RX(SOCKET, unsigned long*);

extern char *pan_net_get_joint_config_TX(SOCKET, unsigned long, unsigned long);
extern void  pan_net_get_joint_config_RX(SOCKET, double*);

extern char*       pan_net_get_frames_TX(SOCKET, unsigned long);
extern frame_data* pan_net_get_frames_RX(SOCKET, unsigned long *);

extern char *pan_net_get_frame_TX(SOCKET, unsigned long, unsigned long);
extern void  pan_net_get_frame_RX(SOCKET, double*);

extern char *pan_net_get_frame_as_radians_TX(SOCKET, unsigned long, unsigned long);
extern void  pan_net_get_frame_as_radians_RX(SOCKET, double*);

extern char *pan_net_get_surface_elevation_TX(SOCKET, unsigned char, float, float);
extern float pan_net_get_surface_elevation_RX(SOCKET, char*);

extern char *pan_net_get_surface_elevations_TX(SOCKET, unsigned char, unsigned long, float *);
extern void  pan_net_get_surface_elevations_RX(SOCKET, float*, char*);

extern char *pan_net_get_surface_patch_TX(SOCKET, unsigned char, float, float, unsigned long, unsigned long, float, float);
extern void  pan_net_get_surface_patch_RX(SOCKET, float*, char*);

extern char *         pan_net_get_viewpoint_by_radians_TX(SOCKET, double, double, double, double, double, double);
extern unsigned char *pan_net_get_viewpoint_by_radians_RX(SOCKET, unsigned long *);

extern char * pan_net_quit_TX(SOCKET);
/*            pan_net_quit_RX() is not required.  */

extern char *         pan_net_get_viewpoint_by_frame_TX(SOCKET, unsigned long, unsigned long);
extern unsigned char *pan_net_get_viewpoint_by_frame_RX(SOCKET, unsigned long *);

extern char *pan_net_get_camera_properties_TX(SOCKET, unsigned long);
extern int   pan_net_get_camera_properties_RX(SOCKET, unsigned long *, unsigned long *, double *, double *, double *, double *, double *, double *, double *, double *, double *);

extern char *         pan_net_get_viewpoint_by_camera_TX(SOCKET, unsigned long);
extern unsigned char *pan_net_get_viewpoint_by_camera_RX(SOCKET, unsigned long *);

extern char *pan_net_get_view_as_dem_TX(SOCKET, unsigned long, unsigned char, unsigned long, unsigned long, float, float, float);
extern void  pan_net_get_view_as_dem_RX(SOCKET, float*, char*);

extern char  *pan_net_get_lidar_measurement_d_TX(SOCKET, double,double,double, double,double,double,double, double,double,double, double,double,double, double,double,double, double,double,double, double,double,double, double,double,double);
extern float *pan_net_get_lidar_measurement_d_RX(SOCKET, float *,float *, unsigned long *,unsigned long *, float *,float *, unsigned long *,unsigned long *, unsigned long *, unsigned long *, float *,float *, float *, float *, float *, float *, float *, float *);

extern char * pan_net_get_time_tag_TX(SOCKET);
extern double pan_net_get_time_tag_RX(SOCKET, char *);

extern char  *pan_net_get_lidar_measurement_s_TX(SOCKET, float,float,float, float,float,float,float, float,float,float, float,float,float, float,float,float, float,float,float, float,float,float, float,float,float);
extern float *pan_net_get_lidar_measurement_s_RX(SOCKET, float *,float *, unsigned long *,unsigned long *, float *,float *, unsigned long *,unsigned long *, unsigned long *, unsigned long *, float *,float *, float *, float *, float *, float *, float *, float *);

extern char  *pan_net_get_lidar_snapshot_TX(SOCKET, unsigned long, double, double, double, double, double, double, double);
extern float *pan_net_get_lidar_snapshot_RX(SOCKET, unsigned long *, unsigned long *);

extern char *pan_net_set_viewpoint_by_degrees_s_TX(SOCKET, float, float, float, float, float, float);
/*           pan_net_set_viewpoint_by_degrees_s_RX() is not required.  */

extern char *pan_net_set_viewpoint_by_quaternion_s_TX(SOCKET, float, float, float, float, float, float, float);
/*           pan_net_set_viewpoint_by_quaternion_s_RX() is not required.  */

extern char *pan_net_set_ambient_light_TX(SOCKET, float, float, float);
/*           pan_net_set_ambient_light_RX() is not required.  */

extern char *pan_net_set_sun_colour_TX(SOCKET, float, float, float);
/*           pan_net_set_sun_colour_RX() is not required.  */

extern char *pan_net_set_sky_type_TX(SOCKET, unsigned long);
/*           pan_net_set_sky_type_RX() is not required.  */

extern char *pan_net_set_field_of_view_by_degrees_TX(SOCKET, float);
/*           pan_net_set_field_of_view_by_degrees_RX() is not required.  */

extern char *pan_net_set_field_of_view_TX(SOCKET, float);
/*           pan_net_set_field_of_view_RX() is not required.  */

extern char *pan_net_set_aspect_ratio_TX(SOCKET, float);
/*           pan_net_set_aspect_ratio_RX() is not required.  */

extern char *pan_net_set_boulder_view_TX(SOCKET, unsigned long, int);
/*           pan_net_set_boulder_view_RX() is not required.  */

extern char *pan_net_set_surface_view_TX(SOCKET, unsigned long, int, int);
/*           pan_net_set_surface_view_RX() is not required.  */

extern char *pan_net_set_lidar_parameters_TX(SOCKET, float, float, unsigned long, unsigned long, float, float, unsigned long, unsigned long, unsigned long, unsigned long, float, float, float, float, float, float, float, float, float, float);
/*           pan_net_set_lidar_parameters_RX() is not required.  */

extern char *pan_net_set_corner_cubes_s_TX(SOCKET, unsigned long, unsigned long, float *);
/*           pan_net_set_corner_cubes_s_RX() is not required.  */

extern char *pan_net_set_corner_cubes_TX(SOCKET, unsigned long, unsigned long, float *);
/*           pan_net_set_corner_cubes_RX() is not required.  */

extern char *pan_net_set_corner_cube_attitude_TX(SOCKET, float, float, float, float, float, float, float, float, float, float, float, float, float);
/*           pan_net_set_corner_cube_attitude_RX() is not required.  */

extern char *pan_net_set_viewpoint_by_degrees_d_TX(SOCKET, double, double, double, double, double, double);
/*           pan_net_set_viewpoint_by_degrees_d_RX() is not required.  */

extern char *pan_net_set_viewpoint_by_angle_d_TX(SOCKET, double, double, double, double, double, double);
/*           pan_net_set_viewpoint_by_angle_d_RX() is not required.  */

extern char *pan_net_set_viewpoint_by_quaternion_d_TX(SOCKET, double, double, double, double, double, double, double);
/*           pan_net_set_viewpoint_by_quaternion_d_RX() is not required.  */

extern char *pan_net_set_object_position_attitude_TX(SOCKET, unsigned long, double, double, double, double, double, double, double);
/*           pan_net_set_object_position_attitude_RX() is not required.  */

extern char *pan_net_set_object_position_TX(SOCKET, unsigned long, double, double, double, double, double, double, double);
/*           pan_net_set_object_position_RX() is not required.  */

extern char *pan_net_set_sun_by_degrees_TX(SOCKET, double, double, double);
/*           pan_net_set_sun_by_degrees_RX() is not required.  */

extern char *pan_net_set_sun_position_TX(SOCKET, double, double, double);
/*           pan_net_set_sun_position_RX() is not required.  */

extern char *pan_net_set_joint_config_TX(SOCKET, unsigned long, unsigned long, double[9]);
/*           pan_net_set_joint_config_RX() is not required.  */

extern char *pan_net_set_star_quaternion_TX(SOCKET, double, double, double, double);
/*           pan_net_set_star_quaternion_RX() is not required.  */

extern char *pan_net_set_star_magnitudes_TX(SOCKET, double);
/*           pan_net_set_star_magnitudes_RX() is not required.  */

extern char *pan_net_set_secondary_by_degrees_TX(SOCKET, double, double, double);
/*           pan_net_set_lidar_scan_RX() is not required.  */

extern char *pan_net_set_global_time_TX(SOCKET s, double t);
/*           pan_net_set_global_time_RX() is not required.  */

extern char *pan_net_set_object_view_TX(SOCKET, unsigned long, unsigned long);
/*           pan_net_set_object_view_RX() is not required.  */

extern char *pan_net_set_viewpoint_by_radians_TX(SOCKET, double, double, double, double, double, double);
/*           pan_net_set_viewpoint_by_radians_RX() is not required.  */

extern char *pan_net_set_field_of_view_by_radians_TX(SOCKET, float);
/*           pan_net_set_field_of_view_by_radians_RX() is not required.  */

extern char *pan_net_set_sun_by_radians_TX(SOCKET, double, double, double);
/*           pan_net_set_sun_by_radians_RX() is not required.  */

extern char *pan_net_set_secondary_by_radians_TX(SOCKET, double, double, double);
/*           pan_net_set_secondary_by_radians_RX() is not required.  */

extern char *pan_net_set_sky_rgb_TX(SOCKET, float, float, float);
/*           pan_net_set_sky_rgb_RX() is not required.  */

extern char *pan_net_set_sky_cie_TX(SOCKET, float, float, float);
/*           pan_net_set_sky_cie_RX() is not required.  */

extern char *pan_net_set_atmosphere_tau_TX(SOCKET, float, float, float, float, float, float);
/*           pan_net_set_atmosphere_tau_RX() is not required.  */

extern char *pan_net_set_global_fog_mode_TX(SOCKET, unsigned long);
/*           pan_net_set_global_fog_mode_RX() is not required.  */

extern char *pan_net_set_global_fog_properties_TX(SOCKET, double, double, double, double);
/*           pan_net_set_global_fog_properties_RX() is not required.  */

extern char *pan_net_set_atmosphere_mode_TX(SOCKET, unsigned long, unsigned long, unsigned long);
/*           pan_net_set_atmosphere_mode_RX() is not required.  */

extern char *pan_net_select_camera_TX(SOCKET, unsigned long);
/*           pan_net_select_camera_RX() is not required.  */

extern char *pan_net_bind_light_to_camera_TX(SOCKET, unsigned long, unsigned long, unsigned char);
/*           pan_net_bind_light_to_camera_RX() is not required.  */

extern char *pan_net_configure_light_by_degrees_TX(SOCKET, unsigned long, double, double, double, double, double);
/*           pan_net_configure_light_by_degrees_RX() is not required.  */

extern char *pan_net_configure_light_by_radians_TX(SOCKET, unsigned long, double, double, double, double, double);
/*           pan_net_configure_light_by_radians_RX() is not required.  */

extern char *pan_net_set_light_position_direction_TX(SOCKET, unsigned long, double, double, double, double, double, double);
/*           pan_net_set_light_position_direction_RX() is not required.  */

extern char *pan_net_render_to_hold_buffer_TX(SOCKET, unsigned long, unsigned long);
/*           pan_net_render_to_hold_buffer_RX() is not required.  */

extern char *pan_net_display_hold_buffer_TX(SOCKET, unsigned long);
/*           pan_net_display_hold_buffer_RX() is not required.  */

extern char *pan_net_set_corner_cubes_d_TX(SOCKET, unsigned long, unsigned long, double *);
/*           pan_net_set_corner_cubes_d_RX() is not required.  */

extern char *pan_net_set_projection_mode_TX(SOCKET, unsigned long, unsigned long);
/*           pan_net_set_projection_mode_RX() is not required.  */

extern char *pan_net_set_ortho_field_of_view_TX(SOCKET, unsigned long, double, double);
/*           pan_net_set_ortho_field_of_view_RX() is not required.  */

extern char *pan_net_set_lidar_scan_TX(SOCKET, unsigned long, unsigned long, double *);
/*           pan_net_set_lidar_scan_RX() is not required.  */

extern char *pan_net_set_camera_motion_TX(SOCKET, unsigned long, double, double, double, double, double, double, double, double, double, double, double, double, double, double, double, double, double, double);
/*           pan_net_set_camera_motion_RX() is not required.  */

#endif
