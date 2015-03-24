#include <stdlib.h>
#include <stdio.h>
#include "mruby.h"
#include "mruby/compile.h"
#include "mruby/value.h"
#include "mruby/array.h"
#include "mruby/string.h"
#include "mruby/hash.h"

#include "osal.h"
#include "emvlib_Prolin.h"

/*#define E_EMV_ERROR (mrb_class_get_under(mrb,mrb_class_get_under(mrb,mrb_class_get(mrb,"PAX"),"EMV"),"EMVError"))*/

/*#include "CLEntryAPI_Prolin.h"*/
/*#include "ClssApi_Wave_prolin.h"*/
/*#include "ClssApi_MC_prolin.h"*/

/*Callbacks*/

int  cEMVSetParam(void) { return EMV_OK; };
unsigned char cEMVSM3(unsigned char *paucMsgIn, int nMsglenIn,unsigned char *paucResultOut) { return EMV_OK; };
unsigned char cEMVSM2Verify(unsigned char *paucPubkeyIn,unsigned char *paucMsgIn,int nMsglenIn, unsigned char *paucSignIn, int nSignlenIn) { return EMV_OK; };
int  cEMVInputAmount(unsigned long *AuthAmt, unsigned long *CashBackAmt) { return EMV_OK; };
int cEMVPedVerifyPlainPin(uchar ucIccSlot, uchar *pucExpPinLenIn, uchar *ucIccRespOut, uchar ucMode,ulong ulTimeoutMs) { return EMV_OK; };
int cEMVPedVerifyCipherPin(uchar ucIccSlot, uchar *pExpPinLenIn, RSA_PINKEY *tRsaPinKeyIn, uchar *pucIccRespOut, uchar ucMode,ulong ulTimeoutMs) { return EMV_OK; };
int  cEMVGetHolderPwd(int TryFlag, int RemainCnt, unsigned char *pin) { return EMV_OK; };
int  cCertVerify(void) { return EMV_OK; };
void cEMVVerifyPINOK(void) { return; };
unsigned char cEMVPiccIsoCommand(unsigned char cid,APDU_SEND *ApduSend,APDU_RESP *ApduRecv) { return EMV_OK; };
int  cEMVUnknowTLVData(unsigned short Tag, unsigned char *dat, int len) { return EMV_OK; };
int cEMVWaitAppSel(int TryCnt, EMV_APPLIST List[], int AppNum) { return EMV_OK; };
unsigned char cEMVIccIsoCommand(uchar ucslot, APDU_SEND *tApduSend, APDU_RESP *tApduRecv) { return EMV_OK; };

/*Callbacks*/

/*[>CTLS Callbacks<]*/
/*[>Generical<]*/
/*unsigned char cPiccIsoCommand_Entry(uchar cid, APDU_SEND *ApduSend, APDU_RESP *ApduRecv) { return EMV_OK; };*/
/*void DelayMs(ushort Ms) { sleep(Ms/1000); };*/

/*[>MasterCard<]*/
/*unsigned char cPiccIsoCommand_MC(uchar cid, APDU_SEND *ApduSend, APDU_RESP *ApduRecv) { return EMV_OK; };*/
/*int cClssGetUnknowTLVData_MC(uchar *pucTag, uchar ucTagLen, uint unLen, uchar *pucdata) { return EMV_OK; };*/
/*[>Wave<]*/
/*int cClssGetUnknowTLVData_Wave(unsigned short usTag, unsigned char *pucData, int nLen) { return EMV_OK; };*/
/*uchar cPiccIsoCommand_Wave(uchar cid,APDU_SEND *ApduSend,APDU_RESP *ApduRecv) { return EMV_OK; };*/
/*[>CTLS Callbacks<]*/

