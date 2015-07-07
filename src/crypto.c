#include <stdlib.h>
#include <stdio.h>
#include "mruby.h"
#include "mruby/compile.h"
#include "mruby/value.h"
#include "mruby/array.h"
#include "mruby/string.h"
#include "mruby/hash.h"

#include "osal.h"

#define PED_TLK 0x01
#define PED_TMK 0x02
#define PED_TPK 0x03
#define PED_TAK 0x04
#define PED_TDK 0x05
#define PED_TIK 0x10

static mrb_value
mrb_s_crypto_delete_all_keys(mrb_state *mrb, mrb_value klass)
{
  return mrb_fixnum_value(OsPedEraseKeys());
}

static mrb_value
mrb_s_crypto_load_ipek(mrb_state *mrb, mrb_value klass)
{
  char key[50];
  char keyBlock[256];
  mrb_int key_index, key_type, key_length, ret;

  memset(key, 0, sizeof(key));
  memset(keyBlock, 0, sizeof(keyBlock));

  mrb_get_args(mrb, "iis", &key_index, &key_type, &key);

  if (key_type == 0) key_length = 8;
  else if (key_type == 1) key_length = 16;

  keyBlock[0] = 0x03;                                                     // format
  keyBlock[2] = 0;                                                        // source key index, 0 for plaintext
  keyBlock[3] = key_index;                                                // dest key index
  keyBlock[11] = PED_TIK;                                                 // dest key type
  keyBlock[12] = key_length;                                              // dest key size
  memcpy(&keyBlock[13], key, key_length);                                 // key
  keyBlock[37] = 0;                                                       // kcv mode
  // memset(&keyBlock[38], 0, 128);                                       // kcv data
  // keyBlock[166]                                                        // kcv result
  memcpy(&keyBlock[174], "\xFF\xFF\x01\x23\x45\x67\x89\xA0\x00\x01", 10); // ksn

  ret = OsPedWriteTIK(keyBlock);

  return mrb_fixnum_value(ret);
}

static mrb_value
mrb_s_crypto_get_pin_dukpt(mrb_state *mrb, mrb_value klass)
{
  char pan[50];
  mrb_int key_index, key_type, maxlen, ret;

  memset(pan, 0, sizeof(pan));

  mrb_get_args(mrb, "iisi", &key_index, &key_type, &pan, &maxlen);

  return mrb_fixnum_value(ret);
}

void
mrb_crypto_init(mrb_state* mrb)
{
  struct RClass *pax;
  struct RClass *crypto;

  pax      = mrb_class_get(mrb, "PAX");
  crypto   = mrb_define_class_under(mrb, pax, "Crypto", mrb->object_class);

  mrb_define_class_method(mrb, crypto , "delete_all_keys", mrb_s_crypto_delete_all_keys, MRB_ARGS_NONE());
  mrb_define_class_method(mrb, crypto , "load_ipek", mrb_s_crypto_load_ipek, MRB_ARGS_REQ(3));
  mrb_define_class_method(mrb, crypto , "get_pin_dukpt", mrb_s_crypto_get_pin_dukpt, MRB_ARGS_REQ(4));
}
