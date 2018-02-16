class PAX
  class Magnetic
    def self.read
      if self._read == 0
        PAX::Audio.beep(7, 60)
        1
      else
        0
      end
    end
  end
end

