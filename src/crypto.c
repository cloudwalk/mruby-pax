#include <stdlib.h>
#include <stdio.h>
#include "mruby.h"
#include "mruby/compile.h"
#include "mruby/value.h"
#include "mruby/array.h"
#include "mruby/string.h"
#include "mruby/hash.h"

#include "osal.h"

static mrb_value
mrb_s_crypto_delete_all_keys(mrb_state *mrb, mrb_value klass)
{
  return mrb_fixnum_value(OsPedEraseKeys());
}

unsigned int crc_table[256];

void create_crc_table(void)
{
  int index;
  unsigned char counter;
  unsigned int data;
  uint16_t accumulator;

  for (index=0; index<256; index++)
  {
    data = index << 8;
    accumulator = 0;
    for (counter=1; counter<9; counter++)
    {
      if ( (( data ^ accumulator ) & 0x8000) == 0)
        accumulator = accumulator<<1;
      else
        accumulator = (accumulator<<1) ^ 0x1021;
      data = data << 1;
    }
    crc_table[index]=accumulator;
  }
}

static mrb_value
mrb_s_crc16(mrb_state *mrb, mrb_value klass)
{
  mrb_int accumulator;
  mrb_value buf;
  int index;
  unsigned char index2;
  int size;
  char *pbuf;

  mrb_get_args(mrb, "Si", &buf, &accumulator);

  size = RSTRING_LEN(buf);
  pbuf = (char *)RSTRING_PTR(buf);

  for (index=0; index<size; index++) {
    index2      = (accumulator>>8)^pbuf[index];
    accumulator = (accumulator<<8)^crc_table[ index2 ];
  }

  return mrb_fixnum_value((uint16_t)accumulator);
}

void
mrb_crypto_init(mrb_state* mrb)
{
  struct RClass *pax, *crypto;

  create_crc_table();

  pax      = mrb_class_get(mrb, "PAX");
  crypto   = mrb_define_class_under(mrb, pax, "Crypto", mrb->object_class);

  mrb_define_class_method(mrb , crypto , "delete_all_keys" , mrb_s_crypto_delete_all_keys , MRB_ARGS_NONE());
  mrb_define_class_method(mrb , crypto , "crc16"           , mrb_s_crc16                  , MRB_ARGS_REQ(1));
}