static mrb_value
mrb_s_get_emv_parameter(mrb_state *mrb, mrb_value klass)
{
  EMV_PARAM parameter;
  mrb_value hash;

  memset(&parameter, 0, sizeof(parameter));

  EMVGetParameter(&parameter);
  hash = mrb_funcall(mrb, klass, "parameter_default", 0);

  /*TODO Scalone: loss data is posible in conversation from unsigned char to const char*/
  mrb_hash_set(mrb, hash, mrb_str_new_cstr(mrb, "MerchName"), mrb_str_new_static(mrb, &parameter.MerchName, 256));
  mrb_hash_set(mrb, hash, mrb_str_new_cstr(mrb, "MerchCateCode"), mrb_str_new_static(mrb, &parameter.MerchCateCode, 2));
  mrb_hash_set(mrb, hash, mrb_str_new_cstr(mrb, "MerchId"), mrb_str_new_static(mrb, &parameter.MerchId, 15));
  mrb_hash_set(mrb, hash, mrb_str_new_cstr(mrb, "TermId"), mrb_str_new_static(mrb, &parameter.TermId, 8));
  mrb_hash_set(mrb, hash, mrb_str_new_cstr(mrb, "TerminalType"), mrb_str_new_static(mrb, &parameter.TerminalType, 1));
  mrb_hash_set(mrb, hash, mrb_str_new_cstr(mrb, "Capability"), mrb_str_new_static(mrb, &parameter.Capability, 3));
  mrb_hash_set(mrb, hash, mrb_str_new_cstr(mrb, "ExCapability"), mrb_str_new_static(mrb, &parameter.ExCapability, 5));
  mrb_hash_set(mrb, hash, mrb_str_new_cstr(mrb, "TransCurrExp"), mrb_str_new_static(mrb, &parameter.TransCurrExp, 1));
  mrb_hash_set(mrb, hash, mrb_str_new_cstr(mrb, "ReferCurrExp"), mrb_str_new_static(mrb, &parameter.ReferCurrExp, 1));
  mrb_hash_set(mrb, hash, mrb_str_new_cstr(mrb, "ReferCurrCode"), mrb_str_new_static(mrb, &parameter.ReferCurrCode, 2));
  mrb_hash_set(mrb, hash, mrb_str_new_cstr(mrb, "CountryCode"), mrb_str_new_static(mrb, &parameter.CountryCode, 2));
  mrb_hash_set(mrb, hash, mrb_str_new_cstr(mrb, "TransCurrCode"), mrb_str_new_static(mrb, &parameter.TransCurrCode, 2));
  mrb_hash_set(mrb, hash, mrb_str_new_cstr(mrb, "ReferCurrCon"), mrb_str_new_static(mrb, &parameter.ReferCurrCon, 1));
  mrb_hash_set(mrb, hash, mrb_str_new_cstr(mrb, "TransType"), mrb_str_new_static(mrb, &parameter.TransType, 1));
  mrb_hash_set(mrb, hash, mrb_str_new_cstr(mrb, "ForceOnline"), mrb_str_new_static(mrb, &parameter.ForceOnline, 1));
  mrb_hash_set(mrb, hash, mrb_str_new_cstr(mrb, "GetDataPIN"), mrb_str_new_static(mrb, &parameter.GetDataPIN, 1));
  mrb_hash_set(mrb, hash, mrb_str_new_cstr(mrb, "SurportPSESel"), mrb_str_new_static(mrb, &parameter.SurportPSESel, 1));

  return hash;
}

