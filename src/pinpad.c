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
mrb_s_pinpad_get_pin(mrb_state *mrb, mrb_value klass)
{
  // OsPedGetPinBlock(ucPinKeyID, "0,4,5,6,7,8", szPAN, glProcInfo.sPinBlock, 0, USER_OPER_TIMEOUT*1000);
}

void
mrb_pinpad_init(mrb_state* mrb)
{
  struct RClass *pax;
  struct RClass *pinpad;

  pax      = mrb_class_get(mrb, "PAX");
  pinpad   = mrb_define_class_under(mrb, pax, "Pinpad", mrb->object_class);

  mrb_define_class_method(mrb, pinpad , "get_pin", mrb_s_pinpad_get_pin, MRB_ARGS_REQ(2));
}
