#ifndef MRUBY_EMV_H
#define MRUBY_EMV_H

#include "mruby.h"
#include "mruby/compile.h"
#include "mruby/value.h"

void get_rgba(mrb_state *mrb, mrb_value klass, int *r, int *g, int *b);
int getAsteriskSize(void);
int fix_x(int x);
int fix_y(int y);

#endif /* MRUBY_EMV_H */

