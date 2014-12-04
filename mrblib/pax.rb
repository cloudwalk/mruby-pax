class PAX
  Network = ::Network
  IO      = ::IO

  class IO
    def self.read_card(timeout)
      time = Time.now + (timeout.to_f / 1000.0)
      PAX.magnetic_open

      loop do
        break if PAX.magnetic_read == 1 || time <= Time.now
      end

      PAX.magnetic_tracks
    ensure
      PAX.magnetic_close
    end
  end

  class Audio
    def self.beep(tone, milliseconds)
      PAX.beep(tone, milliseconds)
    end
  end

  class Runtime
    def self.execute(app)
      PAX._execute(app)
    end
  end

  class Display
    def self.clear
      PAX.display_clear
      screen_x = 0
      screen_y = 0
    end

    def self.clear_line(line)
      PAX.display_clear_line(line)
    end

    def self.display_bitmap(path, row = 0, column = 0)
      PAX.print_bitmap(path, row, column)
    end
  end

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

    def self.battery
      PAX._battery
    end
  end

  # Method should be implemented on platform class
  #  class Platform
  #    def self.start file
  #      require 'da_funk.mrb'
  #      require '<platform.mrb>'
  #
  #      app = Device::Support.path_to_class file
  #
  #      app.call
  #    end
  #  end
  def self.start(file = "./mrb/main.mrb")
    # TODO ./file has some leak problem after 4 tries
    begin
      # Library responsable for common code and API syntax for the user
      require "./mrb/da_funk.mrb"
      # Platform library responsible for implement the adapter for DaFunk
      # class Device #DaFunk abstraction
      #   self.adapter =
      require "./mrb/pax.mrb"
      require file

      # Method to contantize name of file, example:
      #   main.mrb/main.rb - Main
      app = Device::Support.path_to_class file

      loop do
        # Main should implement method call
        #  method call was need to avoid memory leak on irep table
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
