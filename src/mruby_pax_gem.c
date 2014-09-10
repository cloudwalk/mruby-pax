#include <stdlib.h>
#include <stdio.h>
#include "mruby.h"
#include "mruby/value.h"
#include "mruby/array.h"

#ifdef PAX
#include "posapi.h"
#include "posapi_all.h"
#endif

#define DONE mrb_gc_arena_restore(mrb, 0)

mrb_value
mrb_sleep(mrb_state *mrb, mrb_value self)
{
  mrb_int miliseconds;
  mrb_get_args(mrb, "i", &miliseconds);

#ifdef PAX
  DelayMs(miliseconds);
#endif

  return mrb_fixnum_value(miliseconds);
}

mrb_value
mrb__pax_time(mrb_state *mrb, mrb_value self)
{
  uchar time[7];
  const char sTime[20];

  memset(&time, 0, sizeof(time));
  memset(&sTime, 0, sizeof(sTime));

#ifdef PAX
  GetTime(&time);
#endif

  sprintf(sTime, "20%02X-%02X-%02X %02X:%02X:%02X", (int)time[0], (int)time[1], (int)time[2], (int)time[3], (int)time[4], (int)time[5]);

  return mrb_str_new_cstr(mrb, sTime);
}

mrb_value
mrb__serial(mrb_state *mrb, mrb_value self)
{
  char serial[32];

  memset(&serial, 0, sizeof(serial));

#ifdef PAX
  ReadSN(&serial);
#endif

  return mrb_str_new_cstr(mrb, serial);
}

mrb_value
mrb__set_backlight(mrb_state *mrb, mrb_value self)
{
  mrb_int mode;
  uchar uMode[1];

  mrb_get_args(mrb, "i", &mode);

  sprintf(uMode, "%d", (int)mode);

#ifdef PAX
  ScrBackLight(uMode);
#endif

  return mrb_fixnum_value(mode);
}

mrb_value
mrb__battery(mrb_state *mrb, mrb_value self)
{
#ifdef PAX
  return mrb_fixnum_value(BatteryCheck());
#endif
}

void
mrb_mruby_pax_gem_init(mrb_state* mrb)
{
  struct RClass *krn;
  struct RClass *tc;
  struct RClass *pax;

  krn = mrb->kernel_module;
  tc  = mrb_class_get(mrb, "Time");
  pax = mrb_define_class(mrb, "PAX", mrb->object_class);

  mrb_define_method(mrb       , krn , "__sleep__"   , mrb_sleep          , MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb , tc  , "_pax_time"   , mrb__pax_time      , MRB_ARGS_NONE());
  mrb_define_class_method(mrb , pax , "_serial"     , mrb__serial        , MRB_ARGS_NONE());
  mrb_define_class_method(mrb , pax , "_backlight=" , mrb__set_backlight , MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb , pax , "_battery"    , mrb__battery       , MRB_ARGS_NONE());
}

void
mrb_mruby_pax_gem_final(mrb_state* mrb)
{
}