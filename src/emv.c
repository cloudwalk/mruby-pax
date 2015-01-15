#include <stdlib.h>
#include <stdio.h>
#include "mruby.h"
#include "mruby/compile.h"
#include "mruby/value.h"
#include "mruby/array.h"
#include "mruby/string.h"
#include "mruby/hash.h"

#include "osal.h"
#include "emvlib_Prolin.h"


static mrb_value
get_emv_parameter(mrb_state *mrb, mrb_value klass)
{
  EMV_PARAM parameter;
  mrb_value hash;

  EMVGetParameter(&parameter);

  memset(&parameter, 0, sizeof(parameter));

  hash = mrb_funcall(mrb, klass, "parameter_default", 0);

  mrb_hash_set(mrb, hash, mrb_str_new_cstr(mrb, "MerchName"), mrb_str_new_cstr(mrb, parameter.MerchName));
  mrb_hash_set(mrb, hash, mrb_str_new_cstr(mrb, "MerchCateCode"), mrb_str_new_cstr(mrb, parameter.MerchCateCode));
  mrb_hash_set(mrb, hash, mrb_str_new_cstr(mrb, "MerchId"), mrb_str_new_cstr(mrb, parameter.MerchId));
  mrb_hash_set(mrb, hash, mrb_str_new_cstr(mrb, "TermId"), mrb_str_new_cstr(mrb, parameter.TermId));
  mrb_hash_set(mrb, hash, mrb_str_new_cstr(mrb, "TerminalType"), mrb_str_new_cstr(mrb, parameter.TerminalType));
  mrb_hash_set(mrb, hash, mrb_str_new_cstr(mrb, "Capability"), mrb_str_new_cstr(mrb, parameter.Capability));
  mrb_hash_set(mrb, hash, mrb_str_new_cstr(mrb, "ExCapability"), mrb_str_new_cstr(mrb, parameter.ExCapability));
  mrb_hash_set(mrb, hash, mrb_str_new_cstr(mrb, "TransCurrExp"), mrb_str_new_cstr(mrb, parameter.TransCurrExp));
  mrb_hash_set(mrb, hash, mrb_str_new_cstr(mrb, "ReferCurrExp"), mrb_str_new_cstr(mrb, parameter.ReferCurrExp));
  mrb_hash_set(mrb, hash, mrb_str_new_cstr(mrb, "ReferCurrCode"), mrb_str_new_cstr(mrb, parameter.ReferCurrCode));
  mrb_hash_set(mrb, hash, mrb_str_new_cstr(mrb, "CountryCode"), mrb_str_new_cstr(mrb, parameter.CountryCode));
  mrb_hash_set(mrb, hash, mrb_str_new_cstr(mrb, "TransCurrCode"), mrb_str_new_cstr(mrb, parameter.TransCurrCode));
  mrb_hash_set(mrb, hash, mrb_str_new_cstr(mrb, "ReferCurrCon"), mrb_str_new_cstr(mrb, parameter.ReferCurrCon));
  mrb_hash_set(mrb, hash, mrb_str_new_cstr(mrb, "TransType"), mrb_str_new_cstr(mrb, parameter.TransType));
  mrb_hash_set(mrb, hash, mrb_str_new_cstr(mrb, "ForceOnline"), mrb_str_new_cstr(mrb, parameter.ForceOnline));
  mrb_hash_set(mrb, hash, mrb_str_new_cstr(mrb, "GetDataPIN"), mrb_str_new_cstr(mrb, parameter.GetDataPIN));
  mrb_hash_set(mrb, hash, mrb_str_new_cstr(mrb, "SurportPSESel"), mrb_str_new_cstr(mrb, parameter.SurportPSESel));

  return hash;
}
mrb_value
mrb_s_core_init(mrb_state *mrb, mrb_value klass)
{
  return mrb_fixnum_value(EMVCoreInit());
}

void
mrb_emv_init(mrb_state* mrb)
{
  struct RClass *pax;
  struct RClass *emv;

  pax = mrb_class_get(mrb, "PAX");
  emv = mrb_define_class(mrb, "EMV", pax);

  mrb_define_class_method(mrb, emv, "core_init", mrb_s_core_init , MRB_ARGS_NONE());
}