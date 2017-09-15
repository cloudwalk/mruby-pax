class PAX
  class Audio
    def self.volume=(value)
      @volume = value
      if value == 0
        PAX::System._os_set_value("persist.sys.sound.enable", "false")
      else
        path = "/sys/devices/platform/keypad/buzzer_keypad_volume"
        if File.exists? path
          File.open("/sys/devices/platform/keypad/buzzer_keypad_volume", "w") {|f| f.write("#{value}\n") }
        end
      end
    rescue IOError
    end

    def self.volume
      @volume || 0
    end
  end
end

