#ifndef __ZT_PROTOCOL_H__
#define __ZT_PROTOCOL_H__
#include "global_types_def.h"


#define MAX_GT_DEV_ID_BYTE_LEN 		8
#define MAX_GT_VER_STR_LEN 			16
#define MAX_GT_IMEI_LEN (16) 
#define MAX_GT_SEND_LEN 256


typedef enum
{	
    EN_GT_LT_GPS = 0,
	EN_GT_LT_CELL ,
    EN_GT_LT_END
} GT_LOC_TYPE_EN;

typedef enum
{
    EN_GT_SWT_OFF = 0,
	EN_GT_SWT_ON,
    EN_GT_SWT_END
} GT_SWITCH_VALUE_EN;

typedef enum
{
    EN_GT_AT_PWR_LOW = 0,
	EN_GT_AT_PWR_OFF = 1,
	EN_GT_AT_VIBR = 2,
	EN_GT_AT_OIL_PWR = 3,
	EN_GT_AT_SPEED = 6,
    EN_GT_AT_END
} GT_ALARM_TYPE_EN;

typedef enum
{
	EN_GT_DT_ADMIN_NUM = 0x00,
	EN_GT_DT_PWD = 0x01,
	EN_GT_DT_USER = 0x02,
	
	EN_GT_DT_UP_INTV = 0x03,
	EN_GT_DT_HB_INTV = 0x04,
	EN_GT_DT_SMS_ALARM_INTV = 0x05,
	
	EN_GT_DT_TEMP_THR = 0x06,
	EN_GT_DT_VIBR_THR = 0x07,
	EN_GT_DT_SPEED_THR = 0x08,
	
	EN_GT_DT_LANG = 0x09,
	EN_GT_DT_TIME_ZONE = 0x0a,
	
	EN_GT_DT_ALARM_SWITCH = 0x0b,
	EN_GT_DT_SMS_ALARM_SWITCH = 0x0c,
	
	EN_GT_DT_LOC = 0x0d,
	EN_GT_DT_IGNORE_ALARM = 0x0e,
	EN_GT_DT_LOG_LEVEL = 0x0f,
	
	EN_GT_DT_SET_TIME = 0x10,
	EN_GT_DT_SHUTDOWN = 0x12,
	EN_GT_DT_RESTORE = 0x13,
	EN_GT_DT_SMS = 0x14,

	EN_GT_DT_DEFENCE = 0x16,
	EN_GT_DT_SERVER = 0x17,
	EN_GT_DT_APN = 0x18,
	EN_GT_DT_SMS_CENTER = 0x19,
	EN_GT_DT_VER = 0x1A,

	EN_GT_DT_PWR_OIL_SWITCH = 0x20,
	EN_GT_DT_IO = 0x21,
	EN_GT_DT_IMSI = 0x22,
	
    EN_GT_CT_END
} GT_DATA_TYPE_EN;

typedef enum
{
    EN_GT_GS_A = 1,	
    EN_GT_GS_V,
    EN_GT_GS_INV
} GT_GPS_STATE_EN;

typedef enum
{
    EN_GT_EAST = 0,
    EN_GT_SOUTH,
    EN_GT_WEST,
    EN_GT_NORTH,
    EN_GT_INV
} GT_ORIENTATION_EN;

typedef enum
{
	EN_GT_GM_N = 0,
	EN_GT_GM_A,
    EN_GT_GM_D,
    EN_GT_GM_E,
    EN_GT_GM_INV
} GT_GPS_MODE_EN;

typedef enum
{
	EN_GT_PT_BASE = 0,
	EN_GT_PT_LOGIN,
	EN_GT_PT_GPS,
	EN_GT_PT_STATUS,	
	EN_GT_PT_HB,
	EN_GT_PT_ALARM,	
	EN_GT_PT_DEV_DATA,	
	EN_GT_PT_CONTROL,
	EN_GT_PT_GIVE_BACK,
	EN_GT_PT_LBS,
	
	EN_GT_PT_SRV_DATA = 0x20,

    EN_GT_PT_END
} GT_PROT_TYPE_EN;
typedef enum
{
	EN_INIT_STATE = 0,
	EN_LOGING_STATE,
	EN_WORKING_STATE,	
}work_state;

