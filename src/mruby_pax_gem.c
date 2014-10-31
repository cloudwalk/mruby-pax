#include <stdlib.h>
#include <stdio.h>
#include "mruby.h"
#include "mruby/value.h"
#include "mruby/array.h"

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

void
mrb_mruby_pax_gem_init(mrb_state* mrb)
{
  struct RClass *pax;

  pax = mrb_define_class(mrb, "PAX", mrb->object_class);

  mrb_define_class_method(mrb , pax , "_serial"     , mrb_s__serial        , MRB_ARGS_NONE());
  mrb_define_class_method(mrb , pax , "_backlight=" , mrb_s__set_backlight , MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb , pax , "_battery"    , mrb_s__battery       , MRB_ARGS_NONE());
}

void
mrb_mruby_pax_gem_final(mrb_state* mrb)
{
}