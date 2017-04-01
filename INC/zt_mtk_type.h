#ifndef __ZT_MTK_TYPE_H__
#define __ZT_MTK_TYPE_H__
#include "global_types_def.h"


typedef enum
{
    SRV_NW_INFO_SA_SEARCHING,       /* Searching network */
    SRV_NW_INFO_SA_NO_SERVICE,      /* No service */
    SRV_NW_INFO_SA_LIMITED_SERVICE, /* Limited service, emergency call only */
    SRV_NW_INFO_SA_FULL_SERVICE,    /* Full service */

    SRV_NW_INFO_SA_END_OF_ENUM
} srv_nw_info_service_availability_enum;

typedef enum
{
    MMI_SIM_NONE = 0, /* No bit-wise operation for none case */
    MMI_SIM1 = 0x0001,
    MMI_SIM2 = 0x0002,
    MMI_SIM3 = 0x0004,
    MMI_SIM4 = 0x0008,
    MMI_SIM_END_OF_ENUM
}mmi_sim_enum;

typedef enum
{
	ZT_GSENSOR_CHECK_TIMER=0,
	ZT_UPLOAD_TIMER,
	ZT_HB_TIMER,
	ZT_NETWORK_CHECK_TIMER,
	ZT_GPS_PERIOD_TIMER,
	ZT_SMART_LUNDONG_CHECK_TIMER,
	ZT_KEY_DETECT_TIMER,
	ZT_LOCK_DELAY_TIMER,
	ZT_TANGZE_LOCK_TIMER,
	ZT_GPS_PWR_CHECK_TIMER,
	ZT_DIANMEN_LOCK_TIMER,
	ZT_DIANMEN_UNLOCK_TIMER,
	ZT_CLOSE_DIANCHI_LOCK_TIMER,
	ZT_LOGIN_TIMER,
	ZT_ONLINE_CHECK_PROTECT_TIMER,
	ZT_GPS_DELAY_OFF_TIMER,

	ZT_MAX_TIMER=49,
}Timer_ID;

typedef struct
{
    kal_uint16 nYear;
    kal_uint8 nMonth;
    kal_uint8 nDay;
    kal_uint8 nHour;
    kal_uint8 nMin;
    kal_uint8 nSec;
    kal_uint8 DayIndex; /* 0=Sunday */
} applib_time_struct;

typedef enum
{
	NVRAM_EF_ZT_HALL_LID=0,
	NVRAM_EF_ZT_BT_NAME_LID,
	NVRAM_EF_ZT_BT_MAC_LID,
	NVRAM_EF_ZT_DIANMEN_LID,
	NVRAM_EF_GT_TEMP_THR_LID,
	NVRAM_EF_GT_VIBR_THR_LID,
	NVRAM_EF_GT_SPEED_THR_LID,
	NVRAM_EF_GT_ALARM_SWITCH_LID,
	NVRAM_EF_GT_DEFENCE_LID,
	NVRAM_EF_GT_DEV_TYPE_LID,
	NVRAM_EF_GT_RESERVE_LID,
}NVRAM_EF_LID;

typedef enum 
{
   BATTERY_LOW_POWEROFF = 0,
   BATTERY_LOW_TX_PROHIBIT,
   BATTERY_LOW_WARNING,
   BATTERY_LEVEL_0,
   BATTERY_LEVEL_1,
   BATTERY_LEVEL_2,
   BATTERY_LEVEL_3, 
   BATTERY_LEVEL_4 = BATTERY_LEVEL_3, /* BATTERY_LEVEL_4 */
   BATTERY_LEVEL_5 = BATTERY_LEVEL_3, /* BATTERY_LEVEL_5 */
   BATTERY_LEVEL_6 = BATTERY_LEVEL_3, /* BATTERY_LEVEL_6 */
   BATTERY_LEVEL_LAST = 9

} battery_level_enum;

#define YEARFORMATE  2000

typedef struct __rtc 
{
	kal_uint8		rtc_sec;    /* seconds after the minute   - [0,59]  */
	kal_uint8		rtc_min;    /* minutes after the hour     - [0,59]  */
	kal_uint8		rtc_hour;   /* hours after the midnight   - [0,23]  */
	kal_uint8		rtc_day;    /* day of the month           - [1,31]  */
	kal_uint8		rtc_mon;    /* months 		               - [1,12] */
	kal_uint8		rtc_wday;   /* days in a week 		      - [1,7] */
	kal_uint8		rtc_year;   /* years                      - [0,127] */
} t_rtc;

