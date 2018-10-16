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
#define TLBS 1<<13


#define DEBUG_DLEVEL  TMAIN|TSOC|TPROT|TPERI  //TMAIN|TSOC|TGPS|TSEN|TUART|TCONT|TI2C|TMCU|TBAT|TPROT|TPERI|TBT|TOTA|TLBS    

//#define __HW_2018__	//新版硬件
//#define __CHAOWEI__ 
//#define __WAIMAI__	//外卖版本，与蓝牙只走串口协议
//#define __BAT_PROT__	//电池通讯协议

//#define __MEILING__
//#define __BAOJIA__
#endif
