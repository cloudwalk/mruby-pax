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
mrb_s_pinpad_load_pin_key(mrb_state *mrb, mrb_value klass)
{
  char *key;
  char kcvData[8];
  unsigned char dataIn[184];
  mrb_int key_index, key_type, key_length, ret;

  memset(dataIn, 0, sizeof(dataIn));
  memset(kcvData, 0x00, sizeof(kcvData));

  mrb_get_args(mrb, "iis", &key_index, &key_type, &key, &key_length);

  dataIn[0] = 0x03;                                                              // format
  dataIn[2] = 0;                                                                 // source key index, 0 for plaintext
  dataIn[3] = key_index;                                                         // dest key index
  dataIn[11] = PED_TPK;                                                          // dest key type
  dataIn[12] = key_length;                                                       // dest key size
  memcpy(dataIn+13, key, key_length);                                            // key
  dataIn[13+24] = 0;                                                             // kcv mode
  memcpy(dataIn+13+24+1+128, kcvData, 8);                                        // kcv result
  memcpy(dataIn+13+24+1+128+8, "\x11\x12\x13\x14\x15\x16\x17\x18\x19\x20", 10);  // random data

  ret = OsPedWriteKey(dataIn);

  return mrb_fixnum_value(ret);
}

static mrb_value
mrb_s_pinpad_load_ipek(mrb_state *mrb, mrb_value klass)
{
  char *key;
  char *ksn;
  char kcvData[8];
  unsigned char dataIn[184];
  mrb_int key_index, key_type, key_length, ksn_length, ret;

  memset(dataIn, 0, sizeof(dataIn));
  memset(kcvData, 0x00, sizeof(kcvData));

  mrb_get_args(mrb, "iiss", &key_index, &key_type, &key, &key_length, &ksn, &ksn_length);

  dataIn[0] = 0x03;                               // format
  dataIn[2] = 0;                                  // source key index, 0 for plaintext
  dataIn[3] = key_index;                          // dest key index
  dataIn[11] = PED_TIK;                           // dest key type
  dataIn[12] = key_length;                        // dest key size
  memcpy(dataIn+13, key, key_length);             // key
  dataIn[13+24] = 0;                              // kcv mode
  memcpy(dataIn+13+24+1+128, kcvData, 8);         // kcv result
  memcpy(dataIn+13+24+1+128+8, ksn, ksn_length);  // ksn

  ret = OsPedWriteTIK(dataIn);

  return mrb_fixnum_value(ret);
}

static mrb_value
mrb_s_pinpad_get_pin(mrb_state *mrb, mrb_value klass)
{
  char ksn[16];
  char pinblock[64];
  char maxlen[] = "0,1,2,3,4,5,6,7,8,9,10,11,12";
  mrb_value pan;
  mrb_int key_index, ret;

  memset(ksn, 0, sizeof(ksn));
  memset(pinblock, 0, sizeof(pinblock));

  mrb_get_args(mrb, "is", &key_index, &pan);

  ret = OsPedGetPinBlock(key_index, (const unsigned char *)RSTRING_PTR(pan), maxlen, 0x00, 30000, (unsigned char *)&pinblock);

  if (ret == 0) {
    return mrb_str_new(mrb, pinblock, 8);
  } else {
    return mrb_fixnum_value(ret);
  }
}

static mrb_value
mrb_s_pinpad_get_pin_dukpt(mrb_state *mrb, mrb_value klass)
{
  char ksn[16];
  char dataIn[16];
  unsigned char dataOut[64];
  char maxlen[] = "0,1,2,3,4,5,6,7,8,9,10,11,12";
  mrb_value pan;
  mrb_int key_index, ret;
  mrb_value hash;

  memset(ksn, 0, sizeof(ksn));
  memset(dataIn, 0, sizeof(dataIn));
  memset(dataOut, 0, sizeof(dataOut));

  mrb_get_args(mrb, "iS", &key_index, &pan);

  ret = OsPedGetPinDukpt(key_index, (const unsigned char *)RSTRING_PTR(pan),
      maxlen, 0x20, 30000, (unsigned char *)&ksn, (unsigned char *)&dataOut);

  hash = mrb_funcall(mrb, klass, "dukpt_default", 0);
  if (ret == 0)
  {
    ret = OsPedIncreaseKsnDukpt(key_index);
    mrb_hash_set(mrb, hash, mrb_str_new_cstr(mrb, "KSN"), mrb_str_new(mrb, (char *)&ksn, 10));
    mrb_hash_set(mrb, hash, mrb_str_new_cstr(mrb, "PINBLOCK"), mrb_str_new(mrb, (char *)&dataOut, 8));
  } else if (ret == ERR_PED_DUKPT_NEED_INC_KSN || ret == ERR_PED_NO_PIN_INPUT) {
    OsPedIncreaseKsnDukpt(key_index);
  }
  mrb_hash_set(mrb, hash, mrb_str_new_cstr(mrb, "RETURN"), mrb_fixnum_value(ret));
  return hash;
}
}

void
mrb_pinpad_init(mrb_state* mrb)
{
  struct RClass *pax;
  struct RClass *pinpad;

  pax      = mrb_class_get(mrb, "PAX");
  pinpad   = mrb_define_class_under(mrb, pax, "Pinpad", mrb->object_class);

  mrb_define_class_method(mrb, pinpad , "load_pin_key", mrb_s_pinpad_load_pin_key, MRB_ARGS_REQ(3));
  mrb_define_class_method(mrb, pinpad , "load_ipek", mrb_s_pinpad_load_ipek, MRB_ARGS_REQ(4));
  mrb_define_class_method(mrb, pinpad , "get_pin", mrb_s_pinpad_get_pin, MRB_ARGS_REQ(2));
  mrb_define_class_method(mrb, pinpad , "get_pin_dukpt", mrb_s_pinpad_get_pin_dukpt, MRB_ARGS_REQ(2));
}
