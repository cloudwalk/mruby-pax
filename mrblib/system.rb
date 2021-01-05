#
# @file system.rb
# @brief mruby-pax system utilities.
# @platform Pax Prolin
#
# @copyright Copyright (c) 2016 CloudWalk, Inc.
#

# mruby-pax C/C++ interface exposure.
class PAX
  # Subclass definition.
  class System
    # (osal.h) enum POWER_TYPE macros
    POWER_ADAPTER         = 1
    POWER_USB             = 2
    POWER_BATTERY         = 3
    POWER_WPC             = 4

    # (osal.h) System Manager macros
    FILE_TYPE_APP         = 1
    FILE_TYPE_APP_PARAM   = 2
    FILE_TYPE_SYS_LIB     = 3
    FILE_TYPE_PUB_KEY     = 4
    FILE_TYPE_AUP         = 5
    FILE_TYPE_KERNEL      = 6
    FILE_TYPE_RAMDISK     = 7
    FILE_TYPE_BASE        = 8
    FILE_TYPE_FWP         = 9

    # Deprecated macros
    DEFAULT_BACKLIGHT     = 1
    RET_OK                = 0

    # Returns the device serial number.
    def self.serial
      self._serial
    end

    # Defines screen brightness level [0~100].
    def self.backlight=(level)
      level > 100 && level = 100
      level < 0 && level = 0
      level = ((level * 7) / 100).to_i

      if level == 0
        self._kb_backlight = 0
        self._sleep_mode = 1
      else
        self._kb_backlight = 1
        self._sleep_mode = 0
      end

      self._backlight = level
    end

    # (deprecated) Returns default backlight for DaFunk:
    def self.backlight
      # - 0: Turn off
      # - 1: (D200) Keep on for 30 seconds
      # - 2: (D200) Always on

      DEFAULT_BACKLIGHT
    end

    # Defines system execution mode: 0 (active), 1 (screensaver) and 2
    # (sleep).
    def self.sleep_mode=(mode)
      self._sleep_mode = mode
    end

    # Defines keyboard backlight behavior: 0 to disable, non-zero to enable.
    def self.kb_backlight=(level)
      self._kb_backlight = level
    end

    # Defines the battery capacity type of return (percentage or scale).
    def self.battery_capacity_type
      'percentage' # otherwise, 'scale'
    end

    # Returns current battery capacity (%) in the range [-1~100].
    def self.battery
      _battery.to_i
    end

    # Checks if device is connected to any power supply.
    def self.power_supply
      [POWER_ADAPTER, POWER_USB].include?(self._power_supply)
    end

    # Returns the device model (downcased).
    def self.model
      self._model.to_s.downcase
    end

    # Reboots the device.
    def self.reboot
      self._reboot
    end

    # Returns device brand (downcased).
    def self.brand
      'pax'
    end

    # Updates the .aip application.
    def self.update(path)
      ret = self.install('MAINAPP', path, FILE_TYPE_APP)
      if ret == RET_OK
        true
      else
        ContextLog.info "System Update - Error [#{path}][#{ret.inspect}]"
        false
      end
    end

    # Returns versions from EMV, OS, PINPAD (built-in PED) and SDK.
    def self.versions
      unless @versions
        @versions = {}

        @versions['EMV'] = PAX::EMV.version
        @versions['OS'] = self._os_version
        @versions['Pinpad'] = self._pinpad_version
        @versions['SDK'] = self._osal_version
      end
      @versions
    end

    def self.teardown
      PAX::Printer.thread_kill
    end

    class << self
      alias_method :restart, :reboot
    end
  end
end
