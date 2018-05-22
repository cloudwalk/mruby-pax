class Time
  def hwclock(timezone = nil)
    ret = PAX::System.hwclock(self.year, self.month, self.day, self.hour, self.min, self.sec)
    if ret == 0
      timezone = timezone.to_s.strip.empty? ? "-0300" : timezone
      PAX::System._os_set_value("persist.sys.timezone.tz", timezone)
    end
    ret
  end
end

