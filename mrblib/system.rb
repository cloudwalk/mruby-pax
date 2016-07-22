class PAX
  class System
    DEFAULT_BACKLIGHT = 1
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

