class PAX
  Network = ::Network

  class Audio
    def self.beep(tone, milliseconds)
      PAX.beep(tone, milliseconds)
    end
  end

  class Display
    SLOTS_AVAILABLES = 8
    def self.clear
      PAX.display_clear
    end

    def self.clear_line(line)
      PAX.display_clear_line(line)
    end

    def self.display_bitmap(path, row = 0, column = 0)
      PAX.print_bitmap(path, row, column)
    end

    def self.print_in_line(buf, row=0, column=0)
      __print__(buf, row, column)
    end

    def self.status_bar_slots_available
      SLOTS_AVAILABLES
    end

    def self.main_image
      "walk_paxd200.bmp"
    end
  end

  def self.setup
    Screen.setup(21, 7)
    begin
      require 'posxml_parser'
      require 'cloudwalk_handshake'
      CloudwalkHandshake.configure!
    rescue LoadError
    rescue NameError
    end
  end
end

