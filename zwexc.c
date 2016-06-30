/**************************************************************************************************

    Author        : Paul Onions
    Creation date : 30 June 2016

    COMMERCIAL IN CONFIDENCE
    (C) 2016 Silicon Infusion Limited

    Silicon Infusion Limited                 
    CP House
    Otterspool Way
    Watford WD25 8HP
    Hertfordshire, UK
    Tel: +44 (0)1923 650404
    Fax: +44 (0)1923 650374
    Web: www.siliconinfusion.com

    This is an unpublished work the copyright of which vests in Silicon Infusion
    Limited. All rights reserved. The information contained herein is the
    property of Silicon Infusion Limited and is supplied without liability for
    errors or omissions. No part may be reproduced or used except as authorised
    by contract or other written permission. The copyright and the foregoing
    restriction on reproduction and use extend to all media in which the
    information may be embodied.

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

