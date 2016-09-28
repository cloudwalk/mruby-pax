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
      "main_#{PAX::System.model}.bmp"
    end
  end

  def self.set_os_values
    PAX::System._os_set_value("persist.sys.sound.enable", "true")
    PAX::Audio.volume = 5
  end

  def self.define_device_modules
    Device.const_set(:Pinpad, PAX::Pinpad)
    Device.const_set(:EMV, PAX::EMV)
  end

  def self.screen_definition
    case PAX::System.model
    when "d200"
      [21, 7]
    when "s920"
      [21, 14]
    else
      [21, 7]
    end
  end

  def self.printer_start
    ret = Device::Printer.start
    ContextLog.info("Error Printer start [#{ret}]") if ret != PAX::Printer::RET_OK
  end

  def self.setup
    Screen.setup(*screen_definition)
    begin
      define_device_modules
      require 'posxml_parser'
      require 'cloudwalk_handshake'
      CloudwalkHandshake.configure!
      self.set_os_values
      self.printer_start
    rescue LoadError => e
      ContextLog.exception(e, e.backtrace)
    rescue NameError => e
      ContextLog.exception(e, e.backtrace)
    end
  end
end

