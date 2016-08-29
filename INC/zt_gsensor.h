#ifndef __ZT_GSENSOR_H__
#define __ZT_GSENSOR_H__
#include "global_types_def.h"

typedef struct{
	kal_uint8 x;
	kal_uint8 y;
	kal_uint8 z;
}zt_acce_struct;

extern kal_bool zt_gsensor_check_is_motionless(void);
extern kal_bool zt_gsensor_check_is_moving(void);
extern kal_bool zt_gsensor_check_is_shake_sharp(void);
void zt_gsensor_init(void);

#endif
