#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>

#include "mruby.h"
#include "mruby/compile.h"
#include "mruby/variable.h"
#include "mruby/value.h"
#include "mruby/array.h"
#include "mruby/string.h"
#include "mruby/hash.h"

#include "osal.h"
#include "ui.h"

static int SetOpt(int fd, int nSpeed, int nBits, char nEvent, int nStop)
{
  struct termios newtio, oldtio;

  if (tcgetattr(fd, &oldtio) != 0) {
    return -1;
  }
  bzero(&newtio, sizeof(newtio));
  newtio.c_cflag |= CLOCAL | CREAD;
  newtio.c_cflag &= ~CSIZE;
  newtio.c_cflag &= ~CRTSCTS;
  newtio.c_lflag &= ~ICANON;

  switch (nBits) {
    case 7:
      newtio.c_cflag |= CS7;
      break;
    case 8:
    default:
      newtio.c_cflag |= CS8;
      break;
  }

  switch (nEvent) {
    case 'O':
      newtio.c_cflag |= PARENB;
      newtio.c_cflag |= PARODD;
      newtio.c_iflag |= (INPCK | ISTRIP);
      break;
    case 'E':
      newtio.c_iflag |= (INPCK | ISTRIP);
      newtio.c_cflag |= PARENB;
      newtio.c_cflag &= ~PARODD;
      break;
    case 'N':
    default:
      newtio.c_cflag &= ~PARENB;
      break;
  }

  switch (nSpeed) {
    case 1200:
      cfsetispeed(&newtio, B1200);
      cfsetospeed(&newtio, B1200);
      break;
    case 2400:
      cfsetispeed(&newtio, B2400);
      cfsetospeed(&newtio, B2400);
      break;
    case 4800:
      cfsetispeed(&newtio, B4800);
      cfsetospeed(&newtio, B4800);
      break;
    case 9600:
      cfsetispeed(&newtio, B9600);
      cfsetospeed(&newtio, B9600);
      break;
    case 19200:
      cfsetispeed(&newtio, B19200);
      cfsetospeed(&newtio, B19200);
      break;
    case 38400:
      cfsetispeed(&newtio, B38400);
      cfsetospeed(&newtio, B38400);
      break;
    case 57600:
      cfsetispeed(&newtio, B57600);
      cfsetospeed(&newtio, B57600);
      break;
    case 115200:
      cfsetispeed(&newtio, B115200);
      cfsetospeed(&newtio, B115200);
      break;
    default:
      cfsetispeed(&newtio, B9600);
      cfsetospeed(&newtio, B9600);
      break;
  }

  if (nStop == 1) {
    newtio.c_cflag &= ~CSTOPB;
  }
  else if (nStop == 2) {
    newtio.c_cflag |= CSTOPB;
  }

  newtio.c_cc[VTIME] = 0;
  newtio.c_cc[VMIN] = 0;
  tcflush(fd, TCIFLUSH);
  if ((tcsetattr(fd, TCSANOW, &newtio)) != 0) {
    return -1;
  }
  return 0;
}

static int time_compare(struct timeval tv1, struct timeval tv2)
{
  if (tv1.tv_sec > tv2.tv_sec) {
    return 1;
  }
  else if (tv1.tv_sec < tv2.tv_sec) {
    return -1;
  }
  else if (tv1.tv_usec > tv2.tv_usec) {
    return 1;
  }
  else if (tv1.tv_usec < tv2.tv_usec) {
    return -1;
  }
  else {
    return 0;
  }
}

static void time_add_ms(struct timeval *tv, int ms)
{
  tv->tv_sec += ms / 1000;
  tv->tv_usec += ((ms % 1000) * 1000);
}

void OsWlPortReset(int fp)
{
  tcflush(fp, TCIOFLUSH);
}

static int port_err(int err)
{
  if (err == EFAULT)
    return -1013;
  return -1014;
}

