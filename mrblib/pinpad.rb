class PAX

  class Pinpad
    RET_OK              = 0
    LOADKEY_DES         = 0
    LOADKEY_3DES        = 1

    PED_INT_TMK         = 2
    PED_INT_TLK         = 1
    PED_INT_TDK         = 5
    MODE_ECB_DECRYPTION = 0
    MODE_ECB_ENCRYPTION = 1
    MODE_CBC_DECRYPTION = 2
    MODE_CBC_ENCRYPTION = 3
    MODE_OFB_DECRYPTION = 4
    MODE_OFB_ENCRYPTION = 5

    TDK_SLOT            = 1
    TWK_SLOT            = 2

    PED_TMK             = "\x02"
    PED_TLK             = "\x01"
    PED_TDK             = "\x05"
    MODE_DECRYPTION     = "\x00"
    MODE_ENCRYPTION     = "\x01"

    DUKPT_DEFAULT = {
      "KSN"      => "",
      "PINBLOCK" => ""
    }

    DES_DEFAULT = {
      "BLOCK"  => "",
      "RETURN" => ""
    }

    class << self
      attr_accessor :pinpad
    end

    def self.dukpt_default
      DUKPT_DEFAULT.dup
    end

    def self.des_default
      DES_DEFAULT.dup
    end

    def self.load_tdk(slot_mk, wk, slot_tdk = 1)
      value = "\x03" + PAX::Pinpad::PED_TMK + slot_mk.chr + slot_tdk.chr + "0000000" +
        PAX::Pinpad::PED_TDK + wk.size.chr + wk + "00000000" + "\x00" +
        ("0" * 128) + "00000000" + "0000000000"

      ret = PAX::Pinpad.load_key(value)
      ContextLog.info "TDK #{ret.inspect}"
      #ret = PAX::Pinpad.derive(PAX::Pinpad::PED_TMK, 17, PAX::Pinpad::PED_TDK, 1, 2)
      #ret = PAX::Pinpad.derive(2, 17, 5, 1, 2, 0)
      #ContextLog.info "TWK #{ret.inspect}"
      ret
    end

  end
end
