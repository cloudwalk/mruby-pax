#include "mruby.h"
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

void
mrb_mruby_pax_gem_init(mrb_state* mrb)
{
  struct RClass *krn;
  krn = mrb->kernel_module;
  mrb_define_method(mrb, krn, "__sleep__", mrb_sleep, MRB_ARGS_REQ(1));
}

void
mrb_mruby_pax_gem_final(mrb_state* mrb)
{
}