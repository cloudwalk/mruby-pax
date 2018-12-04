#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "mruby.h"
#include "mruby/compile.h"
#include "mruby/value.h"
#include "mruby/variable.h"
#include "mruby/array.h"
#include "mruby/string.h"
#include "mruby/hash.h"

#include "mruby/ext/context_log.h"

#include <unistd.h>

#include "ui.h"
#include "keyboard.h"
#include "osal.h"
#include "emvlib_Prolin.h"

mrb_state *current_mrb;
mrb_value current_klass;

void emv_applist_to_hash(mrb_state *mrb, mrb_value hash, EMV_APPLIST parameter);
int IccIsoCommand(uchar ucslot, APDU_SEND *tApduSend, APDU_RESP *tApduRecv);

void logEMVError(void)
{
  mrb_value context;
  char buf[1024];
  char paucAssistInfo[1024];
  mrb_int iRet, pnErrorCode, nExpAssistInfoLen;

  memset(&buf, 0, sizeof(buf));
  memset(&paucAssistInfo, 0, sizeof(paucAssistInfo));

  iRet = EMVGetDebugInfo(&nExpAssistInfoLen, &paucAssistInfo, &pnErrorCode);

  if (iRet == EMV_OK) {
    sprintf(&buf, "EMVGetDebugInfo [%d]", pnErrorCode);
    context = mrb_const_get(current_mrb, mrb_obj_value(current_mrb->object_class), mrb_intern_lit(current_mrb, "ContextLog"));
    mrb_funcall(current_mrb, context, "info", 1, mrb_str_new_cstr(current_mrb, buf));
  }
}

/*
 *CTLS
 *#include "CLEntryAPI_Prolin.h"
 *#include "ClssApi_Wave_prolin.h"
 *#include "ClssApi_MC_prolin.h"
 */

/*Callbacks*/

/**
 * @fn	int cEMVPedVerifyPlainPin (uchar IccSlot,uchar *ExpPinLenIn,uchar *IccRespOut,uchar Mode,ulong TimeoutMs)
 * @brief	EMV callback function for offline access and plaintext PIN checking
 * @param	[in] IccSlot
 * @param	[in] ExpPinLenIn
 * @param	[in] Mode
 * @param	[in] TimeoutMs
 * @param	[out] IccRespOut
 * @return int
 * @author	Prolin App developer
 * @date	2013-05-20
 */
int cEMVPedVerifyPlainPin (uchar IccSlot,uchar *ExpPinLenIn,uchar *IccRespOut,uchar Mode,ulong TimeoutMs)
{
  mrb_value hash, block;

  hash = mrb_funcall(current_mrb, current_klass, "internal_get_pin_plain", 2,
      mrb_fixnum_value((mrb_int)IccSlot), mrb_str_new_cstr(current_mrb, ExpPinLenIn));
  block  = mrb_hash_get(current_mrb, hash, mrb_str_new_lit(current_mrb , "block"));

  memcpy(IccRespOut, RSTRING_PTR(block), RSTRING_LEN(block));

  return mrb_fixnum(mrb_hash_get(current_mrb, hash, mrb_str_new_lit(current_mrb , "return")));
}

/**
 * @fn int cEMVPedVerifyCipherPin (uchar IccSlot,uchar *ExpPinLenIn,RSA_PINKEY *RsaPinKeyIn, uchar *IccRespOut, uchar Mode, ulong TimeoutMs)
 * @brief	EMV回调函数，实现脱机密文PIN的获取和密文PIN的校验
 * @param	[in] IccSlot 卡片所在的卡座号
 * @param	[in] ExpPinLenIn	可输入的合法密码长度字符串
 * @param	[in] RsaPinKeyIn	加密所需数据结构
 * @param	[in] Mode	IC卡命令模式
 * @param	[in] TimeoutMs	输入PIN的超时时间
 * @param	[out] IccRespOut 卡片响应的状态码
 * @return int
 * @author	Prolin App developer
 * @date	2013-05-20
 */
int cEMVPedVerifyCipherPin (uchar IccSlot,uchar *ExpPinLenIn,RSA_PINKEY *RsaPinKeyIn, uchar *IccRespOut, uchar Mode, ulong TimeoutMs)
{
  mrb_value response, block, rsa;

  rsa = mrb_hash_new(current_mrb);
  mrb_hash_set(current_mrb, rsa, mrb_str_new_lit(current_mrb, "modulus_length"), mrb_fixnum_value(RsaPinKeyIn->modlen));
  mrb_hash_set(current_mrb, rsa, mrb_str_new_lit(current_mrb, "modulus"), mrb_str_new(current_mrb, RsaPinKeyIn->mod, sizeof(RsaPinKeyIn->mod)));
  mrb_hash_set(current_mrb, rsa, mrb_str_new_lit(current_mrb, "exponent"), mrb_str_new(current_mrb, RsaPinKeyIn->exp, sizeof(RsaPinKeyIn->exp)));
  mrb_hash_set(current_mrb, rsa, mrb_str_new_lit(current_mrb, "random_length"), mrb_fixnum_value(RsaPinKeyIn->iccrandomlen));
  mrb_hash_set(current_mrb, rsa, mrb_str_new_lit(current_mrb, "random"), mrb_str_new(current_mrb, RsaPinKeyIn->iccrandom, sizeof(RsaPinKeyIn->iccrandom)));

  response = mrb_funcall(current_mrb, current_klass, "internal_verify_cipher_pin", 3,
      mrb_fixnum_value((mrb_int)IccSlot), mrb_str_new_cstr(current_mrb, ExpPinLenIn), rsa);

  block  = mrb_hash_get(current_mrb, response, mrb_str_new_lit(current_mrb , "block"));

  memcpy(IccRespOut, RSTRING_PTR(block), RSTRING_LEN(block));

  return mrb_fixnum(mrb_hash_get(current_mrb, response, mrb_str_new_lit(current_mrb , "return")));
}

