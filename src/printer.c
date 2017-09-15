#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "mruby.h"
#include "mruby/compile.h"
#include "mruby/value.h"
#include "mruby/array.h"
#include "mruby/string.h"
#include "mruby/hash.h"

#include "osal.h"
#include "xui.h"
#include "ui.h"
#include "keyboard.h"

struct tagBITMAPFILEHEADER {
  unsigned short bfType;
  unsigned int bfSize;
  unsigned short bfReserved1;
  unsigned short bfReserved2;
  unsigned int bfOffBits;
} __attribute__ ((__packed__));

typedef struct tagBITMAPFILEHEADER BITMAPFILEHEADER;

struct tagBITMAPINFOHEADER {
  unsigned int biSize;
  unsigned int biWidth;
  unsigned int biHeight;
  unsigned short biPlanes;
  unsigned short biBitCount;
  unsigned int biCompression;
  unsigned int biSizeImage;
  unsigned int biXPelsPerMeter;
  unsigned int biYPelsPerMeter;
  unsigned int biClrUsed;
  unsigned int biClrImportant;
} __attribute__ ((__packed__));

typedef struct tagBITMAPINFOHEADER BITMAPINFOHEADER;

/*
 *  0 - Success
 * -1 - Open BMP file error
 * -2 - Problem reading file
 * -3 - Bmp not monochrome
 * -4 - The width of bitmap must <= 192
 * -5 - Lseek to current position failed
 */
static int
bmp_convert(char *file, unsigned char *logo)
{
  int i, j, k, m, bFlag, ibit;
  int fh;
  int iLcdLines, nMaxByte;
  BITMAPFILEHEADER tBmFileHeader;
  BITMAPINFOHEADER tBmInfoHeader;
  unsigned char bLogo, bTemp, bData, temp[8];
  unsigned long DataSizePerLine;
  char sBuf[100];
  unsigned char b, ss, pMem[524000];
  unsigned char *tlogo;

  fh = open(file, O_RDONLY);
  if (fh == -1) {
    /*DEBUG*/
    /*display("open bmp file err!");*/
    return -1;
  }

  if (read(fh, &tBmFileHeader, sizeof(BITMAPFILEHEADER)) == -1) {
    /*DEBUG*/
    /*display("Problem reading file");*/
    close(fh);
    return -2;
  }

  if (read(fh, &tBmInfoHeader, sizeof(BITMAPINFOHEADER)) == -1) {
    /*DEBUG*/
    /*display("Problem reading file");*/
    close(fh);
    return -2;
  }

  if (tBmInfoHeader.biBitCount != 1) {
    /*DEBUG*/
    /*display("biBitCount=%x, Must be monochrome bitmap file!!!", tBmInfoHeader.biBitCount);*/
    close(fh);
    return -3;
  }

  if (tBmInfoHeader.biWidth > 576) {
    /*DEBUG*/
    /*display("The width of bitmap must <= 192!!!");*/
    close(fh);
    return -4;
  }

  if (read(fh, &temp, 8) == -1) {
    /*DEBUG*/
    /*display("Problem reading file");*/
    close(fh);
    return -2;
  }

  if ((temp[0] == 0xFF) && (temp[1] == 0xFF) && (temp[2] == 0xFF)) {
    bFlag = 1;
  } else if((temp[0] == 0x00) && (temp[1] == 0x00) && (temp[2] == 0x00)) {
    bFlag = 0;
  }

  ibit = tBmInfoHeader.biWidth%8;
  if ( 0 == ibit ) {
    b = 0x00;
  } else {
    b = 2^(tBmInfoHeader.biWidth/8);
  }

  DataSizePerLine= (tBmInfoHeader.biWidth*tBmInfoHeader.biBitCount+31)/8;
  DataSizePerLine= DataSizePerLine/4*4;

  tlogo=logo;
  iLcdLines = tBmInfoHeader.biHeight;
  *tlogo = (unsigned char)iLcdLines;
  tlogo++;

  memset(pMem, 0xff, iLcdLines*tBmInfoHeader.biWidth);
  for(i=tBmInfoHeader.biHeight-1,j=0; i>=0; i--,j++) {
    if (lseek(fh, tBmFileHeader.bfOffBits+i*DataSizePerLine, SEEK_SET) == -1) {
      /*DEBUG*/
      /*display("lseek to current position failed");*/
      close(fh);
      return -5;
    }

    if (read(fh, sBuf, DataSizePerLine) == -1) {
      /*DEBUG*/
      /*display("Problem reading file");*/
      close(fh);
      return -2;
    }

    if(bFlag) {
      for (k = 0; k < DataSizePerLine; k++)
      {
        if (k < tBmInfoHeader.biWidth/8) {
          pMem[j*DataSizePerLine+k] = ~(sBuf[k]);
        } else if(k == tBmInfoHeader.biWidth/8) {
          pMem[j*DataSizePerLine+k] = ~(sBuf[k] | (0xFF - b));
        } else {
          pMem[j*DataSizePerLine+k] = 0x00;
        }
      }
    } else {
      memcpy(pMem+j*DataSizePerLine, sBuf, DataSizePerLine);
    }
  }
  close(fh);

  for(i=0; i<iLcdLines; i++) {
    nMaxByte =72;
    *tlogo = (unsigned char)(nMaxByte/256);
    tlogo++;
    *tlogo = (unsigned char)(nMaxByte%256);
    tlogo++;

    for(j=0; j<(int)DataSizePerLine; j++) {
      if (j >= 72) break;

      bLogo = 0x00;
      bData= *(pMem+i*DataSizePerLine+j);
      for(m=7; m>=0; m--) {
        bTemp = (bData>>m)&0x01;
        if (bTemp == 0x00)
          bLogo |= 0x01<<m;
      }

      if (j ==tBmInfoHeader.biWidth/8) {
        ss =0x00;
        for(m=0; m<tBmInfoHeader.biWidth%8; m++) {
          ss |=1<<(7-m);
        }
        bLogo &=ss;
      }
      *tlogo = bLogo;
      tlogo++;

      if (j ==tBmInfoHeader.biWidth/8) {
        j++;
        break;
      }
    }
    if (DataSizePerLine < 72) {
      memset(sBuf, 0, sizeof(sBuf));
      for(m=0; j<72; m++,j++) {
        *tlogo = (unsigned char)sBuf[m];
        tlogo++;
      }
    }
  }
  return 0;
}

