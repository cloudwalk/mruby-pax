class PAX

  class Crypto
    LOADKEY_DES         = 0
    LOADKEY_3DES        = 1

    class << self
      attr_accessor :crypto
    end
  end
end