/**
 * @fn	int  cEMVIccIsoCommand(uchar ucslot, APDU_SEND *tApduSend, APDU_RESP *tApduRecv)
 * @brief	EMV回调函数，实现接触式读卡操作
 * @param	[in] ucslot 卡片逻辑通道号
 * @param	[in] tApduSend	发送给ICC卡命令数据结构
 * @param	[out] tApduRecv 从ICC卡返回的数据结构
 * @return int
 * @author	Prolin App developer
 * @date	2013-05-20
 */
uchar cEMVIccIsoCommand(uchar ucslot, APDU_SEND *tApduSend, APDU_RESP *tApduRecv)
{
  int iRet;

  iRet = IccIsoCommand(ucslot, tApduSend, tApduRecv);

  /*DEBUG*/
  /*ContextLog(current_mrb, 0, "IccIsoCommand iRet [%d] SWA [%d] SWB [%d]", iRet, tApduRecv->SWA, tApduRecv->SWB);*/

  if (iRet == 0) return EMV_OK;
  else if(iRet == 1 || iRet == 2) return iRet;
  else {
    if(tApduRecv->SWA == 0x90 && tApduRecv->SWB == 0x00) {
      /*DEBUG*/
      /*ContextLog(current_mrb, 0, "IccIsoCommand return EMV_OK");*/
      return EMV_OK;
    } else if(tApduRecv->SWA == 0x63 && (tApduRecv->SWB & 0xc0) == 0xc0) {
      /*DEBUG*/
      /*ContextLog(current_mrb, 0, "IccIsoCommand return [%d]", (tApduRecv->SWB & 0x0F) + 1);*/
      return ((tApduRecv->SWB & 0x0F) + 1);
    } else if(tApduRecv->SWA == 0x69 && (tApduRecv->SWB == 0x83 || tApduRecv->SWB == 0x84)) {
      /*DEBUG*/
      /*ContextLog(current_mrb, 0, "IccIsoCommand return EMV_RSP_ERR");*/
      return EMV_RSP_ERR;
    } else {
      /*DEBUG*/
      /*ContextLog(current_mrb, 0, "IccIsoCommand return 0x01");*/
      return 0x01;
    }
  }
}

// Callback function required by EMV core.
// in EMV ver 2.1+, this function is called before GPO
int cEMVSetParam(void)
{
  return EMV_OK;
}

unsigned char cEMVSM3(unsigned char *paucMsgIn, int nMsglenIn,unsigned char *paucResultOut)
{
  return EMV_OK;
}

unsigned char cEMVSM2Verify(unsigned char *paucPubkeyIn,unsigned char *paucMsgIn,int nMsglenIn, unsigned char *paucSignIn, int nSignlenIn)
{
  return EMV_OK;
}

// it is acallback function for EMV kernel,
// for displaying a amount input box,
// developer customize
int cEMVInputAmount(ulong *AuthAmt, ulong *CashBackAmt)
{
  return EMV_OK;
}

// Callback function required by EMV core.
// Wait holder enter PIN.
// developer customized.
// Modified by Kim_LinHB 2014-6-8 v1.01.0000
int cEMVGetHolderPwd(int iTryFlag, int iRemainCnt, uchar *pszPlainPin)
{
  mrb_value hash, block;
  mrb_int iRet=0;

  if (pszPlainPin == NULL) {
    hash = mrb_funcall(current_mrb, current_klass, "internal_get_pin_block", 3,
        mrb_fixnum_value((mrb_int)iTryFlag), mrb_fixnum_value((mrb_int)iRemainCnt),
        mrb_nil_value());
  } else {
    hash = mrb_funcall(current_mrb, current_klass, "internal_get_pin_block", 3,
        mrb_fixnum_value((mrb_int)iTryFlag), mrb_fixnum_value((mrb_int)iRemainCnt),
        mrb_str_new(current_mrb, pszPlainPin, 8));
  }
  block = mrb_hash_get(current_mrb, hash, mrb_str_new_lit(current_mrb , "block"));

  if (! mrb_nil_p(block)) {
    memcpy(&pszPlainPin, RSTRING_PTR(block), 8);
  }

  return mrb_fixnum(mrb_hash_get(current_mrb, hash, mrb_str_new_lit(current_mrb , "return")));
}

// 持卡人认证例程
// Callback function required by EMV core.
// Don't need to care about this function
int cCertVerify(void)
{
  return -1;
}

unsigned char cEMVPiccIsoCommand(unsigned char cid,APDU_SEND *ApduSend,APDU_RESP *ApduRecv)
{
  return 0;
}

// 处理DOL的过程中，EMV库遇到不识别的TAG时会调用该回调函数，要求应用程序处理
// 如果应用程序无法处理，则直接返回-1，提供该函数只为解决一些不符合EMV的特殊
// 应用程序的要求，一般情况下返回-1即可
// Callback function required by EMV core.
// When processing DOL, if there is a tag that EMV core doesn't know about, core will call this function.
// developer should offer processing for proprietary tag.
// if really unable to, just return -1
int cEMVUnknowTLVData(ushort iTag, uchar *psDat, int iDataLen)
{
  return 0;
}

// Modified by Kim_LinHB 2014-5-31
// for displaying a application list to card holder to select
// if there is only one application in the chip, then EMV kernel will not call this callback function
int cEMVWaitAppSel(int TryCnt, EMV_APPLIST List[], int AppNum)
{
  int iCnt, iAppCnt;
  mrb_value hash, array, labels;
  APPLABEL_LIST stAppList[MAX_APP_NUM];

  array  = mrb_ary_new(current_mrb);
  EMVGetLabelList(stAppList, &iAppCnt);

  for (iCnt = 0; iCnt < iAppCnt && iCnt<MAX_APP_NUM; iCnt++) {
    hash = mrb_hash_new(current_mrb);
    emv_applist_to_hash(current_mrb, hash, List[iCnt]);
    mrb_hash_set(current_mrb, hash, mrb_str_new_lit(current_mrb, "label"),
        mrb_str_new_cstr(current_mrb, (const char*)stAppList[iCnt].aucAppLabel));
    mrb_hash_set(current_mrb, hash, mrb_str_new_lit(current_mrb, "aid"),
        mrb_str_new(current_mrb, (const char*)stAppList[iCnt].aucAID, 16));
    mrb_hash_set(current_mrb, hash, mrb_str_new_lit(current_mrb, "index"), mrb_fixnum_value(iCnt));
    mrb_ary_push(current_mrb, array, hash);
  }

  if (iAppCnt == 1) return 0;

  return mrb_fixnum(mrb_funcall(current_mrb, current_klass, "internal_app_select", 2, array, mrb_fixnum_value(TryCnt)));
}