static mrb_value
mrb_pax_printer_s__open(mrb_state *mrb, mrb_value self)
{
  mrb_int ret = OsPrnOpen(PRN_REAL, NULL);
  OsPrnReset();
  return mrb_fixnum_value(ret);
}

static mrb_value
mrb_pax_printer_s__reset(mrb_state *mrb, mrb_value self)
{
  OsPrnReset();
  return mrb_nil_value();
}

static mrb_value
mrb_pax_printer_s__close(mrb_state *mrb, mrb_value self)
{
  OsPrnClose();
  return mrb_nil_value();
}

static mrb_value
mrb_pax_printer_s__font(mrb_state *mrb, mrb_value self)
{
  mrb_value filename;

  mrb_get_args(mrb, "S", &filename);

  return mrb_fixnum_value(OsPrnSetFont(RSTRING_PTR(filename)));
}

static mrb_value
mrb_pax_printer_s__level(mrb_state *mrb, mrb_value self)
{
  mrb_int level=0;

  mrb_get_args(mrb, "i", &level);
  OsPrnSetGray(level);

  return mrb_nil_value();
}

static mrb_value
mrb_pax_printer_s__size(mrb_state *mrb, mrb_value self)
{
  mrb_int singlecode_width=0,singlecode_height=0,multicode_width=0,multicode_height=0;

  mrb_get_args(mrb, "iiii", &singlecode_width, &singlecode_height, &multicode_width, &multicode_height);

  OsPrnSelectFontSize(singlecode_width, singlecode_height, multicode_width, multicode_height);
  return mrb_nil_value();
}

static mrb_value
mrb_pax_printer_s__feed(mrb_state *mrb, mrb_value self)
{
  mrb_int size;

  mrb_get_args(mrb, "i", &size);
  OsPrnFeed(size);
  OsPrnStart();
  OsPrnReset();

  return mrb_nil_value();
}

static mrb_value
mrb_pax_printer_s__print(mrb_state *mrb, mrb_value self)
{
  mrb_int ret;
  mrb_value buf;
  unsigned char buffer[2048];

  memset(&buffer, 0, sizeof(buffer));

  mrb_get_args(mrb, "S", &buf);

  strncat(&buffer[0], RSTRING_PTR(buf), RSTRING_LEN(buf));

  OsPrnPrintf(buffer);
  OsPrnSetGray(3);
  OsPrnSetSpace(0,0);
  OsPrnSetIndent(0,0);
  ret = OsPrnStart();
  OsPrnReset();

  return mrb_fixnum_value(ret);
}

static mrb_value
mrb_pax_printer_s__print_bmp(mrb_state *mrb, mrb_value self)
{
  mrb_int ret;
  mrb_value path;
  unsigned char *buf;

  mrb_get_args(mrb, "S", &path);

  buf = (unsigned char*)mrb_malloc(mrb, sizeof(unsigned char)*20000);

  ret = bmp_convert(RSTRING_PTR(path), buf);

  if (ret == 0) {
    OsPrnPutImage(buf);
    OsPrnStart();
    OsPrnReset();
  }

  mrb_free(mrb, buf);
  return mrb_fixnum_value(ret);
}

static mrb_value
mrb_pax_printer_s__check(mrb_state *mrb, mrb_value self)
{
  return mrb_fixnum_value(OsPrnCheck());
}

void
mrb_printer_init(mrb_state* mrb)
{
  struct RClass *pax;
  struct RClass *printer;

  pax   = mrb_class_get(mrb, "PAX");
  printer = mrb_define_class_under(mrb, pax, "Printer", mrb->object_class);

  mrb_define_class_method(mrb , printer , "_open"      , mrb_pax_printer_s__open      , MRB_ARGS_NONE());
  mrb_define_class_method(mrb , printer , "_reset"     , mrb_pax_printer_s__reset     , MRB_ARGS_NONE());
  mrb_define_class_method(mrb , printer , "_close"     , mrb_pax_printer_s__close     , MRB_ARGS_NONE());
  mrb_define_class_method(mrb , printer , "_font="     , mrb_pax_printer_s__font      , MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb , printer , "_level="    , mrb_pax_printer_s__level     , MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb , printer , "_size"      , mrb_pax_printer_s__size      , MRB_ARGS_REQ(4));
  mrb_define_class_method(mrb , printer , "_feed"      , mrb_pax_printer_s__feed      , MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb , printer , "_print"     , mrb_pax_printer_s__print     , MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb , printer , "_print_bmp" , mrb_pax_printer_s__print_bmp , MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb , printer , "_check"     , mrb_pax_printer_s__check     , MRB_ARGS_NONE());
}

