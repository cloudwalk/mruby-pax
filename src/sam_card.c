#include <stdlib.h>
#include <stdio.h>
#include "mruby.h"
#include "mruby/compile.h"
#include "mruby/value.h"
#include "mruby/array.h"
#include "mruby/string.h"
#include "mruby/hash.h"

#include "osal.h"
#include "mruby/ext/context_log.h"

/*
 * = How to debug
 *   This function call be called to store any string at the file "data/app/MAINAPP/main/<year>-<month>-<day>.log"
 *   that can be access via Telnet "telnet 127.0.0.1 2323", remember to start the telnet server at TermAssist
 *
 * ContextLog(mrb, 0, "2TLV [%d][%s][%d]", iTag, psDat, iDataLen);
 */

static mrb_value
mrb_sam_card_power(mrb_state *mrb, mrb_value self)
{
  mrb_int status, historical_size, ret;
  mrb_value historical;

  mrb_get_args(mrb, "i|iS", &status, &historical_size, &historical);

  if (status == 1) { // Turn on
  } else { // Turn off
  }

  return mrb_fixnum_value(ret);
}

static mrb_value
mrb_sam_card_send(mrb_state *mrb, mrb_value self)
{
  mrb_int ret;
  mrb_value historical;

  mrb_get_args(mrb, "S", &buffer);

  /*
   *Do something with buffer:
   *  - get pointer: RSTRING_PTR(buffer);
   *  - get size: RSTRING_LEN(buffer);
   */

  return mrb_fixnum_value(ret);
}

static mrb_value
mrb_sam_card_read(mrb_state *mrb, mrb_value self)
{
  mrb_int ret;
  mrb_value buffer;

  /*
   * Return mruby string object:
   * - From NULL pointer char array: mrb_str_new_cstr(mrb, "VALUE"), 
   * - From delimited char array size: mrb_str_new(mrb, &value, 10))
   */

  return buffer;
}

void
mrb_touch_init(mrb_state* mrb)
{
  struct RClass *pax, *sam_card;

  pax      = mrb_class_get(mrb, "PAX");
  sam_card = mrb_define_class_under(mrb, pax, "SamCard", mrb->object_class);

  mrb_define_class_method(mrb , sam_card , "power" , mrb_sam_card_power , MRB_ARGS_REQ(1) | MRB_ARGS_OPT(2));
  mrb_define_class_method(mrb , sam_card , "send"  , mrb_sam_card_send  , MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb , sam_card , "read"  , mrb_sam_card_read  , MRB_ARGS_NONE());
}

