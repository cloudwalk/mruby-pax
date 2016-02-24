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

static mrb_value
mrb_s__serial(mrb_state *mrb, mrb_value self)
{
  char serial[10];

  memset(&serial, 0, sizeof(serial));

	OsRegGetValue("ro.fac.sn", serial);

  return mrb_str_new_cstr(mrb, serial);
}

static mrb_value
mrb_s__set_backlight(mrb_state *mrb, mrb_value self)
{
  mrb_int mode;

  mrb_get_args(mrb, "i", &mode);

  OsKbBacklight(mode);

  return mrb_fixnum_value(mode);
}

static mrb_value
mrb_s_battery(mrb_state *mrb, mrb_value self)
{
  return mrb_fixnum_value(OsCheckBattery());
}

static mrb_value
mrb_addrinfo_s__ip(mrb_state *mrb, mrb_value self)
{
  mrb_value host;
  mrb_int ret=-1;
  char dnsAddr[50]="\0";

  mrb_get_args(mrb, "o", &host);

  if (mrb_string_p(host)) {
    ret = OsNetDns(RSTRING_PTR(host), (char *)&dnsAddr, 5000);
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

  return mrb_nil_value();
}

static mrb_value
mrb_pax_s_reboot(mrb_state *mrb, mrb_value self)
{
  return mrb_fixnum_value(OsReboot());
}

static mrb_value
mrb_pax_s_hwclock(mrb_state *mrb, mrb_value self)
{
  ST_TIME t;
  mrb_int year, month, day, hour, minute, second;

  mrb_get_args(mrb, "iiiiii", &year, &month, &day, &hour, &minute, &second);

  t.Year   = year;
  t.Month  = month;
  t.Day    = day;
  t.Hour   = hour;
  t.Minute = minute;
  t.Second = second;

  return mrb_fixnum_value(OsSetTime(&t));
}

static mrb_value
mrb_pax_s__os_version(mrb_state *mrb, mrb_value self)
{
  char version[32]="\0";

  memset(&version, 0, sizeof(version));

  OsGetSysVer(TYPE_OS_VER, version);

  return mrb_str_new_cstr(mrb, version);
}

static mrb_value
mrb_pax_s__osal_version(mrb_state *mrb, mrb_value self)
{
  char version[32]="\0";

  memset(&version, 0, sizeof(version));

  OsGetSysVer(TYPE_OSAL_VER, version);

  return mrb_str_new_cstr(mrb, version);
}

static mrb_value
mrb_pax_s__pinpad_version(mrb_state *mrb, mrb_value self)
{
  char version[32]="\0";

  memset(&version, 0, sizeof(version));

  OsGetSysVer(TYPE_PED_VER, version);

  return mrb_str_new_cstr(mrb, version);
}

static mrb_value
mrb_system_s_model(mrb_state *mrb, mrb_value self)
{
  char model[32]="\0";

  memset(&model, 0, sizeof(model));

	OsRegGetValue("ro.fac.mach", model);

  return mrb_str_new_cstr(mrb, model);
}

void
mrb_system_init(mrb_state* mrb)
{
  struct RClass *pax, *system;

  pax    = mrb_define_class(mrb, "PAX", mrb->object_class);
  system = mrb_define_class_under(mrb, pax, "System", mrb->object_class);

  mrb_define_class_method(mrb , system , "_serial"         , mrb_s__serial             , MRB_ARGS_NONE());
  mrb_define_class_method(mrb , system , "_backlight="     , mrb_s__set_backlight      , MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb , system , "battery"         , mrb_s_battery             , MRB_ARGS_NONE());
  mrb_define_class_method(mrb , system , "_ip"             , mrb_addrinfo_s__ip        , MRB_ARGS_OPT(1));
  mrb_define_class_method(mrb , system , "beep"            , mrb_pax_s_beep            , MRB_ARGS_REQ(2));
  mrb_define_class_method(mrb , system , "_reboot"         , mrb_pax_s_reboot          , MRB_ARGS_NONE());
  mrb_define_class_method(mrb , system , "hwclock"         , mrb_pax_s_hwclock         , MRB_ARGS_REQ(6));
  mrb_define_class_method(mrb , system , "_os_version"     , mrb_pax_s__os_version     , MRB_ARGS_NONE());
  mrb_define_class_method(mrb , system , "_osal_version"   , mrb_pax_s__osal_version   , MRB_ARGS_NONE());
  mrb_define_class_method(mrb , system , "_pinpad_version" , mrb_pax_s__pinpad_version , MRB_ARGS_NONE());
  mrb_define_class_method(mrb , system , "model"           , mrb_system_s_model        , MRB_ARGS_NONE());
}

