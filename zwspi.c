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

static const char* const device[MAX_DSPI] = { "/dev/spidev3.0", "/dev/spidev3.1" };  /* Devices */

uint8_t device_rdshift[MAX_DSPI] = { 0, 0 };  /* Number of clocks by which read-data is delayed, per device */

int device_fd[MAX_DSPI] = { -1, -1 };  /* Current file-descriptor in use, per device */

static int zwspiTransfer(int _fd, uint8_t _cmd, uint32_t _address, uint16_t _count, const uint32_t _txData[], uint32_t _rxData[]);

int zwspiOpen(uint8_t _dspi)
{
  int fd = -1;
  uint8_t dspi = _dspi & 0x0F;            /* DSPI number in lower 4 bits of _dspi */
  uint8_t rdshift = (_dspi & 0xF0) >> 4;  /* Read-data shift amount in upper 4 bits of _dspi */

  if (dspi < MAX_DSPI)
  {
    fd = open(device[dspi], O_RDWR);
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

      /* Read-data bit-shift amount */
      device_rdshift[dspi] = rdshift;

      /* Record file-descriptor assigned to this device -- so we can
         tell which device we are inside the read/write routines. */
      device_fd[dspi] = fd;
    }
    else
    {
      zwDebug("zwspiOpen: open error - %s", strerror(errno));
    }
  }
  else
  {
      zwDebug("zwspiOpen: invalid DSPI %d", dspi);
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
    for (int n = 0; n < MAX_DSPI; n++)
    {
      if (device_fd[n] == _fd)
        device_fd[n] = -1;
    }
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
      size_t len = 1 + 4 + 2 + 4*_count + 1;

      /* Allocate space for transmit and receive data */
      uint8_t tx_buf[len];
      uint8_t rx_buf[len];

      /* Build up Zwire message header */
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

      /* Build up Zwire message payload */
      for (uint16_t i = 0; i < _count; ++i)
      {
        uint32_t data = (_txData != NULL) ? _txData[i] : 0;
        *pTx++ = (uint8_t)((data >> 24u) & 0xFFu);
        *pTx++ = (uint8_t)((data >> 16u) & 0xFFu);
        *pTx++ = (uint8_t)((data >> 8u) & 0xFFu);
        *pTx++ = (uint8_t)(data & 0xFFu);
      }

      /* Add extra byte of padding to allow extra clock cycles for read data */
      *pTx++ = 0;

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
        /* Find the bit-shift amount to apply to the read-data to compensate
           for clock delays on the SPI link */
        uint8_t rdshift = 0;
        for (int n = 0; n < MAX_DSPI; n++)
        {
          if (device_fd[n] == _fd)
          {
            rdshift = device_rdshift[n];
          }
        }

        /* Do the read-data bit-shift -- shift 8 bits */
        if ((rdshift & 0x8) != 0)
          for (int n = 8; n < len; n++)
            rx_buf[n-1] = rx_buf[n];

        /* Do the read-data bit-shift -- shift 4 bits */
        if ((rdshift & 0x4) != 0)
          for (int n = 8; n < len; n++)
            rx_buf[n-1] = ((rx_buf[n-1] & 0x0F) << 4) | ((rx_buf[n] & 0xF0) >> 4);

        /* Do the read-data bit-shift -- shift 2 bits */
        if ((rdshift & 0x2) != 0)
          for (int n = 8; n < len; n++)
            rx_buf[n-1] = ((rx_buf[n-1] & 0x3F) << 2) | ((rx_buf[n] & 0xC0) >> 6);

        /* Do the read-data bit-shift -- shift 1 bit */
        if ((rdshift & 0x1) != 0)
          for (int n = 8; n < len; n++)
            rx_buf[n-1] = ((rx_buf[n-1] & 0x7F) << 1) | ((rx_buf[n] & 0x80) >> 7);

        /* Copy result into receive data as 32-bit register words (unless null) */
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


