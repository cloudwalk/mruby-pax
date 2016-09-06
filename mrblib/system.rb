class PAX
  class System
    DEFAULT_BACKLIGHT = 1
    RET_OK            = 0

    def self.serial
      PAX::System._serial
    end

    def self.backlight=(level)
      PAX::System._backlight = level
    end

    def self.backlight
      DEFAULT_BACKLIGHT
    end

    def self.reboot
      PAX::System._reboot
    end

    POWER_ADAPTER = 1
    POWER_USB     = 2
    POWER_BATTERY = 3

    def self.power_supply
      power = self._power_supply
      power == POWER_ADAPTER || power == POWER_USB
    end

    BATTERY_LEVEL_0        = 0	# Power  0~20%
    BATTERY_LEVEL_1        = 1	# Power 20~40%
    BATTERY_LEVEL_2        = 2	# Power 40~60%
    BATTERY_LEVEL_3        = 3	# Power 60~80%
    BATTERY_LEVEL_4        = 4	# Power 80~100%
    BATTERY_LEVEL_CHARGE   = 5	# Battery is being charged
    BATTERY_LEVEL_COMPLETE = 6	# Battery charge complete
    BATTERY_LEVEL_ABSENT   = 7	# Battery is absent

    def self.battery
      case value = self._battery
      when 0..4
        value * 25
      when 5
        50
      when 6
        100
      else
        -1
      end
    end

    def self.brand
      "pax"
    end

    FILE_TYPE_APP       = 1 # Application
    FILE_TYPE_APP_PARAM = 2 # Application parameter
    FILE_TYPE_SYS_LIB   = 3 # Dynamic system library
    FILE_TYPE_PUB_KEY   = 4 # User public key
    FILE_TYPE_AUP       = 5 # Application upgrade kit
    FILE_TYPE_KERNEL    = 6	# Firmware kernel, "firmware-kernel"
    FILE_TYPE_RAMDISK   =	7	# Firmware ramdisk, "firmware-ramdisk"
    FILE_TYPE_BASE      = 8	# Firmware base, "firmware-base"

    def self.update(path)
      ret_install = self.install("MAINAPP", path, FILE_TYPE_APP)
      if ret_install == RET_OK
        true
      else
        ContextLog.info "System Update - Error [#{path}][#{ret_install.inspect}]"
        false
      end
    end

    def self.versions
      unless @versions
        @versions = Hash.new
        @versions["OS"]     = self._os_version
        @versions["SDK"]    = self._osal_version
        @versions["EMV"]    = PAX::EMV.version
        @versions["Pinpad"] = self._pinpad_version
      end
      @versions
    end

    class << self
      alias_method :restart, :reboot
    end
  end
end

