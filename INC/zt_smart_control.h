#ifndef __ZT_SMART_CONTROL_H__
#define __ZT_SMART_CONTROL_H__
#include "global_types_def.h"

/* commands from APP */
enum { 
	CHECK_PHONE_WHETHER_VERIFIED_CMD = 0x0c,
	/* default pwd or not*/
	INQUIRE_PWD_STATUS_CMD = 0x04,
	SET_PWD_CMD = 0x06,
	INQUIRE_MODE_CMD = 0x0a,
	READ_VOLTAGE_VALUE = 0x0b,
	SWITCH_TO_GANYING_MODE_CMD = 0x08,
	SWITCH_TO_YAOKONG_MODE_CMD = 0x07,
	PWD_VERIFY_CMD = 0x05,
	LOCK_CMD = 0x01,
	UNLOCK_CMD = 0x02,
	SEARCH_CMD = 0x03,
	LOW_VOLTAGE_NOTICE_CMD = 0x09,
	KEEP_QUIET_CMD = 0x0d,
	SET_SENSITIVITY = 0x0e,
	READ_WHETHER_PHONE_NEED_TO_ALERT = 0x0f,
	SC_LAST_CMD,
};

typedef struct
{
	kal_uint8 type;		//0x01 寻车,0x02 锁车，0x03 报警开关
	kal_uint8 para[6];
}command_struct;

typedef struct
{
	kal_uint16 adc;
	kal_uint8 electric_gate;
}battery_struct;

typedef enum
{
	HIGH_SPEED = 1,
	LOW_SPEED,
}SPEED;
typedef enum
{
	HIGH = 1,
	LOW,
}QIANYA;
typedef enum
{
	DIANDONG =1,
	ZHULI,
	RENLI,
}DONGLI;
typedef struct
{
	kal_uint8 fault;
	kal_uint32 hall;
	SPEED tiaosu;
	QIANYA qianya;
	DONGLI zhuli;
	kal_uint8 xiufu;
}controller_struct;

#pragma pack (1)

typedef struct
{
	kal_uint8 tiaosu:1;
	kal_uint8 qianya:1;
	kal_uint8 zhuli:2;
	kal_uint8 lock:1;
	kal_uint8 alarm:1;
}status_struct;

typedef struct
{
	kal_uint8 fault;
	status_struct status;
	kal_uint32 hall;
	battery_info_struct bat;
}ebike_struct;
#pragma pack ()

extern void zt_smart_proc_network_data(kal_uint8 value_len, kal_uint8* value_data);
extern void zt_smart_update_network_data(kal_uint8* update_data);
extern kal_uint16 zt_adc_get_value(void);
extern void zt_smart_init(void);
extern kal_uint8 get_electric_gate_status(void);

#endif
