/**
 * @file system.c
 * @brief mruby-pax system utilities.
 * @platform Pax Prolin
 * @date 2015-01-14
 *
 * @copyright Copyright (c) 2015 CloudWalk, Inc.
 *
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "mruby.h"
#include "mruby/array.h"
#include "mruby/compile.h"
#include "mruby/hash.h"
#include "mruby/string.h"
#include "mruby/value.h"

#include "osal.h"
#include "runtime_system.h"
#include "ui.h"
#include "xui.h"

/********************/
/* Global variables */
/********************/

/* Extern */

int reload_flag = 0;

/* Static */

static pthread_mutex_t system_lock;

/*********************/
/* Private functions */
/*********************/

/**
 * @brief Returns the device serial number.
 */
static mrb_value
mrb_s__serial(mrb_state *mrb, mrb_value self)
{
    char serial[255 + 1];
    mrb_int len;

    memset(&serial, 0, sizeof(serial));

    len = OsRegGetValue("ro.fac.sn", serial);

    if (len > 0)
        return mrb_str_new(mrb, serial, len);
    else
        return mrb_str_new(mrb, 0, 0);
}

/**
 * @brief Defines screen brightness level. Default value is 8. Values outside
 * the range [0~10] result into no action.
 */
static mrb_value
mrb_s__set_backlight(mrb_state *mrb, mrb_value self)
{
    mrb_int mode;

    mrb_get_args(mrb, "i", &mode);

    OsScrBrightness(mode);

    return mrb_fixnum_value(mode);
}

/**
 * @brief Defines system execution mode: 0 (active), 1 (screensaver) and 2
 * (sleep). At screensaver mode, CPU is kept active.
 */
static mrb_value
mrb_s__set_sleep_mode(mrb_state *mrb, mrb_value self)
{
    mrb_int mode;

    mrb_get_args(mrb, "i", &mode);

    OsSysSleepEx(mode);

    return mrb_fixnum_value(mode);
}

/**
 * @brief Defines keyboard backlight behavior: 0 to disable, non-zero to
 * enable.
 */
static mrb_value
mrb_s__set_kb_backlight(mrb_state *mrb, mrb_value self)
{
    mrb_int mode;

    mrb_get_args(mrb, "i", &mode);

    OsKbBacklight(mode);

    return mrb_fixnum_value(mode);
}

/**
 * @brief Returns current battery capacity (%). In the event of an error, the
 * last known value in the range [-1~100] is returned.
 */
static mrb_value
mrb_s_battery(mrb_state *mrb, mrb_value self)
{
    static int battery_percentage = -1;

    FILE *fd;
    char capacity_value[255 + 1];
    mrb_value return_value;

    pthread_mutex_lock(&system_lock);

    memset(&return_value, 0, sizeof(mrb_value));

    fd = fopen("/sys/class/power_supply/battery/capacity", "r");

    if (fd)
    {
        if (fgets(capacity_value, sizeof(capacity_value), fd))
        {
            battery_percentage = atoi(capacity_value);
        }

        fclose(fd);
    }

    return_value = mrb_fixnum_value(battery_percentage);

    pthread_mutex_unlock(&system_lock);

    return return_value;
}

/**
 * @brief Checks if device is connected to any power supply.
 */
static mrb_value
mrb_s__power_supply(mrb_state *mrb, mrb_value self)
{
    return mrb_fixnum_value((int) OsCheckPowerSupply());
}

/**
 * @brief Returns a given host IP address (DNS).
 */
static mrb_value
mrb_addrinfo_s__ip(mrb_state *mrb, mrb_value self)
{
    mrb_value host;
    mrb_int ret = -1;
    char dnsAddr[50] = "\0";

    mrb_get_args(mrb, "o", &host);

    if (mrb_string_p(host))
    {
        ret = OsNetDns(RSTRING_PTR(host), (char *)&dnsAddr, 30000);
    }

    if (ret == RET_OK)
        return mrb_str_new_cstr(mrb, dnsAddr);
    else
        return host;
}

/**
 * @brief Plays a beep according to given tone and milliseconds.
 */
static mrb_value
mrb_pax_audio_s_beep(mrb_state *mrb, mrb_value self)
{
    mrb_int tone, milliseconds;

    mrb_get_args(mrb, "ii", &tone, &milliseconds);

    OsBeep(tone, milliseconds);

    return mrb_nil_value();
}

/**
 * @brief Reboots the device.
 */
static mrb_value
mrb_pax_s_reboot(mrb_state *mrb, mrb_value self)
{
    return mrb_fixnum_value(OsReboot());
}

/**
 * @brief Defines the time at the system clock.
 */
static mrb_value
mrb_pax_s_hwclock(mrb_state *mrb, mrb_value self)
{
    ST_TIME t;
    mrb_int year, month, day, hour, minute, second;

    mrb_get_args(mrb, "iiiiii", &year, &month, &day, &hour, &minute, &second);

    t.Year = year;
    t.Month = month;
    t.Day = day;
    t.Hour = hour;
    t.Minute = minute;
    t.Second = second;

    return mrb_fixnum_value(OsSetTime(&t));
}

/**
 * @brief Returns operational system version.
 */
static mrb_value
mrb_pax_s__os_version(mrb_state *mrb, mrb_value self)
{
    char version[31] = "\0";

    memset(&version, 0, sizeof(version));

    OsGetSysVer(TYPE_OS_VER, version);

    return mrb_str_new_cstr(mrb, version);
}

/**
 * @brief Returns PAX API library version.
 */
