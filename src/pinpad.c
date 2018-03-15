#include <stdlib.h>
#include <stdio.h>
#include "mruby.h"
#include "mruby/compile.h"
#include "mruby/value.h"
#include "mruby/variable.h"
#include "mruby/array.h"
#include "mruby/string.h"
#include "mruby/hash.h"

#include "osal.h"
#include "ui.h"

#define PED_TLK 0x01
#define PED_TMK 0x02
#define PED_TPK 0x03
#define PED_TAK 0x04
#define PED_TDK 0x05
#define PED_TIK 0x10

int screen_x;
int screen_y;
int line_width;
int line_height;
int iAsteriskSize = 0;
char model[64]="\0";

static int fix_x(int x)
{
  return x * line_width;
}

static int fix_y(int y)
{
  return y * line_height;
}

static int
getAsteriskSize(void)
{
  if (iAsteriskSize == 0) {
    OsRegGetValue("ro.fac.mach", model);
    if (strcmp(model, "d200") == 0)
      iAsteriskSize = 24;
    else
      iAsteriskSize = 16;
  }
  return iAsteriskSize;
}

static void
get_rgba(mrb_state *mrb, mrb_value klass, int *r, int *g, int *b)
{
  mrb_value value;

  *r = 0;
  *g = 0;
  *b = 0;

  value = mrb_funcall(mrb, klass, "r", 0);
  if (! mrb_nil_p(value)) *r = mrb_fixnum(value);

  value = mrb_funcall(mrb, klass, "g", 0);
  if (! mrb_nil_p(value)) *g = mrb_fixnum(value);

  value = mrb_funcall(mrb, klass, "b", 0);
  if (! mrb_nil_p(value)) *b = mrb_fixnum(value);
}

static mrb_value
mrb_s_pinpad_load_pin_key(mrb_state *mrb, mrb_value klass)
{
  mrb_value key;
  char kcvData[8];
  unsigned char dataIn[184];
  mrb_int key_index, key_type, ret;

  memset(dataIn, 0, sizeof(dataIn));
  memset(kcvData, 0x00, sizeof(kcvData));

  mrb_get_args(mrb, "iiS", &key_index, &key_type, &key);

  dataIn[0] = 0x03;                                                              // format
  dataIn[2] = 0;                                                                 // source key index, 0 for plaintext
  dataIn[3] = key_index;                                                         // dest key index
  dataIn[11] = PED_TPK;                                                          // dest key type
  dataIn[12] = RSTRING_LEN(key);                                                       // dest key size
  memcpy(dataIn+13, RSTRING_PTR(key), RSTRING_LEN(key));                         // key
  dataIn[13+24] = 0;                                                             // kcv mode
  memcpy(dataIn+13+24+1+128, kcvData, 8);                                        // kcv result
  memcpy(dataIn+13+24+1+128+8, "\x11\x12\x13\x14\x15\x16\x17\x18\x19\x20", 10);  // random data

  ret = OsPedWriteKey(dataIn);

  return mrb_fixnum_value(ret);
}

static mrb_value
mrb_s_pinpad_load_ipek(mrb_state *mrb, mrb_value klass)
{
  mrb_value key, ksn;
  char kcvData[8];
  unsigned char dataIn[184];
  mrb_int key_index, key_type, ret;

  memset(dataIn, 0, sizeof(dataIn));
  memset(kcvData, 0x00, sizeof(kcvData));

  mrb_get_args(mrb, "iiSS", &key_index, &key_type, &key, &ksn);

  dataIn[0] = 0x03;                                                 // format
  dataIn[2] = 0;                                                    // source key index, 0 for plaintext
  dataIn[3] = key_index;                                            // dest key index
  dataIn[11] = PED_TIK;                                             // dest key type
  dataIn[12] = RSTRING_LEN(key);                                    // dest key size
  memcpy(dataIn+13, RSTRING_PTR(key), RSTRING_LEN(key));            // key
  dataIn[13+24] = 0;                                                // kcv mode
  memcpy(dataIn+13+24+1+128, kcvData, 8);                           // kcv result
  memcpy(dataIn+13+24+1+128+8, RSTRING_PTR(ksn), RSTRING_LEN(ksn)); // ksn

  ret = OsPedWriteTIK(dataIn);

  return mrb_fixnum_value(ret);
}

