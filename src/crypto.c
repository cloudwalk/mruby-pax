#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "mruby.h"
#include "mruby/compile.h"
#include "mruby/value.h"
#include "mruby/array.h"
#include "mruby/string.h"
#include "mruby/hash.h"

#include "osal.h"

/*NewDes interface*/
static unsigned char  f[] = {
  32,137,239,188,102,125,221,72,212,68,81,37,86,237,147,149,
  70,229,17,124,115,207,33,20,122,143,25,215,51,183,138,142,
  146,211,110,173,1,228,189,14,103,78,162,36,253,167,116,255,
  158,45,185,50,98,168,250,235,54,141,195,247,240,63,148,2,
  224,169,214,180,62,22,117,108,19,172,161,159,160,47,43,171,
  194,175,178,56,196,112,23,220,89,21,164,130,157,8,85,251,
  216,44,94,179,226,38,90,119,40,202,34,206,35,69,231,246,
  29,109,74,71,176,6,60,145,65,13,77,151,12,127,95,199,
  57,101,5,232,150,210,129,24,181,10,121,187,48,193,139,252,
  219,64,88,233,96,128,80,53,191,144,218,11,106,132,155,104,
  91,136,31,42,243,66,126,135,30,26,87,186,182,154,242,123,
  82,166,208,39,152,190,113,205,114,105,225,84,73,163,99,111,
  204,61,200,217,170,15,198,28,192,254,134,234,222,7,236,248,
  201,41,177,156,92,131,67,249,245,184,203,9,241,0,27,46,
  133,174,75,18,93,209,100,120,76,213,16,83,4,107,140,52,
  58,55,3,244,97,197,238,227,118,49,79,230,223,165,153,59
};

/*Packing From ASCII to Bin*/
void Compacta(unsigned char *fonte, unsigned char *destino, long tam)
{
  char           *pt;
  char           *pt1;
  int  i;

  for ( i = 0;   i < (tam >> 1);   destino[i++] = (unsigned char)0x00);
  pt = (char *)fonte;

  for ( i = 0; i < (tam >> 1); i++ ) {
    pt1 = pt +1;
    (*pt >= 'A' && *pt <= 'F') ? (*pt = (*pt - '7') << 4) : (*pt = (*pt & 0x0f) << 4);
    (*pt1 >= 'A' && *pt1 <= 'F') ? (*pt1 = *pt1 - '7') : (*pt1 = *pt1 & 0x0f);
    destino[i] = (unsigned char) (*pt | *pt1);
    pt += 2;
  }
  return;
}

/*Unpacking from ASCII to Bin*/
void Descompacta(unsigned char *fonte, unsigned char *destino, long tam)
{
	int i;
	int j = 0;
	unsigned char c;

	for ( i = 0; i < (tam << 1); destino[i++] = (unsigned char)0x00);
	for ( i = 0; i < tam; i++) {
		c = fonte[i];
		c = (c & 0xf0) >> 4;
		(c <= 9) ? (c += '0') : (c += 0x37);
		destino[j++] = (unsigned char)c;
		c = (fonte[i] & 0x0f);
		(c <= 9) ? (c += '0') : (c += 0x37);
		destino[j++] = c;
	}
	return;
}

/*encrypting/decrypting chain*/
static void Code(unsigned char *b, unsigned char *key, long initi, long delta, long fini)
{
  int i = (int)initi;

  for (;;)
  {
    b[4] ^= f[b[0] ^ key[i++]];
    if (i == 15) i = 0;
    b[5] ^= f[b[1] ^ key[i++]];
    if (i == 15) i = 0;
    b[6] ^= f[b[2] ^ key[i++]];
    if (i == 15) i = 0;
    b[7] ^= f[b[3] ^ key[i]];
    i += delta;

    if (i > 14) i -= 15;
    if (i == fini) return;

    b[1] ^= f[b[4] ^ key[i++]];
    b[2] ^= f[b[4] ^ b[5]];
    b[3] ^= f[b[6] ^ key[i++]];
    b[0] ^= f[b[7] ^ key[i]];
    i += delta;
    if (i > 14) i -= 15;
  }
}

