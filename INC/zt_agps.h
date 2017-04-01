#ifndef __ZT_AGPS_H__
#define __ZT_AGPS_H__
#include "zt_trace.h"
#include "zt_mtk_type.h"

typedef struct 
{
	char cmd[6];
	char user[32];
	char pwd[20];
	float lat;
	float lon;
	float pacc;
}agps_struct;

void zt_agps_request(void);
void zt_agps_login_package(void);
void zt_agps_parse(RcvDataPtr GetRcvData);
void zt_agps_set_location(float lat,float lon);

#endif