// 如果不需要提示密码验证成功，则直接返回就可以了
// Callback function required by EMV core.
// Display "EMV PIN OK" info. (plaintext/enciphered PIN)
// Modified by Kim_LinHB 2014-6-8 v1.01.0000
void cEMVVerifyPINOK(void)
{
  return;
}

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
  hash = mrb_hash_new(mrb);

  /*TODO Scalone: loss data is posible in conversation from unsigned char to const char*/
  mrb_hash_set(mrb , hash , mrb_str_new_lit(mrb , "MerchName")     , mrb_str_new(mrb , (const char *)&parameter.MerchName     , 256));
  mrb_hash_set(mrb , hash , mrb_str_new_lit(mrb , "MerchCateCode") , mrb_str_new(mrb , (const char *)&parameter.MerchCateCode , 2));
  mrb_hash_set(mrb , hash , mrb_str_new_lit(mrb , "MerchId")       , mrb_str_new(mrb , (const char *)&parameter.MerchId       , 15));
  mrb_hash_set(mrb , hash , mrb_str_new_lit(mrb , "TermId")        , mrb_str_new(mrb , (const char *)&parameter.TermId        , 8));
  mrb_hash_set(mrb , hash , mrb_str_new_lit(mrb , "TerminalType")  , mrb_str_new(mrb , (const char *)&parameter.TerminalType  , 1));
  mrb_hash_set(mrb , hash , mrb_str_new_lit(mrb , "Capability")    , mrb_str_new(mrb , (const char *)&parameter.Capability    , 3));
  mrb_hash_set(mrb , hash , mrb_str_new_lit(mrb , "ExCapability")  , mrb_str_new(mrb , (const char *)&parameter.ExCapability  , 5));
  mrb_hash_set(mrb , hash , mrb_str_new_lit(mrb , "TransCurrExp")  , mrb_str_new(mrb , (const char *)&parameter.TransCurrExp  , 1));
  mrb_hash_set(mrb , hash , mrb_str_new_lit(mrb , "ReferCurrExp")  , mrb_str_new(mrb , (const char *)&parameter.ReferCurrExp  , 1));
  mrb_hash_set(mrb , hash , mrb_str_new_lit(mrb , "ReferCurrCode") , mrb_str_new(mrb , (const char *)&parameter.ReferCurrCode , 2));
  mrb_hash_set(mrb , hash , mrb_str_new_lit(mrb , "CountryCode")   , mrb_str_new(mrb , (const char *)&parameter.CountryCode   , 2));
  mrb_hash_set(mrb , hash , mrb_str_new_lit(mrb , "TransCurrCode") , mrb_str_new(mrb , (const char *)&parameter.TransCurrCode , 2));
  mrb_hash_set(mrb , hash , mrb_str_new_lit(mrb , "ReferCurrCon")  , mrb_str_new(mrb , (const char *)&parameter.ReferCurrCon  , 1));
  mrb_hash_set(mrb , hash , mrb_str_new_lit(mrb , "TransType")     , mrb_str_new(mrb , (const char *)&parameter.TransType     , 1));
  mrb_hash_set(mrb , hash , mrb_str_new_lit(mrb , "ForceOnline")   , mrb_str_new(mrb , (const char *)&parameter.ForceOnline   , 1));
  mrb_hash_set(mrb , hash , mrb_str_new_lit(mrb , "GetDataPIN")    , mrb_str_new(mrb , (const char *)&parameter.GetDataPIN    , 1));
  mrb_hash_set(mrb , hash , mrb_str_new_lit(mrb , "SurportPSESel") , mrb_str_new(mrb , (const char *)&parameter.SurportPSESel , 1));

  return hash;
}

  static void