void NewDes(short int oper, unsigned char *dado, unsigned char *chave)
{
  unsigned char  vetaux[17];
  long           initi;
  long           delta;
  long           fini;
  int  i;

	if (oper & 1) {
		initi = 11;
		delta =  9;
		fini  = 12;

		if (oper & 0x10) {
			Compacta((unsigned char *)dado,(unsigned char *)vetaux,(long)16);
			for (i = 0; i < 8; i++ ) dado[i] = vetaux[i];
		}
	} else {
		initi = 0;
		fini  = 0;
		delta = 1;
	}

  Code(dado, chave, initi, delta, fini);

  if ( oper == 0x10 ) {
    Descompacta((unsigned char *)dado,(unsigned char *)vetaux,(long)8);
    strncpy((char *)dado,(char *)vetaux,16);
  }
}

/*NewDes interface end*/

static mrb_value
mrb_s_crypto_delete_all_keys(mrb_state *mrb, mrb_value klass)
{
  return mrb_fixnum_value(OsPedEraseKeys());
}

unsigned int crc_table[256];

void create_crc_table(void)
{
  int index;
  unsigned char counter;
  unsigned int data;
  uint16_t accumulator;

  for (index=0; index<256; index++)
  {
    data = index << 8;
    accumulator = 0;
    for (counter=1; counter<9; counter++)
    {
      if ( (( data ^ accumulator ) & 0x8000) == 0)
        accumulator = accumulator<<1;
      else
        accumulator = (accumulator<<1) ^ 0x1021;
      data = data << 1;
    }
    crc_table[index]=accumulator;
  }
}

static mrb_value
mrb_s_crc16(mrb_state *mrb, mrb_value klass)
{
  mrb_int accumulator;
  mrb_value buf;
  int index;
  unsigned char index2;
  int size;
  char *pbuf;

  mrb_get_args(mrb, "Si", &buf, &accumulator);

  size = RSTRING_LEN(buf);
  pbuf = (char *)RSTRING_PTR(buf);

  for (index=0; index<size; index++) {
    index2      = (accumulator>>8)^pbuf[index];
    accumulator = (accumulator<<8)^crc_table[ index2 ];
  }

  return mrb_fixnum_value((uint16_t)accumulator);
}

static mrb_value
mrb_s_crypto_newdes_encrypt(mrb_state *mrb, mrb_value klass)
{
  mrb_value buf, key;
  unsigned char sBin[32 + 1]={0};

  mrb_get_args(mrb, "SS", &buf, &key);

  memcpy(sBin, RSTRING_PTR(buf), RSTRING_LEN(buf));

	NewDes(0x00, (unsigned char *)sBin, (unsigned char *)RSTRING_PTR(key));

  return mrb_str_new(mrb, (char *)&sBin, 8);
}

static mrb_value
mrb_s_crypto_newdes_decrypt(mrb_state *mrb, mrb_value klass)
{
  mrb_value buf, key;
  unsigned char sBin[32 + 1]={0};

  mrb_get_args(mrb, "SS", &buf, &key);

  memcpy(sBin, RSTRING_PTR(buf), RSTRING_LEN(buf));

	NewDes(0x01, (unsigned char *)&sBin, (unsigned char *)RSTRING_PTR(key));

  return mrb_str_new(mrb, (char *)&sBin, 8);
}

void
mrb_crypto_init(mrb_state* mrb)
{
  struct RClass *pax, *crypto, *newdes;

  create_crc_table();

  pax      = mrb_class_get(mrb, "PAX");
  crypto   = mrb_define_class_under(mrb, pax   , "Crypto", mrb->object_class);
  newdes   = mrb_define_class_under(mrb, crypto, "Newdes", mrb->object_class);

  mrb_define_class_method(mrb , crypto , "delete_all_keys" , mrb_s_crypto_delete_all_keys , MRB_ARGS_NONE());
  mrb_define_class_method(mrb , crypto , "crc16"           , mrb_s_crc16                  , MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb , newdes , "encrypt"         , mrb_s_crypto_newdes_encrypt  , MRB_ARGS_REQ(2));
  mrb_define_class_method(mrb , newdes , "decrypt"         , mrb_s_crypto_newdes_decrypt  , MRB_ARGS_REQ(2));
}