static void
set_emv_parameter(mrb_state *mrb, mrb_value klass, mrb_value hash)
{
  EMV_PARAM parameter;
  mrb_value value;
  mrb_int iValue;

  memset(&parameter, 0, sizeof(parameter));

  memset(&value, 0, sizeof(value));
  value = mrb_hash_get(mrb, hash, mrb_str_new_cstr(mrb, "MerchName"));
  strncpy(&parameter.MerchName, RSTRING_PTR(value), 256);

  memset(&value, 0, sizeof(value));
  value = mrb_hash_get(mrb, hash, mrb_str_new_cstr(mrb, "MerchCateCode"));
  strncpy(&parameter.MerchCateCode, RSTRING_PTR(value), 2);

  memset(&value, 0, sizeof(value));
  value = mrb_hash_get(mrb, hash, mrb_str_new_cstr(mrb, "MerchId"));
  strncpy(&parameter.MerchId, RSTRING_PTR(value), 15);

  memset(&value, 0, sizeof(value));
  value = mrb_hash_get(mrb, hash, mrb_str_new_cstr(mrb, "TermId"));
  strncpy(&parameter.TermId, RSTRING_PTR(value), 8);

  memset(&value, 0, sizeof(value));
  value = mrb_hash_get(mrb, hash, mrb_str_new_cstr(mrb, "TerminalType"));
  parameter.TerminalType = (unsigned char)RSTRING_PTR(value);

  memset(&value, 0, sizeof(value));
  value = mrb_hash_get(mrb, hash, mrb_str_new_cstr(mrb, "Capability"));
  strncpy(&parameter.Capability, RSTRING_PTR(value), 3);

  memset(&value, 0, sizeof(value));
  value = mrb_hash_get(mrb, hash, mrb_str_new_cstr(mrb, "ExCapability"));
  strncpy(&parameter.ExCapability, RSTRING_PTR(value), 5);

  memset(&value, 0, sizeof(value));
  value = mrb_hash_get(mrb, hash, mrb_str_new_cstr(mrb, "TransCurrExp"));
  parameter.TransCurrExp = (unsigned char)RSTRING_PTR(value);

  memset(&value, 0, sizeof(value));
  value = mrb_hash_get(mrb, hash, mrb_str_new_cstr(mrb, "ReferCurrExp"));
  parameter.ReferCurrExp = (unsigned char)RSTRING_PTR(value);

  memset(&value, 0, sizeof(value));
  value = mrb_hash_get(mrb, hash, mrb_str_new_cstr(mrb, "ReferCurrCode"));
  strncpy(&parameter.ReferCurrCode, RSTRING_PTR(value), 2);

  memset(&value, 0, sizeof(value));
  value = mrb_hash_get(mrb, hash, mrb_str_new_cstr(mrb, "CountryCode"));
  strncpy(&parameter.CountryCode, RSTRING_PTR(value), 2);

  memset(&value, 0, sizeof(value));
  value = mrb_hash_get(mrb, hash, mrb_str_new_cstr(mrb, "TransCurrCode"));
  strncpy(&parameter.TransCurrCode, RSTRING_PTR(value), 2);

  memset(&value, 0, sizeof(value));
  iValue = mrb_fixnum(mrb_hash_get(mrb, hash, mrb_str_new_cstr(mrb, "ReferCurrCon")));
  parameter.ReferCurrCon = iValue;

  memset(&value, 0, sizeof(value));
  value = mrb_hash_get(mrb, hash, mrb_str_new_cstr(mrb, "TransType"));
  parameter.TransType = (unsigned char)(*RSTRING_PTR(value));

  memset(&value, 0, sizeof(value));
  value = mrb_hash_get(mrb, hash, mrb_str_new_cstr(mrb, "ForceOnline"));
  parameter.ForceOnline = (unsigned char)RSTRING_PTR(value);

  memset(&value, 0, sizeof(value));
  value = mrb_hash_get(mrb, hash, mrb_str_new_cstr(mrb, "GetDataPIN"));
  parameter.GetDataPIN = (unsigned char)RSTRING_PTR(value);

  memset(&value, 0, sizeof(value));
  value = mrb_hash_get(mrb, hash, mrb_str_new_cstr(mrb, "SurportPSESel"));
  parameter.SurportPSESel = (unsigned char)RSTRING_PTR(value);

  EMVSetParameter(&parameter);
}

mrb_value
mrb_s_core_init(mrb_state *mrb, mrb_value klass)
{
  return mrb_fixnum_value(EMVCoreInit());
}

static mrb_value
mrb_s_set_emv_parameter(mrb_state *mrb, mrb_value klass)
{
  mrb_value hash;

  mrb_get_args(mrb, "o", &hash);

  if (mrb_hash_p(hash))
    set_emv_parameter(mrb, klass, hash);
  else
    mrb_raise(mrb, E_ARGUMENT_ERROR, "object isn't a hash");

  return mrb_nil_value();
}

