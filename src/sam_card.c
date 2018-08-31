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

int ToSDK(int slot)
{
	switch(slot) {
		case 1:
			return ICC_SAM1_SLOT;
		case 2:
			return ICC_SAM2_SLOT;
		case 3:
			return ICC_SAM3_SLOT;
		case 4:
			return ICC_SAM4_SLOT;
		default:
			return 1;
	}
}
 
int PowerOn(mrb_state *mrb, int slot, int *historical_size, char *historical)
{
	unsigned char atr[35];
	int size = 0;
	int ret = 0;
	
	// Adjust slot according to SDK
	slot = ToSDK(slot);

	ret = OsIccOpen(slot);
	ContextLog(mrb, 0, "OsIccOpen(%d) = %d", slot, ret);
	
	if (ret == ERR_DEV_NOT_EXIST) {
		// invalid SAM
		return -2;
	}
	
	ret = OsIccDetect(slot);
	ContextLog(mrb, 0, "OsIccDetect(%d) = %d", slot, ret);
	
	if (ret != RET_OK) {
		// card not present
		return -7;
	}
	
	ret = OsIccInit(slot, 0x20, atr);
	ContextLog(mrb, 0, "OsIccInit(%d, 0x20, %p) = %d", slot, atr, ret);
	
	if (ret == RET_OK) {
		size = (int) atr[0];
		
		*historical_size = size;
		// skip first ATR byte because it is its size
		memcpy(historical, (atr + 1), size);
		
		return 0;
	} else {
		// TODO: check for VCC (-4) and VPP (-5) errors
		
		switch (ret) {
			case ERR_SCI_HW_STEP:	
				return -3; // SamCard mudo
			case ERR_SCI_HW_PARITY:		
			case ERR_SCI_HW_TCK:			
			case ERR_SCI_ATR_TS:		
			case ERR_SCI_ATR_TA1:			
			case ERR_SCI_ATR_TD1:			
			case ERR_SCI_ATR_TA2:			
			case ERR_SCI_ATR_TB1:			
			case ERR_SCI_ATR_TB2:			
			case ERR_SCI_ATR_TC2:			
			case ERR_SCI_ATR_TD2:			
			case ERR_SCI_ATR_TA3:			
			case ERR_SCI_ATR_TB3:			
			case ERR_SCI_ATR_TC3:			
			case ERR_SCI_T_ORDER:			
			case ERR_SCI_PPS_PPSS:		
			case ERR_SCI_PPS_PPS0:		
			case ERR_SCI_PPS_PCK:						
			case ERR_SCI_PARAM:
				return -6; // Erro de comunicação
			default:
				return -8; // Erro Desconhecido
		}
	}
}

int PowerDown(mrb_state *mrb, int slot) 
{
	// Adjust slot according to SDK
	slot = ToSDK(slot);
	
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

int SendAPDU(mrb_state *mrb, int slot, char *in, int sizeIn, char *out, int *sizeOut)
{
	ST_APDU_REQ req;
	ST_APDU_RSP rsp;
	int ret;
	int size;
	
	// Adjust slot according to SDK
	slot = ToSDK(slot);
	
	memset(&req, 0, sizeof(req));
	
	if (sizeIn > 0) {
		req.Cmd[0] = in[0]; /*CLA*/
	}
	if (sizeIn > 1) {
		req.Cmd[1] = in[1]; /*INS*/
	}
	if (sizeIn > 2) {
		req.Cmd[2] = in[2]; /*P1 */
	}
	if (sizeIn > 3) {
		req.Cmd[3] = in[3]; /*P2 */
	}
	if (sizeIn > 5) {       //LC is optional, for it to be present, LE also must be present
		req.LC     = in[4]; /*LC */
	}
	if (req.LC > 0) {
		memcpy(req.DataIn, (in + 5), req.LC);
	}
	if (sizeIn > (4 + req.LC)) {
		int adjust = sizeIn == 5 ? 4 : 5 + req.LC;
		req.LE     = *(in + adjust);
	}

	ContextLog(mrb, 0, "ST_APDU_REQ = %02x%02x%02x%02x %02x %p %02x", req.Cmd[0], req.Cmd[1], req.Cmd[2], req.Cmd[3], req.LC, req.DataIn, req.LE);
	
	ret = OsIccExchange(slot, 0x01, &req, &rsp);
	ContextLog(mrb, 0, "OsIccExchange(%d, 0x01, %p, %p) = %d", slot, &req, &rsp, ret);
	
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
		switch (ret) {
			//TODO:
			//•	-4: Problema VCC
			//•	-5: Problema VPP
			case ERR_SCI_HW_NOCARD:		
				return -1; // Sem resposta
			case ERR_DEV_NOT_EXIST:
				return -2; // SamCard Invalido
			case ERR_SCI_HW_STEP:	
				return -3; // SamCard mudo
			case ERR_SCI_HW_PARITY:		
			case ERR_SCI_HW_TCK:				
			case ERR_SCI_T_ORDER:			
			case ERR_SCI_PPS_PPSS:		
			case ERR_SCI_PPS_PPS0:		
			case ERR_SCI_PPS_PCK:			
			case ERR_SCI_T0_PARAM:		
			case ERR_SCI_T0_REPEAT:		
			case ERR_SCI_T0_PROB:			
			case ERR_SCI_T1_PARAM:		
			case ERR_SCI_T1_BWT:			
			case ERR_SCI_T1_CWT:			
			case ERR_SCI_T1_BREP:			
			case ERR_SCI_T1_LRC:			
			case ERR_SCI_T1_NAD:			
			case ERR_SCI_T1_LEN:			
			case ERR_SCI_T1_PCB:			
			case ERR_SCI_T1_SRC:			
			case ERR_SCI_T1_SRL:			
			case ERR_SCI_T1_SRA:			
			case ERR_SCI_PARAM:
				return -6; // Erro de comunicação
			case ERR_SCI_HW_TIMEOUT:
				return -7; // SamCard Removido
			default:
				return -8; // Erro Desconhecido
		}
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
    ret = PowerOn(mrb, slot, &historical_size, historical);
  } else { 
    // Turn off
	ret = PowerDown(mrb, slot);
  }

  array = mrb_ary_new(mrb);
  mrb_ary_push(mrb, array, mrb_fixnum_value(ret));

  if (ret == RET_OK && status == 1) {
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

  ret = SendAPDU(mrb, slot, (char *)RSTRING_PTR(in), in_size, out, &out_size);
   
  array = mrb_ary_new(mrb);
  mrb_ary_push(mrb, array, mrb_fixnum_value(ret));

  if (ret == RET_OK) {
	mrb_ary_push(mrb, array, mrb_str_new(mrb, out, out_size));
  }
  
  return array;
}

void
mrb_sam_card_init(mrb_state* mrb)
{
  struct RClass *pax, *sam_card;

  pax      = mrb_class_get(mrb, "PAX");
  sam_card = mrb_define_class_under(mrb, pax, "SamCard", mrb->object_class);

  mrb_define_class_method(mrb , sam_card , "power" , mrb_sam_card_power , MRB_ARGS_REQ(2));
  mrb_define_class_method(mrb , sam_card , "send"  , mrb_sam_card_send  , MRB_ARGS_REQ(2));
}

