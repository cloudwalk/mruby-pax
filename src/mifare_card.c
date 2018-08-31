#include <stdlib.h>
#include <stdio.h>
#include "mruby.h"
#include "mruby/compile.h"
#include "mruby/value.h"
#include "mruby/array.h"
#include "mruby/string.h"
#include "mruby/hash.h"
#include "mruby/data.h"
#include "mruby/class.h"
#include "mruby/variable.h"

#include "osal.h"
#include "mruby/ext/context_log.h"

/*
 * = How to debug
 *   This function call be called to store any string at the file "data/app/MAINAPP/main/<year>-<month>-<day>.log"
 *   that can be access via Telnet "telnet 127.0.0.1 2323", remember to start the telnet server at TermAssist
 *
 * ContextLog(mrb, 0, "2TLV [%d][%s][%d]", iTag, psDat, iDataLen);
 */

/**
 * card info
 */
typedef struct {
  int status;
  unsigned char uid[16];
  int type;
  int size;
} card_t;

/**
 * current selected card info
 */
static card_t selected;


int DetectCards(mrb_state *mrb, int timeout)
{
  int ret = 0;
  char type;
  unsigned char atqx[16];
  ST_TIMER exit;

  // always open logic
  int open = OsPiccOpen();

  //ContextLog(mrb, 0, "OsPiccOpen = %d", open);

  // start timer
  OsTimerSet(&exit, timeout);

  // clear selected state
  selected.status = -1;
  selected.type = -1;
  selected.size = -1;
  memset(selected.uid, 0, sizeof(selected.uid));


  while (TRUE) {
    memset(atqx, 0, sizeof(atqx));
    int current = OsPiccPoll(&type, atqx);

    //if (current == RET_OK)
    //{
      //ContextLog(mrb, 0, "OsPiccPoll(%c, %02x%02x) = %d", type, atqx[0], atqx[1], current);
    //}
    //else
    //{
      //ContextLog(mrb, 0, "OsPiccPoll = %d", current);
    //}

    // exit by unresolvable collision
    if (current == PCD_ERR_COLL_FLAG)
    {
      // multiple cards detected
      ret = -3;
      break;
    }

    // exit by non-mifare card
    if (current == RET_OK && type != 'A') {
      // card is not mifare
      ret = -2;
      break;
    }

    if (current == RET_OK) {
      // derive size from ATQX
      if ((atqx[0] & 0xE0) == 0) selected.size = 4;
      if ((atqx[0] & 0x40) != 0) selected.size = 7;

      break;
    }

    // exit by timeout
    if (OsTimerCheck(&exit) == 0) {
      // Timeout Error
      ret = -1;
      break;
    }

    OsSleep(10);
  }

  return ret;
}

int ActivateCard(mrb_state *mrb)
{
  unsigned char sak[2];
  int ret = 0;

  memset(sak, 0, sizeof(sak));
  ret = OsPiccAntiSel('A', selected.uid, 0x00, sak);

  //ContextLog(mrb, 0, "OsPiccAntiSel('A', %p, 0x00, 0x%02x) = %d", selected.uid, *sak, ret);

  if (ret == RET_OK) {
    // if card needs activation
    if(*sak == 0x20)
    {
      // card is not mifare compatible
      return -2;
    } else {
      // status is OK
      selected.status = 0;
      // type is ULTRALIGHT or CLASSIC
      selected.type = *sak == 0x00 ? 1 : 0;

      return 0;
    }
  } else {
    // status is NOK
    selected.status = -1;

    switch (ret) {
      case PCD_ERR_WTO_FLAG:
        return -1;
      case PCD_ERR_COLL_FLAG:
        return -3;
      default:
        selected.status = -1;
        return -4;
    }
  }
}

int AuthenticateSector(mrb_state *mrb, int keyType, unsigned char key[], int sector)
{
  int ret = 0;
  unsigned char group = keyType == 0 ? 'A' : 'B';
  // transforms sector into *last* block of the sector (of 4 blocks)
  int authBlock = (sector * 4) + 3;

  ret = OsMifareAuthority(
      selected.uid,
      authBlock,
      group,
      key);

  //ContextLog(mrb, 0, "OsMifareAuthority(%p, %d, %c, %p) = %d", selected.uid, authBlock, group, key, ret);

  if (ret == RET_OK) {
    return 0;
  } else {
    // Fail to Auth is Timeout...
    return -1;
  }
}

