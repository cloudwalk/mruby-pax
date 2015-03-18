module Kernel
  PAX_KEY1       = 2
  PAX_KEY2       = 3
  PAX_KEY3       = 4
  PAX_KEY4       = 5
  PAX_KEY5       = 6
  PAX_KEY6       = 7
  PAX_KEY7       = 8
  PAX_KEY8       = 9
  PAX_KEY9       = 10
  PAX_KEY0       = 11
  PAX_KEYCANCEL  = 223
  PAX_KEYTIMEOUT = 18
  PAX_KEYCLEAR   = 14
  PAX_KEYENTER   = 28
  PAX_KEYSHARP   = 55
  PAX_KEYF1      = 59
  PAX_KEYALPHA   = 69
  PAX_KEYF2      = 60
  PAX_KEYF3      = 61
  PAX_KEYF4      = 62
  PAX_KEYFUNC    = 102
  PAX_KEYUP      = 103
  PAX_KEYDOWN    = 108
  PAX_KEYMENU    = 139

  IO_INPUT_NUMBERS = :numbers
  IO_INPUT_LETTERS = :letters
  IO_INPUT_SECRET  = :secret

  INPUT_NUMBERS = 36
  INPUT_LETTERS = 20
  INPUT_SECRET  = 28

  PAX_KEYS = {
    PAX_KEY0       => "0",
    PAX_KEY1       => "1",
    PAX_KEY2       => "2",
    PAX_KEY3       => "3",
    PAX_KEY4       => "4",
    PAX_KEY5       => "5",
    PAX_KEY6       => "6",
    PAX_KEY7       => "7",
    PAX_KEY8       => "8",
    PAX_KEY9       => "9",
    PAX_KEYF1      => 0x01.chr,
    PAX_KEYF2      => 0x02.chr,
    PAX_KEYF3      => 0x03.chr,
    PAX_KEYF4      => 0x04.chr,
    PAX_KEYFUNC    => 0x06.chr,
    PAX_KEYUP      => 0x07.chr,
    PAX_KEYDOWN    => 0x08.chr,
    PAX_KEYMENU    => 0x09.chr,
    PAX_KEYALPHA   => 0x10.chr,
    PAX_KEYSHARP   => 0x11.chr,
    PAX_KEYTIMEOUT => 0x12.chr,
    PAX_KEYENTER   => 0x0D.chr,
    PAX_KEYCLEAR   => 0x0F.chr,
    PAX_KEYCANCEL  => 0x1B.chr
  }

  # TODO Refactor needed
  def __printstr__(str, y = nil, x = nil)
    Screen.y = (y || Screen.y || 0)
    Screen.x = (x || Screen.x || 0)

    Screen.add_y(1) if str == "\n"

    str.split("\n").each_with_index do |string,index|
      Screen.add_y(1) if index > 0

      if (Screen.x + string.size) < Screen::SCREEN_X_SIZE
        _printstr__(string, Screen.y, Screen.x)
        Screen.x += string.size
      else
        space = Screen::SCREEN_X_SIZE - Screen.x
        _printstr__("#{string[0..(space - 1)]}", Screen.y, Screen.x)
        Screen.add_y(1)
        __printstr__("#{string[(space)..-1]}")
      end
    end
  end

  def gets(separator = 0x0D.chr, limit = Screen::SCREEN_X_SIZE, mode = IO_INPUT_LETTERS)
    Device::IO.get_string(1, limit, mode).split(separator).first
  end

  def getc(timeout_io = nil)
    timeout_io ||= IO.timeout
    convert_key(PAX._getc(timeout_io))
  end

  private
  def convert_key(value)
    PAX_KEYS[value] || 0x1B.chr
  end

  def convert_input_type(type)
    case type
    when IO_INPUT_NUMBERS then INPUT_NUMBERS
    when IO_INPUT_LETTERS then INPUT_LETTERS
    when IO_INPUT_SECRET then INPUT_SECRET
    else
      INPUT_LETTERS
    end
  end
end

