/*
 * Standard IO and file routines.
 */
#include <stdio.h>

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "floating.h"

/* This function is designed to provide information about
   the IEEE floating point value passed in.  Note that this
   ONLY works on systems where sizeof(float) == 4.

   For a normal floating point number it it should have a + or -
   depending on the sign bit, then the significand in binary in the
   format of the most significant bit, a decimal point, and then the
   remaining 23 bits, a space, and then the exponent as 2^ and then a
   signed integer.  As an example, the floating point number .5 is
   represented as "+1.00000000000000000000000 2^-1" (without the
   quotation marks).

   There are a couple of special cases: 0 should be "+0" or "-0"
   depending on the sign.  An infinity should be "+INF" or "-INF", and
   a NAN should be "NaN".

   For denormalized numbers, write them with a leading 0. and then the
   bits in the denormalized value.

   It should be safe: The output should be truncated if the buffer is
   not sufficient to include all the data.
*/
char *floating_info(union floating f, char *buf, size_t buflen){
	// initialization
	int t = 0;
	int exponent = -126;
	int power = 0;
	int p = 0;
	int a = 3;
	char ex[5];
	int i;
	char buff[64];

	// special cases
	if (buflen == 0){
		return buf;
	}

	// check if the input is denormalized in some way by seeing if the exponent is all zeros
	for (int k = 23; k <= 30; k++){
		if ((f.as_int >> k) & 0x1){
			t++;
		}
	}

	// if denormalized
	if (t == 0){
		// set the first characters
		buff[1] = "0";
		buff[2] = ".";
        // check if the number is positive or negative
        if ((f.as_int >> 31) & 0x1){
            // set the sign accordingly
            buff[0] = "-";
        }
        else{
            buff[0] = "+";
        }

		// convert the bits in the mantissa to decimals
		for (i = 22; i >= 0; i--){
			// check each bit if it's 1, and tag on 1 if so
			if ((f.as_int >> i) & 0x1){
				buff[a] = "1";
			}
			// otherwise tag on 0
			else{
				buff[a] = "0";
			}
			a++;
		}
	}

	// if normalized
	else{
		// set the first characters
		buff[1] = "1";
		buff[2] = ".";
        // check if the number is positive or negative
        if ((f.as_int >> 31) & 0x1){
            // set the sign accordingly
            buff[0] = "-";
        }
        else{
            buff[0] = "+";
        }

        // convert the bits in the exponent to decimals
        for (i = 23; i <= 30; i++){
            // only counts if the bit is 1
            if ((f.as_int >> i) & 0x1){
                p = p + (2^power);
            }
            power++;
        }
        exponent = -127 + p;

		// convert the bits in the mantissa to decimals
		for (i = 22; i >= 0; i--){
			// check each bit if it's 1, and tag on 1 if so
			if ((f.as_int >> i) & 0x1){
				buff[a] = "1";
			}
			// otherwise tag on 0
			else{
				buff[a] = "0";
			}
			a++;
		}
	}

	// tag on the ending power of 2 and null terminator
	strcat(buff, " 2^");
	snprintf(ex, 5, "%d", exponent);
	strcat(buff, ex);
	strcat(buff, '\0');

	// copy it all over
	strncat(buf, buff, buflen);

	// return
	return buf;
}

/* This function is designed to provide information about
   the 16b IEEE floating point value passed in with the same exact format.  */