set_emv_parameter(mrb_state *mrb, mrb_value klass, mrb_value hash)
{
  EMV_PARAM parameter;
  mrb_value value;
  mrb_int iValue;

  memset(&parameter, 0, sizeof(parameter));
  EMVSetParameter(&parameter);

  memset(&parameter, 0, sizeof(parameter));

  EMVGetParameter(&parameter);

  /*memset(&value, 0, sizeof(value));*/
  /*value = mrb_hash_get(mrb, hash, mrb_str_new_lit(mrb, "MerchName"));*/
  /*memcpy(&parameter.MerchName, RSTRING_PTR(value), 256);*/

  /*memset(&value, 0, sizeof(value));*/
  /*value = mrb_hash_get(mrb, hash, mrb_str_new_lit(mrb, "MerchCateCode"));*/
  /*memcpy(&parameter.MerchCateCode, RSTRING_PTR(value), 2);*/

  memset(&value, 0, sizeof(value));
  value = mrb_hash_get(mrb, hash, mrb_str_new_lit(mrb, "MerchId"));
  memcpy(&parameter.MerchId, RSTRING_PTR(value), 15);

  memset(&value, 0, sizeof(value));
  value = mrb_hash_get(mrb, hash, mrb_str_new_lit(mrb, "TermId"));
  memcpy(&parameter.TermId, RSTRING_PTR(value), 8);

  memset(&value, 0, sizeof(value));
  value = mrb_hash_get(mrb, hash, mrb_str_new_lit(mrb, "TerminalType"));
  memcpy(&parameter.TerminalType, RSTRING_PTR(value), 1);

  memset(&value, 0, sizeof(value));
  value = mrb_hash_get(mrb, hash, mrb_str_new_lit(mrb, "Capability"));
  memcpy(&parameter.Capability, RSTRING_PTR(value), 3);

  memset(&value, 0, sizeof(value));
  value = mrb_hash_get(mrb, hash, mrb_str_new_lit(mrb, "ExCapability"));
  memcpy(&parameter.ExCapability, RSTRING_PTR(value), 5);

  memset(&value, 0, sizeof(value));
  value = mrb_hash_get(mrb, hash, mrb_str_new_lit(mrb, "TransCurrExp"));
  memcpy(&parameter.TransCurrExp, RSTRING_PTR(value), 1);

  memset(&value, 0, sizeof(value));
  value = mrb_hash_get(mrb, hash, mrb_str_new_lit(mrb, "ReferCurrExp"));
  memcpy(&parameter.ReferCurrExp, RSTRING_PTR(value), 1);

  memset(&value, 0, sizeof(value));
  value = mrb_hash_get(mrb, hash, mrb_str_new_lit(mrb, "ReferCurrCode"));
  memcpy(&parameter.ReferCurrCode, RSTRING_PTR(value), 2);

  memset(&value, 0, sizeof(value));
  value = mrb_hash_get(mrb, hash, mrb_str_new_lit(mrb, "CountryCode"));
  memcpy(&parameter.CountryCode, RSTRING_PTR(value), 2);

  memset(&value, 0, sizeof(value));
  value = mrb_hash_get(mrb, hash, mrb_str_new_lit(mrb, "TransCurrCode"));
  memcpy(&parameter.TransCurrCode, RSTRING_PTR(value), 2);

  memset(&value, 0, sizeof(value));
  iValue = mrb_fixnum(mrb_hash_get(mrb, hash, mrb_str_new_lit(mrb, "ReferCurrCon")));
  parameter.ReferCurrCon = iValue;

  memset(&value, 0, sizeof(value));
  value = mrb_hash_get(mrb, hash, mrb_str_new_lit(mrb, "TransType"));
  memcpy(&parameter.TransType, RSTRING_PTR(value), 1);

  memset(&value, 0, sizeof(value));
  value = mrb_hash_get(mrb, hash, mrb_str_new_lit(mrb, "ForceOnline"));
  parameter.ForceOnline = 0;

  memset(&value, 0, sizeof(value));
  value = mrb_hash_get(mrb, hash, mrb_str_new_lit(mrb, "GetDataPIN"));
  parameter.GetDataPIN = 1;

  memset(&value, 0, sizeof(value));
  value = mrb_hash_get(mrb, hash, mrb_str_new_lit(mrb, "SurportPSESel"));
  parameter.SurportPSESel = 1;

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
emv_applist_to_hash(mrb_state *mrb, mrb_value hash, EMV_APPLIST parameter)
{
  /*TODO Scalone: loss data is posible in conversation from unsigned char to const char*/
  /*TODO Scalone AppName 16 instead 32 for now*/
  mrb_hash_set(mrb , hash , mrb_str_new_lit(mrb , "AppName")         , mrb_str_new(mrb      , (const char *)&parameter.AppName         , 16));
  mrb_hash_set(mrb , hash , mrb_str_new_lit(mrb , "AID")             , mrb_str_new(mrb      , (const char *)&parameter.AID             , 16));
  mrb_hash_set(mrb , hash , mrb_str_new_lit(mrb , "AidLen")          , mrb_str_new(mrb      , (const char *)&parameter.AidLen          , 1));
  mrb_hash_set(mrb , hash , mrb_str_new_lit(mrb , "SelFlag")         , mrb_str_new(mrb      , (const char *)&parameter.SelFlag         , 1));
  mrb_hash_set(mrb , hash , mrb_str_new_lit(mrb , "Priority")        , mrb_str_new(mrb      , (const char *)&parameter.Priority        , 1));
  mrb_hash_set(mrb , hash , mrb_str_new_lit(mrb , "TargetPer")       , mrb_str_new(mrb      , (const char *)&parameter.TargetPer       , 1));
  mrb_hash_set(mrb , hash , mrb_str_new_lit(mrb , "MaxTargetPer")    , mrb_str_new(mrb      , (const char *)&parameter.MaxTargetPer    , 1));
  mrb_hash_set(mrb , hash , mrb_str_new_lit(mrb , "FloorLimitCheck") , mrb_str_new(mrb      , (const char *)&parameter.FloorLimitCheck , 1));
  mrb_hash_set(mrb , hash , mrb_str_new_lit(mrb , "RandTransSel")    , mrb_str_new(mrb      , (const char *)&parameter.RandTransSel    , 1));
  mrb_hash_set(mrb , hash , mrb_str_new_lit(mrb , "VelocityCheck")   , mrb_str_new(mrb      , (const char *)&parameter.VelocityCheck   , 1));
  mrb_hash_set(mrb , hash , mrb_str_new_lit(mrb , "FloorLimit")      , mrb_fixnum_value(parameter.FloorLimit));
  mrb_hash_set(mrb , hash , mrb_str_new_lit(mrb , "Threshold")       , mrb_str_new(mrb      , (const char *)&parameter.Threshold       , 1));
  mrb_hash_set(mrb , hash , mrb_str_new_lit(mrb , "TACDenial")       , mrb_str_new(mrb      , (const char *)&parameter.TACDenial       , 5));
  mrb_hash_set(mrb , hash , mrb_str_new_lit(mrb , "TACOnline")       , mrb_str_new(mrb      , (const char *)&parameter.TACOnline       , 5));
  mrb_hash_set(mrb , hash , mrb_str_new_lit(mrb , "TACDefault")      , mrb_str_new(mrb      , (const char *)&parameter.TACDefault      , 5));
  mrb_hash_set(mrb , hash , mrb_str_new_lit(mrb , "AcquierId")       , mrb_str_new(mrb      , (const char *)&parameter.AcquierId       , 5));
  mrb_hash_set(mrb , hash , mrb_str_new_lit(mrb , "dDOL")            , mrb_str_new(mrb      , (const char *)&parameter.dDOL            , 256));
  mrb_hash_set(mrb , hash , mrb_str_new_lit(mrb , "tDOL")            , mrb_str_new(mrb      , (const char *)&parameter.tDOL            , 256));
  mrb_hash_set(mrb , hash , mrb_str_new_lit(mrb , "Version")         , mrb_str_new(mrb      , (const char *)&parameter.Version         , 3));
  mrb_hash_set(mrb , hash , mrb_str_new_lit(mrb , "RiskManData")     , mrb_str_new(mrb      , (const char *)&parameter.RiskManData     , 10));
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
    hash = mrb_hash_new(mrb);
    emv_applist_to_hash(mrb, hash, parameter);

    return hash;
  } else {
    return mrb_fixnum_value(ret);
  }
}

  static int
