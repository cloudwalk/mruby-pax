class Time
  def hwclock
    PAX.hwclock(self.year, self.month, self.day, self.hour, self.min, self.sec)
  end
end
