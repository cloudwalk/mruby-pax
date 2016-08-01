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