static mrb_value
mrb_s_pinpad_get_pin_block(mrb_state *mrb, mrb_value klass)
{
  char pinblock[64];
  mrb_value pan, len, hash, screen;
  mrb_int slot, ret, timeout, column, line, r, g, b;

  memset(pinblock, 0, sizeof(pinblock));

  mrb_get_args(mrb, "iSSi", &slot, &pan, &len, &timeout);

  screen = mrb_const_get(mrb, mrb_obj_value(mrb->object_class), mrb_intern_lit(mrb, "STDOUT"));
  column = mrb_fixnum(mrb_funcall(mrb, screen, "x", 0));
  line   = mrb_fixnum(mrb_funcall(mrb, screen, "y", 0));

  get_rgba(mrb, klass, &r, &g, &b);

  OsPedSetAsteriskLayout(fix_x(column) + 1, fix_y(line) + line_height, getAsteriskSize(),
      RGB(r, g, b), PED_ASTERISK_ALIGN_LEFT);
  ret = OsPedGetPinBlock(slot, (const unsigned char *)RSTRING_PTR(pan),
      RSTRING_PTR(len), 0x00, timeout, (unsigned char *)&pinblock);
  display_clear();

  hash = mrb_hash_new(mrb);
  if (ret == RET_OK)
    mrb_hash_set(mrb, hash, mrb_str_new_lit(mrb , "block"), mrb_str_new(mrb, (const char *)&pinblock, 8));
  mrb_hash_set(mrb, hash, mrb_str_new_lit(mrb , "ped"), mrb_fixnum_value(ret));

  return hash;
}

static mrb_value
mrb_s_pinpad__get_pin_dukpt(mrb_state *mrb, mrb_value klass)
{
  char ksn[16];
  char dataIn[16];
  unsigned char dataOut[64];
  mrb_value pan, hash, len, screen;
  mrb_int key_index, ret, timeout, column, line, r, g, b;

  memset(ksn, 0, sizeof(ksn));
  memset(dataIn, 0, sizeof(dataIn));
  memset(dataOut, 0, sizeof(dataOut));

  mrb_get_args(mrb, "iSSi", &key_index, &pan, &len, &timeout);

  screen = mrb_const_get(mrb, mrb_obj_value(mrb->object_class), mrb_intern_lit(mrb, "STDOUT"));
  column = mrb_fixnum(mrb_funcall(mrb, screen, "x", 0));
  line   = mrb_fixnum(mrb_funcall(mrb, screen, "y", 0));

  get_rgba(mrb, klass, &r, &g, &b);

  OsPedSetAsteriskLayout(fix_x(column) + 1, fix_y(line) + line_height, getAsteriskSize(),
      RGB(r, g, b), PED_ASTERISK_ALIGN_LEFT);
  ret = OsPedGetPinDukpt(key_index, (const unsigned char *)RSTRING_PTR(pan),
      RSTRING_PTR(len), 0x20, timeout, (unsigned char *)&ksn, (unsigned char *)&dataOut);
  display_clear();

  hash = mrb_hash_new(mrb);
  if (ret == RET_OK) {
    ret = OsPedIncreaseKsnDukpt(key_index);
    mrb_hash_set(mrb, hash, mrb_str_new_cstr(mrb, "ksn"), mrb_str_new(mrb, (char *)&ksn, 10));
    mrb_hash_set(mrb, hash, mrb_str_new_cstr(mrb, "block"), mrb_str_new(mrb, (char *)&dataOut, 8));
  } else {
    OsPedIncreaseKsnDukpt(key_index);
  }
  mrb_hash_set(mrb, hash, mrb_str_new_cstr(mrb, "ped"), mrb_fixnum_value(ret));
  return hash;
}

static mrb_value
mrb_s_pinpad_encrypt_dukpt(mrb_state *mrb, mrb_value klass)
{
  char ksn[16];
  char dataIn[16];
  unsigned char dataOut[64];
  mrb_value hash, value;
  mrb_int key_index, ret;

  memset(ksn, 0, sizeof(ksn));
  memset(dataIn, 0, sizeof(dataIn));
  memset(dataOut, 0, sizeof(dataOut));

  mrb_get_args(mrb, "iS", &key_index, &value);

  ret = OsPedDesDukpt(key_index, 0x01, NULL, 8, (const unsigned char *)RSTRING_PTR(value),
      (unsigned char *)&dataOut, (unsigned char *)&ksn, 0x01);

  hash = mrb_hash_new(mrb);
  if (ret == RET_OK) {
    ret = OsPedIncreaseKsnDukpt(key_index);
    mrb_hash_set(mrb, hash, mrb_str_new_cstr(mrb, "block"), mrb_str_new(mrb, (char *)&dataOut, 8));
    mrb_hash_set(mrb, hash, mrb_str_new_cstr(mrb, "ksn"), mrb_str_new(mrb, (char *)&ksn, 10));
  } else {
    OsPedIncreaseKsnDukpt(key_index);
  }

  mrb_hash_set(mrb, hash, mrb_str_new_cstr(mrb, "ped"), mrb_fixnum_value(ret));
  return hash;
}

