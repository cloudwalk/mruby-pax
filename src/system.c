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

static mrb_value
mrb_addrinfo_s__ip(mrb_state *mrb, mrb_value self)
{
  mrb_value host;
  mrb_int ret;
  char dnsAddr[50]="\0";

  mrb_get_args(mrb, "o", &host);

  if (mrb_string_p(host)) {
    ret = OsNetDns(RSTRING_PTR(host), &dnsAddr, 5000);
  }

  if (ret == RET_OK)
    return mrb_str_new(mrb, (void *)&dnsAddr, strlen(dnsAddr));
  else
    return host;
}

static mrb_value
mrb_pax_s_beep(mrb_state *mrb, mrb_value self)
{
  mrb_int tone, milliseconds;

  mrb_get_args(mrb, "ii", &tone, &milliseconds);

  OsBeep(tone, milliseconds);
}

static mrb_value
mrb_pax_execute(mrb_state *mrb, mrb_value self)
{
  char code[1024];
  char *s_app_name;
  mrb_state *mrb2;
  mrb_value app_name;
  mrbc_context *c;

  mrb_get_args(mrb, "S", &app_name);

  if (mrb_string_p(app_name)) {
    s_app_name = RSTRING_PTR(app_name);
  } else
    return mrb_false_value();

  memset(code, 0, sizeof(code));
  sprintf(code, "PAX.execute(\"%s\")", s_app_name);

  mrb2 = mrb_open();

  c = mrbc_context_new(mrb2);
  mrb_load_string_cxt(mrb2, code, c);
  mrbc_context_free(mrb2, c);
  mrb_close(mrb2);

  return mrb_true_value();
}

void
mrb_system_init(mrb_state* mrb)
{
  struct RClass *pax;
  struct RClass *krn;

  krn = mrb->kernel_module;
  pax = mrb_define_class(mrb, "PAX", mrb->object_class);

  mrb_define_class_method(mrb , pax , "_serial"            , mrb_s__serial                , MRB_ARGS_NONE());
  mrb_define_class_method(mrb , pax , "_backlight="        , mrb_s__set_backlight         , MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb , pax , "_battery"           , mrb_s__battery               , MRB_ARGS_NONE());
  mrb_define_class_method(mrb , pax , "_ip"                , mrb_addrinfo_s__ip           , MRB_ARGS_OPT(1));
  mrb_define_class_method(mrb , pax , "beep"               , mrb_pax_s_beep               , MRB_ARGS_REQ(2));
  mrb_define_class_method(mrb , pax , "_execute"           , mrb_pax_execute              , MRB_ARGS_REQ(1));
}
