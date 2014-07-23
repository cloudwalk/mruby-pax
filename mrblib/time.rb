class Time
  def self.now
    datetime = self._pax_time
    date, time = datetime.split(" ")
    date = date.split("-")
    time = time.split(":")
    Time.mktime(date[0].to_i, date[1].to_i, date[2].to_i, time[0].to_i, time[1].to_i, time[2].to_i)
  end
end