typedef struct MYTIME
{
    kal_int16 nYear;
    kal_int8 nMonth;
    kal_int8 nDay;
    kal_int8 nHour;
    kal_int8 nMin;
    kal_int8 nSec;
    kal_int8 DayIndex; /* 0=Sunday */
} MYTIME;



#define LOCAL_PARA_HDR \
   kal_uint8	ref_count; \
   kal_uint8    lp_reserved; \
   kal_uint16	msg_len;
/*  common local_para header */
typedef struct local_para_struct {
    /* ref_count: reference count; 
     * lp_reserved : reserved for future; 
     * msg_len  : total length including this header.
     */
    LOCAL_PARA_HDR
#ifdef __BUILD_DOM__
    ;
#endif
} local_para_struct;
#define PEER_BUFF_HDR \
   kal_uint16	pdu_len; \
   kal_uint8	ref_count; \
   kal_uint8   	pb_resvered; \
   kal_uint16	free_header_space; \
   kal_uint16	free_tail_space;
/* peer buffer header, user should treat it as opaque type */
typedef struct peer_buff_struct {
   PEER_BUFF_HDR 
#ifdef __BUILD_DOM__
    ;
#endif
} peer_buff_struct;

/* Define module type. */
#define oslModuleType   module_type

/* Define Message type. */
#define oslMsgType      msg_type

/* Define local parameter type. */
#define oslParaType     local_para_struct

/* Define peer buff type. */
#define oslPeerBuffType peer_buff_struct

typedef struct
{
	LOCAL_PARA_HDR
	kal_uint16 dataLen;
	kal_uint8 data[1024];	//MAX_UART_BUF_SIZE
}bt_msg_struct;

typedef struct 
{
    kal_int16 nYear;
    kal_int8 nMonth;
    kal_int8 nDay;
    kal_int8 nHour;
    kal_int8 nMin;
    kal_int8 nSec;
} GPSDATETIME;

typedef struct 
{
	GPSDATETIME dt;
	kal_uint8 state;
	double latitude;
	kal_uint8 NS;
	double longitude;
	kal_uint8 EW;
	double speed;
	double angle;
	double magnetic_value;
	kal_uint8 magnetic_ind;
	kal_uint8 mode;	//A 自主 D 差分 E估算 N 数据无效

	kal_uint8 sat_uesd;	//可用卫星
	double altitude;	//海拔
	double hdop;	//精度

	kal_uint8 sat_view;	//可见卫星
}gps_info_struct;

typedef struct{
	kal_uint8 ym_type;		// 1 ip, 2 ym
	kal_uint8 domain[64];
	kal_uint8 ip[16];
	kal_uint8 ip_len;
	int port;
}network_info_struct;

typedef enum{
	CONNECT_NONE=-1,
	CONNECT_LONG,
	CONNECT_ONE,
}CONNECT_TYPE;

typedef struct{
	kal_int8 app_id;
	kal_int8 socket_id;
	CONNECT_TYPE conn_type;
	network_info_struct network_info;
	FuncPtr login_callback;
	FuncPtr destory_callback;
	PsExtFuncPtr parse_callback;
}network_link_struct;

typedef struct{
	CONNECT_TYPE conn_type;
	network_info_struct network_info;
	FuncPtr login_callback;
	FuncPtr destory_callback;
	PsExtFuncPtr parse_callback;
}network_para_struct;

typedef struct 
{
	kal_uint8  sig_stren;
	kal_uint16 mcc;
	kal_uint8 mnc;
	kal_uint16 lac_sid;
	kal_uint16 cellid_nid;
	kal_uint16 bid;		
}cell_info_struct;

typedef struct
{
	cell_info_struct lbs_server;
	cell_info_struct lbs_nbr[6];
	kal_uint8 lbs_nbr_num;
}lbs_info_struct;

typedef struct
{
	LOCAL_PARA_HDR
	lbs_info_struct lbs;
}lbs_msg_struct;

typedef struct
{
	kal_uint8 addr;		//0x1a 电动车控制器，0x1b充电站控制器，0x1c单片机蓝牙智控器
	kal_uint8 value_len;
	kal_uint8 value[18];
}kfd_controller_data_struct;

