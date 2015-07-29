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

    class << self
      attr_accessor :icc
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

    def self.init
      self.icc = PAX::ICCard.smart_card_init
      self._init
    end

    def self.load_apps(table)
      table.apps.each do |app|
        self.add_app(self.parse_app(app))
      end
    end

    def self.load_pkis(table)
      table.pkis.each do |pki|
        self.add_pki(self.parse_pki(pki))
      end
    end

    def self.parse_app(row)
      general = EMV_PARAMETER_DEFAULT.dup
      app     = EMV_APP_DEFAULT.dup

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
      general["ReferCurrCode"]  = "\x08\x40"

      # ReferCurrExp - reference currency exponent (default: “0x02”)
      general["ReferCurrExp"]  = "\x02"

      general["ReferCurrCon"]   = 1000

      # TransType - set current transaction type
      #  EMV_CASH or EMV_GOODS or EMV_SERVICE or EMV_GOODS& EMV_CASHBACK or EMV_SERVICE& EMV_CASHBACK
      #  (refer to appendix A for macro definitions)
      general["TransType"]       = "\x40"

      # ForceOnline (bMustConnect ProcessTransaction) - merchant force online
      #  (1 means always online transaction)
      general["ForceOnline"]     = "\x00"

      # GetDataPIN (bRequirePIN ProcessTransaction) - read the IC card PIN retry counter before verify the PIN or not
      #  (1 : read, 0 : not read, default : 1)
      general["GetDataPIN"]      = "\x01"

      # SurportPSESel (CTLS) - support PSE selection mode or not
      #  (1 : support, 0 : not support, default : 1)
      general["SurportPSESel"]   = "\x01"

      # APP Parameter
      # :label=>"SMARTCON iEMV ON",
      app["AppName"]         = row.label
      #Device::Display.clear; p "[#{row.label}]"; getc

      # :aid=>"AAA00000010103000000000000000000",
      app["AID"]             = [row.aid].pack("H*")
      #Device::Display.clear; p "[#{row.aid}]"; getc

      # :aid_length=>"07",
      app["AidLen"]          = [row.aid_length].pack("H*")
      #Device::Display.clear; p "[#{row.aid_length}]"; getc

      # :terminal_floor_limit=>"0000C350",
      app["FloorLimit"]      = row.terminal_floor_limit.to_i(16)
      #Device::Display.clear; p "[#{row.terminal_floor_limit}]"; getc

      # :terminal_action_code_default=>"5000000000",
      app["TACDefault"]      = [row.terminal_action_code_default].pack("H*")
      #Device::Display.clear; p "[#{row.terminal_action_code_default}]"; getc

      # :terminal_action_code_denial=>"0000000000",
      app["TACDenial"]       = [row.terminal_action_code_denial].pack("H*")
      #Device::Display.clear; p "[#{row.terminal_action_code_denial}]"; getc

      # :terminal_action_code_online=>"0000000000",
      app["TACOnline"]       = [row.terminal_action_code_online].pack("H*")
      #Device::Display.clear; p "[#{row.terminal_action_code_online}]"; getc

      # :acquirer_id=>"04",
      app["AcquierId"]       = [("FFFFFFFFFFFF" + row.acquirer_id)[-12..-1]].pack("H*");
      #app["AcquierId"]       = "0000"
      #Device::Display.clear; p "[#{row.acquirer_id}]"; getc

      # :tdol=>"0000000000000000000000000000000000000000",
      app["tDOL"]            = "#{(row.tdol.size / 2).to_i.chr}#{[row.tdol].pack("H*")}"
      #Device::Display.clear; p "[#{row.tdol}]"; getc

      # :ddol=>"9F37049F47018F019F3201000000000000000000",
      ddol = row.ddol.gsub("00", "")
      app["dDOL"]            = "#{(ddol.size / 2).to_i.chr}#{[ddol].pack("H*")}"
      #Device::Display.clear; p "[#{row.ddol}]"; getc

      # :application_version_number_1=>"008C",
      app["Version"]         = [row.application_version_number_1].pack("H*")
      #Device::Display.clear; p "[#{row.application_version_number_1}]"; getc

      # = ProcessTransaction

      # = CTLS
      # Threshold (dwThresholdValue ProcessTransaction or CTLS) - Threshold (provided by acquirer)
      #  (Refer to the risk management in EMV specification)
      #  Notes: If TargetPer=99 and Threshold=0xffffffff, all the transaction will be done online.
      app["Threshold"]         = "\x00"
      #
      # TargetPer (bTargetPercentage ProcessTransaction) - Target percent (0 – 99) (provided by acquirer)
      #  (Refer to the risk management in EMV specification.)
      app["TargetPer"]         = "\x10"
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
      app["MaxTargetPer"]      = "\x10"
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
      app["SelFlag"]         = "\x01"
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
      [general, app]
    end

    def self.parse_pki(row)
      pki = EMV_PKI_DEFAULT.dup

      #:rid=>"AAA0000001"
      pki["RID"]         = [row.rid].pack("H*")

      # KeyID - key index
      #pki["KeyID"]       = [row.index].pack("H*")
      pki["KeyID"]       = [row.ca_public_key_index].pack("H*")

      # HashInd - HASH arithmetic index (must be 1)
      pki["HashInd"]     = "1".to_i.chr

      # ArithInd - RSA arithmetic index (must be 1)
      pki["ArithInd"]     = "1".to_i.chr

      #:ca_public_key_modulus_byte_length=>"176"
      pki["ModulLen"]    = row.ca_public_key_modulus_byte_length.to_i.chr

      #:ca_public_key_modulus=>"D9E36579B94A5FF3150B64643D85C06E6E9F0682BE56CDD69FCB053913495BDBC327DA3CAC0EA2A0DA1D55DF7C66A0C6F6A9039FA72753C434F4A63BED54062799DF1F6D6E1F315A8F4109721126E11F4FF562C18A4AE6A4D9F0C2A5C2A8E44D6A98628C7E25290584F0F3D9ECE6566FDB7688596649BEC89A1CBC8BBED075538300D0D83FF8755E1CE73668908C387E14ACDF0F9F1DE436A5A07308812D6AE3A16170EDF2522B36FBE94358F50C0B69000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
      pki["Modul"]       = [row.ca_public_key_modulus].pack("H*")

      #:ca_public_key_exponent_byte_length=>"3"
      pki["ExponentLen"] = row.ca_public_key_exponent_byte_length.to_i.chr

      #:ca_public_key_exponent=>"010001"
      pki["Exponent"]    = [row.ca_public_key_exponent].pack("H*")

      # ExpDate (baExpirationDate ProcessTransaction)
      pki["ExpDate"]    = ["160410"].pack("H*")

      #:ca_public_key_check_sum=>"4dddd1d9b9a3b2eeace63a5ba9dd6f4441ce10af00000000"
      pki["CheckSum"]    = [row.ca_public_key_check_sum].pack("H*")

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
  end
end