static mrb_value
mrb_s_get_emv_app(mrb_state *mrb, mrb_value klass)
{
  EMV_APPLIST parameter;
  mrb_value hash;
  mrb_int index;
  int ret=EMV_NOT_FOUND;

  memset(&parameter, 0, sizeof(parameter));

  mrb_get_args(mrb, "i", &index);

  ret = EMVGetApp(index, &parameter);

  if (ret == EMV_OK)
  {
    hash = mrb_funcall(mrb, klass, "app_default", 0);
    /*TODO Scalone: loss data is posible in conversation from unsigned char to const char*/
    mrb_hash_set(mrb , hash , mrb_str_new_cstr(mrb , "AppName")         , mrb_str_new_static(mrb , &parameter.AppName         , 33));
    mrb_hash_set(mrb , hash , mrb_str_new_cstr(mrb , "AID")             , mrb_str_new_static(mrb , &parameter.AID             , 17));
    mrb_hash_set(mrb , hash , mrb_str_new_cstr(mrb , "AidLen")          , mrb_str_new_static(mrb , &parameter.AidLen          , 1));
    mrb_hash_set(mrb , hash , mrb_str_new_cstr(mrb , "SelFlag")         , mrb_str_new_static(mrb , &parameter.SelFlag         , 1));
    mrb_hash_set(mrb , hash , mrb_str_new_cstr(mrb , "Priority")        , mrb_str_new_static(mrb , &parameter.Priority        , 1));
    mrb_hash_set(mrb , hash , mrb_str_new_cstr(mrb , "TargetPer")       , mrb_str_new_static(mrb , &parameter.TargetPer       , 1));
    mrb_hash_set(mrb , hash , mrb_str_new_cstr(mrb , "MaxTargetPer")    , mrb_str_new_static(mrb , &parameter.MaxTargetPer    , 1));
    mrb_hash_set(mrb , hash , mrb_str_new_cstr(mrb , "FloorLimitCheck") , mrb_str_new_static(mrb , &parameter.FloorLimitCheck , 1));
    mrb_hash_set(mrb , hash , mrb_str_new_cstr(mrb , "RandTransSel")    , mrb_str_new_static(mrb , &parameter.RandTransSel    , 1));
    mrb_hash_set(mrb , hash , mrb_str_new_cstr(mrb , "VelocityCheck")   , mrb_str_new_static(mrb , &parameter.VelocityCheck   , 1));
    mrb_hash_set(mrb , hash , mrb_str_new_cstr(mrb , "FloorLimit")      , mrb_str_new_static(mrb , &parameter.FloorLimit      , 1));
    mrb_hash_set(mrb , hash , mrb_str_new_cstr(mrb , "Threshold")       , mrb_str_new_static(mrb , &parameter.Threshold       , 1));
    mrb_hash_set(mrb , hash , mrb_str_new_cstr(mrb , "TACDenial")       , mrb_str_new_static(mrb , &parameter.TACDenial       , 6));
    mrb_hash_set(mrb , hash , mrb_str_new_cstr(mrb , "TACOnline")       , mrb_str_new_static(mrb , &parameter.TACOnline       , 6));
    mrb_hash_set(mrb , hash , mrb_str_new_cstr(mrb , "TACDefault")      , mrb_str_new_static(mrb , &parameter.TACDefault      , 6));
    mrb_hash_set(mrb , hash , mrb_str_new_cstr(mrb , "AcquierId")       , mrb_str_new_static(mrb , &parameter.AcquierId       , 6));
    mrb_hash_set(mrb , hash , mrb_str_new_cstr(mrb , "dDOL")            , mrb_str_new_static(mrb , &parameter.dDOL            , 256));
    mrb_hash_set(mrb , hash , mrb_str_new_cstr(mrb , "tDOL")            , mrb_str_new_static(mrb , &parameter.tDOL            , 256));
    mrb_hash_set(mrb , hash , mrb_str_new_cstr(mrb , "Version")         , mrb_str_new_static(mrb , &parameter.Version         , 3));
    mrb_hash_set(mrb , hash , mrb_str_new_cstr(mrb , "RiskManData")     , mrb_str_new_static(mrb , &parameter.RiskManData     , 10));

    return hash;
  } else {
    return mrb_fixnum_value(ret);
    /*mrb_raisef(mrb, E_EMV_ERROR, "object isn't a hash", );*/
  }
}

