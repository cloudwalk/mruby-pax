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

void mrb_system_init(mrb_state* mrb);
void mrb_display_init(mrb_state* mrb);
void mrb_magnetic_init(mrb_state* mrb);
void mrb_emv_init(mrb_state* mrb);
void mrb_icc_init(mrb_state* mrb);
void mrb_crypto_init(mrb_state* mrb);
void mrb_pinpad_init(mrb_state* mrb);
void mrb_printer_init(mrb_state* mrb);
void mrb_touch_init(mrb_state* mrb);

void
mrb_mruby_pax_gem_init(mrb_state* mrb)
{
  mrb_system_init(mrb); DONE;
  mrb_display_init(mrb); DONE;
  mrb_magnetic_init(mrb); DONE;
  mrb_emv_init(mrb); DONE;
  mrb_icc_init(mrb); DONE;
  mrb_crypto_init(mrb); DONE;
  mrb_pinpad_init(mrb); DONE;
  mrb_printer_init(mrb); DONE;
  mrb_touch_init(mrb); DONE;
}

void
mrb_mruby_pax_gem_final(mrb_state* mrb)
{
}
