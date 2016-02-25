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

void
mrb_crypto_init(mrb_state* mrb)
{
  struct RClass *pax, *crypto;

  pax      = mrb_class_get(mrb, "PAX");
  crypto   = mrb_define_class_under(mrb, pax, "Crypto", mrb->object_class);
  mrb_define_class_method(mrb, crypto , "delete_all_keys", mrb_s_crypto_delete_all_keys, MRB_ARGS_NONE());
}
