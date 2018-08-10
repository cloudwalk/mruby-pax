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


int PowerOn(int slot, int *historical_size, char *historical)
{
	unsigned char atr[35];
	int size = 0;
	int ret = OsIccOpen(slot);
	
	if (ret == ERR_DEV_NOT_EXIST) {
		// invalid SAM
		return -2;
	}
	
	ret = OsIccDetect(slot);
	
	if (ret != RET_OK) {
		// card not present
		return -7;
	}
	
	ret = OsIccInit(slot, 0x20, atr);
	
	if (ret == RET_OK) {
		size = (int) atr[0];
		
		*historical_size = size;
		// skip first ATR byte because it is its size
		memcpy(historical, (atr + 1), size);
		
		return 0;
	} else {
		// TODO: check for VCC (-4) and VPP (-5) errors
		// TODO: commucation error (-6)
		// card mute
		return -3;
	}
}

int PowerDown(int slot) 
{
	// first, check if card is inserted
	// it will also power down card
	int ret = OsIccDetect(slot);
	
	if (ret == RET_OK) {
		OsIccClose(slot);
		return 0;
	} else {
		// card already removed
		return 1;
	}
}

int SendAPDU(int slot, char *in, int sizeIn, char *out, int *sizeOut)
{
	ST_APDU_REQ req;
	ST_APDU_RSP rsp;
	int ret;
	int size;
	
	memset(&req, 0, sizeof(req));
	memcpy(&req, in, sizeIn);
	
	ret = OsIccExchange(slot, 0x01, &req, &rsp);
	
	if (ret == RET_OK) {
		size = rsp.LenOut;
		// outSize is DataOut size + 1 for SW1 + 1 for SW2
		*sizeOut = size + 2;
		
		// copy DataOut to out
		memcpy(out, rsp.DataOut, size);
		// prepend SW1SW2 to out
		out[size + 0] = rsp.SWA;
		out[size + 1] = rsp.SWB;
		
		return 0;
	} else {
		//TODO:
		//•	-1: Sem resposta
		//•	-2: SamCard Invalido
		//•	-3: SamCard mudo
		//•	-4: Problema VCC
		//•	-5: Problema VPP
		//•	-6: Erro de comunicação
		//•	-7: SamCard removido
		return -1;
	}
}

static mrb_value
mrb_sam_card_power(mrb_state *mrb, mrb_value self)
{
  // input
  mrb_int status;
  mrb_int slot;
  // output
  mrb_int ret; 
  mrb_int historical_size;
  char historical[35];
  
  mrb_value array;

  // cleaning buffer
  memset(historical, 0, sizeof(historical));
  
  mrb_get_args(mrb, "ii", &slot, &status);

  if (status == 1) {
	// Turn on
    ret = PowerOn(slot, &historical_size, historical);
  } else { 
    // Turn off
	ret = PowerDown(slot);
  }

  array = mrb_ary_new(mrb);
  mrb_ary_push(mrb, array, mrb_fixnum_value(ret));

  if (ret == RET_OK) {
	mrb_ary_push(mrb, array, mrb_str_new(mrb, historical, historical_size));
  }
  
  return array;
}

static mrb_value
mrb_sam_card_send(mrb_state *mrb, mrb_value self)
{
  // input
  mrb_int slot;
  mrb_int in_size;
  mrb_value in;
  // output
  mrb_int ret; 
  mrb_int out_size;
  char out[35];
  
  mrb_value array;

  mrb_get_args(mrb, "iS", &slot, &in);

  in_size = RSTRING_LEN(in);

  ret = SendAPDU(slot, (char *)RSTRING_PTR(in), in_size, out, &out_size);
   
  array = mrb_ary_new(mrb);
  mrb_ary_push(mrb, array, mrb_fixnum_value(ret));

  if (ret == RET_OK) {
	mrb_ary_push(mrb, array, mrb_str_new(mrb, out, out_size));
  }
  
  return array;
}

void
mrb_touch_init(mrb_state* mrb)
{
  struct RClass *pax, *sam_card;

  pax      = mrb_class_get(mrb, "PAX");
  sam_card = mrb_define_class_under(mrb, pax, "SamCard", mrb->object_class);

  mrb_define_class_method(mrb , sam_card , "power" , mrb_sam_card_power , MRB_ARGS_REQ(2));
  mrb_define_class_method(mrb , sam_card , "send"  , mrb_sam_card_send  , MRB_ARGS_REQ(2));
}

