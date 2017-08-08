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
mrb__print__(mrb_state *mrb, mrb_value self)
{
  mrb_value obj, x, y;
  mrb_int row = 0, column = 0;
  mrb_int len;
  char *s;

  mrb_get_args(mrb, "ooo", &obj, &y, &x);

  if (mrb_fixnum_p(y)) row    = mrb_fixnum(y);
  if (mrb_fixnum_p(x)) column = mrb_fixnum(x);

  if (mrb_string_p(obj)) {
    s = RSTRING_PTR(obj);
    len = RSTRING_LEN(obj);
    xdisplay(s, len, column, row);
  }

  return obj;
}

mrb_value
mrb_pax_s__getc(mrb_state *mrb, mrb_value self)
{
  mrb_value obj;
  mrb_int timeout=0;

  mrb_get_args(mrb, "o", &obj);

  if (mrb_fixnum_p(obj)) timeout = mrb_fixnum(obj);

	/*return mrb_fixnum_value(GetKey(mrb, timeout));*/
	return mrb_fixnum_value(GetMainKey(mrb, timeout));
}

mrb_value
mrb_pax_s__getc_read(mrb_state *mrb, mrb_value self)
{
  mrb_value obj;
  mrb_int timeout=0;

  mrb_get_args(mrb, "o", &obj);

  if (mrb_fixnum_p(obj)) timeout = mrb_fixnum(obj);

  return mrb_fixnum_value(GetMainKey(mrb, timeout));
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
  mrb_value path, x, y;
  mrb_int row = 0, column = 0;

  mrb_get_args(mrb, "Soo", &path, &y, &x);

  if (mrb_fixnum_p(y)) row    = mrb_fixnum(y);
  if (mrb_fixnum_p(x)) column = mrb_fixnum(x);

  if (mrb_string_p(path)) {
    display_bitmap(RSTRING_PTR(path), row, column);
  }

  return mrb_nil_value();
}

mrb_value
mrb_display_s_print_status_bar(mrb_state *mrb, mrb_value self)
{
  mrb_value path;
  mrb_int slot = 0;

  mrb_get_args(mrb, "io", &slot, &path);

  if (mrb_string_p(path)) {
    if (XuiSetStatusbarIcon(slot, RSTRING_PTR(path)) == 0)
      return mrb_true_value();
    else
      return mrb_nil_value();
  } else {
    if (XuiSetStatusbarIcon(slot, NULL) == 0)
      return mrb_true_value();
    else
      return mrb_nil_value();
  }
}

void
mrb_display_init(mrb_state* mrb)
{
  struct RClass *pax, *krn, *dsp;

  krn = mrb->kernel_module;
  pax = mrb_class_get(mrb, "PAX");
  dsp = mrb_define_class_under(mrb, pax, "Display", mrb->object_class);

  mrb_define_method(mrb       , krn , "__print__"          , mrb__print__                   , MRB_ARGS_REQ(3));
  mrb_define_class_method(mrb , pax , "_getc"              , mrb_pax_s__getc                , MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb , pax , "_getc_read"         , mrb_pax_s__getc_read           , MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb , pax , "display_clear"      , mrb_pax_s_display_clear        , MRB_ARGS_NONE());
  mrb_define_class_method(mrb , pax , "display_clear_line" , mrb_pax_s_display_clear_line   , MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb , pax , "print_bitmap"       , mrb_pax_s_print_bitmap         , MRB_ARGS_REQ(3));
  mrb_define_class_method(mrb , dsp , "print_status_bar"   , mrb_display_s_print_status_bar , MRB_ARGS_REQ(2));
}
