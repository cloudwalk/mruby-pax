class PAX
  IO = ::IO

  class IO
    def self.get_string(min, max, mode = IO_INPUT_LETTERS)
      PAX._gets(min, max, input_type(mode), Screen.y, Screen.x)
    end
  end
end
