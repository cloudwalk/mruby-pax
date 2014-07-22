module Kernel
  def sleep(seconds)
    __sleep__ seconds * 1000
  end
end