typedef enum
{
	VOICE_UNLOCK=5,
	VOICE_LOCK=6,
	VOICE_SEARCH=7,
	VOICE_ALARM=8,
}ZT_VOICE_TYPE;

typedef enum
{
	MSG_ID_ZT_BEGIN=0,
	MSG_ID_UART_SEND_TO_MMI_REQ=MSG_ID_ZT_BEGIN,
	MSG_ID_MMI_SEND_TO_BT_REQ,
	MSG_ID_BT_SEND_TO_MMI_REQ,
	MSG_ID_ADC_UEM_SEND_TO_MMI_REQ,
	MSG_ID_HALL_EINT_SEND_MMI_REQ,
	MSG_ID_LUNDONG_SEND_MMI_REQ,
	MSG_ID_LBS_SEND_MMI_REQ,
	MSG_ID_UART2_SEND_TO_MMI_REQ,
	MSG_ID_BAT_SEND_TO_MMI_REQ,
	MSG_ID_ZT_END=MSG_ID_ZT_BEGIN+20,
}MSG_ID;

typedef struct
{
	LOCAL_PARA_HDR
	kal_uint16 adc_value;
	kal_uint16 aver_value;
}adc_msg_struct;

typedef struct
{
	LOCAL_PARA_HDR
	kal_uint32 data;
}eint_msg_struct;

/* opaque type for kal timer identity */
typedef struct
{
    kal_uint8 unused;
} *kal_timerid;

/* timer (kal timer, stack timer, event scheduler) callback function prototype */
typedef void (*kal_timer_func_ptr)(void *param_ptr);

/* define how many miliseconds per system tick represent */
#define KAL_MILLISECS_PER_TICK      4.615

/* DOM-NOT_FOR_SDK-BEGIN */
/* Following defines are internal to the KAL */
#define KAL_TICKS_10_MSEC           (2)         /* 10 msec */
#define KAL_TICKS_50_MSEC           (10)        /* 50 msec */
#define KAL_TICKS_100_MSEC          (21)        /* 100 msec */
#define KAL_TICKS_500_MSEC          (108)       /* 500 msec */
#define KAL_TICKS_1024_MSEC         (221)       /* 1024 msec */
#define KAL_TICKS_1_SEC             (216)       /* 1 sec */
/*since rmc_context.h defined KAL_TICKS_2_SEC*/
#define KAL_TICKS_2_SEC_2           (433)       /* 2 sec */
#define KAL_TICKS_3_SEC             (650)       /* 3 sec */
#define KAL_TICKS_5_SEC             (1083)      /* 5 sec */
#define KAL_TICKS_30_SEC            (6500)      /* 30 sec */
#define KAL_TICKS_1_MIN             (13000)     /* 1 min */
#define KAL_MSEC_64_TICKS           (295)       /* 64 ticks */
#define KAL_MSEC_256_TICKS          (1181)      /* 256 ticks */

typedef int             FS_HANDLE;
typedef kal_uint16	UI_character_type;
typedef int (*RcvDataPtr)(char*,int);

/* FS_Seek Parameter */
typedef enum
{
    FS_FILE_BEGIN,
    FS_FILE_CURRENT,
    FS_FILE_END
} FS_SEEK_POS_ENUM;

//-- FS_Open flags
/* The file is opened for read and write access. */
#define FS_READ_WRITE           0x00000000L

/* The file is opened for read only access. */
#define FS_READ_ONLY            0x00000100L
#define FS_OPEN_DIR             0x00000800L
#define FS_CREATE               0x00010000L
#define FS_CREATE_ALWAYS        0x00020000L
#define FS_NO_ERROR 0

typedef enum {
      uart_port1=0,
      uart_port2,
      uart_port3,
      uart_max_port,      
      uart_port_null = 99    /* a uart port for those who uses physical port */
} UART_PORT;

typedef struct
{
	kal_uint16 sum_vol;
	kal_uint16 currnt;
	kal_uint16 des_cap;
	kal_uint16 des_vol;
	kal_uint16 fcc;
	kal_uint16 residual_cap;
	kal_uint16 per_cap;
	kal_uint16 cycle_count;
	kal_uint16 temperatue;
	kal_uint16 status;
	kal_uint16 bunch;
}battery_info_struct;

typedef struct
{
	LOCAL_PARA_HDR
	battery_info_struct bat;	
}battery_msg_struct;
#endif