int ReadBlock(mrb_state *mrb, unsigned char sector, unsigned char block, unsigned char *data)
{
  int target = (sector * 4) + block;
  int ret = OsMifareOperate('R',
      target,
      data,
      0);

  //ContextLog(mrb, 0, "OsMifareOperate('R', %d, %p, 0) = %d", target, data, ret);

  switch (ret) {
    case RET_OK:
      return 0;
    case PCD_ERR_WTO_FLAG:
      return -1;
    case PCD_ERR_AUT_FLAG:
      return -2;
    default:
      return -4;
  }
}

int WriteBlock(mrb_state *mrb, unsigned char sector, unsigned char block, unsigned char *data)
{
  int target = (sector * 4) + block;
  int ret = OsMifareOperate('W',
      target,
      data,
      0);

  //ContextLog(mrb, 0, "OsMifareOperate('W', %d, %p, 0) = %d", target, data, ret);

  switch (ret) {
    case RET_OK:
      return 0;
    case PCD_ERR_WTO_FLAG:
      return -1;
    case PCD_ERR_AUT_FLAG:
      return -2;
    default:
      return -4;
  }
}

int IncrementValue(mrb_state *mrb, unsigned char sector, unsigned char block, unsigned char *value)
{
  int target = (sector * 4) + block;
  int ret = OsMifareOperate('+',
      target,
      value,
      target);

  //ContextLog(mrb, 0, "OsMifareOperate('+', %d, %p, %d) = %d", target, value, target, ret);

  switch (ret) {
    case RET_OK:
      return 0;
    case PCD_ERR_WTO_FLAG:
      return -1;
    case PCD_ERR_AUT_FLAG:
      return -2;
    default:
      return -4;
  }
}

int DecrementValue(mrb_state *mrb, unsigned char sector, unsigned char block, unsigned char *value)
{
  int target = (sector * 4) + block;
  int ret = OsMifareOperate('-',
      target,
      value,
      target);

  //ContextLog(mrb, 0, "OsMifareOperate('-', %d, %p, %d) = %d", target, value, target, ret);

  switch (ret) {
    case RET_OK:
      return 0;
    case PCD_ERR_WTO_FLAG:
      return -1;
    case PCD_ERR_AUT_FLAG:
      return -2;
    default:
      return -4;
  }
}

int RestoreBlock(mrb_state *mrb, unsigned char sector, unsigned char sourceBlock, unsigned char destBlock)
{
  int targetSource = (sector * 4) + sourceBlock;
  int targetDest = (sector * 4) + destBlock;
  unsigned char data[16] = {0};

  int ret = OsMifareOperate('>',
      targetSource,
      data,
      targetDest);

  //ContextLog(mrb, 0, "OsMifareOperate('>', %d, %p, %d) = %d", targetSource, data, targetDest, ret);

  switch (ret) {
    case RET_OK:
      return 0;
    case PCD_ERR_WTO_FLAG:
      return -1;
    case PCD_ERR_AUT_FLAG:
      return -2;
    default:
      return -4;
  }
}

  static mrb_value
mrb_mifare_card_detect(mrb_state *mrb, mrb_value self)
{
  // input
  mrb_int timeout = 0;
  // output
  mrb_int ret = RET_OK;

  mrb_get_args(mrb, "i", &timeout);

  ret = DetectCards(mrb, timeout);

  return mrb_fixnum_value(ret);
}

  static mrb_value
mrb_mifare_card_activate(mrb_state *mrb, mrb_value self)
{
  // output
  mrb_int ret = RET_OK;

  ret = ActivateCard(mrb);

  return mrb_fixnum_value(ret);
}

  static mrb_value
mrb_mifare_card_uid(mrb_state *mrb, mrb_value self)
{
  // output
  mrb_int ret = RET_OK;

  mrb_value array;

  // transfer status from current selected card to ret
  ret = selected.status;

  array = mrb_ary_new(mrb);
  mrb_ary_push(mrb, array, mrb_fixnum_value(ret));

  if (ret == RET_OK) {
    mrb_ary_push(mrb, array, mrb_str_new(mrb, (const char *) selected.uid, selected.size));
    mrb_ary_push(mrb, array, mrb_fixnum_value(selected.type));
  }

  return array;
}

  static mrb_value
mrb_mifare_card_auth_sector(mrb_state *mrb, mrb_value self)
{
  // input
  mrb_int keyType;
  mrb_value key;
  mrb_int sector;
  // output
  mrb_int ret = RET_OK;

  mrb_get_args(mrb, "iSi", &keyType, &key, &sector);

  ret = AuthenticateSector(mrb, keyType, (unsigned char *)RSTRING_PTR(key), sector);

  return mrb_fixnum_value(ret);
}

  static mrb_value
