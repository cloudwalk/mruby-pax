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

static mrb_value
mrb_magnetic_s_open(mrb_state *mrb, mrb_value self)
{
  mrb_int ret;

  ret = OsMsrOpen();

  OsMsrReset();

  return mrb_fixnum_value(ret);
}

/*TODO Scalone REMOVE ALL MAGNETIC FUNCTIONS FROM HERE USE PURE RUBY IMPLEMENTATION WITH IO*/
static mrb_value
mrb_magnetic_s_read(mrb_state *mrb, mrb_value self)
{
  mrb_int ret;

  ret = OsMsrSwiped();

  return mrb_fixnum_value(ret);
}

static mrb_value
mrb_magnetic_s_close(mrb_state *mrb, mrb_value self)
{
  OsMsrClose();

  return mrb_nil_value();
}

/*{:track1 => "", :track2 => "", :track3 => ""}*/
static mrb_value
mrb_magnetic_s_tracks(mrb_state *mrb, mrb_value self)
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
  mrb_hash_set(mrb, hash, mrb_symbol_value(mrb_intern_cstr(mrb, "track1")), mrb_str_new_cstr(mrb, (const char *)&track1.TrackData));
  mrb_hash_set(mrb, hash, mrb_symbol_value(mrb_intern_cstr(mrb, "track2")), mrb_str_new_cstr(mrb, (const char *)&track2.TrackData));
  mrb_hash_set(mrb, hash, mrb_symbol_value(mrb_intern_cstr(mrb, "track3")), mrb_str_new_cstr(mrb, (const char *)&track3.TrackData));

  return hash;
}

void
mrb_magnetic_init(mrb_state* mrb)
{
  struct RClass *pax;
  struct RClass *magnetic;

  pax      = mrb_class_get(mrb, "PAX");
  magnetic = mrb_define_class(mrb, "Magnetic", pax);

  mrb_define_class_method(mrb , magnetic , "open"      , mrb_magnetic_s_open      , MRB_ARGS_NONE());
  mrb_define_class_method(mrb , magnetic , "read"      , mrb_magnetic_s_read      , MRB_ARGS_NONE());
  mrb_define_class_method(mrb , magnetic , "close"     , mrb_magnetic_s_close     , MRB_ARGS_NONE());
  mrb_define_class_method(mrb , magnetic , "tracks"    , mrb_magnetic_s_tracks    , MRB_ARGS_REQ(1));
}

