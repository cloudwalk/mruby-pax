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

    EMV_PARAMETER_DEFAULT = {
      "MerchName"     = "",
      "MerchCateCode" = "",
      "MerchId"       = "",
      "TermId"        = "",
      "TerminalType"  = "",
      "Capability"    = "",
      "ExCapability"  = "",
      "TransCurrExp"  = "",
      "ReferCurrExp"  = "",
      "ReferCurrCode" = "",
      "CountryCode"   = "",
      "TransCurrCode" = "",
      "ReferCurrCon"  = "",
      "TransType"     = "",
      "ForceOnline"   = "",
      "GetDataPIN"    = "",
      "SurportPSESel" = ""
    }

    def self.parameter_default
      EMV_PARAMETER_DEFAULT.dup
    end
  end
end
