class PrinterControl
  attr_accessor :flag_print

  def initialize
    self.flag_print = false
  end

  def print
    PAX::Printer._print_buffer
  end
end

