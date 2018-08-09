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
#include "prolin_barcode_lib.h"

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

static inline int module_is_set(ST_BITMAP *Bitmap, int y_coord, int x_coord)
{
  return !(Bitmap->Data[(y_coord * Bitmap->Width + x_coord)*4] & 0xff);
}

/*
 * @ Convert1DPrinterDataPixel - Convert from ST_BITMAP to the buffer of OsPrnPutImage function, Valid only for barcode1D.
 * @ Bitmap:	ST_BITMAP
 * @ Pixel:		Convert pixels to Pixel * Pixel.
 * @ Times:		the height of the barcode1D amplification factor.
 * @ Buf:		the buffer for print.
 * @ Size:		the size of buffer.
 */
static int Convert1DPrinterDataPixel(ST_BITMAP* Bitmap, int Pixel, int Times, unsigned char* Buf, int Size)
{
  int i = 0, j = 0, r = 0, ModuleValue = 0;
  int Width = Bitmap->Width;
  int Rows = Bitmap->Height;
  unsigned long OffSet = 0;
  int RowLen = ((Width * Pixel) + 7)/8 + 2;
  int SizeByte = (RowLen * Rows * Pixel * Times + 1) * sizeof(unsigned char);
  unsigned char* RowBuf = (unsigned char*)malloc(RowLen);
  unsigned char* Barcode = (unsigned char*)malloc(SizeByte);
  unsigned char *BarcodePtr = Barcode;

  if (Size < SizeByte) {
    /*printf("Convert1DPrinterDataPixel size is not enough \n");*/
    free(RowBuf);
    free(Barcode);
    return -1;
  }

  memset(Barcode, 0, SizeByte);
  BarcodePtr[0] =  Rows * Pixel * Times; /* the max is 255*/
  BarcodePtr++;

  for (r = 0; r < Rows; r++)
  {
    memset(RowBuf, 0, RowLen);
    OffSet = 0;
    RowBuf[0] = (((Width & 0xff00) >> 8) * Pixel + 7)/8;
    RowBuf[1] = ((Width & 0xff) * Pixel + 7)/8;
    for (i = 0; i < Width; i++) {
      ModuleValue = module_is_set(Bitmap, r, i);
      for (j = 0; j < Pixel; j++) {
        RowBuf[2 + (OffSet/8)] |= ModuleValue << (7 - OffSet%8);
        OffSet++;
      }
    }
    for(i = 0; i < Pixel * Times; i++) {
      memcpy(BarcodePtr, RowBuf, RowLen);
      BarcodePtr += RowLen;
    }
  }

  memcpy(Buf, Barcode, SizeByte);
  free(Barcode);
  free(RowBuf);

  return 0;
}

static mrb_value
mrb_pax_printer_s__open(mrb_state *mrb, mrb_value self)
{
  mrb_int ret = 0;
  ret = OsPrnOpen(PRN_REAL, NULL);
  /*OsPrnReset();*/
  /*OsPrnSetParam(1);*/
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

  return mrb_nil_value();
}

static mrb_value
mrb_pax_printer_s__print(mrb_state *mrb, mrb_value self)
{
  mrb_value buf;
  unsigned char buffer[2048];

  memset(&buffer, 0, sizeof(buffer));

  mrb_get_args(mrb, "S", &buf);

  strncat(&buffer[0], RSTRING_PTR(buf), RSTRING_LEN(buf));

  OsPrnPrintf(buffer);

  return mrb_fixnum_value(0);
}

static mrb_value
mrb_pax_printer_s__print_buffer(mrb_state *mrb, mrb_value self)
{
  mrb_int ret;

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
  }

  mrb_free(mrb, buf);
  return mrb_fixnum_value(0);
}

static mrb_value
mrb_pax_printer_s__check(mrb_state *mrb, mrb_value self)
{
  return mrb_fixnum_value(OsPrnCheck());
}

static mrb_value
mrb_pax_printer_s__print_barcode(mrb_state *mrb, mrb_value self)
{
  mrb_int ret;
  mrb_value string;
  ST_ENCODED_INFO encode_info = {0};
  ST_BITMAP bitmap = {0};
  unsigned char* buf = NULL;
  int bufsize = 0;
  int pixel = 4;
  int times = 20;

  mrb_get_args(mrb, "S", &string);

  bitmap.Data = (unsigned char*)malloc(400*400*4);
  bitmap.Size = 400 * 400 * 4;
  memset(bitmap.Data, 0, bitmap.Size);

  encode_info.Type = EAN13,
    encode_info.String = RSTRING_PTR(string),
    encode_info.Len = RSTRING_LEN(string),
    encode_info.SizeLevel = 1,
    encode_info.CorrectionLevel = 1,

    OsBarcodeGetBitmap(&encode_info, &bitmap);

  bufsize =((((bitmap.Width * pixel) + 7)/8 + 2) * bitmap.Height * pixel * times + 1) * sizeof(unsigned char);
  buf = (unsigned char*)malloc(bufsize);
  ret = Convert1DPrinterDataPixel(&bitmap, pixel, times, buf, bufsize);

  if (ret == 0) OsPrnPutImage(buf);

  free(bitmap.Data);
  free(buf);

  return mrb_fixnum_value(ret);
}

void
mrb_printer_init(mrb_state* mrb)
{
  struct RClass *pax;
  struct RClass *printer;

  pax   = mrb_class_get(mrb, "PAX");
  printer = mrb_define_class_under(mrb, pax, "Printer", mrb->object_class);

  mrb_define_class_method(mrb , printer , "_open"          , mrb_pax_printer_s__open          , MRB_ARGS_NONE());
  mrb_define_class_method(mrb , printer , "_reset"         , mrb_pax_printer_s__reset         , MRB_ARGS_NONE());
  mrb_define_class_method(mrb , printer , "_close"         , mrb_pax_printer_s__close         , MRB_ARGS_NONE());
  mrb_define_class_method(mrb , printer , "_font"          , mrb_pax_printer_s__font          , MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb , printer , "_level="        , mrb_pax_printer_s__level         , MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb , printer , "_size"          , mrb_pax_printer_s__size          , MRB_ARGS_REQ(4));
  mrb_define_class_method(mrb , printer , "_feed"          , mrb_pax_printer_s__feed          , MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb , printer , "_print"         , mrb_pax_printer_s__print         , MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb , printer , "_print_buffer"  , mrb_pax_printer_s__print_buffer  , MRB_ARGS_NONE());
  mrb_define_class_method(mrb , printer , "_print_bmp"     , mrb_pax_printer_s__print_bmp     , MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb , printer , "_check"         , mrb_pax_printer_s__check         , MRB_ARGS_NONE());
  mrb_define_class_method(mrb , printer , "_print_barcode" , mrb_pax_printer_s__print_barcode , MRB_ARGS_REQ(1));
}

