class Time
  def hwclock(offset = nil)
    time = cw_utc(offset)
    ret = PAX::System.hwclock(time.year, time.month, time.day, time.hour, time.min, time.sec)
    if ret == 0
      if offset
        if offset.to_s.include? "+"
          timezone = offset.to_s.sub("+", "-")
        else
          timezone = offset.to_s.sub("-", "+")
        end
      else
        timezone = "-0300"
      end
      PAX::System._os_set_value("persist.sys.timezone.tz", timezone)
    end
    ret
  end

  def cw_utc(offset = nil)
    if offset && (! offset.to_s.strip.empty?) && cw_utc?
      factor = offset.to_s[0..2].to_i
      self + (60 * 60 * factor)
    else
      self
    end
  end

  def cw_utc?
    if Object.const_defined?(:Device)
      DaFunk::ParamsDat.file["time_utc"] == "1"
    end
  rescue
    false
  end
end