mrb_mifare_card_read_block(mrb_state *mrb, mrb_value self)
{
  // input
  mrb_int sector;
  mrb_int block;
  // output
  mrb_int ret = RET_OK;
  unsigned char data[16];

  mrb_value array;

  mrb_get_args(mrb, "ii", &sector, &block);

  ret = ReadBlock(mrb, sector, block, data);

  array = mrb_ary_new(mrb);
  mrb_ary_push(mrb, array, mrb_fixnum_value(ret));

  if (ret == RET_OK) {
    mrb_ary_push(mrb, array, mrb_str_new(mrb, (char *)data, 16));
  }

  return array;
}

  static mrb_value
mrb_mifare_card_write_block(mrb_state *mrb, mrb_value self)
{
  // input
  mrb_int sector;
  mrb_int block;
  mrb_value in;
  // output
  mrb_int ret = RET_OK;

  mrb_get_args(mrb, "iiS", &sector, &block, &in);

  // ensure 16 bytes data input
  if (RSTRING_LEN(in) == 16) {
    ret = WriteBlock(mrb, sector, block, (unsigned char *)RSTRING_PTR(in));
  } else {
    ret = -4;
  }

  return mrb_fixnum_value(ret);
}

  static mrb_value
mrb_mifare_card_increment_value(mrb_state *mrb, mrb_value self)
{
  // input
  mrb_int sector;
  mrb_int block;
  mrb_value in;
  // output
  mrb_int ret = RET_OK;

  mrb_get_args(mrb, "iiS", &sector, &block, &in);

  // ensure 4 bytes data input
  if (RSTRING_LEN(in) == 4) {
    ret = IncrementValue(mrb, sector, block, (unsigned char *)RSTRING_PTR(in));
  } else {
    ret = -4;
  }

  return mrb_fixnum_value(ret);
}

  static mrb_value
mrb_mifare_card_decrement_value(mrb_state *mrb, mrb_value self)
{
  // input
  mrb_int sector;
  mrb_int block;
  mrb_value in;
  // output
  mrb_int ret = RET_OK;

  mrb_get_args(mrb, "iiS", &sector, &block, &in);

  // ensure 4 bytes data input
  if (RSTRING_LEN(in) == 4) {
    ret = DecrementValue(mrb, sector, block, (unsigned char *)RSTRING_PTR(in));
  } else {
    ret = -4;
  }

  return mrb_fixnum_value(ret);
}

  static mrb_value
mrb_mifare_card_restore_block(mrb_state *mrb, mrb_value self)
{
  // input
  mrb_int sector;
  mrb_int source;
  mrb_int dest;
  // output
  mrb_int ret = RET_OK;

  mrb_get_args(mrb, "iii", &sector, &source, &dest);

  ret = RestoreBlock(mrb, sector, source, dest);

  return mrb_fixnum_value(ret);
}

  void
mrb_mifare_card_init(mrb_state* mrb)
{
  struct RClass *pax, *mifare_card;

  pax         = mrb_class_get(mrb, "PAX");
  mifare_card = mrb_define_class_under(mrb, pax, "MifareCard", mrb->object_class);

  mrb_define_class_method(mrb , mifare_card , "detect"          , mrb_mifare_card_detect          , MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb , mifare_card , "activate"        , mrb_mifare_card_activate        , MRB_ARGS_REQ(0));
  mrb_define_class_method(mrb , mifare_card , "uid"             , mrb_mifare_card_uid             , MRB_ARGS_REQ(0));
  mrb_define_class_method(mrb , mifare_card , "auth_sector"     , mrb_mifare_card_auth_sector     , MRB_ARGS_REQ(3));
  mrb_define_class_method(mrb , mifare_card , "read_block"      , mrb_mifare_card_read_block      , MRB_ARGS_REQ(2));
  mrb_define_class_method(mrb , mifare_card , "write_block"     , mrb_mifare_card_write_block     , MRB_ARGS_REQ(3));
  mrb_define_class_method(mrb , mifare_card , "increment_value" , mrb_mifare_card_increment_value , MRB_ARGS_REQ(3));
  mrb_define_class_method(mrb , mifare_card , "decrement_value" , mrb_mifare_card_decrement_value , MRB_ARGS_REQ(3));
  mrb_define_class_method(mrb , mifare_card , "restore_block"   , mrb_mifare_card_restore_block   , MRB_ARGS_REQ(3));
}

