#include <stdlib.h>
#include <stdio.h>
#include "mruby.h"
#include "mruby/value.h"
#include "mruby/array.h"
#include "mruby/string.h"

#include "osal.h"
#include "xui.h"
#include "ui.h"

#define DONE mrb_gc_arena_restore(mrb, 0)

mrb_value
mrb_s__serial(mrb_state *mrb, mrb_value self)
{
  char serial[128];

  memset(&serial, 0, sizeof(serial));

	OsRegGetValue("ro.fac.sn", serial);

  return mrb_str_new_cstr(mrb, serial);
}

mrb_value
mrb_s__set_backlight(mrb_state *mrb, mrb_value self)
{
  mrb_int mode;

  mrb_get_args(mrb, "i", &mode);

  OsKbBacklight(mode);

  return mrb_fixnum_value(mode);
}

mrb_value
mrb_s__battery(mrb_state *mrb, mrb_value self)
{
  char battery[128];

  memset(&battery, 0, sizeof(battery));

  OsRegGetValue("ro.fac.battery", battery);

  return mrb_str_new_cstr(mrb, battery);
}

mrb_value
mrb_pax_s__getc(mrb_state *mrb, mrb_value self)
{
  XuiClearKey();
  return mrb_fixnum_value(XuiGetKey());
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
mrb_pax_s_magnetic_open(mrb_state *mrb, mrb_value self)
{
  mrb_int ret;

  ret = OsMsrOpen();

  OsMsrReset();

  return mrb_fixnum_value(ret);
}

/*TODO Scalone REMOVE ALL MAGNETIC FUNCTIONS FROM HERE USE PURE RUBY IMPLEMENTATION WITH IO*/
mrb_value
mrb_pax_s_magnetic_read(mrb_state *mrb, mrb_value self)
{
  mrb_int ret;

  ret = OsMsrSwiped();

  return mrb_fixnum_value(ret);
}
void
mrb_mruby_pax_gem_init(mrb_state* mrb)
{
  struct RClass *pax;
  struct RClass *krn;

  krn = mrb->kernel_module;
  pax = mrb_define_class(mrb, "PAX", mrb->object_class);

  mrb_define_method(mrb       , krn , "_printstr__"        , mrb__printstr__              , MRB_ARGS_REQ(3));

  mrb_define_class_method(mrb , pax , "_serial"            , mrb_s__serial                , MRB_ARGS_NONE());
  mrb_define_class_method(mrb , pax , "_backlight="        , mrb_s__set_backlight         , MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb , pax , "_battery"           , mrb_s__battery               , MRB_ARGS_NONE());
  mrb_define_class_method(mrb , pax , "_getc"              , mrb_pax_s__getc              , MRB_ARGS_NONE());
  mrb_define_class_method(mrb , pax , "_gets"              , mrb_pax_s__gets              , MRB_ARGS_REQ(5));
  mrb_define_class_method(mrb , pax , "display_clear"      , mrb_pax_s_display_clear      , MRB_ARGS_NONE());
  mrb_define_class_method(mrb , pax , "display_clear_line" , mrb_pax_s_display_clear_line , MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb , pax , "magnetic_open"      , mrb_pax_s_magnetic_open      , MRB_ARGS_NONE());
  mrb_define_class_method(mrb , pax , "magnetic_read"      , mrb_pax_s_magnetic_read      , MRB_ARGS_NONE());
}

void
mrb_mruby_pax_gem_final(mrb_state* mrb)
{
}