int OsWlPortRecv(int fp, void *RecvBuf, int RecvLen, int TimeoutMs)
{
  char *buf = RecvBuf;
  int ret, err, total;
  struct timeval tv1, tv2;

  /* Flush the input and output pools. */
  if (buf == NULL && RecvLen == 0) {
    OsWlPortReset(fp);
    return 0;
  }

  if (buf == NULL || RecvLen < 0 || TimeoutMs < 0)
    return -1;

  if (!RecvLen)
    return 0;

  if (TimeoutMs < 100 && TimeoutMs > 0) {
    TimeoutMs = 100;
  }

  if (gettimeofday(&tv1, NULL) < 0) {
    return -3209;
  }
  time_add_ms(&tv1, TimeoutMs);

  total = 0;
  while (RecvLen > 0) {
    ret = read(fp, buf, RecvLen);
    if (ret < 0) {
      err = errno;
      if (err != EAGAIN && err != EINTR)
        return port_err(err);
    } else {
      buf += ret;
      RecvLen -= ret;
      total += ret;
      if (RecvLen <= 0)
        break;
      if (gettimeofday(&tv2, NULL) < 0) {
        return -3209;
      }
      if (time_compare(tv1, tv2) <= 0) {
        break;
      }
    }
  }
  return total;
}

static mrb_value
mrb_serial_open(mrb_state *mrb, mrb_value self)
{
  mrb_value com, fd, parity;
  mrb_int fp, speed, databits, stopbits;

  mrb_get_args(mrb, "SiiSi", &com, &speed, &databits, &parity, &stopbits);

  fd = mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@fd"));
  if (! mrb_nil_p(fd) && mrb_fixnum(fd) >= 0) {
    close(mrb_fixnum(fd));
  }

  fp = open(RSTRING_PTR(com), O_RDWR | O_NOCTTY);

  if(fp < 0) {
    return mrb_fixnum_value(fp);
  }

  if (SetOpt(fp, speed, databits, (char)RSTRING_PTR(parity), stopbits) < 0)
    return mrb_fixnum_value(-1);

  tcflush(fp, TCIOFLUSH);

  fd = mrb_fixnum_value(fp);
  mrb_iv_set(mrb, self, mrb_intern_lit(mrb, "@fd"), fd);

  return fd;
}

static mrb_value
mrb_serial_close(mrb_state *mrb, mrb_value self)
{
  mrb_value fd;
  mrb_int ret = -1;

  fd = mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@fd"));
  if (! mrb_nil_p(fd)) {
    ret = close(mrb_fixnum(fd));
    mrb_iv_set(mrb, self, mrb_intern_lit(mrb, "@fd"), mrb_nil_value());
  }

  return mrb_fixnum_value(ret);
}

static mrb_value
mrb_serial_send(mrb_state *mrb, mrb_value self)
{
  mrb_value fd, buf;
  mrb_int ret = -1;

  mrb_get_args(mrb, "S", &buf);

  fd = mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@fd"));
  if (! mrb_nil_p(fd)) {
    ret = write(mrb_fixnum(fd), RSTRING_PTR(buf), RSTRING_LEN(buf));
  }

  return mrb_fixnum_value(ret);
}

static mrb_value
mrb_serial_recv(mrb_state *mrb, mrb_value self)
{
  mrb_value fd;
  mrb_int timeout = 0, size = 0, len = 0;
  unsigned char buf[2048];

  memset(buf, 0, sizeof(buf));

  mrb_get_args(mrb, "i", &size);

  fd      = mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@fd"));
  timeout = mrb_fixnum(mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@timeout")));

  if (! mrb_nil_p(fd)) {
    len = OsWlPortRecv(mrb_fixnum(fd), &buf, size, timeout);
  }

  return mrb_str_new(mrb, (char *)&buf, len);
}

  void
mrb_serial_init(mrb_state* mrb)
{
  struct RClass *pax;
  struct RClass *serial;

  pax    = mrb_class_get(mrb, "PAX");
  serial = mrb_define_class_under(mrb, pax, "Serial", mrb->object_class);

  mrb_define_method(mrb , serial , "open"  , mrb_serial_open  , MRB_ARGS_REQ(5));
  mrb_define_method(mrb , serial , "close" , mrb_serial_close , MRB_ARGS_NONE());
  mrb_define_method(mrb , serial , "send"  , mrb_serial_send  , MRB_ARGS_REQ(1));
  mrb_define_method(mrb , serial , "recv"  , mrb_serial_recv  , MRB_ARGS_REQ(1));
}