add_emv_app(mrb_state *mrb, mrb_value klass, mrb_value hash)
{
  EMV_APPLIST parameter;
  mrb_value value;
  mrb_int iValue;

  memset(&parameter, 0, sizeof(parameter));

  memset(&value, 0, sizeof(value));
  value = mrb_hash_get(mrb, hash, mrb_str_new_lit(mrb, "AppName"));
  memcpy(&parameter.AppName, RSTRING_PTR(value), RSTRING_LEN(value));

  memset(&value, 0, sizeof(value));
  value = mrb_hash_get(mrb, hash, mrb_str_new_lit(mrb, "AID"));
  memcpy(&parameter.AID, RSTRING_PTR(value), 14);

  memset(&value, 0, sizeof(value));
  value = mrb_hash_get(mrb, hash, mrb_str_new_lit(mrb, "AidLen"));
  memcpy(&parameter.AidLen, RSTRING_PTR(value), 1);

  memset(&value, 0, sizeof(value));
  value = mrb_hash_get(mrb, hash, mrb_str_new_lit(mrb, "SelFlag"));
  memcpy(&parameter.SelFlag, RSTRING_PTR(value), 1);

  memset(&value, 0, sizeof(value));
  value = mrb_hash_get(mrb, hash, mrb_str_new_lit(mrb, "Priority"));
  memcpy(&parameter.Priority, RSTRING_PTR(value), 1);

  memset(&value, 0, sizeof(value));
  value = mrb_hash_get(mrb, hash, mrb_str_new_lit(mrb, "TargetPer"));
  memcpy(&parameter.TargetPer, RSTRING_PTR(value), 1);

  memset(&value, 0, sizeof(value));
  value = mrb_hash_get(mrb, hash, mrb_str_new_lit(mrb, "MaxTargetPer"));
  memcpy(&parameter.MaxTargetPer, RSTRING_PTR(value), 1);

  memset(&value, 0, sizeof(value));
  value = mrb_hash_get(mrb, hash, mrb_str_new_lit(mrb, "FloorLimitCheck"));
  memcpy(&parameter.FloorLimitCheck, RSTRING_PTR(value), 1);

  memset(&value, 0, sizeof(value));
  value = mrb_hash_get(mrb, hash, mrb_str_new_lit(mrb, "RandTransSel"));
  memcpy(&parameter.RandTransSel, RSTRING_PTR(value), 1);

  memset(&value, 0, sizeof(value));
  value = mrb_hash_get(mrb, hash, mrb_str_new_lit(mrb, "VelocityCheck"));
  memcpy(&parameter.VelocityCheck, RSTRING_PTR(value), 1);
  parameter.VelocityCheck = 1;

  iValue = mrb_fixnum(mrb_hash_get(mrb, hash, mrb_str_new_lit(mrb, "FloorLimit")));
  parameter.FloorLimit = iValue;

  memset(&value, 0, sizeof(value));
  value = mrb_hash_get(mrb, hash, mrb_str_new_lit(mrb, "Threshold"));
  memcpy(&parameter.Threshold, RSTRING_PTR(value), 1);

  memset(&value, 0, sizeof(value));
  value = mrb_hash_get(mrb, hash, mrb_str_new_lit(mrb, "TACDenial"));
  memcpy(&parameter.TACDenial, RSTRING_PTR(value), 6);

  memset(&value, 0, sizeof(value));
  value = mrb_hash_get(mrb, hash, mrb_str_new_lit(mrb, "TACOnline"));
  memcpy(&parameter.TACOnline, RSTRING_PTR(value), 6);

  memset(&value, 0, sizeof(value));
  value = mrb_hash_get(mrb, hash, mrb_str_new_lit(mrb, "TACDefault"));
  memcpy(&parameter.TACDefault, RSTRING_PTR(value), 6);

  memset(&value, 0, sizeof(value));
  value = mrb_hash_get(mrb, hash, mrb_str_new_lit(mrb, "AcquierId"));
  memcpy(&parameter.AcquierId, RSTRING_PTR(value), 6);

  memset(&value, 0, sizeof(value));
  value = mrb_hash_get(mrb, hash, mrb_str_new_lit(mrb, "dDOL"));
  memcpy(&parameter.dDOL, RSTRING_PTR(value), RSTRING_LEN(value));

  memset(&value, 0, sizeof(value));
  value = mrb_hash_get(mrb, hash, mrb_str_new_lit(mrb, "tDOL"));
  memcpy(&parameter.tDOL, RSTRING_PTR(value), RSTRING_LEN(value));

  memset(&value, 0, sizeof(value));
  value = mrb_hash_get(mrb, hash, mrb_str_new_lit(mrb, "Version"));
  memcpy(&parameter.Version, RSTRING_PTR(value), RSTRING_LEN(value));

  memset(&value, 0, sizeof(value));
  value = mrb_hash_get(mrb, hash, mrb_str_new_lit(mrb, "RiskManData"));
  memcpy(&parameter.RiskManData, RSTRING_PTR(value), 10);

  return EMVAddApp(&parameter);
}

  static mrb_value
