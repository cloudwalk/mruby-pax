class PAX
  class Touch
    def self.getxy(timeout = 10_000)
      hash = self._getxy(timeout)
      if hash["return"] == 1
        PAX::Audio.beep(7, 60)
        [hash["x"], hash["y"]]
      end
    end

    def self.getxy_stream(timeout = 10_000)
      hash = self._getxy_stream(timeout)
      if hash["return"] == 1
        PAX::Audio.beep(7, 60)
        [hash["x"], hash["y"]]
      end
    end
  end
end

