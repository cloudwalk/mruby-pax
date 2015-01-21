#include <stdlib.h>
#include <stdio.h>
#include "mruby.h"
#include "mruby/compile.h"
#include "mruby/value.h"
#include "mruby/array.h"
#include "mruby/string.h"
#include "mruby/hash.h"

#include "osal.h"
#include "ui.h"

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

mrb_value
mrb_pax_s_magnetic_close(mrb_state *mrb, mrb_value self)
{
  OsMsrClose();

  return mrb_nil_value();
}

/*{:track1 => "", :track2 => "", :track3 => ""}*/
mrb_value
mrb_pax_s_magnetic_tracks(mrb_state *mrb, mrb_value self)
{
  /*char track1[79+1], track2[37+1], track3[107+1];*/
  ST_MSR_DATA track1;
  ST_MSR_DATA track2;
  ST_MSR_DATA track3;
  mrb_value hash;

  memset(&track1, 0, sizeof(track1));
  memset(&track2, 0, sizeof(track2));
  memset(&track3, 0, sizeof(track3));

  OsMsrRead(&track1, &track2, &track3);

  hash = mrb_hash_new(mrb);
  mrb_hash_set(mrb, hash, mrb_symbol_value(mrb_intern_cstr(mrb, "track1")), mrb_str_new_cstr(mrb, track1.TrackData));
  mrb_hash_set(mrb, hash, mrb_symbol_value(mrb_intern_cstr(mrb, "track2")), mrb_str_new_cstr(mrb, track2.TrackData));
  mrb_hash_set(mrb, hash, mrb_symbol_value(mrb_intern_cstr(mrb, "track3")), mrb_str_new_cstr(mrb, track3.TrackData));

  return hash;
}

void
mrb_magnetic_init(mrb_state* mrb)
{
  struct RClass *pax;
  struct RClass *krn;

  krn = mrb->kernel_module;
  pax = mrb_class_get(mrb, "PAX");

  mrb_define_class_method(mrb , pax , "magnetic_open"      , mrb_pax_s_magnetic_open      , MRB_ARGS_NONE());
  mrb_define_class_method(mrb , pax , "magnetic_read"      , mrb_pax_s_magnetic_read      , MRB_ARGS_NONE());
  mrb_define_class_method(mrb , pax , "magnetic_close"     , mrb_pax_s_magnetic_close     , MRB_ARGS_NONE());
  mrb_define_class_method(mrb , pax , "magnetic_tracks"    , mrb_pax_s_magnetic_tracks    , MRB_ARGS_REQ(1));
}