static int
set_emv_app(mrb_state *mrb, mrb_value klass, mrb_value hash)
{
  EMV_APPLIST parameter;
  mrb_value value;
  mrb_int iValue;

  memset(&parameter, 0, sizeof(parameter));

  memset(&value, 0, sizeof(value));
  value = mrb_hash_get(mrb, hash, mrb_str_new_cstr(mrb, "AppName"));
  strncpy(&parameter.AppName, RSTRING_PTR(value), 33);

  memset(&value, 0, sizeof(value));
  value = mrb_hash_get(mrb, hash, mrb_str_new_cstr(mrb, "AID"));
  strncpy(&parameter.AppName, RSTRING_PTR(value), 33);

  memset(&value, 0, sizeof(value));
  value = mrb_hash_get(mrb, hash, mrb_str_new_cstr(mrb, "AidLen"));
  parameter.AidLen = (unsigned char)RSTRING_PTR(value);

  memset(&value, 0, sizeof(value));
  value = mrb_hash_get(mrb, hash, mrb_str_new_cstr(mrb, "SelFlag"));
  parameter.SelFlag = (unsigned char)RSTRING_PTR(value);

  memset(&value, 0, sizeof(value));
  value = mrb_hash_get(mrb, hash, mrb_str_new_cstr(mrb, "Priority"));
  parameter.Priority = (unsigned char)RSTRING_PTR(value);

  memset(&value, 0, sizeof(value));
  value = mrb_hash_get(mrb, hash, mrb_str_new_cstr(mrb, "TargetPer"));
  parameter.TargetPer = (unsigned char)RSTRING_PTR(value);

  memset(&value, 0, sizeof(value));
  value = mrb_hash_get(mrb, hash, mrb_str_new_cstr(mrb, "MaxTargetPer"));
  parameter.MaxTargetPer = (unsigned char)RSTRING_PTR(value);

  memset(&value, 0, sizeof(value));
  value = mrb_hash_get(mrb, hash, mrb_str_new_cstr(mrb, "FloorLimitCheck"));
  parameter.FloorLimitCheck = (unsigned char)RSTRING_PTR(value);

  memset(&value, 0, sizeof(value));
  value = mrb_hash_get(mrb, hash, mrb_str_new_cstr(mrb, "RandTransSel"));
  parameter.RandTransSel = (unsigned char)RSTRING_PTR(value);

  memset(&value, 0, sizeof(value));
  value = mrb_hash_get(mrb, hash, mrb_str_new_cstr(mrb, "VelocityCheck"));
  parameter.VelocityCheck = (unsigned char)RSTRING_PTR(value);

  memset(&value, 0, sizeof(value));
  value = mrb_hash_get(mrb, hash, mrb_str_new_cstr(mrb, "FloorLimit"));
  parameter.FloorLimit = (unsigned char)RSTRING_PTR(value);

  memset(&value, 0, sizeof(value));
  iValue = mrb_fixnum(mrb_hash_get(mrb, hash, mrb_str_new_cstr(mrb, "FloorLimit")));
  parameter.FloorLimit = iValue;

  memset(&value, 0, sizeof(value));
  value = mrb_hash_get(mrb, hash, mrb_str_new_cstr(mrb, "Threshold"));
  parameter.Threshold = (unsigned char)RSTRING_PTR(value);

  memset(&value, 0, sizeof(value));
  value = mrb_hash_get(mrb, hash, mrb_str_new_cstr(mrb, "TACDenial"));
  strncpy(&parameter.TACDenial, RSTRING_PTR(value), 6);

  memset(&value, 0, sizeof(value));
  value = mrb_hash_get(mrb, hash, mrb_str_new_cstr(mrb, "TACOnline"));
  strncpy(&parameter.TACOnline, RSTRING_PTR(value), 6);

  memset(&value, 0, sizeof(value));
  value = mrb_hash_get(mrb, hash, mrb_str_new_cstr(mrb, "TACDefault"));
  strncpy(&parameter.TACDefault, RSTRING_PTR(value), 6);

  memset(&value, 0, sizeof(value));
  value = mrb_hash_get(mrb, hash, mrb_str_new_cstr(mrb, "AcquierId"));
  strncpy(&parameter.AcquierId, RSTRING_PTR(value), 6);

  memset(&value, 0, sizeof(value));
  value = mrb_hash_get(mrb, hash, mrb_str_new_cstr(mrb, "dDOL"));
  strncpy(&parameter.dDOL, RSTRING_PTR(value), 256);

  memset(&value, 0, sizeof(value));
  value = mrb_hash_get(mrb, hash, mrb_str_new_cstr(mrb, "tDOL"));
  strncpy(&parameter.tDOL, RSTRING_PTR(value), 256);

  memset(&value, 0, sizeof(value));
  value = mrb_hash_get(mrb, hash, mrb_str_new_cstr(mrb, "Version"));
  strncpy(&parameter.Version, RSTRING_PTR(value), 3);

  memset(&value, 0, sizeof(value));
  value = mrb_hash_get(mrb, hash, mrb_str_new_cstr(mrb, "RiskManData"));
  strncpy(&parameter.RiskManData, RSTRING_PTR(value), 10);

  return EMVAddApp(&parameter);
}

