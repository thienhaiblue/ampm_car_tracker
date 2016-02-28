/*----------------------------------------------------------------------------
 * Name:    DTMF.h
 * Purpose: DTMF Signal Processing Defines
 * Version: V1.00
 * Note(s):
 *----------------------------------------------------------------------------
 * This file is part of the uVision/ARM development tools.
 * This software may only be used under the terms of a valid, current,
 * end user licence from KEIL for a compatible version of KEIL software
 * development tools. Nothing else gives you the right to use this software.
 *
 * This software is supplied "AS IS" without warranties of any kind.
 *
 * Copyright (c) 2008 Keil - An ARM Company. All rights reserved.
 *----------------------------------------------------------------------------
 * History:
 *          V1.00 Initial Version
 *----------------------------------------------------------------------------*/

#ifndef __DTMF_H
#define __DTMF_H

#define DTMF_DATA_DEEP	128
#define DTMFsz  256               // DTMF Input Buffer

typedef struct DTMF  {
  unsigned int   AIindex;         // Input Data Index
  unsigned int   AIcheck;         // Index Window Trigger for DTMF check
  unsigned char  digit;           // detected digit
  unsigned char  early;           // early detected digit
  unsigned char  new;             // set to 1 when new digit detected
  unsigned char  d[4];			  // last four detected digits
  unsigned int   d_i;             // index
  unsigned short AInput[DTMFsz];  // A/D Input Data
} DTMF;

extern DTMF dail1;                // DTMF info of one input

extern void DTMF_Detect (DTMF *t);// check for valid DTMF tone


#endif // __DTMF_H