mrb_s_add_emv_app(mrb_state *mrb, mrb_value klass)
{
  mrb_value hash;
  mrb_int ret=EMV_OK;

  mrb_get_args(mrb, "o", &hash);

  if (mrb_hash_p(hash))
    ret = add_emv_app(mrb, klass, hash);
  else
    mrb_raise(mrb, E_ARGUMENT_ERROR, "object isn't a hash");

  return mrb_fixnum_value(ret);
}

  static mrb_value
mrb_s_del_emv_app(mrb_state *mrb, mrb_value klass)
{
  mrb_value aid;
  mrb_int ret=EMV_OK;

  mrb_get_args(mrb, "s", &aid);

  if (mrb_string_p(aid))
    ret = EMVDelApp((unsigned char *)RSTRING_PTR(aid), RSTRING_LEN(aid));
  else
    mrb_raise(mrb, E_ARGUMENT_ERROR, "object isn't a string");

  return mrb_fixnum_value(ret);
}

  static mrb_value
mrb_s_del_emv_apps(mrb_state *mrb, mrb_value klass)
{
  return mrb_fixnum_value(EMVDelAllApp());
}

  static mrb_value
mrb_s_get_emv_pki(mrb_state *mrb, mrb_value klass)
{
  EMV_CAPK parameter;
  mrb_value hash;
  mrb_int index;
  int ret=EMV_NOT_FOUND;

  memset(&parameter, 0, sizeof(parameter));

  mrb_get_args(mrb, "i", &index);

  ret = EMVGetCAPK(index, &parameter);

  if (ret == EMV_OK) {
    hash = mrb_hash_new(mrb);

    /*TODO Scalone: loss data is posible in conversation from unsigned char to const char*/
    mrb_hash_set(mrb , hash , mrb_str_new_lit(mrb , "RID")         , mrb_str_new(mrb , (const char *)&parameter.RID         , 5));
    mrb_hash_set(mrb , hash , mrb_str_new_lit(mrb , "KeyID")       , mrb_str_new(mrb , (const char *)&parameter.KeyID       , 1));
    mrb_hash_set(mrb , hash , mrb_str_new_lit(mrb , "HashInd")     , mrb_str_new(mrb , (const char *)&parameter.HashInd     , 1));
    mrb_hash_set(mrb , hash , mrb_str_new_lit(mrb , "ArithInd")    , mrb_str_new(mrb , (const char *)&parameter.ArithInd    , 1));
    mrb_hash_set(mrb , hash , mrb_str_new_lit(mrb , "ModulLen")    , mrb_str_new(mrb , (const char *)&parameter.ModulLen    , 1));
    mrb_hash_set(mrb , hash , mrb_str_new_lit(mrb , "Modul")       , mrb_str_new(mrb , (const char *)&parameter.Modul       , 248));
    mrb_hash_set(mrb , hash , mrb_str_new_lit(mrb , "ExponentLen") , mrb_str_new(mrb , (const char *)&parameter.ExponentLen , 1));
    mrb_hash_set(mrb , hash , mrb_str_new_lit(mrb , "Exponent")    , mrb_str_new(mrb , (const char *)&parameter.Exponent    , 3));
    mrb_hash_set(mrb , hash , mrb_str_new_lit(mrb , "ExpDate")     , mrb_str_new(mrb , (const char *)&parameter.ExpDate     , 3));
    mrb_hash_set(mrb , hash , mrb_str_new_lit(mrb , "CheckSum")    , mrb_str_new(mrb , (const char *)&parameter.CheckSum    , 20));

    return hash;
  } else {
    return mrb_fixnum_value(ret);
  }
}

  static int
add_emv_pki(mrb_state *mrb, mrb_value klass, mrb_value hash)
{
  EMV_CAPK parameter;
  mrb_value value;

  memset(&parameter, 0, sizeof(parameter));

  memset(&value, 0, sizeof(value));
  value = mrb_hash_get(mrb, hash, mrb_str_new_lit(mrb, "RID"));
  memcpy(&parameter.RID, RSTRING_PTR(value), 5);

  memset(&value, 0, sizeof(value));
  value = mrb_hash_get(mrb, hash, mrb_str_new_lit(mrb, "KeyID"));
  memcpy(&parameter.KeyID, RSTRING_PTR(value), 1);

  memset(&value, 0, sizeof(value));
  value = mrb_hash_get(mrb, hash, mrb_str_new_lit(mrb, "HashInd"));
  memcpy(&parameter.HashInd, RSTRING_PTR(value), 1);

  memset(&value, 0, sizeof(value));
  value = mrb_hash_get(mrb, hash, mrb_str_new_lit(mrb, "ArithInd"));
  memcpy(&parameter.ArithInd, RSTRING_PTR(value), 1);

  memset(&value, 0, sizeof(value));
  value = mrb_hash_get(mrb, hash, mrb_str_new_lit(mrb, "ModulLen"));
  memcpy(&parameter.ModulLen, RSTRING_PTR(value), 1);

  memset(&value, 0, sizeof(value));
  value = mrb_hash_get(mrb, hash, mrb_str_new_lit(mrb, "Modul"));
  memcpy(&parameter.Modul, RSTRING_PTR(value), (int)parameter.ModulLen);

  memset(&value, 0, sizeof(value));
  value = mrb_hash_get(mrb, hash, mrb_str_new_lit(mrb, "ExponentLen"));
  memcpy(&parameter.ExponentLen, RSTRING_PTR(value), 1);

  memset(&value, 0, sizeof(value));
  value = mrb_hash_get(mrb, hash, mrb_str_new_lit(mrb, "Exponent"));
  memcpy(&parameter.Exponent, RSTRING_PTR(value), (int)parameter.ExponentLen);

  memset(&value, 0, sizeof(value));
  value = mrb_hash_get(mrb, hash, mrb_str_new_lit(mrb, "ExpDate"));
  memcpy(&parameter.ExpDate, RSTRING_PTR(value), 3);

  memset(&value, 0, sizeof(value));
  value = mrb_hash_get(mrb, hash, mrb_str_new_lit(mrb, "CheckSum"));
  memcpy(&parameter.CheckSum, RSTRING_PTR(value), 20);

  return EMVAddCAPK(&parameter);
}

  static mrb_value
