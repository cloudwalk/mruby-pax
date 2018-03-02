class PrinterControl
  attr_accessor :flag_print, :kill, :mutex

  def initialize
    self.flag_print = false
    self.mutex      = Mutex.new
  end

  def print
    PAX::Printer._print_buffer
  end

  def kill!
    self.mutex.synchronize do
      @kill = true
    end
  end
end

