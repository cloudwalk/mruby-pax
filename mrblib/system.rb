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

    # Static class variables
    @@battery_capacity = -1
    @@semaphore = Mutex.new
    @@timestamp = [Time.now, 0]

    # Returns the device serial number.
    def self.serial
      _serial
    end

    # Defines screen brightness level [0~100].
    def self.backlight=(level)
      level.negative? && level = 0

      level > 100 && level = 100

      ret = _backlight = (level * 7 / 100).to_i

      if level.zero?
        _kb_backlight = 0
        _sleep_mode = 1
      else
        _kb_backlight = 1
        _sleep_mode = 0
      end

      ret
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
      _sleep_mode = mode
    end

    # Defines keyboard backlight behavior: 0 to disable, non-zero to enable.
    def self.kb_backlight=(level)
      _kb_backlight = level
    end

    # Returns current battery capacity (%) in the range [-1~100].
    def self.battery
      @@semaphore.lock

      @@timestamp[1] = Time.now

      if @@timestamp[1] > @@timestamp[0]
        @@timestamp[0] = Time.now + 4 # To prevent stressing both file system
                                      # and battery

        @@battery_capacity = _battery

        ContextLog.info "System Battery - #{__method__}"
      end

      ContextLog.info "System Battery - Capacity [#{@@battery_capacity}%]"

      capacity = @@battery_capacity

      @@semaphore.unlock

      capacity
    end

    # Checks if device is connected to any power supply.
    def self.power_supply
      [POWER_SOURCE_ADAPTER, POWER_SOURCE_USB].include?(_power_supply)
    end

    # Returns the device model (downcased).
    def self.model
      _model.to_s.downcase
    end

    # Reboots the device.
    def self.reboot
      _reboot
    end

    # Returns device brand (downcased).
    def self.brand
      'pax'
    end

    # Updates the main application.
    def self.update(path)
      ret = install('MAINAPP', path, FILE_TYPE_APP)
      if !ret
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
        @versions['OS'] = _os_version
        @versions['Pinpad'] = _pinpad_version
        @versions['SDK'] = _osal_version
      end
      @versions
    end

    def self.teardown
      PAX::Printer.thread_kill
    end

    class << self
      alias restart reboot
    end
  end
end