static mrb_value
mrb_s_set_emv_app(mrb_state *mrb, mrb_value klass)
{
  mrb_value hash;
  mrb_int ret=EMV_OK;

  mrb_get_args(mrb, "o", &hash);

  if (mrb_hash_p(hash))
    ret = set_emv_app(mrb, klass, hash);
  else
    mrb_raise(mrb, E_ARGUMENT_ERROR, "object isn't a hash");

  return mrb_fixnum_value(ret);
}

static mrb_value
mrb_s_set_emv_del_app(mrb_state *mrb, mrb_value klass)
{
  mrb_value aid;
  mrb_int ret=EMV_OK;

  mrb_get_args(mrb, "S", &aid);

  if (mrb_string_p(aid))
    ret = EMVDelApp(RSTRING_PTR(aid), RSTRING_LEN(aid));
  else
    mrb_raise(mrb, E_ARGUMENT_ERROR, "object isn't a string");

  return mrb_fixnum_value(ret);
}

static mrb_value
mrb_s_set_emv_del_apps(mrb_state *mrb, mrb_value klass)
{
  return mrb_fixnum_value(EMVDelAllApp());
}

void
mrb_emv_init(mrb_state* mrb)
{
  struct RClass *pax;
  struct RClass *emv;
  struct RClass *error;

  pax = mrb_class_get(mrb, "PAX");
  emv = mrb_define_class_under(mrb, pax, "EMV",  mrb->object_class);
  /*error = mrb_define_class_under(mrb, pax, "EMVError", mrb->eStandardError_class);*/

  mrb_define_class_method(mrb, emv, "core_init", mrb_s_core_init , MRB_ARGS_NONE());
  mrb_define_class_method(mrb, emv, "get_parameter", mrb_s_get_emv_parameter , MRB_ARGS_NONE());
  mrb_define_class_method(mrb, emv, "set_parameter", mrb_s_set_emv_parameter , MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb, emv, "get_app", mrb_s_get_emv_app , MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb, emv, "set_app", mrb_s_set_emv_app , MRB_ARGS_REQ(1));

  mrb_define_class_method(mrb, emv, "del_app", mrb_s_set_emv_del_app , MRB_ARGS_REQ(1));
}