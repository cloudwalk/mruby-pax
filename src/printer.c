#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>

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

typedef struct{
  unsigned short bfType;
  unsigned int bfSize;
  unsigned short bfReserved1;
  unsigned short bfReserved2;
  unsigned int bfOffBits;
}BmpFileHeader;

typedef struct{
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
}BmpInfoHeader;

#define PRINTER_GET_RESULT		_IOR('P', 1, int)
#define PRINTER_CHECK_STATUS	_IOR('P', 2, int)

#define PRINTER_DEV "/dev/printer"
#define MAX_WIDTH 384

#define buf2uint32(buf)\
  (((unsigned int)(*((buf) + 3)) << 24) + \
   ((unsigned int)(*((buf) + 2)) << 16) + \
   ((unsigned int)(*((buf) + 1)) << 8) + \
   ((unsigned int)(*((buf)))))
#define buf2uint16(buf)\
  (((unsigned short)(*((buf) + 1)) << 8) + \
   ((unsigned short)(*((buf)))))

#define bmp_set_var(var, buf, offset)	do{\
  if(sizeof(var) == 2){\
    var = buf2uint16(buf + offset);\
    offset += 2;\
  } else if(sizeof(var) == 4){\
    var = buf2uint32(buf + offset);\
    offset += 4;\
  }\
}while(0)

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

static int printer_get_reuslt(int fd)
{
  int ret;
  unsigned char status[2] = {0};

  ret = ioctl(fd, PRINTER_GET_RESULT, status);
  if(ret == 0){
    switch(status[0]){
      case 0x00:
        ret = RET_OK;
        break;
      case 0x01:
        ret = ERR_PRN_BUSY;
        break;
      case 0x02:
        ret = ERR_PRN_PAPEROUT;
        break;
      case 0x03:
        ret = ERR_PRN_OVERHEAT;
        break;
      case 0x04:
        ret = ERR_PRN_OUTOFMEMORY;
        break;
      case 0x05:
        ret = ERR_PRN_OVERVOLTAGE;
        break;
      default:
        ret = ERR_PRN_BUSY;
        break;
    }
  }else{
    ret = ERR_PRN_BUSY;
  }
  return ret;
}

static void bmp_4bytesalign_convert(unsigned char* line, unsigned int width, unsigned int not_4bytes_align)
{
  unsigned int align_bytes = width / 32 * 4;
  unsigned int tail_bytes;
  unsigned char buf[4] = {0};

  tail_bytes = buf2uint32(line + align_bytes);
  if(not_4bytes_align >= 1 && not_4bytes_align <= 8){
    buf[0] = (1 << (8 - not_4bytes_align)) - 1;
    buf[1] = 0xFF;
    buf[2] = 0xFF;
    buf[3] = 0xFF;
    tail_bytes |= buf2uint32(buf) ;
  }else if(not_4bytes_align >= 9 && not_4bytes_align <= 16){
    buf[0] = 0;
    buf[1] = (1 << (8 - not_4bytes_align % 8)) - 1;
    buf[2] = 0xFF;
    buf[3] = 0xFF;
    tail_bytes |= buf2uint32(buf) ;
  }else if(not_4bytes_align >= 17 && not_4bytes_align <= 24){
    buf[0] = 0;
    buf[1] = 0;
    buf[2] = (1 << (8 - not_4bytes_align % 8)) - 1;
    buf[3] = 0xFF;
    tail_bytes |= buf2uint32(buf) ;
  }else if(not_4bytes_align >= 25 && not_4bytes_align <= 31){
    buf[0] = 0;
    buf[1] = 0;
    buf[2] = 0;
    buf[3] = (1 << (8 - not_4bytes_align % 8)) - 1;
    tail_bytes |= buf2uint32(buf) ;
  }
  memmove(line + align_bytes, &tail_bytes, sizeof(tail_bytes));
}

