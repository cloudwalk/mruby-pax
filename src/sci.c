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
mrb_s_icc_open(mrb_state *mrb, mrb_value klass)
{
  mrb_int id;

  mrb_get_args(mrb, "i", &id);

  return mrb_fixnum_value(sci_open(id));
}

static mrb_value
mrb_s_icc_close(mrb_state *mrb, mrb_value klass)
{
  mrb_int id;

  mrb_get_args(mrb, "i", &id);
  return mrb_fixnum_value(sci_close(id));
}

static mrb_value
mrb_s_icc_detect(mrb_state *mrb, mrb_value klass)
{
  mrb_int id;

  mrb_get_args(mrb, "i", &id);
  return mrb_fixnum_value(sci_detect(id));
}

void
mrb_icc_init(mrb_state* mrb)
{
  struct RClass *pax;
  struct RClass *icc;

  pax = mrb_class_get(mrb, "PAX");
  icc = mrb_define_class_under(mrb, pax, "ICCard",  mrb->object_class);

  mrb_define_class_method(mrb , icc , "open"   , mrb_s_icc_open   , MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb , icc , "close"  , mrb_s_icc_close  , MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb , icc , "detect" , mrb_s_icc_detect , MRB_ARGS_REQ(1));
}

