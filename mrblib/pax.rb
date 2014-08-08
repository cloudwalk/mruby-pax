class PAX
  Network = ::Network
  IO = ::IO
  class System
    DEFAULT_BACKLIGHT = 1
    def self.serial
      PAX._serial
    end

    def self.backlight=(level)
      PAX._backlight = level
    end

    def self.backlight
      DEFAULT_BACKLIGHT
    end
  end

  def self.start(file = "main.mrb")
    # TODO ./file has some leak problem after 4 tries
    begin
      require "da_funk.mrb"
      require "pax.mrb"
      require file

      app = Device::Support.path_to_class file

      loop do
        app.call
      end
    rescue => @exception
      puts "#{@exception.class}: #{@exception.message}"
      puts "#{@exception.backtrace[0..2].join("\n")}"
      IO.getc
      return nil
    end
  end
end