mrb_s_add_emv_pki(mrb_state *mrb, mrb_value klass)
{
  mrb_value hash;
  mrb_int ret=EMV_OK;

  mrb_get_args(mrb, "o", &hash);

  if (mrb_hash_p(hash))
    ret = add_emv_pki(mrb, klass, hash);
  else
    mrb_raise(mrb, E_ARGUMENT_ERROR, "object isn't a hash");

  return mrb_fixnum_value(ret);
}

  static mrb_value
mrb_s_del_emv_pki(mrb_state *mrb, mrb_value klass)
{
  mrb_value keyID, rid;
  mrb_int ret=EMV_OK;
  unsigned char key;

  mrb_get_args(mrb, "ss", &keyID, &rid);

  if (mrb_string_p(keyID) && mrb_string_p(rid)) {
    memcpy(&key, RSTRING_PTR(keyID), 1);
    ret = EMVDelCAPK(key, (unsigned char *)RSTRING_PTR(rid));
  } else
    mrb_raise(mrb, E_ARGUMENT_ERROR, "object isn't a string");

  return mrb_fixnum_value(ret);
}

  static mrb_value
mrb_emv_s_del_pkis(mrb_state *mrb, mrb_value klass)
{
  mrb_int i;
  EMV_CAPK capk;

  for(i=0;i< MAX_KEY_NUM; i ++) {
    memset(&capk, 0, sizeof(capk));
    if(EMVGetCAPK(i, &capk) == EMV_OK) EMVDelCAPK(capk.KeyID, capk.RID);
  }

  return mrb_nil_value();
}

  static mrb_value
mrb_s_check_emv_pki(mrb_state *mrb, mrb_value klass)
{
  mrb_value keyID, rid;
  mrb_int ret=EMV_OK;

  mrb_get_args(mrb, "ss", &keyID, &rid);

  if (mrb_string_p(keyID) && mrb_string_p(rid))
    ret = EMVCheckCAPK((unsigned char *)RSTRING_PTR(keyID), (unsigned char *)RSTRING_PTR(rid));
  else
    mrb_raise(mrb, E_ARGUMENT_ERROR, "object isn't a string");

  return mrb_fixnum_value(ret);
}

/*Check core_init before initialize, system error*/
  static mrb_value
mrb_s_emv__init(mrb_state *mrb, mrb_value klass)
{
  char attr[40] = {0};
  OsIccInit(0, 0, attr);
  EMVInitTLVData();
  return mrb_true_value();
}

int already_read = 0;

  static mrb_value
mrb_s_emv_app_select(mrb_state *mrb, mrb_value klass)
{
  mrb_int slot, number;

  current_mrb = mrb;
  current_klass = klass;

  mrb_get_args(mrb, "ii", &slot, &number);

  already_read = 0;

  return mrb_fixnum_value(EMVAppSelect(slot, number));
}

  static mrb_value
mrb_s_emv_read_data(mrb_state *mrb, mrb_value klass)
{
  if (already_read == 1)
    return mrb_fixnum_value(EMV_OK);
  else {
    already_read = 1;
    return mrb_fixnum_value(EMVReadAppData());
  }
}

  static mrb_value
mrb_s_emv_get_tlv(mrb_state *mrb, mrb_value klass)
{
  mrb_int tag, len;
  unsigned char data[4096];

  memset(data, 0, sizeof(data));

  mrb_get_args(mrb, "i", &tag);

  if (EMVGetTLVData(tag, (unsigned char *)&data, &len) == EMV_OK)
    return mrb_str_new(mrb, (const char *)&data, len);
  else
    return mrb_str_new(mrb, 0, 0);
}

  static mrb_value
mrb_s_emv_set_tlv(mrb_state *mrb, mrb_value klass)
{
  mrb_int tag, ret;
  mrb_value data;

  mrb_get_args(mrb, "iS", &tag, &data);
  ret = EMVSetTLVData(tag, (unsigned char *)RSTRING_PTR(data), RSTRING_LEN(data));
  return mrb_fixnum_value(ret);
}

  static mrb_value
mrb_s_emv_version(mrb_state *mrb, mrb_value klass)
{
  char paucVer[20];

  memset(&paucVer, 0, sizeof(paucVer));

  if (EMVReadVerInfo((char *)&paucVer) == EMV_OK)
    return mrb_str_new_cstr(mrb, paucVer);
  else
    return mrb_str_new(mrb, 0, 0);
}

  static mrb_value
mrb_s_emv_random(mrb_state *mrb, mrb_value klass)
{
  unsigned char sRandom[4096];
  mrb_int len=0;

  memset(&sRandom, 0, sizeof(sRandom));

  mrb_get_args(mrb, "i", &len);

  OsGetRandom((unsigned char *)&sRandom, len);

  return mrb_str_new(mrb, (const char*)&sRandom, len);
}

  static mrb_value
mrb_s_card_auth(mrb_state *mrb, mrb_value klass)
{
  return mrb_fixnum_value(EMVCardAuth());
}

  static mrb_value
mrb_s_start_transaction(mrb_state *mrb, mrb_value klass)
{
  mrb_int ret;
  unsigned char ACType;
  unsigned long ulCashback=0;
  mrb_value hash, amount, cashback;

  mrb_get_args(mrb, "SS", &amount, &cashback);

  current_mrb = mrb;
  current_klass = klass;

  ulCashback = strtoul(RSTRING_PTR(cashback), NULL, 10);
  if (ulCashback == 0)
    ret = EMVStartTrans(strtoul(RSTRING_PTR(amount), NULL, 10), NULL, &ACType);
  else
    ret = EMVStartTrans(strtoul(RSTRING_PTR(amount), NULL, 10), ulCashback, &ACType);

  hash = mrb_hash_new(mrb);

  if (ret == EMV_OK)
    mrb_hash_set(mrb, hash, mrb_str_new_lit(mrb, "ACTYPE"), mrb_fixnum_value((int)ACType));
  else
    mrb_hash_set(mrb, hash, mrb_str_new_lit(mrb, "ACTYPE"), mrb_fixnum_value(0));

  mrb_hash_set(mrb, hash, mrb_str_new_lit(mrb, "RETURN"), mrb_fixnum_value(ret));

  return hash;
}

  static mrb_value