typedef enum
{
	IP_MODE,
	DOMAIN_MODE,	
}CONNECT_MODE;
typedef struct
{
	CONNECT_MODE mode;
	char reserve[511];
}NVRAM_RESERVE;

#pragma pack (1)	//ǿ���ֽڶ���
typedef struct 
{	
	kal_uint16	stop;
}gps_tracker_msg_tail_struct;
typedef struct 
{
	kal_uint16 start;     	//��ʼλ
	kal_uint16	crc;		//crcУ��
	kal_uint8	pack_len;	//������	---- ���ݳ���//zzt.20150715.add note
	kal_uint8	prot_type;	//Э������
	kal_uint16 sn;	 		//�����к�
	kal_uint8 datetime[6];	//��ʱ��
}gps_tracker_msg_head_struct;

typedef struct 
{
	kal_uint8 pwr_low_ind:1;
	kal_uint8 pwr_off_ind:1;
	kal_uint8 vibr_ind:1;
	kal_uint8 oil_pwr_ind:1;
	kal_uint8 sos_ind:1;
	kal_uint8 temp_ind:1;
	kal_uint8 speed_ind:1;
	kal_uint8 fence_ind:1;
	
	kal_uint8 pwr_level;
	kal_uint8 vibr_value;
	kal_int8 temp;
	kal_uint16 speed;
}gps_tracker_alarm_struct;

typedef struct 
{	
	kal_uint8 pwr_low:1;//�͵籨������
	kal_uint8 pwr_off:1;//�ϵ�澯����
	kal_uint8 vibr:1;   //�𶯸澯����
	kal_uint8 oil_pwr:1;//���͵�澯����
	kal_uint8 sos:1;	 //sos�澯����
	kal_uint8 temp:1;   //�¶ȸ澯����
	kal_uint8 speed:1;  //���ٸ澯����
	kal_uint8 fence:1;
}gps_tracker_switch_struct;


/********* 0x01 ��½��****************/
typedef struct 
{
	kal_uint8   dev_id[MAX_GT_DEV_ID_BYTE_LEN];	//�豸id 15λIMEIת���ɵ�8λ�ֽ���
	kal_uint8   dev_type;					//�豸����
	kal_uint32 auth_code;					//��֤��
}gps_tracker_login_req_content_struct;

/********* 0x02 GPS ��****************/
typedef struct 
{	
	kal_uint8 lat_ind:1;	//0 ��γ 1 ��γ
	kal_uint8 long_ind:1;	//0  ����1����
	kal_uint8 mode:2;   	//0 ʵʱ 1 ��� 2 ���� 3 ��Ч �������ʵʱ����ֻ��߹��㣬˵��gps�Ѿ��ɹ���λ��
}gps_tracker_property_struct;

typedef struct 
{
	kal_uint8 loc_type;       	//��λ����
	kal_uint8 reserv_satnum;	//ǰ��λgps�������� + ����λ������
	gps_tracker_property_struct property;		//gps��������
	kal_uint32 latitude;		//γ��
	kal_uint32 longitude;		//����
	kal_uint16 speed;			//�ٶ�
	kal_uint16 course;			//����

	kal_uint8 reserv_sigstren;	//ǰ4bit��վ���ݱ������� ��4bit��վ�ź�ǿ��0-15
	kal_uint16 mcc;			//mcc
	kal_uint8  mnc;			//mnc
	kal_uint16 lac_sid;		//lac
	kal_uint16 cellid_nid;		//cellid
	kal_uint16 bid;		    //�ݱ���
}gps_tracker_gps_req_content_struct;

/********* 0x03 status ��****************/
typedef struct 
{	
	kal_uint8 oil_pwr_state:1;		//���͵�״̬ 0 �͵��ͨ 1 ���Ͷϵ�
	kal_uint8 sos_state:1;			//sos ״̬ 0 ��sos  1 ��sos
	kal_uint8 volt_level:3;        //��ѹ�ȼ� 0-6
	
	kal_uint8 temp;				//���϶��¶ȣ���δʵ��
}gps_tracker_status_req_content_struct;

