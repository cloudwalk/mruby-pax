#include "mruby.h"
#include "mruby/value.h"
#include "mruby/array.h"

#include "posapi.h"
#include "posapi_all.h"

#define DONE mrb_gc_arena_restore(mrb, 0)

mrb_value
mrb_sleep(mrb_state *mrb, mrb_value self)
{
  mrb_int miliseconds;
  mrb_get_args(mrb, "i", &miliseconds);

  DelayMs(miliseconds);

  return mrb_fixnum_value(miliseconds);
}

mrb_value
mrb__pax_time(mrb_state *mrb, mrb_value self)
{
  uchar time[7];
  const char sTime[20];

  memset(&time, 0, sizeof(time));
  memset(&sTime, 0, sizeof(sTime));

  GetTime(&time);

  sprintf(sTime, "20%02X-%02X-%02X %02X:%02X:%02X", (int)time[0], (int)time[1], (int)time[2], (int)time[3], (int)time[4], (int)time[5]);

  return mrb_str_new_cstr(mrb, sTime);
}


void
mrb_mruby_pax_gem_init(mrb_state* mrb)
{
  struct RClass *krn;
  struct RClass *tc;

  krn = mrb->kernel_module;
  tc  = mrb_class_get(mrb, "Time");

  mrb_define_method(mrb, krn, "__sleep__", mrb_sleep, MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb, tc, "_pax_time", mrb__pax_time, MRB_ARGS_NONE());
}

void
mrb_mruby_pax_gem_final(mrb_state* mrb)
{
}