#include <stdlib.h>
#include <stdio.h>
#include "mruby.h"
#include "mruby/compile.h"
#include "mruby/value.h"
#include "mruby/array.h"
#include "mruby/string.h"
#include "mruby/hash.h"

#include "osal.h"
#include "xui.h"
#include "ui.h"
#include "touch_screen.h"

static mrb_value
mrb_touch_s__getxy(mrb_state *mrb, mrb_value self)
{
  mrb_int x = 0, y = 0, ret = 0, timeout = 0;
  mrb_value hash;

  mrb_get_args(mrb, "i", &timeout);

  ret = GetTouchScreen(timeout, &x, &y, 1);

  hash = mrb_hash_new(mrb);
  if (ret == 1 && x <= S920_SCREEN_X && y <= S920_SCREEN_Y && x > 0 && y > 0) {
    mrb_hash_set(mrb , hash , mrb_str_new_lit(mrb , "x")      , mrb_fixnum_value(x));
    mrb_hash_set(mrb , hash , mrb_str_new_lit(mrb , "y")      , mrb_fixnum_value(y));
    mrb_hash_set(mrb , hash , mrb_str_new_lit(mrb , "return") , mrb_fixnum_value(ret));
  }

  return hash;
}

static mrb_value
mrb_touch_s__getxy_stream(mrb_state *mrb, mrb_value self)
{
  mrb_int x = 0, y = 0, ret = 0, timeout = 0;
  mrb_value hash;

  mrb_get_args(mrb, "i", &timeout);

  ret = GetTouchScreen(timeout, &x, &y, 0);

  hash = mrb_hash_new(mrb);
  if (ret == 1 && x <= S920_SCREEN_X && y <= S920_SCREEN_Y && x > 0 && y > 0) {
    mrb_hash_set(mrb , hash , mrb_str_new_lit(mrb , "x")      , mrb_fixnum_value(x));
    mrb_hash_set(mrb , hash , mrb_str_new_lit(mrb , "y")      , mrb_fixnum_value(y));
    mrb_hash_set(mrb , hash , mrb_str_new_lit(mrb , "return") , mrb_fixnum_value(ret));
  }

  return hash;
}

static mrb_value
mrb_touch_s_clear(mrb_state *mrb, mrb_value self)
{
  ClearTouchScreen();
  return mrb_nil_value();
}

void
mrb_touch_init(mrb_state* mrb)
{
  struct RClass *pax, *touch;

  pax   = mrb_class_get(mrb, "PAX");
  touch = mrb_define_class_under(mrb, pax, "Touch", mrb->object_class);

  mrb_define_class_method(mrb , touch , "_getxy"        , mrb_touch_s__getxy        , MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb , touch , "_getxy_stream" , mrb_touch_s__getxy_stream , MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb , touch , "clear"         , mrb_touch_s_clear         , MRB_ARGS_NONE());
}

