class PAX
  class EMV
    EMV_SCRIPT_PROC_UNIONPAY = 1
    EMV_SCRIPT_PROC_NORMAL   = 0
    CLSS_SLOT                = 0xff
    CONTACT_SLOT             = 0x00
    MAX_REVOCLIST_NUM        = 30
    MAX_APP_NUM              = 100
    MAX_APP_ITEMS            = 17
    MAX_KEY_NUM              = 7
    PART_MATCH               = 0x00
    FULL_MATCH               = 0x01
    EMV_CASH                 = 0x01
    EMV_GOODS                = 0x02
    EMV_SERVICE              = 0x04
    EMV_CASHBACK             = 0x08
    EMV_INQUIRY              = 0x10
    EMV_TRANSFER             = 0x20
    EMV_PAYMENT              = 0x40
    EMV_ADMIN                = 0x80
    EMV_CASHDEPOSIT          = 0x90
    EMV_OK                   = 0
    ICC_RESET_ERR            = -1
    ICC_CMD_ERR              = -2
    ICC_BLOCK                = -3
    EMV_RSP_ERR              = -4
    EMV_APP_BLOCK            = -5
    EMV_NO_APP               = -6
    EMV_USER_CANCEL          = -7
    EMV_TIME_OUT             = -8
    EMV_DATA_ERR             = -9
    EMV_NOT_ACCEPT           = -10
    EMV_DENIAL               = -11
    EMV_KEY_EXP              = -12
    EMV_NO_PINPAD            = -13
    EMV_NO_PASSWORD          = -14
    EMV_SUM_ERR              = -15
    EMV_NOT_FOUND            = -16
    EMV_NO_DATA              = -17
    EMV_OVERFLOW             = -18
    NO_TRANS_LOG             = -19
    RECORD_NOTEXIST          = -20
    LOGITEM_NOTEXIST         = -21
    ICC_RSP_6985             = -22
    EMV_FILE_ERR             = -24
    EMV_PARAM_ERR            = -30
    EMV_PED_TIMEOUT          = 0x01
    EMV_PED_WAIT             = 0x02
    EMV_PED_FAIL             = 0x03

    REFER_APPROVE            = 0x01
    REFER_DENIAL             = 0x02
    ONLINE_APPROVE           = 0x00
    ONLINE_FAILED            = 0x01
    ONLINE_REFER             = 0x02
    ONLINE_DENIAL            = 0x03
    ONLINE_ABORT             = 0x04
    AC_AAC                   = 0x00
    AC_TC                    = 0x01
    AC_ARQC                  = 0x02
    AC_AAC_HOST              = 0x03

    PED_RET_ERR_NO_PIN_INPUT  = -305
    PED_RET_ERR_INPUT_CANCEL  = -306
    PED_RET_ERR_WAIT_INTERVAL = -307
    PED_RET_ERR_NO_ICC        = -316
    PED_RET_ERR_ICC_NO_INIT   = -317

    # TODO Scalone implement default values
    EMV_PARAMETER_DEFAULT = {
      "MerchName"     => "",
      "MerchCateCode" => "",
      "MerchId"       => "",
      "TermId"        => "",
      "TerminalType"  => "",
      "Capability"    => "",
      "ExCapability"  => "",
      "TransCurrExp"  => "",
      "ReferCurrExp"  => "",
      "ReferCurrCode" => "",
      "CountryCode"   => "",
      "TransCurrCode" => "",
      "ReferCurrCon"  => 0,
      "TransType"     => "",
      "ForceOnline"   => "",
      "GetDataPIN"    => "",
      "SurportPSESel" => ""
    }

    EMV_APP_DEFAULT = {
      "AppName"         => "",
      "AID"             => "",
      "AidLen"          => "",
      "SelFlag"         => "",
      "Priority"        => "",
      "TargetPer"       => "",
      "MaxTargetPer"    => "",
      "FloorLimitCheck" => "",
      "RandTransSel"    => "",
      "VelocityCheck"   => "",
      "FloorLimit"      => 0,
      "Threshold"       => "",
      "TACDenial"       => "",
      "TACOnline"       => "",
      "TACDefault"      => "",
      "AcquierId"       => "",
      "dDOL"            => "",
      "tDOL"            => "",
      "Version"         => "",
      "RiskManData"     => ""
    }

    EMV_PKI_DEFAULT = {
      "RID"         => "",
      "KeyID"       => "",
      "HashInd"     => "",
      "ArithInd"    => "",
      "ModulLen"    => "",
      "Modul"       => "",
      "ExponentLen" => "",
      "Exponent"    => "",
      "ExpDate"     => "",
      "CheckSum"    => ""
    }

    EMV_SCRIPT_DEFAULT = {
      "ADVICE"      => "",
      "ACTYPE"      => ""
    }

    class << self
      attr_accessor :icc, :select_block, :get_pin_plain_block, :verify_cipher_pin_block,
        :get_pin_block_block, :pkis
    end

    def self.parameter_default
      EMV_PARAMETER_DEFAULT.dup
    end

    def self.app_default
      EMV_APP_DEFAULT.dup
    end

    def self.pki_default
      EMV_PKI_DEFAULT.dup
    end

    def self.script_default
      EMV_SCRIPT_DEFAULT.dup
    end

    def self.init
      self._init
    end

    def self.icc_init
      self.icc = PAX::ICCard.smart_card_init
    end

    def self.load_apps(apps)
      apps.each do |app|
        self.add_app(self.parse_app(app)[1])
      end
    end

    def self.load_pkis(rows)
      self.pkis = {}
      logs = Hash.new
      rows.each do |row|
        parsed = self.parse_pki(row)
        rid = parsed["RID"].unpack("H10").first.upcase
        self.pkis[rid] ||= []
        self.pkis[rid] << parsed
      end
    end

    def self.row_to_app(row, version = nil)
      app = EMV_APP_DEFAULT.dup

      # APP Parameter
      # :label=>"SMARTCON iEMV ON",
      app["AppName"]         = row.label

      # :aid=>"AAA00000010103000000000000000000",
      app["AID"]             = [row.aid].pack("H*")

      # :aid_length=>"07",
      app["AidLen"]          = [row.aid_length].pack("H*")

      # :terminal_floor_limit=>"0000C350",
      app["FloorLimit"]      = row.terminal_floor_limit.to_i(16)

      # :terminal_action_code_default=>"5000000000",
      app["TACDefault"]      = [row.terminal_action_code_default].pack("H*") + "\x00"

      # :terminal_action_code_denial=>"0000000000",
      app["TACDenial"]       = [row.terminal_action_code_denial].pack("H*") + "\x00"

      # :terminal_action_code_online=>"0000000000",
      app["TACOnline"]       = [row.terminal_action_code_online].pack("H*") + "\x00"

      # :acquirer_id=>"04",
      app["AcquierId"]       = [("FFFFFFFFFFFF" + row.acquirer_id)[-12..-1]].pack("H*");

      # :tdol=>"0000000000000000000000000000000000000000",
      app["tDOL"]            = "#{(row.tdol.size / 2).to_i.chr}#{[row.tdol].pack("H*")}"
      #Device::Display.clear; p "[#{row.tdol}]"; getc

      # :ddol=>"9F37049F47018F019F3201000000000000000000",
      ddol = row.ddol.gsub("00", "")
      app["dDOL"]            = "#{(ddol.size / 2).to_i.chr}#{[ddol].pack("H*")}"

      # :application_version_number_1=>"008C",
      case version
      when 1
        app["Version"]       = [row.application_version_number_1].pack("H*")
      when 2
        app["Version"]       = [row.application_version_number_2].pack("H*")
      when 3
        app["Version"]       = [row.application_version_number_3].pack("H*")
      else
        if row.application_version_number_1 != "0000"
          app["Version"]     = [row.application_version_number_1].pack("H*")
        elsif row.application_version_number_2 != "0000"
          app["Version"]     = [row.application_version_number_2].pack("H*")
        else
          app["Version"]     = [row.application_version_number_3].pack("H*")
        end
      end

      # = ProcessTransaction

      # = CTLS
      # Threshold (dwThresholdValue ProcessTransaction or CTLS) - Threshold (provided by acquirer)
      #  (Refer to the risk management in EMV specification)
      #  Notes: If TargetPer=99 and Threshold=0xffffffff, all the transaction will be done online.
      app["Threshold"]         = "\x00"
      #
      # TargetPer (bTargetPercentage ProcessTransaction) - Target percent (0 – 99) (provided by acquirer)
      #  (Refer to the risk management in EMV specification.)
      app["TargetPer"]         = "\x00"
      #
      # FloorLimitCheck (ProcessTransaction if terminal_floor_limit) - For the online only terminal,
      #  check the floor limit or not (1: check, 0 : not check，default:1)
      app["FloorLimitCheck"] = "\x01" # 1 - check, 0 - don't check
      #
      # Priority (CTLS) - priority indicator
      #  (It’s returned by ICC, so nothing needs to be done by application.)
      #
      #
      # MaxTargetPer (bMaximumTarget ProcessTransaction)
      #  Max target percent(0 – 99) (provided by acquirer)
      #  (Refer to the risk management in EMV specification.)
      app["MaxTargetPer"]      = "\x00"
      #
      # = Not Found (contain in emv interface but not in row)
      # VelocityCheck - For the online only terminal,
      #  perform velocity check or not (1 : perform, 0 not perform, default : 1)
      app["VelocityCheck"]         = "\x01"
      #
      # RiskManData (not found, could be bManageRisk) - Risk management data
      #  RiskManData[0] is the length of Risk management data, the others are the value of Risk management data.
      #  (RiskManData[0] default : 0)
      #  (It needn’t be set unless issuer requires.)
      app["RiskManData"]         = "\x00"
      #
      # MerchName - merchant name (usually no need to set)
      #
      # SelFlag - Application selection flag (partial matching PART_MATCH or full matching FULL_MATCH)
      #  (Refer to the macro definition in appendix A.)
      app["SelFlag"]         = PART_MATCH.chr
      #
      # RandTransSel (maybe if bTargetPercentage was set) - For the online only or offline only terminal,
      #  perform random transaction selection or not (1: perform, 0 : not perform, default : 1)
      app["RandTransSel"]    = "\x01"

      # = Unreferenced (contain in row but not in emv interface)
      # identification                                   , 1],
      # index                                            , 2],
      # application_type                                 , 2],
      # table_type                                       , 2],
      # application_version_number_2                     , 4],
      # application_version_number_3                     , 4],
      # reserved                                         , 32]
      # authorization_code_offline_approved              , 2],
      # authorization_code_offline_declined              , 2],
      # authorization_code_unable_online_offline_approved, 2],
      # authorization_code_unable_online_offline_declined, 2]
      app
    end

    def self.row_to_general(row)
      general = EMV_PARAMETER_DEFAULT.dup

      # EMV Parameter
      #:merchant_identifier=>"000000000000001"
      general["MerchId"]       = row.merchant_identifier

      #general["MerchCateCode"] = [row.merchant_category_code].pack("H*")

      #:terminal_identification=>"49000076"
      general["TermId"]        = row.terminal_identification

      #:terminal_type=>"22"
      general["TerminalType"]  = [row.terminal_type].pack("H*")

      #:terminal_capabilities=>"E0E8C0"
      general["Capability"]    = [row.terminal_capabilities].pack("H*")

      #:terminal_additional_capabilities=>"6000F0F000"
      general["ExCapability"]  = [row.terminal_additional_capabilities].pack("H*")

      #:terminal_country_code=>"0" + "076"
      general["CountryCode"]   = ["0" + row.terminal_country_code].pack("H*")

      #:transaction_currency_code=>"0" + "986"
      general["TransCurrCode"] = ["0" + row.transaction_currency_code].pack("H*")

      #:transaction_currency_exponent=>"0" + "2"
      general["TransCurrExp"]  = ["0" + row.transaction_currency_exponent].pack("H*")

      # ReferCurrCode - reference currency code (default: “\x08\x40”)
      #:transaction_currency_code=>"0" + "986"
      general["ReferCurrCode"] = ["0" + row.transaction_currency_code].pack("H*")

      # ReferCurrExp - reference currency exponent (default: “0x02”)
      #:transaction_currency_exponent=>"0" + "2"
      general["ReferCurrExp"]  = ["0" + row.transaction_currency_exponent].pack("H*")

      general["ReferCurrCon"]   = 1000

      # TransType - set current transaction type
      #  EMV_CASH or EMV_GOODS or EMV_SERVICE or EMV_GOODS& EMV_CASHBACK or EMV_SERVICE& EMV_CASHBACK #  (refer to appendix A for macro definitions)
      general["TransType"]       = EMV_PAYMENT.chr
      #general["TransType"]       = EMV_GOODS.chr
      #general["TransType"]       = "\x01"

      # ForceOnline (bMustConnect ProcessTransaction) - merchant force online
      #  (1 means always online transaction)
      general["ForceOnline"]     = "\x00"

      # GetDataPIN (bRequirePIN ProcessTransaction) - read the IC card PIN retry counter before verify the PIN or not
      #  (1 : read, 0 : not read, default : 1)
      general["GetDataPIN"]      = "\x01"

      # SurportPSESel (CTLS) - support PSE selection mode or not
      #  (1 : support, 0 : not support, default : 1)
      general["SurportPSESel"]   = "\x01"

      general
    end

    def self.parse_app(row, version = 3)
      [self.row_to_general(row), self.row_to_app(row, version)]
    end

    def self.parse_pki(row)
      pki = EMV_PKI_DEFAULT.dup

      #:rid=>"AAA0000001"
      pki["RID"]         = [row.rid].pack("H*")

      # KeyID - key index
      # 0x92
      pki["KeyID"]       = [row.ca_public_key_index].pack("H2")

      # HashInd - HASH arithmetic index (must be 1)
      pki["HashInd"]     = "\x01"

      # ArithInd - RSA arithmetic index (must be 1)
      pki["ArithInd"]     = "\x01"

      #:ca_public_key_modulus_byte_length=>"176"
      pki["ModulLen"]    = [row.ca_public_key_modulus_byte_length.to_i].pack("v")

      #:ca_public_key_modulus=>"D9E36579B94A5FF3150B64643D85C06E6E9F0682BE56CDD69FCB053913495BDBC327DA3CAC0EA2A0DA1D55DF7C66A0C6F6A9039FA72753C434F4A63BED54062799DF1F6D6E1F315A8F4109721126E11F4FF562C18A4AE6A4D9F0C2A5C2A8E44D6A98628C7E25290584F0F3D9ECE6566FDB7688596649BEC89A1CBC8BBED075538300D0D83FF8755E1CE73668908C387E14ACDF0F9F1DE436A5A07308812D6AE3A16170EDF2522B36FBE94358F50C0B69000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
      pki["Modul"]       = [row.ca_public_key_modulus].pack("H*")

      #:ca_public_key_exponent_byte_length=>"3"
      pki["ExponentLen"] = row.ca_public_key_exponent_byte_length.to_i.chr

      #:ca_public_key_exponent=>"010001"
      pki["Exponent"]    = [row.ca_public_key_exponent].pack("H*")

      # ExpDate (CTLS only)
      pki["ExpDate"]    = "\x00\x00\x00"

      #:ca_public_key_check_sum=>"4dddd1d9b9a3b2eeace63a5ba9dd6f4441ce10af00000000"
      if row.status_check_sum == "0"
        pki["CheckSum"] = self.pki_check_sum(
          pki["RID"], pki["KeyID"], pki["Modul"], pki["Exponent"],
          row.ca_public_key_exponent_byte_length.to_i,
          row.ca_public_key_modulus_byte_length.to_i)
      else
        pki["CheckSum"] = [row.ca_public_key_check_sum].pack("H*")
      end

      # = ProcessTransaction

      # = Not Found (contain in emv interface but not in row)

      # = Unreferenced for now, will be necessary on pinpad
      # [:master_key_index                                 , 2],
      # [:working_key                                      , 32]

      # = Unused
      # [:length                                           , 3],
      # [:index                                            , 2],
      # [:identification                                   , 1],
      # [:acquirer_id                                      , 2],
      # [:ca_public_key_index                              , 2],
      # [:status_check_sum                                 , 1],
      # [:reserved                                         , 2],
      pki
    end

    def self.internal_get_pin_block(attempt_flag, attempts_remain, block)
      if self.get_pin_block_block
        self.get_pin_block_block.call(attempt_flag, attempts_remain, block)
      else
        ContextLog.info "INTERNAL GET PIN BLOCK [#{attempt_flag}] [#{attempts_remain}] [#{block}]"
        if attempt_flag == 0
          PAX::Display.clear
          puts "ENTER PIN: "
        else
          PAX::Display.clear
          puts "INCORRECT PIN"
          sleep 2
        end

        #Offline plain/enciphered PIN processing below
        if attempts_remain == 1
          PAX::Display.clear
          puts "LAST CHANCE"
          sleep 2
        end

        unless block
          pan = PAX::EMV.get_tlv(0x5A)
          len = "0,4,5,6,7,8,9,10,11,12"
          response = Device::Pinpad.get_pin_block(17, pan, len, Device::IO.timeout)
          case response["ped"]
          when PAX::Pinpad::RET_OK
            response["return"] = PAX::EMV::EMV_OK
          when PAX::EMV::PED_RET_ERR_INPUT_CANCEL
            response["return"] = PAX::EMV::EMV_USER_CANCEL
          else
            response["return"] = PAX::EMV::EMV_NO_PINPAD
          end
          return response
        end

        #Offline PIN, done by core itself since EMV core V25_T1. Application only needs to display prompt message.
        #In this mode, cEMVGetHolderPwd() will be called twice. the first time is to display message to user,
        #then back to kernel and wait PIN. afterwards kernel call this again and inform the process result.
        case response["block"].to_s[0]
        when nil
          PAX::EMV::EMV_OK
        when PAX::EMV::EMV_PED_TIMEOUT.chr
          PAX::EMV::EMV_TIME_OUT
        when PAX::EMV::EMV_PED_WAIT.chr
          PAX::EMV::EMV_OK
        when PAX::EMV::EMV_PED_FAIL.chr
          PAX::EMV::EMV_NO_PINPAD
        else
          PAX::EMV::EMV_OK
        end
        response
      end
    end

    def self.internal_verify_cipher_pin(icc_slot, pin_len, rsa)
      if self.verify_cipher_pin_block
        self.verify_cipher_pin_block.call(icc_slot, pin_len, rsa)
      else
        ContextLog.info "INTERNAL VERIFY CIPHER PIN ICC SLOT #{icc_slot} PIN LEN #{pin_len} RSA #{rsa.inspect}"
        PAX::Display.clear
        puts "ENTER PIN: "
        response = Device::Pinpad.verify_cipher_pin(icc_slot, pin_len, rsa, Device::IO.timeout)
        case response["ped"]
        when PAX::Pinpad::RET_OK
          response["return"] = PAX::EMV::EMV_OK
        when PAX::Pinpad::ERR_PED_NO_PIN_INPUT
          response["return"] = PAX::EMV::PED_RET_ERR_NO_PIN_INPUT
        when PAX::Pinpad::ERR_PED_PIN_INPUT_CANCEL
          response["return"] = PAX::EMV::PED_RET_ERR_INPUT_CANCEL
        when PAX::Pinpad::ERR_PED_ICC_INIT_ERR
          response["return"] = PAX::EMV::PED_RET_ERR_ICC_NO_INIT
        when PAX::Pinpad::ERR_PED_NO_ICC
          response["return"] = PAX::EMV::PED_RET_ERR_NO_ICC
        when PAX::Pinpad::ERR_PED_WAIT_INTERVAL
          response["return"] = PAX::EMV::PED_RET_ERR_WAIT_INTERVAL
        else
          response["return"] = response["ped"]
        end
        response
      end
    end

    def self.internal_get_pin_plain(icc_slot, pin_len)
      if self.get_pin_plain_block
        self.get_pin_plain_block.call(icc_slot, pin_len)
      else
        ContextLog.info "INTERNAL GET PIN ICC SLOT #{icc_slot} PIN LEN #{pin_len}"
        PAX::Display.clear
        puts "ENTER PIN: "
        response = Device::Pinpad.get_pin_plain(icc_slot, pin_len, Device::IO.timeout)
        case response["ped"]
        when PAX::Pinpad::RET_OK
          response["return"] = PAX::EMV::EMV_OK
        when PAX::Pinpad::ERR_PED_NO_PIN_INPUT
          response["return"] = PAX::EMV::PED_RET_ERR_NO_PIN_INPUT
        when PAX::Pinpad::ERR_PED_PIN_INPUT_CANCEL
          response["return"] = PAX::EMV::PED_RET_ERR_INPUT_CANCEL
        when PAX::Pinpad::ERR_PED_ICC_INIT_ERR
          response["return"] = PAX::EMV::PED_RET_ERR_ICC_NO_INIT
        when PAX::Pinpad::ERR_PED_NO_ICC
          response["return"] = PAX::EMV::PED_RET_ERR_NO_ICC
        when PAX::Pinpad::ERR_PED_WAIT_INTERVAL
          response["return"] = PAX::EMV::PED_RET_ERR_WAIT_INTERVAL
        else
          response["return"] = response["ped"]
        end
        response
      end
    end

    def self.internal_app_select(list, tries)
      if self.select_block
        self.select_block.call(list, tries)
      else
        return 0 if list.size == 1

        PAX::Display.clear
        if tries != 0
          puts "\nNOT ACCEPT\n"
          puts "PLEASE TRY AGAIN"
          getc(2000)
        end

        puts "SELECT APLICATION"
        list.each_with_index do |app, index|
          puts "#{index+1} #{app["label"] || app["AppName"]}"
        end

        if (ret = getc(30_000)) == 0x1B.chr
          EMV_USER_CANCEL
        elsif ret == 0x12.chr
          EMV_TIME_OUT
        else
          ret.to_i
        end
      end
    end

    def self.pki_check_sum(rid, key_id, modulus, exponent, exponent_len, modulus_len)
      exponent_treated = exponent.to_s[0..(exponent_len - 1)]
      modulus_treated  = modulus[0..(modulus_len - 1)]
      value = "#{rid}#{key_id}#{modulus_treated}#{exponent_treated}"
      value2 = "#{rid}#{key_id}#{exponent_treated}".unpack("H*")
      Digest::SHA1.digest(value)
    end
  end
end
