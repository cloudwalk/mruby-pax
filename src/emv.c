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

void
mrb_emv_init(mrb_state* mrb)
{
  struct RClass *pax;
  struct RClass *emv;

  pax = mrb_class_get(mrb, "PAX");
  emv = mrb_define_class_under(mrb, pax, "EMV",  mrb->object_class);

  mrb_define_class_method(mrb, emv, "core_init", mrb_s_core_init , MRB_ARGS_NONE());
  mrb_define_class_method(mrb, emv, "get_parameter", mrb_s_get_emv_parameter , MRB_ARGS_NONE());
  mrb_define_class_method(mrb, emv, "set_parameter", mrb_s_set_emv_parameter , MRB_ARGS_REQ(1));
}