/********* 0x04 hb ��****************/
//������������
/********* 0x05 �澯 ��****************/
typedef struct 
{	
	kal_uint8 type;			//�澯����
	kal_uint8 value_len;		//ֵ�ĳ�������
	kal_uint8 value[6];	//�澯ֵ
}gps_tracker_alarm_req_content_struct;

typedef  struct
{
	//ϵͳ����
	kal_int8 	ver[MAX_GT_VER_STR_LEN];
	kal_int8  dev_type;
	
	//��������
	kal_int8 vibr2_thr;	//�����𶯱���
	kal_uint8 vibr_thr;	
	kal_uint16 speed_thr;

	kal_int16 time_zone; //����24ʱ��	
	gps_tracker_switch_struct alarm_switch;		//��������
	kal_int8 defence; 					//0 ���� 1���
} gps_tracker_config_struct;

/********* 0x06 ���� ��****************/
typedef struct 
{	
	kal_uint8 type;			//��������
	kal_uint8 value_len;		//������������ ����
	kal_uint8 value[16];	//��������ֵ
}gps_tracker_data_content_struct;

/****************************0x07��ȡ���������ݰ�**************************/
typedef struct
{
	kal_uint8 addr;		//0x1a �綯����������0x1b���վ��������0x1c��Ƭ�������ǿ���
	kal_uint8 value_len;
	kal_uint8 value[64];
}gps_tracker_control_data_struct;


typedef struct 
{
	kal_uint8 datetime[6];
	kal_uint8 state;
	kal_uint32 latitude;
	kal_uint8 lat_ind;
	kal_uint32 longitude;
	kal_uint8 long_ind;
	kal_uint16 speed;
	kal_uint16 course;
	kal_uint16 magnetic_value;
	kal_uint8 magnetic_ind;
	kal_uint8 mode;//A ���� D ��� E���� N ������Ч

	kal_uint8 sat_uesed;
	kal_uint16 msl_altitude;
	kal_uint16 hdop;
}gps_tracker_gps_struct;

/****************************0x08�������ݰ�******************************/
typedef struct
{
	kal_uint8 reserv_satnum;	//ǰ��λgps�������� + ����λ������
	gps_tracker_property_struct property;		//gps��������
	kal_uint32 latitude;		//γ��
	kal_uint32 longitude;		//����
	kal_uint16 speed;			//�ٶ�
	kal_uint16 course;			//����
}gps_tracker_slim_struct;

typedef struct
{
	kal_uint8 lock_state;
	kal_uint8 gps_data_num;	//max 5
	gps_tracker_slim_struct gps_array[5];
}gps_tracker_give_back_struct;
/****************************0x09��վ���ݰ�******************************/
typedef struct
{
	kal_uint16 mcc;
	kal_uint8 mnc;
	kal_uint16 lac;
	kal_uint16 cellid;
	kal_uint8 sig;
}lbs_cell_struct;
typedef struct
{
	lbs_cell_struct service;
	kal_uint8 nbr_num;
	lbs_cell_struct nbr[6];
}gps_tracker_lbs_struct;

typedef struct
{
	kal_uint8 lat[4];
	kal_uint8 lon[4];
}gps_tracker_lbs_gps_struct;
#pragma pack ()//ǿ���ֽڶ���


extern gps_info_struct kfd_gps_data_array[];
extern gps_tracker_config_struct gps_tracker_config;
extern void kfd_upload_login_package(void);
extern void kfd_free_connect(void);
extern void kfd_protocol_parse(kal_int8 socket_id,RcvDataPtr GetRcvData);
extern void kfd_connect_service(void);
extern void kfd_protocol_init(void);
extern void kfd_upload_give_back_package(kal_uint8 gate);
extern void kfd_reconnect_service(void);
extern kal_uint16 get_crc16(kal_uint8* bytes, kal_uint16 len);
extern void zt_hex_convert_str(kal_uint8 *in,kal_uint8 len, kal_uint8 *out);
extern kal_bool GetNetworkService(void);
extern kal_bool kfd_convert_gps_data_for_protocol(gps_info_struct* gps_data, gps_tracker_gps_struct* kfd_gps_data);
extern gps_info_struct* kfd_get_best_hdop_gps_data(void);

#endif
