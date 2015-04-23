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
#include "keyboard.h"

mrb_value
mrb__printstr__(mrb_state *mrb, mrb_value self)
{
  mrb_value obj;
  mrb_int x, y;
  mrb_int len;
  char *s;

  mrb_get_args(mrb, "oii", &obj, &y, &x);

  if (mrb_string_p(obj)) {
    s = RSTRING_PTR(obj);
    len = RSTRING_LEN(obj);
    xdisplay(s, len, x, y);
  }

  return obj;
}

mrb_value
mrb_pax_s__getc(mrb_state *mrb, mrb_value self)
{
  mrb_int timeout=0;

  mrb_get_args(mrb, "i", &timeout);

  return mrb_fixnum_value(GetKey(timeout));
}

mrb_value
mrb_pax_s__gets(mrb_state *mrb, mrb_value self)
{
  unsigned char sValue[128];
  mrb_int min, max, mode, x, y;

  memset(&sValue, 0, sizeof(sValue));

  mrb_get_args(mrb, "iiiii", &min, &max, &mode, &y, &x);

  get_string(&sValue, min, max, mode, y, x);

  return mrb_str_new_cstr(mrb, sValue);
}

mrb_value
mrb_pax_s_display_clear(mrb_state *mrb, mrb_value self)
{
  display_clear();
  return mrb_nil_value();
}

mrb_value
mrb_pax_s_display_clear_line(mrb_state *mrb, mrb_value self)
{
  mrb_int line;

  mrb_get_args(mrb, "i", &line);

  display_clear_line(line);
  return mrb_nil_value();
}

mrb_value
mrb_pax_s_print_bitmap(mrb_state *mrb, mrb_value self)
{
  mrb_value path;
  mrb_int x, y;
  char *sPath;

  mrb_get_args(mrb, "oii", &path, &y, &x);

  if (mrb_string_p(path)) {
    sPath = RSTRING_PTR(path);
    display_bitmap(sPath, y, x);
  }

  return mrb_nil_value();
}

void
mrb_display_init(mrb_state* mrb)
{
  struct RClass *pax;
  struct RClass *krn;

  krn = mrb->kernel_module;
  pax = mrb_class_get(mrb, "PAX");

  mrb_define_method(mrb       , krn , "_printstr__"        , mrb__printstr__              , MRB_ARGS_REQ(3));
  mrb_define_class_method(mrb , pax , "_getc"              , mrb_pax_s__getc              , MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb , pax , "_gets"              , mrb_pax_s__gets              , MRB_ARGS_REQ(5));
  mrb_define_class_method(mrb , pax , "display_clear"      , mrb_pax_s_display_clear      , MRB_ARGS_NONE());
  mrb_define_class_method(mrb , pax , "display_clear_line" , mrb_pax_s_display_clear_line , MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb , pax , "print_bitmap"       , mrb_pax_s_print_bitmap       , MRB_ARGS_REQ(3));
}
