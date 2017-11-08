class Time
  def hwclock(timezone = nil)
    ret = PAX::System.hwclock(self.year, self.month, self.day, self.hour, self.min, self.sec)
    if ret == 0
      PAX::System._os_set_value("persist.sys.timezone.tz", timezone || "+0300")
    end
    ret
  end
end

