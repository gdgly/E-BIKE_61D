#ifndef __ZT_TRACE_H__
#define __ZT_TRACE_H__
#include "global_types_def.h"

#define TMAIN 1
#define TSOC 1<<1
#define TGPS 1<<2
#define TSEN 1<<3
#define TUART 1<<4
#define TCONT 1<<5
#define TI2C 1<<6
#define TMCU 1<<7
#define TBAT 1<<8
#define TPROT 1<<9
#define TBT 1<<10
#define TPERI 1<<11
#define TOTA 1<<12

#define DEBUG_DLEVEL TMAIN|TPROT 	//|TGPS |TSEN |TI2C|TPROT |TSOC |TBT    //|TUART|TCONT|TI2C|TMCU|TBAT|TPERI|TBT|TSEN|TOTA  

#define ENABLE_LOG
#endif
