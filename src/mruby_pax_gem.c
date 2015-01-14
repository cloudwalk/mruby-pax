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

#define DONE mrb_gc_arena_restore(mrb, 0)

void
mrb_mruby_pax_gem_init(mrb_state* mrb)
{
  mrb_system_init();
  mrb_display_init();
  mrb_magnetic_init();
  mrb_emv_init();
}

void
mrb_mruby_pax_gem_final(mrb_state* mrb)
{
}