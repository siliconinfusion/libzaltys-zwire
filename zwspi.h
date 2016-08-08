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

  Zwire SPI Interface

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
