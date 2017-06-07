
/* Copyright (c) University of Dundee, 2001-2016 */

#ifndef FLOATING_POINT_STRUCTS_H_INCLUDED
#define FLOATING_POINT_STRUCTS_H_INCLUDED

/* Structure for easing big-endian and x86 float translation */
typedef union
{
	float whole;
	struct
	{
		unsigned sign:		 1;
		unsigned exponent:	 8; /* Excess 127 */
		unsigned fraction:	23;
	} parts;
	unsigned char bytes[4];
} bigieee_float_struct;


/* Structure for accessing x86 float components */
typedef union
{
	float whole;
	struct
	{
		unsigned fraction:	23;
		unsigned exponent:	 8; /* Excess 127 */
		unsigned sign:		 1;
	} parts;
	unsigned char bytes[4];
} smallieee_float_struct;


/* Structure for easing big-endian and x86 double translation */
typedef union
{
	double whole;
	struct
	{
		unsigned sign:			 1;
		unsigned exponent:		11; /* Excess 1023 */
		ulonglong fraction:		52;
	} parts;
	unsigned char bytes[8];
} bigieee_double_struct;


/* Structure for accessing x86 double components */
typedef union
{
	double whole;
	struct
	{
		unsigned frac_lo:		32;
		unsigned frac_hi:		20;
		unsigned exponent:		11; /* Excess 1023 */
		unsigned sign:			 1;
	} parts;
	unsigned char bytes[8];
} smallieee_double_struct;

#endif
