class PAX
  class Pinpad
    RET_OK              = 0
    LOADKEY_DES         = 0
    LOADKEY_3DES        = 1

    PED_INT_TMK         = 2
    PED_INT_TLK         = 1
    PED_INT_TDK         = 5
    PED_INT_TIK         = 10
    PED_INT_TPK         = 3
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

    ERR_PED_NO_PIN_INPUT      = -3805
    ERR_PED_PIN_INPUT_CANCEL  = -3806
    ERR_PED_ICC_INIT_ERR      = -3817
    ERR_PED_NO_ICC            = -3816
    ERR_PED_WAIT_INTERVAL     = -3807
    ERR_PED_INPUT_PIN_TIMEOUT = -3815
    ERR_PED_GROUP_IDX_ERR     = -3818
    ERR_PED_NO_KEY            = -3801

    DUKPT_DEFAULT = {
      "KSN"      => "",
      "PINBLOCK" => ""
    }

    DES_DEFAULT = {
      "BLOCK"  => "",
      "RETURN" => ""
    }

    DEFAULT_TIMEOUT = 30

    class << self
      attr_accessor :pinpad, :timeout, :r, :g, :b
    end

    def self.rgb(r,g,b)
      self.r = r
      self.g = g
      self.b = b
    end

    self.rgb(0,0,0)

    def self.timeout
      @timeout || EmvTransaction.timeout || DEFAULT_TIMEOUT
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

      PAX::Pinpad.load_key(value)
    end

    #options[:slot_mk]
    #options[:data]
    #options[:wk]
    def self.encrypt(options)
      ret = self.load_tdk(options[:slot_mk], options[:wk])
      if ret == PAX::Pinpad::RET_OK
        PAX::Pinpad.des(PAX::Pinpad::TDK_SLOT, PAX::Pinpad::MODE_CBC_ENCRYPTION,
                        options[:data])
      else
        des = DES_DEFAULT
        des["RETURN"] = ret
        des
      end
    end

    def self.encrypt_buffer(str)
      slot    = str[1..2].to_i
      message = str[35..-1]
      hash = PAX::Pinpad.encrypt_dukpt(slot, [message].pack("H*"))
      [hash["ped"], hash["block"], hash["ksn"]]
    end

    #options[:slot_mk]
    #options[:data]
    #options[:wk]
    def self.decrypt(options)
      ret = self.load_tdk(options[:slot_mk], options[:wk])
      if ret == PAX::Pinpad::RET_OK
        PAX::Pinpad.des(PAX::Pinpad::TWK_SLOT, PAX::Pinpad::MODE_INT_DECRYPTION,
                        options[:data])
      else
        des = DES_DEFAULT
        des["RETURN"] = ret
        des
      end
    end

    def self.pin(options = {})
      index           = options[:index] || 0
      pan             = options[:pan]
      timeout_seconds = options[:timeout] || self.timeout
      len             = options[:len] || "0,4,5,6,7,8,9,10,11,12"
      message         = options[:message]

      puts message
      if File.exists?("./shared/emv_enter_pin.bmp")
        Device::Display.print_bitmap("./shared/emv_enter_pin.bmp")
        STDOUT.fresh
        STDOUT.y += 1
        STDOUT.x += 1
        Device::Display.print(message[0..20])
        STDOUT.x += 1
        Device::Display.print(message[22..-2])
        STDOUT.x = 1
        STDOUT.y += 2
        EmvTransaction.set_rgb
      else
        puts message
      end

      response = PAX::Pinpad.get_pin_dukpt(index, pan, len, timeout_seconds * 1000)
      response["block"] = response["block"].unpack("H*")[0] if response["block"]
      response["ksn"] = response["ksn"].unpack("H*")[0] if response["ksn"]
      case response["ped"]
      when PAX::Pinpad::RET_OK
        response["return"] = PAX::EMV::EMV_OK
      when PAX::EMV::PED_RET_ERR_INPUT_CANCEL
        response["return"] = PAX::EMV::EMV_USER_CANCEL
      when PAX::Pinpad::ERR_PED_NO_PIN_INPUT
        response["return"] = PAX::EMV::EMV_NO_PASSWORD
      when PAX::Pinpad::ERR_PED_PIN_INPUT_CANCEL
        response["return"] = PAX::EMV::EMV_USER_CANCEL
      when PAX::Pinpad::ERR_PED_ICC_INIT_ERR
        response["return"] = PAX::EMV::EMV_NO_PINPAD
      when PAX::Pinpad::ERR_PED_NO_ICC
        response["return"] = PAX::EMV::EMV_NO_PINPAD
      when PAX::Pinpad::ERR_PED_INPUT_PIN_TIMEOUT
        response["return"] = PAX::EMV::EMV_TIME_OUT
      else
        response["return"] = PAX::EMV::EMV_NO_PINPAD
      end
      response
    end

    def self.get_pin_dukpt(index, pan, len, timeout)
      pan_shifted = "0000" + pan.to_s[-13..-2]
      self._get_pin_dukpt(index, pan_shifted, len, timeout)
    end

    def self.key_ksn(index)
      pin = self._key_ksn(index)
      {
        :pin  => convert_result_to_hex(pin),
        :data => []
      }
    end

    def self.key_kcv(index)
      pin  = self._key_kcv(PED_INT_TIK, index)
      data = self._key_kcv(PED_INT_TPK , index)
      ms3des = self._key_kcv(PED_INT_TMK , index)
      {
        :pin  => convert_result_to_hex(pin),
        :data => convert_result_to_hex(data),
        :ms3des => convert_result_to_hex(ms3des)
      }
    end

    private
    def self.convert_result_to_hex(result)
      ret, binary = result
      [ret, binary.to_s.unpack("H*").first.to_s.upcase]
    end
  end
end
