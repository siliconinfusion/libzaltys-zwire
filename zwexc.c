/**************************************************************************************************

    Author        : Paul Onions
    Creation date : 30 June 2016

    Copyright 2016 Silicon Infusion Limited

    Silicon Infusion Limited                 
    CP House
    Otterspool Way
    Watford WD25 8HP
    Hertfordshire, UK
    Tel: +44 (0)1923 650404
    Fax: +44 (0)1923 650374
    Web: www.siliconinfusion.com

    Licence: MIT, see LICENCE file for details.

***************************************************************************************************

  ZEDwire SPI Interface

**************************************************************************************************/

#include <stdio.h>
#include <stdarg.h>

#include "zwexc.h"

void zwDebug(const char* fmt, ...)
{
#ifdef DEBUG
  fprintf(stderr, "Exception: ");
  va_list args;
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);
  fprintf(stderr, "\n");
#endif
}

