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

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <linux/spi/spidev.h>
#include <arpa/inet.h>

#include "zwspi.h"
#include "zwexc.h"

#define MAX_DSPI                2

typedef enum
{
  eZWSPICmdRead = 0x00,         /* Read N times from the same address */
  eZWSPICmdWrite = 0x01,        /* Write N times to the same address */
  eZWSPICmdSeqRead = 0x02,      /* Read N times from sequential addresses */
  eZWSPICmdSeqWrite = 0x03      /* Write N times to sequential addresses */
} ZWSPICmd;

static const char* const device[MAX_DSPI] = { "/dev/spidev3.0", "/dev/spidev3.1" };

static int zwspiTransfer(int _fd, uint8_t _cmd, uint32_t _address, uint16_t _count, const uint32_t _txData[], uint32_t _rxData[]);

int zwspiOpen(uint8_t _dspi)
{
  int fd = -1;

  if (_dspi < MAX_DSPI)
  {
    fd = open(device[_dspi], O_RDWR);
    if (fd >= 0)
    {
      /* SPI mode */
      uint32_t mode = SPI_CPOL | SPI_CPHA;
      if (ioctl(fd, SPI_IOC_WR_MODE, &mode) != 0)
      {
        goto ioctl_error;
      }

      /* Bits per word */
      uint8_t bits = 8u;
      if (ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits) != 0)
      {
        goto ioctl_error;
      }

      /* Speed Hz */
      uint32_t speed = 11000000;
      if (ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) != 0)
      {
        goto ioctl_error;
      }   
    }
    else
    {
      zwDebug("zwspiOpen: open error - %s", strerror(errno));
    }
  }
  else
  {
      zwDebug("zwspiOpen: invalid DSPI %d", _dspi);
  }

  return fd;

ioctl_error:
  zwDebug("zwspiOpen: ioctl failed - %s", strerror(errno));
  zwspiClose(fd);

  return -1;
}

int zwspiRead(int _fd, uint32_t _address, uint16_t _count, uint32_t _data[])
{
    return zwspiTransfer(_fd, eZWSPICmdRead, _address, _count, NULL, _data);
}

int zwspiWrite(int _fd, uint32_t _address, uint16_t _count, const uint32_t _data[])
{
    return zwspiTransfer(_fd, eZWSPICmdWrite, _address, _count, _data, NULL);
}

int zwspiSeqRead(int _fd, uint32_t _address, uint16_t _count, uint32_t _data[])
{
    return zwspiTransfer(_fd, eZWSPICmdSeqRead, _address, _count, NULL, _data);
}

int zwspiSeqWrite(int _fd, uint32_t _address, uint16_t _count, const uint32_t _data[])
{
    return zwspiTransfer(_fd, eZWSPICmdSeqWrite, _address, _count, _data, NULL);
}

void zwspiClose(int _fd)
{
  if (_fd >= 0)
  {
    close(_fd);
  }
  else
  {
    zwDebug("zwspiClose: invalid fd - %d", _fd);
  }
}

static int zwspiTransfer(int _fd, uint8_t _cmd, uint32_t _address, uint16_t _count, const uint32_t _txData[], uint32_t _rxData[])
{
  int result = -1;

  if (_fd >= 0)
  {
    if (_count > 0)
    {
      /* Calculate size of tx packet */
      size_t len = 7 + 4 * _count;

      /* Allocate space for transmit and receive data */
      uint8_t tx_buf[len];
      uint8_t rx_buf[len];

      /* Build up ZEDwire message header */
      uint8_t* pTx = &tx_buf[0];
      *pTx++ = _cmd;
      uint32_t address = _address;
      *pTx++ = (uint8_t)((address >> 24u) & 0xFFu);
      *pTx++ = (uint8_t)((address >> 16u) & 0xFFu);
      *pTx++ = (uint8_t)((address >> 8u) & 0xFFu);
      *pTx++ = (uint8_t)(address & 0xFFu);
      uint16_t count = _count;
      *pTx++ = (uint8_t)((count >> 8u) & 0xFFu);
      *pTx++ = (uint8_t)(count & 0xFFu);

      /* Build up ZEDwire message payload */
      for (uint16_t i = 0; i < _count; ++i)
      {
        uint32_t data = (_txData != NULL) ? _txData[i] : 0;
        *pTx++ = (uint8_t)((data >> 24u) & 0xFFu);
        *pTx++ = (uint8_t)((data >> 16u) & 0xFFu);
        *pTx++ = (uint8_t)((data >> 8u) & 0xFFu);
        *pTx++ = (uint8_t)(data & 0xFFu);
      }

      /* Assemble full duplex transfer */
      struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)&tx_buf[0],
        .rx_buf = (unsigned long)&rx_buf[0],
        .len = len,
        .delay_usecs = 0,
        .speed_hz = 11000000u,
        .bits_per_word = 8u,
      };

      int ret;
      /* Perform transfer */
      if ((ret = ioctl(_fd, SPI_IOC_MESSAGE(1), &tr)) == len)
      {
        /* Copy result into receive data (unless null) */
        if (_rxData != NULL)
        {
          for (uint16_t i = 0; i < _count; ++i)
          {
            _rxData[i] = ((uint32_t)(rx_buf[7+4*i+0]) << 24) + ((uint32_t)(rx_buf[7+4*i+1]) << 16) +
                         ((uint32_t)(rx_buf[7+4*i+2]) <<  8) + ((uint32_t)(rx_buf[7+4*i+3]) <<  0);
          }
        }

        /* Success */
        result = 0;
      }
      else
      {
        if (ret < 0)
        {
          zwDebug("zwspiTransfer: ioctl failed - %s", strerror(errno));
        }
        else
        {
          zwDebug("zwspiTransfer: short transfer - %d, %s", ret, strerror(errno));
        }
      }
    }
    else
    {
      zwDebug("zwspiTransfer: nothing to do!");
    }
  }
  else
  {
    zwDebug("zwspiTransfer: invalid fd - %d", _fd);
  }

  return result;
}


