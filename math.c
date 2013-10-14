// Copyright John Greb, part of bare-metal-arm project under MIT License

// returns hypotenuse of 3D vector
unsigned short magnitude(short x, short y, short z)
{
	short loops = 0;
	unsigned short result, last; 
	unsigned int square, step;

	if ( x < 0 ) x = -x;
	if ( y < 0 ) y = -y;
	if ( z < 0 ) z = -z;
	square = x*x + y*y + z*z;
	result = x + y + z;
	if ( result < 1 ) return 0;

	do {
		last = result;
		step = square / result;
		result = (result + (unsigned short)(step) )>> 1;
	} while ( (loops++ < 5) && (last != result) );

	return result;
}

// returns upper-most face of case
short upness(short x, short y, short z)
{
	short result = 1;
	short mag = x;

	if ( x < 0 ) {
		result = 4;
		mag = -x;
	}

	if ( y > mag ) {
		result = 2;
		mag = y;
	} else if ( y < -mag ) {
		result = 5; 
		mag = -y;
		}

	if ( z > mag )
		result = 3;
	else if ( z < -mag )
		result = 6;

	return result;
}

// Returns Arctangent of vector, adjusted to suit pitch or roll.
#include "arctan.h"
short findArctan( short x, short y, short z)
{
	unsigned short ux, uy;
	int tangent;
	short result;

	if ( 0 == x ) return 0;
	if ( x < 0 ) ux = -x;
		else ux = x;

	// seperate calc for 2d or 3d vectors
	if ( 0 == z ) {
		if ( y < 0 ) uy = -y;
			else uy = y;
	} else uy = magnitude( y, z, 0);

	if ( uy > ux ) {
		tangent = ((int)ux <<6) /uy;
		result = 90 - arctan[ tangent ];
	} else {
		tangent = ((int)uy <<6) /ux;
		result = arctan[ tangent ];
	}

	// special case for 3d vector (pitch)
	if ( z != 0 ) {
		if ( x < 0 ) return (90 - result);
			else return (result - 90); 
	}

	// normal case for 2d vector (roll)
	if ( y < 0 ) {
		if ( x < 0 ) return (result - 180); 
		return (-result);
	}
	if ( x < 0 ) return (180 - result);
	return result;
}


#ifdef USEARCSIN
// undefined when unused to save memory
#include "arcsin.h"

// Used for finding pitch.
// Limited to +/-90, inaccurate for steep climb/dive.
// TODO: Use lookup table that copes with -ve input.
short findArcsin( short scalar, unsigned short mag)
{
	int sine;
	short result;

	if ( 0 == mag ) return 0;
	if ( scalar < 0 ) sine = -scalar;
		else	sine = scalar;
	sine = (sine << 6 ) / mag;
	result = arcsin[ sine ];
	if ( scalar < 0 ) 
		return ( -result );
	return result;
}
#endif



