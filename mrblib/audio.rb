class PAX
  class Audio
    def self.volume=(value)
      path = "/sys/devices/platform/keypad/buzzer_keypad_volume"
      if File.exists? path
        file = File.open("/sys/devices/platform/keypad/buzzer_keypad_volume", "w+")
        file.write("#{value}\n")
        file.close
      end
    end
  end
end

