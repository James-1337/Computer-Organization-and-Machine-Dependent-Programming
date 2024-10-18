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
  char internal[256];
  char binary_rep[30];
  bool positive = !(f.as_int & 0x80000000);
  int32_t exponent = ((int32_t) ((f.as_int >> 23) & 0xFF)) - 127;
  uint32_t significand = (f.as_int) & 0x7FFFFF;
  int i;

  
  if(exponent == 128 && significand == 0) {
    sprintf(internal, "%sINF", positive ? "+" : "-");
  } else if(exponent == 128) {
    sprintf(internal, "NaN");
  } else if(exponent == -127 && significand == 0){
    sprintf(internal, "%s0", positive ? "+" : "-");
  } else {
    if (exponent == -127) {
      binary_rep[0] = '0';
      exponent += 1;
    } else {
      binary_rep[0] = '1';
    }
    binary_rep[1] = '.';
    for(i = 0; i < 23; ++i){
      binary_rep[i+2] = (significand & (1 << (22-i))) ? '1' : '0';
      binary_rep[i+3] = '\0';
    }
    sprintf(internal, "%s%s 2^%i", positive ? "+" : "-", binary_rep, exponent);
  }
  
  strncpy(buf, internal, buflen-1);
  buf[buflen-1] = '\0';
  return buf;
}

/* This function is designed to provide information about
   the 16b IEEE floating point value passed in with the same exact format.  */
char *ieee_16_info(uint16_t f, char *buf, size_t buflen){
  char internal[256];
  char binary_rep[30];
  bool positive = !(f & 0x8000);
  int32_t exponent = ((int32_t) ((f >> 10) & 0x1F)) - 15;
  uint32_t significand = (f) & 0x3FF;
  int i;
  if(exponent == 16 && significand == 0) {
    sprintf(internal, "%sINF", positive ? "+" : "-");
  } else if(exponent == 16) {
    sprintf(internal, "NaN");
  } else if(exponent == -15 && significand == 0){
    sprintf(internal, "%s0", positive ? "+" : "-");
  } else {
    if (exponent == -15) {
      binary_rep[0] = '0';
      exponent += 1;
    } else {
      binary_rep[0] = '1';
    }
    binary_rep[1] = '.';
    for(i = 0; i < 10; ++i){
      binary_rep[i+2] = (significand & (1 << (9-i))) ? '1' : '0';
      binary_rep[i+3] = '\0';
    }
    sprintf(internal, "%s%s 2^%i", positive ? "+" : "-", binary_rep, exponent);
  }
  
  strncpy(buf, internal, buflen-1);
  buf[buflen-1] = '\0';
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

  // Extract out the various features that we will need.
  bool positive = !(f.as_int & 0x80000000);
  int32_t expvalue = ((int32_t) ((f.as_int >> 23) & 0xFF));
  int32_t exponent = expvalue - 127;
  uint32_t significand = (f.as_int) & 0x7FFFFF;

  // Add back in the explicit 1 in the main, (it may get stripped
  // later)
  uint32_t main = significand >> 13 | 0x400;
  uint32_t remainder = significand & 0x1FFF;

  // A fix for an annoying corner case...
  bool lostnonzero = false;

  /* Is a NaN */
  if(expvalue == 0xFF && significand != 0) return 0xFFFF;

  /* Is subnormal, so we need to adjust the significand */
  if(exponent < -14) {
    /* NOT quite correct if the input is subnormal too, but that is so
       small an exponent that it will get shifted away in that case */
    int shamt = -1 * (exponent + 14);

    /*
     * This is to fix a bug that was missed by me in the
     * initial release.  NO testcase triggers this bug,
     * and there won't be a test case on HW2 that will
     * trigger this bug because we will use the same test
     * cases.
     *
     * Basicalyl, what would happen is if bits of the
     * significand were shifted away and were nonzero,
     * I would lose track of them.  So this is a boolean
     * to see if that happens.
     */
    int lostbits = ((1 << shamt) - 1) & significand;
    lostnonzero = (lostbits != 0);

    /*
     * add back in the implicit 1 and then do the shifting.
     */
    significand = (0x800000 | significand) >> shamt;
    main = significand >> 13;
    remainder = significand & 0x1FFF;
    exponent = -14;
  }
  if(remainder > 0x1000) {
    main += 1;
  }
  // Round to even OR round up if it looks even but it was
  // nonzero bits shifted away in creating a subnormal value.
  if(remainder == 0x1000 && ((main & 1) || lostnonzero)) {
    main += 1;
  }
  // Rounded up so the exponent increased by 1, so fix this
  if(main > 0x7FF) {
    exponent += 1;
    main = main >> 1;
  }

  if(exponent > 15) {
    return positive ? 0x7c00 : 0xfc00;
  }
  
  // Is subnormal
  if(main < 0x400 && exponent == -14) {
    return (positive ? 0x0 : 0x8000) | (main & 0x3FF);
  }
  return (positive ? 0x0 : 0x8000) | (main & 0x3FF) | (((exponent + 15) & 0x1F) << 10);
}
