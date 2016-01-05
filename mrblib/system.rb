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

    def self.battery
      PAX::System._battery
    end

    def self.reboot
      PAX::System._reboot
    end

    # TODO Initially the SDK has no function to get the exactly model
    def self.model
      "D200"
    end

    def self.brand
      "pax"
    end

    def self.versions
      {
        "OS"     => self._os_version
        "SDK"    => self._osal_version,
        "EMV"    => PAX::EMV.version,
        "Pinpad" => self._pinpad_version
      }
    end

    class << self
      alias_method :restart, :reboot
    end
  end
end