static mrb_value
mrb_pax_s__osal_version(mrb_state *mrb, mrb_value self)
{
    char version[31] = "\0";

    memset(&version, 0, sizeof(version));

    OsGetSysVer(TYPE_OSAL_VER, version);

    return mrb_str_new_cstr(mrb, version);
}

/**
 * @brief Returns built-in PED (PINPAD) version.
 */
static mrb_value
mrb_pax_s__pinpad_version(mrb_state *mrb, mrb_value self)
{
    char version[31] = "\0";

    memset(&version, 0, sizeof(version));

    OsGetSysVer(TYPE_PED_VER, version);

    return mrb_str_new_cstr(mrb, version);
}

/**
 * @brief Returns the device model.
 */
static mrb_value
mrb_system_s__model(mrb_state *mrb, mrb_value self)
{
    mrb_int len;
    char model[64] = "\0";

    memset(&model, 0, sizeof(model));

    len = OsRegGetValue("ro.fac.mach", model);
    if (len > 0)
        return mrb_str_new(mrb, model, len);
    else
        return mrb_str_new(mrb, 0, 0);
}

/**
 * @brief Defines a given property value.
 */
static mrb_value
mrb_system_s_os_set_value(mrb_state *mrb, mrb_value self)
{
    mrb_value key, value;
    mrb_int ret;

    mrb_get_args(mrb, "SS", &key, &value);

    ret = OsRegSetValue((char *) RSTRING_PTR(key), (char *) RSTRING_PTR(value));

    if (ret == RET_OK)
        return mrb_true_value();
    else
        return mrb_false_value();
}

/**
 * @brief Returns a given property value.
 */
static mrb_value
mrb_system_s_os_get_value(mrb_state *mrb, mrb_value self)
{
    char value[1024] = "\0";
    mrb_value key;
    mrb_int len;

    memset(&value, 0, sizeof(value));

    mrb_get_args(mrb, "S", &key);

    len = OsRegGetValue(RSTRING_PTR(key), (char *)&value);
    if (len > 0)
        return mrb_str_new(mrb, value, len);
    else
        return mrb_str_new(mrb, 0, 0);
}

/**
 * @brief Installs a component on the device.
 */
static mrb_value
mrb_system_s_install(mrb_state *mrb, mrb_value self)
{
    mrb_int type;
    mrb_value path, name;

    mrb_get_args(mrb, "SSi", &name, &path, &type);

    return mrb_fixnum_value(OsInstallFile(RSTRING_PTR(name), RSTRING_PTR(path), type));
}

/**
 * @brief Signals a mruby reload.
 */
static mrb_value
mrb_system_s_reload(mrb_state *mrb, mrb_value self)
{
    reload_flag = 1; /* TODO: shouldn't be mutex protected?! */

    return mrb_true_value();
}

/********************/
/* Public functions */
/********************/

/**
 * @brief Exposes mruby-pax system utilities.
 */
void mrb_system_init(mrb_state *mrb)
{
    static int mutex_init = 0;

    struct RClass *audio;
    struct RClass *pax;
    struct RClass *system;

    pax = mrb_define_class(mrb, "PAX", mrb->object_class);

    audio = mrb_define_class_under(mrb, pax, "Audio", mrb->object_class);

    mrb_define_class_method(mrb, audio, "beep", mrb_pax_audio_s_beep, MRB_ARGS_REQ(2));

    system = mrb_define_class_under(mrb, pax, "System", mrb->object_class);

    mrb_define_class_method(mrb, system, "_backlight=", mrb_s__set_backlight, MRB_ARGS_REQ(1));
    mrb_define_class_method(mrb, system, "_battery", mrb_s_battery, MRB_ARGS_NONE());
    mrb_define_class_method(mrb, system, "_ip", mrb_addrinfo_s__ip, MRB_ARGS_OPT(1));
    mrb_define_class_method(mrb, system, "_kb_backlight=", mrb_s__set_kb_backlight, MRB_ARGS_REQ(1));
    mrb_define_class_method(mrb, system, "_model", mrb_system_s__model, MRB_ARGS_NONE());
    mrb_define_class_method(mrb, system, "_os_get_value", mrb_system_s_os_get_value, MRB_ARGS_REQ(1));
    mrb_define_class_method(mrb, system, "_os_set_value", mrb_system_s_os_set_value, MRB_ARGS_REQ(2));
    mrb_define_class_method(mrb, system, "_os_version", mrb_pax_s__os_version, MRB_ARGS_NONE());
    mrb_define_class_method(mrb, system, "_osal_version", mrb_pax_s__osal_version, MRB_ARGS_NONE());
    mrb_define_class_method(mrb, system, "_pinpad_version", mrb_pax_s__pinpad_version, MRB_ARGS_NONE());
    mrb_define_class_method(mrb, system, "_power_supply", mrb_s__power_supply, MRB_ARGS_NONE());
    mrb_define_class_method(mrb, system, "_reboot", mrb_pax_s_reboot, MRB_ARGS_NONE());
    mrb_define_class_method(mrb, system, "_serial", mrb_s__serial, MRB_ARGS_NONE());
    mrb_define_class_method(mrb, system, "_sleep_mode=", mrb_s__set_sleep_mode, MRB_ARGS_REQ(1));
    mrb_define_class_method(mrb, system, "hwclock", mrb_pax_s_hwclock, MRB_ARGS_REQ(6));
    mrb_define_class_method(mrb, system, "install", mrb_system_s_install, MRB_ARGS_REQ(3));
    mrb_define_class_method(mrb, system, "reload", mrb_system_s_reload, MRB_ARGS_NONE());

    if (!mutex_init)
    {
        pthread_mutex_init(&system_lock, NULL);

        mutex_init = 1;
    }
}
