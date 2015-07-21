#include <stdlib.h>
#include <stdio.h>
#include "mruby.h"
#include "mruby/compile.h"
#include "mruby/value.h"
#include "mruby/array.h"
#include "mruby/string.h"
#include "mruby/hash.h"

#include "osal.h"

#define PED_TLK 0x01
#define PED_TMK 0x02
#define PED_TPK 0x03
#define PED_TAK 0x04
#define PED_TDK 0x05
#define PED_TIK 0x10

#define DUKPT_PIN_KEY       0X00
#define DUKPT_MAC_BOTH_KEY  0X01
#define DUKPT_MAC_RSP_KEY   0X02
#define DUKPT_DES_KEY       0X03
#define DUKPT_FUTURE_KEY    0XFF

#define DUKPT_PIN_VAR       "\x00\x00\x00\x00\x00\x00\x00\xff\x00\x00\x00\x00\x00\x00\x00\xff\x00\x00\x00\x00\x00\x00\x00\xff"
#define DUKPT_MAC_BOTH_VAR  "\x00\x00\x00\x00\x00\x00\xff\x00\x00\x00\x00\x00\x00\x00\xff\x00\x00\x00\x00\x00\x00\x00\xff\x00"
#define DUKPT_MAC_RSP_VAR   "\x00\x00\x00\x00\xff\x00\x00\x00\x00\x00\x00\x00\xff\x00\x00\x00\x00\x00\x00\x00\xff\x00\x00\x00"
#define DUKPT_DES_KEY_VAR   "\x00\x00\x00\x00\x00\xff\x00\x00\x00\x00\x00\x00\x00\xff\x00\x00\x00\x00\x00\x00\x00\xff\x00\x00"

void make_odd(unsigned char *key, unsigned char ucKeyLen)
{
    unsigned char tmpc,i,j,weight;

    for(i=0;i<ucKeyLen;i++)
    {
        for(weight=j=0;j<8;j++)
        {
            tmpc=key[i]<<(j%8);
            if(tmpc&0x80) weight++; //¼ÆËãkey[i]Õâ¸ö×Ö½ÚÖÐ'1'µÄ¸öÊý
        }
        if(!(weight%2)) //Èô"1"¸öÊýÎªÅ¼ÊýÔòÊ¹key[i]ÖÐ'1'¸öÊýÎªÆæÊý
        {
            if(key[i]&0x01) key[i]&=0xfe;
            else key[i]|=0x01;
        }
    }
}

