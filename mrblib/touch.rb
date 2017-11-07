class PAX
  class Touch
    def self.getxy(timeout = 10_000)
      hash = self._getxy(timeout)
      if hash["return"] == 1
        [hash["x"], hash["y"]]
      end
    end
  end
end