static mrb_value
mrb_s_pinpad_des(mrb_state *mrb, mrb_value klass)
{
  mrb_int index, mode, ret;
  mrb_value data, hash;
  unsigned char out[2048];

  memset(out, 0, sizeof(out));

  mrb_get_args(mrb, "iiS", &index, &mode, &data);

  ret = OsPedDes(index, NULL, (const unsigned char *)RSTRING_PTR(data),
      RSTRING_LEN(data), (unsigned char *)&out, 3);

  hash = mrb_funcall(mrb, klass, "des_default", 0);
  mrb_hash_set(mrb, hash, mrb_str_new_cstr(mrb, "BLOCK"),
      mrb_str_new(mrb, (char *)&out, 8));
  mrb_hash_set(mrb, hash, mrb_str_new_cstr(mrb, "RETURN"),
      mrb_fixnum_value(ret));

  return hash;
}

static mrb_value
mrb_s_pinpad_derive(mrb_state *mrb, mrb_value klass)
{
  mrb_int ret, key_source_type, key_source_index, key_destination_type,
          key_destination_source_index, key_destination_index, mode;

  mrb_get_args(mrb, "iiiiii", &key_source_type, &key_source_index, &key_destination_type,
      &key_destination_source_index, &key_destination_index, &mode);

  ret = OsPedDeriveKey(key_source_type, key_source_index, key_destination_type,
      key_destination_source_index, key_destination_index, 3);

  return mrb_fixnum_value(ret);
}

static mrb_value
mrb_s_pinpad_load_key(mrb_state *mrb, mrb_value klass)
{
  mrb_value data;

  mrb_get_args(mrb, "S", &data);
  OsPedOpen();
  return mrb_fixnum_value(OsPedWriteKey((unsigned char *)RSTRING_PTR(data)));
}

static mrb_value
mrb_s_pinpad_get_pin_plain(mrb_state *mrb, mrb_value klass)
{
  unsigned char ucBlock;
  mrb_value hash, len, screen;
  mrb_int ret, slot, timeout, column, line, r, g, b;

  mrb_get_args(mrb, "iSi", &slot, &len, &timeout);

  screen = mrb_const_get(mrb, mrb_obj_value(mrb->object_class), mrb_intern_lit(mrb, "STDOUT"));
  column = mrb_fixnum(mrb_funcall(mrb, screen, "x", 0));
  line   = mrb_fixnum(mrb_funcall(mrb, screen, "y", 0));

  get_rgba(mrb, klass, &r, &g, &b);

  OsPedSetAsteriskLayout(fix_x(column) + 1, fix_y(line) + line_height, getAsteriskSize(),
      RGB(r, g, b), PED_ASTERISK_ALIGN_LEFT);
  ret = OsPedVerifyPlainPin(slot, RSTRING_PTR(len), 0x00, timeout, &ucBlock);

  hash = mrb_hash_new(mrb);
  mrb_hash_set(mrb, hash, mrb_str_new_lit(mrb , "block"), mrb_str_new(mrb, (const char *)&ucBlock, 2));
  mrb_hash_set(mrb, hash, mrb_str_new_lit(mrb , "ped"), mrb_fixnum_value(ret));

  return hash;
}

static mrb_value
mrb_s_pinpad_verify_cipher_pin(mrb_state *mrb, mrb_value klass)
{
  unsigned char ucBlock;
  ST_RSA_PINKEY stRSAPINKEY;
  mrb_value hash, rsa, modulus, exponent, random, len, screen;
  mrb_int ret, slot, timeout, modulus_len, random_len, column, line, r, g, b;

  mrb_get_args(mrb, "iSoi", &slot, &len, &rsa, &timeout);

  modulus     = mrb_hash_get(mrb, rsa, mrb_str_new_lit(mrb, "modulus"));
  modulus_len = mrb_fixnum(mrb_hash_get(mrb, rsa, mrb_str_new_lit(mrb, "modulus_length")));
  exponent    = mrb_hash_get(mrb, rsa, mrb_str_new_lit(mrb, "exponent"));
  random      = mrb_hash_get(mrb, rsa, mrb_str_new_lit(mrb, "random"));
  random_len  = mrb_fixnum(mrb_hash_get(mrb, rsa, mrb_str_new_lit(mrb, "random_length")));

  memset(&stRSAPINKEY, 0, sizeof(ST_RSA_PINKEY));
  stRSAPINKEY.ModulusLen = modulus_len;
  memcpy(stRSAPINKEY.Modulus, RSTRING_PTR(modulus), sizeof(stRSAPINKEY.Modulus));
  memcpy(stRSAPINKEY.Exponent, RSTRING_PTR(exponent), sizeof(stRSAPINKEY.Exponent));
  stRSAPINKEY.IccRandomLen = random_len;
  memcpy(stRSAPINKEY.IccRandom, RSTRING_PTR(random), sizeof(stRSAPINKEY.IccRandom));

  screen = mrb_const_get(mrb, mrb_obj_value(mrb->object_class), mrb_intern_lit(mrb, "STDOUT"));
  column = mrb_fixnum(mrb_funcall(mrb, screen, "x", 0));
  line   = mrb_fixnum(mrb_funcall(mrb, screen, "y", 0));

  get_rgba(mrb, klass, &r, &g, &b);

  OsPedSetAsteriskLayout(fix_x(column) + 1, fix_y(line) + line_height, getAsteriskSize(),
      RGB(r, g, b), PED_ASTERISK_ALIGN_LEFT);
  ret = OsPedVerifyCipherPin(slot, &stRSAPINKEY, RSTRING_PTR(len), 0x00, timeout, &ucBlock);

  hash = mrb_hash_new(mrb);
  mrb_hash_set(mrb, hash, mrb_str_new_lit(mrb , "block"), mrb_str_new(mrb, (const char *)&ucBlock, 2));
  mrb_hash_set(mrb, hash, mrb_str_new_lit(mrb , "ped"), mrb_fixnum_value(ret));

  return hash;
}