int  Tdes(unsigned char * pszIn,unsigned char * pszOut,int iDataLen,
                  unsigned char * pszKey,int iKeyLen,unsigned char ucMode)
{
    //int iTmp = iDataLen%8;
    int iLoop;

    if(24==iKeyLen)
    {
       if(ucMode==0x01) //Tdes¼ÓÃÜ
       {
           for(iLoop=0;iLoop<iDataLen/8;iLoop++)
           {
              OsDES(pszIn+iLoop*8,pszOut+iLoop*8,pszKey,8,0x01); //¼ÓÃÜ
              OsDES(pszOut+iLoop*8,pszOut+iLoop*8,pszKey+8,8,0x00); //½âÃÜ
              OsDES(pszOut+iLoop*8,pszOut+iLoop*8,pszKey+16,8,0x01); //¼ÓÃÜ
           }
       }
       else if(ucMode==0x00) //Tdes½âÃÜ
       {
           for(iLoop=0;iLoop<iDataLen/8;iLoop++)
           {
              OsDES(pszIn+iLoop*8,pszOut+iLoop*8,pszKey+16,8,0x00); //½âÃÜ
              OsDES(pszOut+iLoop*8,pszOut+iLoop*8,pszKey+8,8,0x01); //¼ÓÃÜ
              OsDES(pszOut+iLoop*8,pszOut+iLoop*8,pszKey,8,0x00); //½âÃÜ
           }

       }
    }
    else if (16==iKeyLen)
    {
       if(ucMode==0x01)
       {
           for(iLoop=0;iLoop<iDataLen/8;iLoop++)
           {
              OsDES(pszIn+iLoop*8,pszOut+iLoop*8,pszKey,8,0x01); //ÃÜÔ¿Ç°8×Ö½Ú¼ÓÃÜ
              OsDES(pszOut+iLoop*8,pszOut+iLoop*8,pszKey+8,8,0x00); //ÃÜÔ¿ºó8×Ö½Ú½âÃÜ
              OsDES(pszOut+iLoop*8,pszOut+iLoop*8,pszKey,8,0x01); //ÃÜÔ¿Ç°8×Ö½Ú¼ÓÃÜ
           }
       }
       else if(ucMode==0x00)
       {
           for(iLoop=0;iLoop<iDataLen/8;iLoop++)
           {
              OsDES(pszIn+iLoop*8,pszOut+iLoop*8,pszKey,8,0x00);
              OsDES(pszOut+iLoop*8,pszOut+iLoop*8,pszKey+8,8,0x01);
              OsDES(pszOut+iLoop*8,pszOut+iLoop*8,pszKey,8,0x00);
           }

       }
       else return -2;


    }
    else  if(8==iKeyLen)
    {

       if(ucMode==0x01)
       {
           for(iLoop=0;iLoop<iDataLen/8;iLoop++)
           {
              OsDES(pszIn+iLoop*8,pszOut+iLoop*8,pszKey,8,0x01);
           }
       }
       else if(ucMode==0x00)
       {
           for(iLoop=0;iLoop<iDataLen/8;iLoop++)
           {
              OsDES(pszIn+iLoop*8,pszOut+iLoop*8,pszKey,8,0x00);
           }
       }
       else return -2;
    }
    else  return -3;

    //¶Ô8È¡Óà²»Îª0µÄÊä³öÊý¾Ý½øÐÐÏàÓ¦ÔËËã
    for(iLoop=0;iLoop<iDataLen%8;iLoop++)
    {
       pszOut[(iDataLen/8)*8+iLoop]=pszIn[(iDataLen/8)*8+iLoop]^0xff;
    }
    return 0;
}


void MAC_Arithmetic1(unsigned char *DataIn, ushort inlen, unsigned char *MacKey, unsigned char MacKeyLen, unsigned char *VerifyOut)
{
    int i,j;
    unsigned char tempdata[65535];

    memset(tempdata,0x00,sizeof(tempdata));
    memset(VerifyOut,0x00,8);
    memcpy(tempdata, DataIn, inlen);
    if(inlen%8) inlen=inlen+8;
    for(i=0; i<inlen/8; i++)
    {
        for(j=0; j<8; j++)
        {
            VerifyOut[j]^=tempdata[i*8+j];
        }
        Tdes(VerifyOut,VerifyOut,8,MacKey,MacKeyLen,1);
    }
}

void dat_bcdtoasc(unsigned char *psOut, unsigned char *psIn, uint uiLength)
{
    static const unsigned char ucHexToChar[16] = {"0123456789ABCDEF"};
    uint uiCnt;

    if ((psIn == NULL) || (psOut == NULL))
    {
        return;
    }

    for(uiCnt = 0; uiCnt < uiLength; uiCnt++)
    {
        psOut[2 * uiCnt]   = ucHexToChar[(psIn[uiCnt] >> 4)];
        psOut[2 * uiCnt + 1] = ucHexToChar[(psIn[uiCnt] & 0x0F)];
    }
    psOut[2 * uiLength] = 0;
}

void DatStrXor(unsigned char *pucIn1, const char *pucIn2,int iLen)
{
    int iLoop;
    unsigned char pucOut[255]={0};

    for(iLoop=0;iLoop<iLen;iLoop++)
    {
        pucOut[iLoop]=pucIn1[iLoop]^pucIn2[iLoop];
    }
    memcpy(pucIn1,pucOut,iLen);
    pucIn1[iLen]=0;
}

static void  StrAND21Bits(unsigned char *pucIn1,unsigned char *pucIn2,unsigned char *pucOut)
{
    int iLoop;
    unsigned char ucTmp=pucOut[0];
    for(iLoop=0;iLoop<3;iLoop++)
    {
        pucOut[iLoop]=pucIn1[iLoop]&pucIn2[iLoop];
    }

    ucTmp&=0xE0;

    pucOut[0]|=ucTmp;

    return;
}

