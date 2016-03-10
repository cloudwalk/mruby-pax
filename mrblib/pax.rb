class PAX
  Network = ::Network

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

  def self.set_os_values
    PAX::System._os_set_value("persist.sys.sound.enable", "true")
    PAX::Audio.volume = 5
  end

  def self.setup
    Screen.setup(21, 7)
    begin
      require 'posxml_parser'
      require 'cloudwalk_handshake'
      CloudwalkHandshake.configure!
      self.set_os_values
    rescue LoadError
    rescue NameError
    end
  end
end

