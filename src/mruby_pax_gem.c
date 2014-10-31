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
  char uMode[1];

  mrb_get_args(mrb, "i", &mode);

  sprintf(uMode, "%d", (int)mode);

#ifdef PAX
  ScrBackLight(uMode);
#endif

  return mrb_fixnum_value(mode);
}

mrb_value
mrb_s__battery(mrb_state *mrb, mrb_value self)
{
#ifdef PAX
  return mrb_fixnum_value(BatteryCheck());
#else
  return mrb_fixnum_value(0);
#endif
}

// TODO: Scalone be careful with the size of this buffer
mrb_value
mrb_s__gets(mrb_state *mrb, mrb_value self)
{
	unsigned char sValue[128];
  mrb_int min, max;

  mrb_get_args(mrb, "ii", &min, &max);

#ifdef PAX
  memset(&sValue, 0, sizeof(sValue));

  GetString(sValue, 0xF5, (int)min, (int)max);

  return mrb_str_new_cstr(mrb, sValue);
#else
  return mrb_nil_value();
#endif
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