static void  StrOR21Bits(unsigned char *pucIn1,unsigned char *pucIn2,unsigned char *pucOut)
{
    int iLoop;
    unsigned char ucTmp=pucOut[0];
    for(iLoop=0;iLoop<3;iLoop++)
    {
        pucOut[iLoop]=pucIn1[iLoop]|pucIn2[iLoop];
    }

    ucTmp&=0xE0;

    pucOut[0]|=ucTmp;

    return;
}

static void StrXor(unsigned char *pucIn1, const char *pucIn2,int iLen,unsigned char *pucOut)
{
    int iLoop;
    for(iLoop=0;iLoop<iLen;iLoop++)
    {
        pucOut[iLoop]=pucIn1[iLoop]^pucIn2[iLoop];
    }
}

int PedGetDukptWorkKey(unsigned char *pccTIK, int iTikLen, unsigned char *pucCurKsn, unsigned char Mode,
    unsigned char *pucOutKey)
{
    unsigned char aucR8[8], aucR8A[8], aucR8B[8], aucR3[3], aucSR[3];
    unsigned char aucCurKey[24];
    unsigned int uiTmp;
    unsigned char aucTmp[128];
    int i;

    memcpy(aucCurKey, pccTIK, iTikLen);

    memcpy(aucR8, pucCurKsn + 2, 8);

    memset(aucR8 + 6, 0x00, 2);
    aucR8[5] &= 0xE0;

    memcpy(aucR3, pucCurKsn + 2 + 5, 3);
    aucR3[0] &= 0x1f;

    memset(aucSR, 0x00, 3);
    aucSR[0] = 0x10;

 TAG1:
    aucTmp[0] = 0;
    StrAND21Bits(aucSR, aucR3, aucTmp);

    if (memcmp(aucTmp, "\x00\x00\x00", 3) == 0)
        goto TAG2;

    StrOR21Bits(aucSR, aucR8 + 5, aucR8 + 5);

    if (iTikLen == 16) {
        StrXor(aucCurKey + 8, (const char *)aucR8, 8, aucR8A);
        OsDES(aucR8A, aucR8A, aucCurKey, 8, 1);

        StrXor(aucR8A, (const char *)aucCurKey + 8, 8, aucR8A);

        StrXor(aucCurKey, "\xc0\xc0\xc0\xc0\x00\x00\x00\x00\xc0\xc0\xc0\xc0\x00\x00\x00\x00",
               iTikLen, aucCurKey);

        StrXor(aucCurKey + 8, (const char *)aucR8, 8, aucR8B);

        OsDES(aucR8B, aucR8B, aucCurKey, 8, 1);
        StrXor(aucR8B, (const char *)aucCurKey + 8, 8, aucR8B);

        memcpy(aucCurKey + 8, aucR8A, 8);
        memcpy(aucCurKey, aucR8B, 8);
    }
    else if (iTikLen == 8) {
        StrXor(aucCurKey, (const char *)aucR8, 8, aucR8A);
        OsDES(aucR8A, aucR8A, aucCurKey, 8, 1);
        StrXor(aucR8A, (const char *)aucCurKey, 8, aucCurKey);
    }

 TAG2:

    uiTmp = (aucSR[0] << 16) + (aucSR[1] << 8) + (aucSR[2] << 0);
    uiTmp >>= 1;
    aucSR[0] = (uiTmp >> 16) & 0x1f;
    aucSR[1] = uiTmp >> 8;
    aucSR[2] = uiTmp;

    if (memcmp(aucSR, "\x00\x00\x00", 3) != 0)
        goto TAG1;
#if 0
    printf("iPedGetDukptWorkKey[aucCurKey]: \n");
        for (i = 0; i < iTikLen; i++)
                printf("%x ", aucCurKey[i]);
        printf("\n");
#endif
    if (Mode == DUKPT_PIN_KEY)
        StrXor(aucCurKey, DUKPT_PIN_VAR, iTikLen, pucOutKey);
    else if (Mode == DUKPT_MAC_BOTH_KEY)
        StrXor(aucCurKey, DUKPT_MAC_BOTH_VAR, iTikLen, pucOutKey);
    else if (Mode == DUKPT_MAC_RSP_KEY)
        StrXor(aucCurKey, DUKPT_MAC_RSP_VAR, iTikLen, pucOutKey);
    else if (Mode == DUKPT_DES_KEY) {
        StrXor(aucCurKey, DUKPT_DES_KEY_VAR, iTikLen, aucTmp);

        for (i = 0; i < iTikLen / 8; i++)
            OsDES(aucTmp + i * 8, pucOutKey + i * 8, aucTmp, iTikLen, 1);
    }
    else if (Mode == DUKPT_FUTURE_KEY)
        memcpy(pucOutKey, aucCurKey, iTikLen);
    else
        return -1;

/*exit:*/
    return 0;
}

