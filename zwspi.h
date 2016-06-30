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

#ifndef ZWSPI_H
#define ZWSPI_H

#include <stdint.h>

int zwspiOpen(uint8_t _dspi);
int zwspiRead(int _fd, uint32_t _address, uint16_t _count, uint32_t _data[]);
int zwspiSeqRead(int _fd, uint32_t _address, uint16_t _count, uint32_t _data[]);
int zwspiSeqWrite(int _fd, uint32_t _address, uint16_t _count, const uint32_t _data[]);
int zwspiWrite(int _fd, uint32_t _address, uint16_t _count, const uint32_t _data[]);
void zwspiClose(int _fd);

#endif /* ZWSPI_H */
