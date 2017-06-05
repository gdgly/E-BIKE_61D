#ifndef __ZT_SMART_CONTROL_H__
#define __ZT_SMART_CONTROL_H__
#include "global_types_def.h"
#include "kfd_protocol.h"

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
	ZHULI=2,
	ZHULI2=3,
	RENLI=4,
	HUNHE=5,
}DONGLI;
typedef enum
{
	VOT36V=1,
	VOT48V=2,	
}DIANYUAN;
typedef enum
{
	XF_INVALID=1,
	XF_OK=2,	
}XIUFU;
typedef struct
{
	kal_uint8 fault;
	kal_uint32 hall;
	SPEED tiaosu;
	QIANYA qianya;
	DONGLI zhuli;
	DIANYUAN dy;
	kal_uint8 xiufu;
}controller_struct;
typedef enum
{
	CMD_CONTROL=1,
	CMD_READ,
}cmd_enum;
typedef enum
{
	ADDR_BAT=0x16,
	ADDR_CONTROL=0x1d,	
}addr_enum;
typedef enum
{
	bat_temp=0x08,
	bat_vol=0x09,
	bat_curr=0x0a,
	bat_cap=0x0f,
	bat_total_cap=0x10,
	bat_cycle=0x17,
	bat_interval=0x47,
	bat_max_interval=0x48,	
}bat_cmd_enum;


#pragma pack (1)

typedef struct
{
	kal_uint16 tiaosu:1;
	kal_uint16 qianya:1;
	kal_uint16 zhuli:3;
	kal_uint16 lock:1;
	kal_uint16 alarm:1;
	kal_uint16 dy:1;
	kal_uint16 xiufu:1;
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
extern void zt_smart_update_network_data(gps_tracker_control_data_struct* package);
extern kal_bool zt_get_bat_connet_status(void);
extern void zt_smart_init(void);
extern kal_uint8 get_electric_gate_status(void);

#endif