char *ieee_16_info(uint16_t f, char *buf, size_t buflen){
	// initialization
	int t = 0;
	int exponent = -14;
	int power = 0;
	int p = 0;
	int a = 3;
	char ex[5];
	int i;
	char buff[64];

	// special cases
	if (buflen == 0){
		return buf;
	}

	// check if the input is denormalized in some way by seeing if the exponent is all zeros
	for (int k = 10; k <= 14; k++){
		if ((f >> k) & 0x1){
			t++;
		}
	}

	// if denormalized
	if (t == 0){
		// set the first characters
		buff[1] = "0";
		buff[2] = ".";
        // check if the number is positive or negative
        if ((f >> 15) & 0x1){
            // set the sign accordingly
            buff[0] = "-";
        }
        else{
            buff[0] = "+";
        }

		// convert the bits in the mantissa to decimals
		for (i = 9; i >= 0; i--){
			// check each bit if it's 1, and tag on 1 if so
			if ((f >> i) & 0x1){
				buff[a] = "1";
			}
			// otherwise tag on 0
			else{
				buff[a] = "0";
			}
			a++;
		}
	}

	// if normalized
	else{
		// set the first characters
		buff[1] = "1";
		buff[2] = ".";
        // check if the number is positive or negative
        if ((f >> 15) & 0x1){
            // set the sign accordingly
            buff[0] = "-";
        }
        else{
            buff[0] = "+";
        }

        // convert the bits in the exponent to decimals
        for (i = 10; i <= 14; i++){
            // only counts if the bit is 1
            if ((f >> i) & 0x1){
                p = p + (2^power);
            }
            power++;
        }
        exponent = -15 + p;

		// convert the bits in the mantissa to decimals
		for (i = 9; i >= 0; i--){
			// check each bit if it's 1, and tag on 1 if so
			if ((f >> i) & 0x1){
				buff[a] = "1";
			}
			// otherwise tag on 0
			else{
				buff[a] = "0";
			}
			a++;
		}
	}

	// tag on the ending power of 2 and null terminator
	strcat(buff, " 2^");
	snprintf(ex, 5, "%d", exponent);
	strcat(buff, ex);
	strcat(buff, '\0');

	// copy it all over
	strncat(buf, buff, buflen);

	// return
	return buf;
}



/* This function converts an IEEE 32b floating point value into a 16b
   IEEE floating point value.  As a reminder: The sign bit is 1 bit,
   the exponent is 5 bits (with a bias of 15), and the significand is
   10 bits.

   There are several corner cases you need to make sure to consider:
   a) subnormal
   b) rounding:  We use round-to-even in case of a tie.
   c) rounding increasing the exponent on the significand.
   d) +/- 0, NaNs, +/- infinity.
 */
uint16_t as_ieee_16(union floating f){
	// initialization
	int sign;
	int exponent = ((f.as_int >> 23) & 0xFF) - 127;
	int mantissa = f.as_int & 0b11111111111111111111111;
	int e = -(exponent+14);
	uint16_t mantissa2;
	int t;


    // check if the number is positive or negative
    if ((f.as_int >> 31) & 0x1){
        // set the sign accordingly
        sign = 0b1;
    }
    else{
        sign = 0b0;
    }

	// check edge cases
    if(exponent < -24){
        return sign << 15;
    }
    // NaN
    if((exponent == 128) && (mantissa)){
        return (sign << 15) | 0b111111111111111;
    }
    // INF
    if(exponent > 15){
        return (sign << 15) | 0b111110000000000;
    }

	// check if the input is denormalized in some way by seeing if the exponent is all zeros
	for (int k = 23; k <= 30; k++){
		if ((f.as_int >> k) & 0x1){
			t++;
		}
	}

	// if denormalized
	if (t == 0){
        // add in the leading 1
        mantissa = mantissa | 0x800000;
        // bitshift the mantissa
        mantissa = mantissa >> e;
        // set the exponent to 0
        exponent = 0b00000;
	}

	// if normalized
	else{
		// check if the exponent is not negative infinity
    	if (exponent >= -14) {
            // bias
            exponent += 15;
        }
	}

	// round to the nearest even least significant but
	mantissa2 = (mantissa>>12) & 0x1;

	// check if the bit needs to be rounded
	if(mantissa2 && (((mantissa>>13) & 0x1) || (f.as_int & 0b111111111111))){
        // check the bits
        mantissa >>= 13;
        mantissa += 1;
        // if overflow
        if (mantissa & 0b10000000000){
            mantissa = 0;
            exponent += 1;
            // if exponent is too big
            if (exponent > 30){
                return (sign << 15) | 0b111110000000000;
            }
        }
    }
    // otherwise
    else{
        mantissa >>= 13;
    }

	// return
	return ((sign << 15) | (exponent << 10) | mantissa);
}