mrb_s_complete_transaction(mrb_state *mrb, mrb_value klass)
{
  mrb_int code;
  mrb_value hash, scripts;
  int result=0, script_result_len=0, script_len=0;
  unsigned char script_result, ACType;

  mrb_get_args(mrb, "iS", &code, &scripts);

  script_len = RSTRING_LEN(scripts);
  result     = EMVCompleteTrans(code, (unsigned char *)RSTRING_PTR(scripts), &script_len, &ACType);
  hash       = mrb_hash_new(mrb);

  if (result != EMV_OK) {
    if (script_len > 0) {
      result = EMVGetScriptResult(&script_result, &script_result_len);
      mrb_hash_set(mrb, hash, mrb_str_new_lit(mrb, "ADVICE"), mrb_str_new(mrb, (const char *)&script_result, script_result_len));
    } else {
      mrb_hash_set(mrb, hash, mrb_str_new_lit(mrb, "ADVICE"), mrb_str_new(mrb, 0, 0));
    }
  }

  mrb_hash_set(mrb, hash, mrb_str_new_lit(mrb, "ACTYPE"), mrb_fixnum_value((int)ACType));
  mrb_hash_set(mrb, hash, mrb_str_new_lit(mrb, "RETURN"), mrb_fixnum_value(result));
  return hash;
}

static mrb_value
mrb_s_emv_param_flag(mrb_state *mrb, mrb_value klass)
{
  mrb_int param, flag=0;

  mrb_get_args(mrb, "i", &param);
  EMVGetParamFlag((unsigned char)param, &flag);

  mrb_fixnum_value(flag);
}

static mrb_value
mrb_s_emv_set_mck_params(mrb_state *mrb, mrb_value klass)
{
  mrb_value hash;
  mrb_int ret, value;
  EMV_MCKPARAM pMCKParam;

  memset(&pMCKParam,0,sizeof(EMV_MCKPARAM));
  mrb_get_args(mrb, "o", &hash);

  ret = EMVGetMCKParam(&pMCKParam);
  if (ret != EMV_OK) return mrb_fixnum_value(ret);

  value = mrb_fixnum(mrb_hash_get(mrb, hash, mrb_str_new_lit(mrb, "ucBypassPin")));
  pMCKParam.ucBypassPin    = value;

  value = mrb_fixnum(mrb_hash_get(mrb, hash, mrb_str_new_lit(mrb, "ucBatchCapture")));
  pMCKParam.ucBatchCapture = value;

  ret = EMVSetMCKParam(&pMCKParam);
  return mrb_fixnum_value(ret);
}

  void
mrb_emv_init(mrb_state* mrb)
{
  struct RClass *pax;
  struct RClass *emv;

  pax = mrb_class_get(mrb, "PAX");
  emv = mrb_define_class_under(mrb, pax, "EMV",  mrb->object_class);

  mrb_define_class_method(mrb , emv , "core_init"            , mrb_s_core_init            , MRB_ARGS_NONE());
  mrb_define_class_method(mrb , emv , "get_parameter"        , mrb_s_get_emv_parameter    , MRB_ARGS_NONE());
  mrb_define_class_method(mrb , emv , "set_parameter"        , mrb_s_set_emv_parameter    , MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb , emv , "get_app"              , mrb_s_get_emv_app          , MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb , emv , "add_app"              , mrb_s_add_emv_app          , MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb , emv , "del_app"              , mrb_s_del_emv_app          , MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb , emv , "del_apps"             , mrb_s_del_emv_apps         , MRB_ARGS_NONE());
  mrb_define_class_method(mrb , emv , "get_pki"              , mrb_s_get_emv_pki          , MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb , emv , "add_pki"              , mrb_s_add_emv_pki          , MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb , emv , "del_pki"              , mrb_s_del_emv_pki          , MRB_ARGS_REQ(2));
  mrb_define_class_method(mrb , emv , "del_pkis"             , mrb_emv_s_del_pkis         , MRB_ARGS_NONE());
  mrb_define_class_method(mrb , emv , "check_pki"            , mrb_s_check_emv_pki        , MRB_ARGS_REQ(2));
  mrb_define_class_method(mrb , emv , "_init"                , mrb_s_emv__init            , MRB_ARGS_NONE());
  mrb_define_class_method(mrb , emv , "app_select"           , mrb_s_emv_app_select       , MRB_ARGS_REQ(2));
  mrb_define_class_method(mrb , emv , "read_data"            , mrb_s_emv_read_data        , MRB_ARGS_NONE());
  mrb_define_class_method(mrb , emv , "get_tlv"              , mrb_s_emv_get_tlv          , MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb , emv , "set_tlv"              , mrb_s_emv_set_tlv          , MRB_ARGS_REQ(2));
  mrb_define_class_method(mrb , emv , "card_auth"            , mrb_s_card_auth            , MRB_ARGS_NONE());
  mrb_define_class_method(mrb , emv , "start_transaction"    , mrb_s_start_transaction    , MRB_ARGS_REQ(2));
  mrb_define_class_method(mrb , emv , "complete_transaction" , mrb_s_complete_transaction , MRB_ARGS_REQ(2));
  mrb_define_class_method(mrb , emv , "random"               , mrb_s_emv_random           , MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb , emv , "version"              , mrb_s_emv_version          , MRB_ARGS_NONE());
  mrb_define_class_method(mrb , emv , "param_flag"           , mrb_s_emv_param_flag       , MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb , emv , "set_mck_params"       , mrb_s_emv_set_mck_params   , MRB_ARGS_REQ(1));
}