static unsigned char* read_bmp_data(unsigned char* buf, unsigned int buf_size, unsigned int* width, unsigned int* height)
{
  BmpFileHeader fileheader;
  BmpInfoHeader infoheader;
  unsigned int i, row, offset = 0;
  if (buf_size <= sizeof(BmpFileHeader) + sizeof(BmpInfoHeader) + 8) {
    return NULL;
  }

#define BMP_SET_VAR(var) bmp_set_var(var,buf,offset)
  BMP_SET_VAR(fileheader.bfType);
  BMP_SET_VAR(fileheader.bfSize);
  BMP_SET_VAR(fileheader.bfReserved1);
  BMP_SET_VAR(fileheader.bfReserved2);
  BMP_SET_VAR(fileheader.bfOffBits);

  BMP_SET_VAR(infoheader.biSize);
  BMP_SET_VAR(infoheader.biWidth);
  BMP_SET_VAR(infoheader.biHeight);
  BMP_SET_VAR(infoheader.biPlanes);
  BMP_SET_VAR(infoheader.biBitCount);
  BMP_SET_VAR(infoheader.biCompression);
  BMP_SET_VAR(infoheader.biSizeImage);
  BMP_SET_VAR(infoheader.biXPelsPerMeter);
  BMP_SET_VAR(infoheader.biYPelsPerMeter);
  BMP_SET_VAR(infoheader.biClrUsed);
  BMP_SET_VAR(infoheader.biClrImportant);
  if (infoheader.biBitCount != 1) {
    return NULL; /* Not a 1bit BMP */
  }
  *width = infoheader.biWidth;
  *height = infoheader.biHeight;

  unsigned int not_4bytes_align = infoheader.biWidth % 32;
  unsigned int row_bytes = (infoheader.biWidth + 31) / 32 * 4;
  unsigned int data_size = infoheader.biHeight * row_bytes;
  unsigned char * data = (unsigned char*)malloc(data_size);
  unsigned char *line, *buf_line;

  for (i = 0; i < infoheader.biHeight; i++) {
    row = infoheader.biHeight - i - 1;
    line = data + i * row_bytes;
    buf_line = buf + fileheader.bfOffBits + row * row_bytes;
    memmove(line, buf_line, row_bytes);
    if (not_4bytes_align != 0)
      bmp_4bytesalign_convert(line, infoheader.biWidth, not_4bytes_align);
  }
  return data;
}

static unsigned char* get_1bitbmp_data(unsigned char* buf, unsigned int buf_size, unsigned int* width, unsigned int* height)
{
  const unsigned char bmp_sig[2] = {0x42, 0x4D};
  int ret;
  unsigned char* data = NULL;

  if (memcmp(buf, bmp_sig, sizeof(bmp_sig)) != 0) {
    return NULL; /* Not a BMP file */
  }

  return read_bmp_data(buf, buf_size, width, height);
}

static unsigned char * onebitimgdata2prndata(unsigned char * data, unsigned int width, unsigned int height)
{
  unsigned int i, j;
  unsigned int row_bytes = (width + 31) / 32 * 4;
  unsigned int max_row_bytes = MAX_WIDTH / 8;
  unsigned int min_row_bytes = row_bytes < max_row_bytes ? row_bytes : max_row_bytes;
  unsigned char * prn_data = malloc(max_row_bytes * height + 2);
  unsigned char *src, *dst;

  for (j = 0; j < height; j++) {
    src = data + row_bytes * j;
    dst = prn_data + 2 + max_row_bytes * j;
    for (i = 0; i < min_row_bytes; i++)
      dst[i] = ~src[i];
    if(min_row_bytes < max_row_bytes)
      memset(dst + min_row_bytes, 0x0, max_row_bytes - min_row_bytes);
  }

  return prn_data;
}

static int printer_print(unsigned char* prndata, unsigned int prndata_size)
{
  int ret, fd;

  prndata[0] = 0x01;
  prndata[1] = 0x00;

  fd = open(PRINTER_DEV, O_RDWR);
  if (fd <= 0)
    return ERR_PRN_BUSY;

  write(fd , prndata, prndata_size);
  ret = printer_get_reuslt(fd);

  close(fd);
  return ret;
}

int print_1bitbmp_buf(unsigned char* buf , unsigned int buf_size)
{
  int ret;
  unsigned int width, height;
  unsigned char *PrnData, *ImgData;

  if(buf == NULL || buf_size ==0){
    return ERR_INVALID_PARAM;
  }

  ImgData = get_1bitbmp_data(buf, buf_size, &width, &height);
  if (ImgData == NULL){
    return ERR_INVALID_PARAM;
  }

  PrnData = onebitimgdata2prndata(ImgData, width, height);
  if(PrnData == NULL){
    free(ImgData);
    return ERR_INVALID_PARAM;
  }

  ret = printer_print(PrnData, (MAX_WIDTH/8)*height+2);
  free(PrnData);
  free(ImgData);

  return ret;
}

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
mrb_pax_printer_s__print_big_bmp(mrb_state *mrb, mrb_value self)
{
  mrb_int ret, size;
  mrb_value path;
  unsigned char *buf;
  FILE* fp;

  mrb_get_args(mrb, "S", &path);

  fp = fopen(RSTRING_PTR(path), "r");

  fseek(fp, 0L, SEEK_END);
  size = ftell(fp);
  fseek(fp, 0L, SEEK_SET);
  buf = (unsigned char*)mrb_malloc(mrb, size);
  fread(buf, 1, size, fp);
  ret = print_1bitbmp_buf(buf, size);

  if (buf) mrb_free(mrb, buf);
  fclose(fp);

  return mrb_fixnum_value(ret);
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

  encode_info.Type = ITF,
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
  mrb_define_class_method(mrb , printer , "_print_big_bmp" , mrb_pax_printer_s__print_big_bmp , MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb , printer , "_check"         , mrb_pax_printer_s__check         , MRB_ARGS_NONE());
  mrb_define_class_method(mrb , printer , "_print_barcode" , mrb_pax_printer_s__print_barcode , MRB_ARGS_REQ(1));
}