static mrb_value
mrb_pinpad_s__key_kcv(mrb_state *mrb, mrb_value klass)
{
  mrb_value array;
  unsigned char kcv[8] = {0x00};
  mrb_int ret, index, type;
  mrb_get_args(mrb, "ii", &type, &index);

  array = mrb_ary_new(mrb);

  if (type == PED_TMK)
  {
    ret = OsPedGetKcv((int)type, (int)index, 0x00, 8, "\x0\x0\x0\x0\x0\x0\x0\x0", &kcv);
    mrb_ary_push(mrb, array, mrb_fixnum_value(ret));
    if (ret == RET_OK) mrb_ary_push(mrb, array, mrb_str_new(mrb, kcv, 3));
  }
  else
  {
    ret = OsPedGetKcv((int)type, (int)index, 0x00, 0, NULL, &kcv);
    mrb_ary_push(mrb, array, mrb_fixnum_value(ret));
    if (ret == RET_OK) mrb_ary_push(mrb, array, mrb_str_new(mrb, kcv, 8));
  }
  return array;
}

static mrb_value
mrb_pinpad_s__key_ksn(mrb_state *mrb, mrb_value klass)
{
  mrb_value array;
  unsigned char ksn[10] = {0x00};
  mrb_int ret, index;

  mrb_get_args(mrb, "i", &index);

  ret = OsPedGetKsnDukpt((int)index, &ksn);

  array = mrb_ary_new(mrb);
  mrb_ary_push(mrb, array, mrb_fixnum_value(ret));
  if (ret == RET_OK) mrb_ary_push(mrb, array, mrb_str_new(mrb, ksn, 10));

  return array;
}

void
mrb_pinpad_init(mrb_state* mrb)
{
  struct RClass *pax;
  struct RClass *pinpad;

  pax      = mrb_class_get(mrb, "PAX");
  pinpad   = mrb_define_class_under(mrb, pax, "Pinpad", mrb->object_class);

  mrb_define_class_method(mrb , pinpad , "load_pin_key"      , mrb_s_pinpad_load_pin_key      , MRB_ARGS_REQ(3));
  mrb_define_class_method(mrb , pinpad , "load_ipek"         , mrb_s_pinpad_load_ipek         , MRB_ARGS_REQ(4));
  mrb_define_class_method(mrb , pinpad , "get_pin_block"     , mrb_s_pinpad_get_pin_block     , MRB_ARGS_REQ(4));
  mrb_define_class_method(mrb , pinpad , "_get_pin_dukpt"    , mrb_s_pinpad__get_pin_dukpt    , MRB_ARGS_REQ(4));
  mrb_define_class_method(mrb , pinpad , "encrypt_dukpt"     , mrb_s_pinpad_encrypt_dukpt     , MRB_ARGS_REQ(2));
  mrb_define_class_method(mrb , pinpad , "get_pin_plain"     , mrb_s_pinpad_get_pin_plain     , MRB_ARGS_REQ(3));
  mrb_define_class_method(mrb , pinpad , "verify_cipher_pin" , mrb_s_pinpad_verify_cipher_pin , MRB_ARGS_REQ(4));
  mrb_define_class_method(mrb , pinpad , "des"               , mrb_s_pinpad_des               , MRB_ARGS_REQ(3));
  mrb_define_class_method(mrb , pinpad , "derive"            , mrb_s_pinpad_derive            , MRB_ARGS_REQ(6));
  mrb_define_class_method(mrb , pinpad , "load_key"          , mrb_s_pinpad_load_key          , MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb , pinpad , "_key_kcv"          , mrb_pinpad_s__key_kcv          , MRB_ARGS_REQ(2));
  mrb_define_class_method(mrb , pinpad , "_key_ksn"          , mrb_pinpad_s__key_ksn          , MRB_ARGS_REQ(1));
}