int s_Dukpt_SpecialDecrypt(unsigned char* pucInOut,unsigned char* pucKey)
{
    unsigned char i;
    for(i=0;i<8;i++) pucInOut[i]=pucInOut[i]^pucKey[i];
    Tdes(pucInOut,pucInOut,8,pucKey,8,PED_DECRYPT);
    for(i=0;i<8;i++) pucInOut[i]=pucInOut[i]^pucKey[i];
    return 0;
}

static mrb_value
mrb_s_crypto_delete_all_keys(mrb_state *mrb, mrb_value klass)
{
  return mrb_fixnum_value(OsPedEraseKeys());
}

static mrb_value
mrb_s_crypto_load_ipek(mrb_state *mrb, mrb_value klass)
{
  char *key;
  char *ksn;
  char kcvData[8];
  char dataIn[184];
  mrb_int key_index, key_type, key_length, ksn_length, ret;

  memset(dataIn, 0, sizeof(dataIn));
  memset(kcvData, 0x00, sizeof(kcvData));

  mrb_get_args(mrb, "iiss", &key_index, &key_type, &key, &key_length, &ksn, &ksn_length);

  dataIn[0] = 0x03;                               // format
  dataIn[2] = 0;                                  // source key index, 0 for plaintext
  dataIn[3] = key_index;                          // dest key index
  dataIn[11] = PED_TIK;                           // dest key type
  dataIn[12] = key_length;                        // dest key size
  memcpy(dataIn+13, key, key_length);             // key
  dataIn[13+24] = 0;                              // kcv mode
  memcpy(dataIn+13+24+1+128, kcvData, 8);         // kcv result
  memcpy(dataIn+13+24+1+128+8, ksn, ksn_length);  // ksn

  ret = OsPedWriteTIK(dataIn);

  return mrb_fixnum_value(ret);
}

static mrb_value
mrb_s_crypto_get_pin_dukpt(mrb_state *mrb, mrb_value klass)
{
  char *pan;
  char ksn[16];
  char dataIn[16];
  char dataOut[64];
  char maxlen[] = "0,1,2,3,4,5,6,7,8,9,10,11,12";
  mrb_int key_index, pan_length, ret;
  mrb_value hash;

  memset(ksn, 0, sizeof(ksn));
  memset(dataIn, 0, sizeof(dataIn));
  memset(dataOut, 0, sizeof(dataOut));

  mrb_get_args(mrb, "is", &key_index, &pan, &pan_length);

  ret = OsPedGetPinDukpt(key_index, pan, maxlen, 0x20, 30000, &ksn, &dataOut);

  if (ret == 0)
  {
    OsPedIncreaseKsnDukpt(key_index);

    hash = mrb_funcall(mrb, klass, "dukpt_default", 0);
    mrb_hash_set(mrb, hash, mrb_str_new_cstr(mrb, "KSN"), mrb_str_new(mrb, ksn, 10));
    mrb_hash_set(mrb, hash, mrb_str_new_cstr(mrb, "PINBLOCK"), mrb_str_new(mrb, dataOut, 8));

    return hash;
  } else {
    return mrb_fixnum_value(ret);
  }
}

void
mrb_crypto_init(mrb_state* mrb)
{
  struct RClass *pax;
  struct RClass *crypto;

  pax      = mrb_class_get(mrb, "PAX");
  crypto   = mrb_define_class_under(mrb, pax, "Crypto", mrb->object_class);

  mrb_define_class_method(mrb, crypto , "delete_all_keys", mrb_s_crypto_delete_all_keys, MRB_ARGS_NONE());
  mrb_define_class_method(mrb, crypto , "load_ipek", mrb_s_crypto_load_ipek, MRB_ARGS_REQ(3));
  mrb_define_class_method(mrb, crypto , "get_pin_dukpt", mrb_s_crypto_get_pin_dukpt, MRB_ARGS_REQ(4));
}
