#include "zt_trace.h"
#include "zt_mtk_type.h"
#include "kfd_protocol.h"
#include "zt_smart_control.h"
#include "zt_gsensor.h"
#include "zt_agps.h"
#include "zt_interface.h"


#define HB_INTERVAL 50	// 50s	
#define DATA_INTERVAL 5	//5s	
#define GT_VER "SW2.0.00_"
#define PACKET_FRAME_LEN (sizeof(gps_tracker_msg_head_struct) + sizeof(gps_tracker_msg_tail_struct))


work_state kfd_work_state = EN_INIT_STATE;
kal_uint8 kfd_hb_send_times = 0;
static kal_uint8 kfd_connect_times = 0;

kal_uint16 kfd_sn = 0;

gps_tracker_config_struct gps_tracker_config;

gps_info_struct kfd_gps_data_array[DATA_INTERVAL];

kal_int8 kfd_soc_app_id;
#ifdef __MEILING__
network_para_struct kfd_network_para ={
	CONNECT_LONG,
	{
	 2,	// 1 ip; 2 domain
	"rentma.mlddc.com",	//domain	
	{139,224,3,220},	//{14,215,133,125},	//{139,224,67,207},	//ip 	
	4,		//ip len
	9000			//port
	},
	NULL,	//kfd_upload_login_package,
	NULL, 	//kfd_free_connect,
	NULL,	//kfd_protocol_parse
};
#else
network_para_struct kfd_network_para ={
	CONNECT_LONG,
	{
	 2,	// 1 ip; 2 domain
	"www.liabar.com",	//domain	//"rentma.bat100.com"
	{139,224,3,220},	//{14,215,133,125},	//{139,224,67,207},	//ip 	
	4,		//ip len
	9000			//port
	},
	NULL,	//kfd_upload_login_package,
	NULL, 	//kfd_free_connect,
	NULL,	//kfd_protocol_parse
};
#endif
static kal_uint16 CRC16_TABLE[] = {
	0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf,
	0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7,
	0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e,
	0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed, 0xcb64, 0xf9ff, 0xe876,
	0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd,
	0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5,
	0x3183, 0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c,
	0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974,
	0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9, 0x2732, 0x36bb,
	0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3,
	0x5285, 0x430c, 0x7197, 0x601e, 0x14a1, 0x0528, 0x37b3, 0x263a,
	0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72,
	0x6306, 0x728f, 0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9,
	0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1,
	0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738,
	0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70,
	0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7,
	0x0840, 0x19c9, 0x2b52, 0x3adb, 0x4e64, 0x5fed, 0x6d76, 0x7cff,
	0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036,
	0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e,
	0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5,
	0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd,
	0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226, 0xd0bd, 0xc134,
	0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c,
	0xc60c, 0xd785, 0xe51e, 0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3,
	0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb,
	0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232,
	0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68, 0x3ff3, 0x2e7a,
	0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1,
	0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb, 0x0e70, 0x1ff9,
	0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330,
	0x7bc7, 0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0x0f78
};	

/*****************************************************************************
 * FUNCTION
 *	GetCRC
 * DESCRIPTION
 *	获取字节数组的 CRC
 * PARAMETERS
 *	void
 * RETURNS
 *	void
 *****************************************************************************/
kal_uint16 get_crc16(kal_uint8* bytes, kal_uint16 len)
{	
	kal_uint16 value = 0xffff;
	int i;

	if (bytes == NULL)
		return 0;

	
	for (i = 0; i < len; i++) {
		value = (value >> 8) ^ CRC16_TABLE[(value ^ bytes[i]) & 0xff];			
	}
	
	return ~value;
}

void zt_hex_convert_str(kal_uint8 *in,kal_uint8 len, kal_uint8 *out)
{
	kal_uint16 i;
	kal_uint8 high,low;

	for(i=0; i<len; i++)
	{
		high = *(in+i)/16;
		low = *(in+i)%16;
		
		if(high>=0 && high<=9)
			*(out+2*i) = high+48;
		else if(high >=10 && high<=15)
			*(out+2*i) = high+55;

		if(low>=0 && low<=9)
			*(out+2*i+1) = low+48;
		else if(low >=10 && low<=15)
			*(out+2*i+1) = low+55;
	}
}

static double GPS_ToRad(double delta)
{
	return delta * 3.1415926 / 180;
}
static double GetDistance(double lat1, double lon1, double lat2, double lon2)
{
	double distance=0;
	double R = 6371.01;

	double dLat = GPS_ToRad(lat2 - lat1);
	double dLon = GPS_ToRad(lon2 - lon1);

	double a = sin(dLat / 2) * sin(dLat / 2) + cos(GPS_ToRad(lat1)) * cos(GPS_ToRad(lat2)) * sin(dLon / 2) * sin(dLon / 2);

	double c = 2 * atan2(sqrt(a), sqrt(abs(1 - a)));

	distance = ceil(R * c * 1000); 

	return distance;
}

void kfd_reconnect_service(void)
{
	zt_trace(TPROT,"%s,times=%d",__func__,kfd_connect_times);
	if(kfd_connect_times > 60)
	{
		kfd_connect_times = 0;
		zt_reset_system();
	}
	else
	{
		kfd_connect_times++;
		zt_socket_free(kfd_soc_app_id);
		kfd_connect_service();
	}
	StartTimer(GetTimerID(ZT_ONLINE_CHECK_PROTECT_TIMER), 60000, kfd_reconnect_service);
}
/*****************************************************************************
 * FUNCTION
 *  kfd_datetime_offset
 * DESCRIPTION
 *  时区转换
 * PARAMETERS
 *  void  
 *  
 * RETURNS
 *  void
 *****************************************************************************/
static void kfd_datetime_offset(applib_time_struct* datetime, S16 offset)
{
	applib_time_struct to_incre = {0};
	applib_time_struct date_offset = {0};
	applib_time_struct result = {0};
	
	memcpy(&to_incre, datetime, sizeof(applib_time_struct));
	
	//注意时区的正负
	if( (kal_uint16)offset < 32768)//正数
	{
		date_offset.nHour = offset/60;
		date_offset.nMin = offset%60;
		applib_dt_increase_time(&to_incre, &date_offset, &result);
	}
	else
	{
		date_offset.nHour = (65536 -(kal_uint16)offset)/60;
		date_offset.nMin = (65536 - (kal_uint16)offset)%60;
		applib_dt_decrease_time(&to_incre, &date_offset, &result);
	}
	
	memcpy(datetime, &result, sizeof(applib_time_struct));
}


/*****************************************************************************
 * FUNCTION
 * kfd_get_sn
 * DESCRIPTION
 *获取sn序列号
 * PARAMETERS
 *	void
 *	
 * RETURNS
 *	void
 *****************************************************************************/
kal_uint16 kfd_get_sn(void)
{
	kfd_sn++;
	return kfd_sn;
}
/*****************************************************************************
 * FUNCTION
 *  kfd_format_cb_to_buffer
 * DESCRIPTION
 *  组包
 起始符2，校验码2，内容长度1，协议类型1，序列号2，	日期时间6，
 内容context_len,	结束符2
 * PARAMETERS
 *  void  
 *  
 * RETURNS
 *  kal_uint8
 *****************************************************************************/
kal_uint8 kfd_format_cb_to_buffer(GT_PROT_TYPE_EN prot_type, kal_uint8 *outbuf, kal_uint8 *context,kal_uint8 context_len)
{
	kal_uint16* crc = NULL;
	kal_uint8 crc_len,sum_len = 0;
	t_rtc rtc;
	applib_time_struct currtime;
	gps_tracker_msg_head_struct head;
	gps_tracker_msg_tail_struct tail;
	
	if(outbuf == NULL)
		return 0;
	head.start = 0xffff;
	head.pack_len = context_len;
	head.prot_type = prot_type;
	head.sn = kfd_get_sn();
	
	RTC_GetTime_(&rtc);
	head.datetime[0] = rtc.rtc_year;
	head.datetime[1] = rtc.rtc_mon;
	head.datetime[2] = rtc.rtc_day;
	head.datetime[3] = rtc.rtc_hour;
	head.datetime[4] = rtc.rtc_min;
	head.datetime[5] = rtc.rtc_sec;
	//zt_trace(TPROT, "%s,rtc format datetime %u/%u/%u %u:%u:%u", __func__,rtc.rtc_year, rtc.rtc_mon, rtc.rtc_day, rtc.rtc_hour, rtc.rtc_min, rtc.rtc_sec);

	tail.stop = 0x0a0d;
	//包头
	memcpy(outbuf,&head,sizeof(gps_tracker_msg_head_struct));
	//包内容
	if(context && context_len>0)
		memcpy(outbuf+sizeof(gps_tracker_msg_head_struct),context,context_len);
	//包尾
	memcpy(outbuf+sizeof(gps_tracker_msg_head_struct)+context_len,&tail,sizeof(gps_tracker_msg_tail_struct));
	
	crc = (kal_uint16*)(outbuf + 2);
	crc_len = 	sizeof(gps_tracker_msg_head_struct)+context_len-4;	
	*crc = get_crc16(outbuf+4, crc_len);
	
	sum_len = sizeof(gps_tracker_msg_head_struct)+context_len+sizeof(gps_tracker_msg_tail_struct);		
	
	return sum_len;
}
/*****************************************************************************
 * FUNCTION
 *  kfd_send_package
 * DESCRIPTION
 *  所有上传包的发送接口
 * PARAMETERS
 * RETURNS
 *  kal_int32 len
 *****************************************************************************/
kal_int32 kfd_send_package(GT_PROT_TYPE_EN prot_type, kal_uint8* context,kal_uint8 context_len)
{
	kal_uint16 len;
	kal_int32 send_len=0;
	kal_uint8 buffer[MAX_GT_SEND_LEN] = {0}; 
	kal_uint8 out[MAX_GT_SEND_LEN] ={0};
	
	len = kfd_format_cb_to_buffer(prot_type, buffer, context, context_len);	
 
	if((kfd_work_state==EN_WORKING_STATE ||kfd_work_state==EN_LOGING_STATE)&&len >0)
	{
		send_len = zt_socket_send(kfd_soc_app_id,buffer,len);
		
		if(send_len != len)
			kfd_reconnect_service();
	}
	
	zt_hex_convert_str(buffer,len, out);
	zt_trace(TPROT,"%s, GT_PROT_TYPE_EN=%d,sig=%d",__func__,prot_type,(U8)srv_nw_info_get_signal_strength_in_percentage(MMI_SIM1));
	zt_trace(TPROT,"%s",out);
	return send_len;
}

/*****************************************************************************
 * FUNCTION
 * kfd_get_gps_data_per_period
 * DESCRIPTION
 *获取最近一个上传周期的GPS数据到数组中?
 * PARAMETERS
 * RETURNS
 * void
 *****************************************************************************/
void kfd_get_gps_data_per_period(void)
{
	kal_uint8 i;
	gps_info_struct* curr_gps_data = (gps_info_struct* )zt_gps_get_curr_gps();

	for(i=DATA_INTERVAL-1; i>=1; i--)
	{
		memcpy(&kfd_gps_data_array[i], &kfd_gps_data_array[i-1], sizeof(gps_info_struct));
	}
	
	memcpy(&kfd_gps_data_array[0], curr_gps_data, sizeof(gps_info_struct));	
	if(curr_gps_data->state!=0)
	zt_trace(TGPS,"valid=%c,use=%d,view=%d,speed=%f",curr_gps_data->state,curr_gps_data->sat_uesd,curr_gps_data->sat_view,curr_gps_data->speed);

	StartTimer(GetTimerID(ZT_GPS_PERIOD_TIMER), 1000, kfd_get_gps_data_per_period);
}

/*****************************************************************************
 * FUNCTION
 * kfd_get_best_hdop_gps_data
 * DESCRIPTION
 * 获取最近一个上传周期的GPS数据最小hdop的gps数据
 * PARAMETERS
 *
 * RETURNS
 * gps_info_struct* 
 *****************************************************************************/
gps_info_struct* kfd_get_best_hdop_gps_data(void)
{
	kal_uint8 i;
	kal_uint8 index=0xff;
	double hdop = 999.9;
	
	for(i=0; i<DATA_INTERVAL; i++)
	{
		if(kfd_gps_data_array[i].state=='A' && kfd_gps_data_array[i].hdop < hdop)
		{
			index = i;
			hdop = kfd_gps_data_array[i].hdop;
		}
	}

	//zt_trace(TGPS,"%s,index=%d,hdop=%f",__func__,index,hdop);
	if(index == 0xff)
	{
		return NULL;
	}
	else
	{
		return &kfd_gps_data_array[index];
	}
}

/*****************************************************************************
 * FUNCTION
 * kfd_lat_long_convert
 * DESCRIPTION
 * 转换原始经纬度为kfd协议规范的经纬度值
 * PARAMETERS
 * double 
 * RETURNS
 * kal_uint32
 *****************************************************************************/
kal_uint32 kfd_lat_long_convert(double latorlong)
{
	kal_uint32 sum;
	sum = latorlong*1800000;
	
	//zt_trace(TGPS,"%s,%f,0x%x",__func__,latorlong,sum);
	return (kal_uint32)sum;
}
/*****************************************************************************
 * FUNCTION
 * kfd_convert_gps_data_for_protocol
 * DESCRIPTION
 * 转换GPS原始数据为kfd协议格式数据
 * PARAMETERS
 *
 * RETURNS
 * kal_bool
 *****************************************************************************/
kal_bool kfd_convert_gps_data_for_protocol(gps_info_struct* gps_data, gps_tracker_gps_struct* kfd_gps_data)
{
	if(!gps_data)
		return KAL_FALSE;
	
	//zt_trace(TPROT,"%s",__func__);

	kfd_gps_data->datetime[0] = (kal_uint8)(gps_data->dt.nYear%100);
	kfd_gps_data->datetime[1] = gps_data->dt.nMonth;
	kfd_gps_data->datetime[2] = gps_data->dt.nDay;
	kfd_gps_data->datetime[3] = gps_data->dt.nHour;
	kfd_gps_data->datetime[4] = gps_data->dt.nMin;
	kfd_gps_data->datetime[5] = gps_data->dt.nSec;

	if(gps_data->state == 'A')
		kfd_gps_data->state =EN_GT_GS_A;
	else if(gps_data->state == 'V')
		kfd_gps_data->state = EN_GT_GS_V;
	else
		kfd_gps_data->state = EN_GT_GS_INV;

	kfd_gps_data->latitude = kfd_lat_long_convert(gps_data->latitude);
	if(gps_data->NS== 'N' )
		kfd_gps_data->lat_ind= EN_GT_NORTH;
	else if(gps_data->NS == 'S')
		kfd_gps_data->lat_ind = EN_GT_SOUTH;
	else
		kfd_gps_data->lat_ind = EN_GT_INV;
	
	kfd_gps_data->longitude = kfd_lat_long_convert(gps_data->longitude);
	if(gps_data->EW == 'E' )
		kfd_gps_data->long_ind= EN_GT_EAST;
	else if(gps_data->EW == 'W')
		kfd_gps_data->long_ind = EN_GT_WEST;
	else
		kfd_gps_data->long_ind = EN_GT_INV;

	kfd_gps_data->speed = gps_data->speed*1.852*100;	//节转换为公里/小时再乘以100
	kfd_gps_data->course = (kal_uint16)(gps_data->angle*100);
	kfd_gps_data->magnetic_value = (kal_uint16)(gps_data->magnetic_value*100);

	if(gps_data->magnetic_ind == 'E' )
		kfd_gps_data->magnetic_ind= EN_GT_EAST;
	else if(gps_data->magnetic_ind == 'W')
		kfd_gps_data->magnetic_ind = EN_GT_WEST;
	else
		kfd_gps_data->magnetic_ind = EN_GT_INV;

	if(gps_data->mode == 'A' )
		kfd_gps_data->mode= EN_GT_GM_A;
	else if(gps_data->mode == 'D')
		kfd_gps_data->mode = EN_GT_GM_D;
	else if(gps_data->mode == 'E')
		kfd_gps_data->mode = EN_GT_GM_E;
	else if(gps_data->mode == 'N')
		kfd_gps_data->mode = EN_GT_GM_N;
	else
		kfd_gps_data->mode = EN_GT_GM_INV;

	kfd_gps_data->sat_uesed = gps_data->sat_uesd;
	kfd_gps_data->msl_altitude = gps_data->altitude*10;	//分米
	kfd_gps_data->hdop = gps_data->hdop*100;

	//zt_trace(TPROT,"lat=%d,long=%d,spd=%d,hdop=%d",kfd_gps_data->latitude,kfd_gps_data->longitude,kfd_gps_data->speed,kfd_gps_data->hdop);
	return KAL_TRUE;
}
/*****************************************************************************
 * FUNCTION
 *  kfd_free_connect
 * DESCRIPTION
 * 网络断开时释放运行机制
 * PARAMETERS
 * RETURNS
 *  void
 *****************************************************************************/
void kfd_free_connect(void)
{
	kfd_work_state = EN_INIT_STATE;
//	StopTimer(GetTimerID(ZT_HB_TIMER));
	StopTimer(GetTimerID(ZT_UPLOAD_TIMER));
	zt_led_close_gsm_led();
}

/*****************************************************************************
 * FUNCTION
 *	hex_str_2_bytes
 * DESCRIPTION
 *	将16进制的串转换成字节数组
 * PARAMETERS
 *	void
 * RETURNS
 *	void
 *****************************************************************************/
kal_uint32 hex_str_2_bytes(kal_char* str, kal_uint32 str_len, kal_uint8* bytes, kal_uint32 bytes_len)
{
	kal_uint32 i;
	kal_uint8 hex = 0;
	kal_uint8 offset = 0;

	if (str == NULL || bytes == NULL)
		return KAL_FALSE;
	
	if((str_len +1) /2 > bytes_len)
	{
		return KAL_FALSE;
	}
	
	if (str_len%2 != 0)
	{
		offset = 1;
	}

	for(i =0; i < str_len; i++)
	{
		//判断hex 字符串的有效性
		if(str[i] >= '0' && str[i] <= '9')
		{
			hex = str[i] - '0';
		}
		else if( str[i] >= 'a' && str[i] <= 'z')
		{
			hex = str[i] - 'a';
		}
		else if(str[i] >= 'A' && str[i] <= 'Z')
		{
			hex = str[i] - 'A';
		}
		else
		{
			return KAL_FALSE;
		}

		bytes[(i + offset)/2] += hex << 4*((i + offset + 1)%2);
	}

	return KAL_TRUE;
}


/*****************************************************************************
 * FUNCTION
 *  kfd_upload_login_package
 * DESCRIPTION
 * 登录包上报处理
 * PARAMETERS
 * RETURNS
 *  void
 *****************************************************************************/
void kfd_upload_login_package(void)
{
	gps_tracker_login_req_content_struct login_package={0};
	kal_uint8 imei[MAX_GT_IMEI_LEN]={0};

	srv_imei_get_imei(MMI_SIM1, imei, MAX_GT_IMEI_LEN);
	zt_trace(TPROT,"%s,dev_id=%s",__func__,imei);
	login_package.dev_type = gps_tracker_config.dev_type;
	login_package.auth_code = 0;
	hex_str_2_bytes(imei, strlen(imei), login_package.dev_id, 8);
	kfd_work_state = EN_LOGING_STATE;
	kfd_send_package(EN_GT_PT_LOGIN,(kal_uint8*)&login_package,sizeof(gps_tracker_login_req_content_struct));
}
/*****************************************************************************
 * FUNCTION
 *  kfd_upload_hb_package
 * DESCRIPTION
 * 心跳包上报处理
 * PARAMETERS
 * RETURNS
 *  void
 *****************************************************************************/
void kfd_upload_hb_package(void)
{
	//zt_trace(TPROT,"%s,kfd_hb_send_times=%d",__func__,kfd_hb_send_times);
	
	kfd_send_package(EN_GT_PT_HB,NULL,0);

	if(kfd_hb_send_times>=2)
	{
		kfd_hb_send_times = 0;
		kfd_reconnect_service();
	}
	else
	{
		kfd_hb_send_times++;
		StartTimer(GetTimerID(ZT_HB_TIMER), HB_INTERVAL*1000, kfd_upload_hb_package);
	}
}

void kfd_upload_give_back_package(kal_uint8 gate)
{
	kal_uint8 i,num=0;
	gps_tracker_give_back_struct give_back_package;
	gps_tracker_gps_struct gps_tracker_gps;
	kal_uint8 len;

/*增加还车时判断主电源如没插上就提示失败*/
	if(zt_get_bat_connect_status())	
		give_back_package.lock_state = gate>0?0:1;
	else
		give_back_package.lock_state = 0;
	
	zt_trace(TPROT,"lock_state=%d,gate=%d",give_back_package.lock_state,gate);

	for(i=0;i<DATA_INTERVAL;i++)
	{
		if(kfd_gps_data_array[i].state=='A')
		{
			memset(&gps_tracker_gps,0,sizeof(gps_tracker_gps_struct));
			kfd_convert_gps_data_for_protocol(&kfd_gps_data_array[i],&gps_tracker_gps);
			
			give_back_package.gps_array[num].latitude = gps_tracker_gps.latitude;
			give_back_package.gps_array[num].longitude = gps_tracker_gps.longitude;
			give_back_package.gps_array[num].speed = gps_tracker_gps.speed;					
			give_back_package.gps_array[num].course= gps_tracker_gps.course;
			give_back_package.gps_array[num].reserv_satnum = gps_tracker_gps.sat_uesed;
			if(gps_tracker_gps.lat_ind == EN_GT_SOUTH)
			{
				give_back_package.gps_array[num].property.lat_ind = 0;	
			}
			else if(gps_tracker_gps.lat_ind == EN_GT_NORTH)
			{
				give_back_package.gps_array[num].property.lat_ind = 1;
			}

			if(gps_tracker_gps.long_ind == EN_GT_WEST)
			{
				give_back_package.gps_array[num].property.long_ind = 0;	
			}
			else if(gps_tracker_gps.long_ind == EN_GT_EAST)
			{
				give_back_package.gps_array[num].property.long_ind = 1;
			}

			if(gps_tracker_gps.mode == EN_GT_GM_A)
			{
				give_back_package.gps_array[num].property.mode = 0;	
			}
			else if(gps_tracker_gps.mode == EN_GT_GM_D)
			{
				give_back_package.gps_array[num].property.mode = 1;
			}
			else if(gps_tracker_gps.mode == EN_GT_GM_E)
			{
				give_back_package.gps_array[num].property.mode = 2;
			}
			else if(gps_tracker_gps.mode == EN_GT_GM_N)
			{
				give_back_package.gps_array[num].property.mode = 3;
			}
		
			num++;
		}
	}
	give_back_package.gps_data_num = num;
	len = 2+sizeof(gps_tracker_slim_struct)*num;
	
	kfd_send_package(EN_GT_PT_GIVE_BACK,(kal_uint8*)&give_back_package, len);
}

kal_bool check_gps_upload_valid(gps_info_struct* gps_info)
{
	if(!zt_gsensor_check_is_moving())
	{
		if(gps_info->hdop>5.5)
		{
			return KAL_FALSE;
		}
	}

	return KAL_TRUE;
}
/*****************************************************************************
 * FUNCTION
 *  kfd_upload_gps_package
 * DESCRIPTION
 * GPS包上报
 * PARAMETERS
 * RETURNS
 *  void
 *****************************************************************************/
kal_bool kfd_upload_gps_package(void)
{
	lbs_info_struct* lbs_info= (lbs_info_struct*)zt_lbs_get_curr_lbs_info();
	gps_info_struct* gps_info = NULL;
	gps_tracker_gps_struct gps_tracker_gps = {0};
	gps_tracker_gps_req_content_struct gps_package;

	//zt_trace(TPROT,"%s",__func__);
	
	gps_info = kfd_get_best_hdop_gps_data();
	if(!gps_info)
		return KAL_FALSE;
	if(!check_gps_upload_valid(gps_info))
		return KAL_FALSE;
	
	kfd_convert_gps_data_for_protocol(gps_info,&gps_tracker_gps);

	if(gps_tracker_gps.mode!= EN_GT_GM_N)
	{				
		//定位类型GPS
		gps_package.loc_type = EN_GT_LT_GPS;
		gps_package.latitude = gps_tracker_gps.latitude;
		gps_package.longitude = gps_tracker_gps.longitude;
		gps_package.speed = gps_tracker_gps.speed;					
		
		//航向
		gps_package.course= gps_tracker_gps.course;
		//可用卫星
		gps_package.reserv_satnum = (kal_uint8)gps_tracker_gps.sat_uesed;

		if(gps_tracker_gps.lat_ind == EN_GT_SOUTH)
		{
			gps_package.property.lat_ind = 0;	
		}
		else if(gps_tracker_gps.lat_ind == EN_GT_NORTH)
		{
			gps_package.property.lat_ind = 1;
		}

		if(gps_tracker_gps.long_ind == EN_GT_WEST)
		{
			gps_package.property.long_ind = 0;	
		}
		else if(gps_tracker_gps.long_ind == EN_GT_EAST)
		{
			gps_package.property.long_ind = 1;
		}

		if(gps_tracker_gps.mode == EN_GT_GM_A)
		{
			gps_package.property.mode = 0;	
		}
		else if(gps_tracker_gps.mode == EN_GT_GM_D)
		{
			gps_package.property.mode = 1;
		}
		else if(gps_tracker_gps.mode == EN_GT_GM_E)
		{
			gps_package.property.mode = 2;
		}
		else if(gps_tracker_gps.mode == EN_GT_GM_N)
		{
			gps_package.property.mode = 3;
		}
	
		if(lbs_info)
		{
			gps_package.reserv_sigstren = (kal_uint8)lbs_info->lbs_server.sig_stren;
			gps_package.mcc =  lbs_info->lbs_server.mcc;
			gps_package.mnc =  lbs_info->lbs_server.mnc;
			gps_package.lac_sid =  lbs_info->lbs_server.lac_sid;
			gps_package.cellid_nid =  lbs_info->lbs_server.cellid_nid;
		}

		gps_package.bid = zt_gsensor_get_curr_shake_value();
		//zt_trace(TPROT, "%s,lat %u;long %u;speed %u;lac %u;cell_id %u", __func__,gps_package.latitude,gps_package.longitude,gps_package.speed,gps_package.lac_sid,gps_package.cellid_nid);
		
		kfd_send_package(EN_GT_PT_GPS,(kal_uint8*)&gps_package, sizeof(gps_tracker_gps_req_content_struct));
		return KAL_TRUE;
	}
	
	return KAL_FALSE;
}

/*****************************************************************************
 * FUNCTION
 *  kfd_upload_alarm_package
 * DESCRIPTION
 * 告警包上报
 * PARAMETERS
 * RETURNS
 *  void
 *****************************************************************************/
void kfd_upload_alarm_package(void)
{
	kal_uint8 ind;
	kal_uint8 i = 0;
	kal_uint8 is_update = 0;
	gps_tracker_alarm_req_content_struct alarm_package={0};
	kal_uint8 package_len = 0;
	gps_tracker_alarm_struct 	gps_tracker_alarm = {0};
	kal_uint8 shake_value = zt_gsensor_get_curr_shake_value();
	gps_info_struct* curr_gps_data = (gps_info_struct*)zt_gps_get_curr_gps();
	kal_uint16 curr_speed = (kal_uint16)(curr_gps_data->speed*1.852);
	
	//zt_trace(TPROT,"%s",__func__);

//速度
	if(curr_speed >= gps_tracker_config.speed_thr)
	{
		gps_tracker_alarm.speed_ind = 1;
		gps_tracker_alarm.speed = curr_speed*100;
	}
	else
	{
		gps_tracker_alarm.speed_ind = 0;
	}
//震动	
	if(shake_value >= gps_tracker_config.vibr_thr && !get_electric_gate_status())	
	{
		//连续探测到N次震动，才告警
		if(zt_gsensor_check_is_shake_sharp())
		{
			gps_tracker_alarm.vibr_ind = 1;
			gps_tracker_alarm.vibr_value = shake_value;
		}
	}
	else
	{
		gps_tracker_alarm.vibr_ind = 0;
	}
//断电
	if(zt_get_bat_connect_status())	
	{
		gps_tracker_alarm.pwr_off_ind =0;
	}
	else
	{
		gps_tracker_alarm.pwr_off_ind =1;
	}
		
	ind = gps_tracker_alarm.pwr_low_ind << EN_GT_AT_PWR_LOW | (gps_tracker_alarm.pwr_off_ind << EN_GT_AT_PWR_OFF) 
			|(gps_tracker_alarm.vibr_ind << EN_GT_AT_VIBR)|(gps_tracker_alarm.oil_pwr_ind << EN_GT_AT_OIL_PWR)
			|(gps_tracker_alarm.speed_ind << EN_GT_AT_SPEED);
	
	for(i = 0; i < EN_GT_AT_END; i++)
	{	
		if(ind & (0x01<<i))
		{	
			zt_trace(TPROT, "%s,[alarm] alarm No:%d",__func__,i);
			if(i == EN_GT_AT_VIBR)
			{
				if(gps_tracker_config.alarm_switch.vibr == EN_GT_SWT_ON)
				{					
					alarm_package.type = EN_GT_AT_VIBR;
					alarm_package.value_len = sizeof(gps_tracker_alarm.vibr_value);
					alarm_package.value[0] = gps_tracker_alarm.vibr_value;
					is_update = KAL_TRUE;
				}

				//本次数据已经处理完毕，清除数据标志
				gps_tracker_alarm.vibr_ind = 0;
				gps_tracker_alarm.vibr_value = 0;
			}	
			else if(i == EN_GT_AT_SPEED)
			{
				if(gps_tracker_config.alarm_switch.speed == EN_GT_SWT_ON)	
				{					
					alarm_package.type = EN_GT_AT_SPEED;
					alarm_package.value_len = sizeof(gps_tracker_alarm.speed);
					alarm_package.value[0] = (gps_tracker_alarm.speed>>8)&0xff;
					alarm_package.value[1] = gps_tracker_alarm.speed&0xff;
					is_update = KAL_TRUE;
				}

				//本次数据已经处理完毕，清除数据标志
				gps_tracker_alarm.speed_ind = 0;
				gps_tracker_alarm.speed = 0;
			}	
			else if(i == EN_GT_AT_PWR_LOW)
			{
				if(gps_tracker_config.alarm_switch.pwr_low == EN_GT_SWT_ON)
				{					
					alarm_package.type = EN_GT_AT_PWR_LOW;
					alarm_package.value_len = sizeof(gps_tracker_alarm.pwr_level);
					alarm_package.value[0] = gps_tracker_alarm.pwr_level;
					is_update = KAL_TRUE;
				}

				//本次数据已经处理完毕，清除数据标志
				gps_tracker_alarm.pwr_low_ind = 0;
				gps_tracker_alarm.pwr_level = 0;
			}
			else if(i == EN_GT_AT_PWR_OFF)
			{
				if(gps_tracker_config.alarm_switch.pwr_off== EN_GT_SWT_ON)
				{					
					alarm_package.type = EN_GT_AT_PWR_OFF;
					alarm_package.value_len = 0;
					is_update = KAL_TRUE;
				}

				//本次数据已经处理完毕，清除数据标志
				gps_tracker_alarm.pwr_off_ind= 0;
			}
			else if(i == EN_GT_AT_OIL_PWR)
			{
				if(gps_tracker_config.alarm_switch.oil_pwr == EN_GT_SWT_ON)
				{					
					alarm_package.type = EN_GT_AT_OIL_PWR;
					alarm_package.value_len = 0;
					is_update = KAL_TRUE;
				}

				//本次数据已经处理完毕，清除数据标志
				gps_tracker_alarm.oil_pwr_ind= 0;
			}
			else
			{
				is_update = KAL_FALSE;
			}
			
			if(is_update)
			{
				is_update = KAL_FALSE;
				package_len = alarm_package.value_len+2;
				kfd_send_package(EN_GT_PT_ALARM, (kal_uint8*)&alarm_package, package_len);
			}	
		}		
	}	
}

void kfd_pre_ebike_package_data(void)
{
	zt_smart_pre_uart_data();
}
/*****************************************************************************
 * FUNCTION
 *  kfd_upload_ebike_package
 * DESCRIPTION
 * 上传电动车数据包
 * PARAMETERS
 * RETURNS
 *  void
 *****************************************************************************/
void kfd_upload_ebike_package(void)
{
	gps_tracker_control_data_struct control_package;
	kal_uint8 package_len;
	
	//zt_trace(TPROT, "%s",__func__);
	zt_smart_update_network_data(&control_package);
	package_len = control_package.value_len + 2;
	kfd_send_package(EN_GT_PT_CONTROL, (kal_uint8*)&control_package, package_len);
}

/*****************************************************************************
 * FUNCTION
 *  kfd_upload_lbs_package
 * DESCRIPTION
 * 上传基站位置数据包
 * PARAMETERS
 * RETURNS
 *  void
 *****************************************************************************/
void kfd_upload_lbs_package(void)
{
	kal_uint8 i;
	gps_tracker_lbs_struct lbs_package;
	kal_uint8 package_len;
	lbs_info_struct* lbs_info =  (lbs_info_struct*)zt_lbs_get_curr_lbs_info(); 

/*检测震动和轮动才上传*/
	if(!(zt_gsensor_check_is_moving()&&zt_smart_check_hall_is_run()))
		return;
	
	zt_trace(TPROT, "%s",__func__);
	lbs_package.service.mcc = lbs_info->lbs_server.mcc;
	lbs_package.service.mnc = lbs_info->lbs_server.mnc;
	lbs_package.service.lac = lbs_info->lbs_server.lac_sid;
	lbs_package.service.cellid = lbs_info->lbs_server.cellid_nid;
	lbs_package.service.sig = lbs_info->lbs_server.sig_stren;
	lbs_package.nbr_num = lbs_info->lbs_nbr_num;
	for(i=0; i<lbs_info->lbs_nbr_num; i++)
	{
		lbs_package.nbr[i].mcc = lbs_info->lbs_nbr[i].mcc;
		lbs_package.nbr[i].mnc = lbs_info->lbs_nbr[i].mnc;
		lbs_package.nbr[i].lac = lbs_info->lbs_nbr[i].lac_sid;
		lbs_package.nbr[i].cellid = lbs_info->lbs_nbr[i].cellid_nid;
		lbs_package.nbr[i].sig = lbs_info->lbs_nbr[i].sig_stren;	
	}
	package_len = sizeof(lbs_cell_struct)+1+sizeof(lbs_cell_struct)*lbs_info->lbs_nbr_num;
	
	kfd_send_package(EN_GT_PT_LBS, (kal_uint8*)&lbs_package, package_len);
}

/*****************************************************************************
 * FUNCTION
 *  kfd_upload_data_package
 * DESCRIPTION
 * 数据包上报总定时程序
 * PARAMETERS
 * RETURNS
 *  void
 *****************************************************************************/
void kfd_upload_data_package(void)
{
	static kal_uint8 delay_index = 0;

	//zt_trace(TPROT, "%s,delay_index=%d",__func__,delay_index);

	if((delay_index+1)%3==0)
	{
		kal_bool ret;
		ret = kfd_upload_gps_package();
		if(!ret&&(delay_index+1)%6==0)	//如果GPS无效就半分钟上传LBS
		{
			kfd_upload_lbs_package();
		}
	}

	if(delay_index%6==0)
	{
	//	zt_lbs_req();
	}
	else if((delay_index+1)%6==0)
	{
		kfd_upload_alarm_package();
		kfd_pre_ebike_package_data();
	}
	else if((delay_index+2)%6==0)
	{
		kfd_upload_ebike_package();
	}
	
	delay_index++;
	if(delay_index>252)
		delay_index = 0;
	
	StartTimer(GetTimerID(ZT_UPLOAD_TIMER), DATA_INTERVAL*1000, kfd_upload_data_package);
}

/*****************************************************************************
 * FUNCTION
 *  kfd_upload_imsi_package
 * DESCRIPTION
 * 上报SIM卡的IMSI号
 * PARAMETERS
 * RETURNS
 *  void
 *****************************************************************************/
void kfd_upload_imsi_package(void)
{	
	gps_tracker_data_content_struct data_package;
	
	data_package.type = EN_GT_DT_IMSI;
	data_package.value_len = strlen((kal_uint8*)zt_get_imsi());
	strcpy(data_package.value, (kal_uint8*)zt_get_imsi());
	zt_trace(TPROT, "IMSI=%s,len=%d",(kal_uint8*)zt_get_imsi(),data_package.value_len);	

	kfd_send_package(EN_GT_PT_DEV_DATA, (kal_uint8*)&data_package, data_package.value_len+2);				
}
/*****************************************************************************
 * FUNCTION
 *  kfd_upload_ver_package
 * DESCRIPTION
 * 更新版本数据包
 * PARAMETERS
 * RETURNS
 *  void
 *****************************************************************************/
void kfd_upload_ver_package(void)
{	
	gps_tracker_data_content_struct data_package;
	
	data_package.type = EN_GT_DT_VER;
	data_package.value_len = MAX_GT_VER_STR_LEN;	//strlen(gps_tracker_config.ver);
	strcpy(data_package.value, gps_tracker_config.ver);
	zt_trace(TPROT, "%s,send ver %s to server",__func__,gps_tracker_config.ver);	

	kfd_send_package(EN_GT_PT_DEV_DATA, (kal_uint8*)&data_package, data_package.value_len+2);				
}

/*****************************************************************************
 * FUNCTION
 *  kfd_upload_command_rsp_package
 * DESCRIPTION
 * 指令的响应包
 * PARAMETERS
 * RETURNS
 *  void
 *****************************************************************************/
void kfd_upload_command_rsp_package(kal_uint8* head)
{
	kal_uint8 rsp[20] = {0};
	kal_uint8 len = 0;
	
	len = sizeof(gps_tracker_msg_head_struct);
	memcpy(rsp, head, len);
	rsp[4] = 0;
	rsp[len++] = 0x0d;
	rsp[len++] = 0x0a;
	zt_socket_send(kfd_soc_app_id,rsp, len);
}
/*****************************************************************************
 * FUNCTION
 *  kfd_srv_data_req_proc
 * DESCRIPTION
 * dev_cfg  配置响应的处理函数，根据不同的消息类型回复不同的响应
 * PARAMETERS
 * RETURNS
 *  void
 *****************************************************************************/
void kfd_srv_data_req_proc(gps_tracker_data_content_struct* data)
{
	S16 error;
	S32 ret;

	if (data == NULL)
		return;
	
	switch(data->type)
	{					
		case EN_GT_DT_ADMIN_NUM:	
			break;
		case EN_GT_DT_PWD:
			break;
		case EN_GT_DT_USER:
			break;
		case EN_GT_DT_UP_INTV:
			break;
		case EN_GT_DT_HB_INTV:
			break;
		case EN_GT_DT_SMS_ALARM_INTV:
			break;
		case EN_GT_DT_TEMP_THR:
			break;
		case EN_GT_DT_VIBR_THR:
			gps_tracker_config.vibr_thr = data->value[0];	
			WriteRecord(GetNvramID(NVRAM_EF_GT_VIBR_THR_LID), 1, &gps_tracker_config.vibr_thr, sizeof(gps_tracker_config.vibr_thr), &error);
			//zt_trace(TPROT,"%s,EN_GT_DT_VIBR_THR,vibr_thr=%d",__func__,gps_tracker_config.vibr_thr);
			break;
		case EN_GT_DT_SPEED_THR:
			if(data->value_len==1)
				gps_tracker_config.speed_thr = data->value[0];
			else if(data->value_len==2)
				gps_tracker_config.speed_thr = data->value[0]*10+data->value[1];
			WriteRecord(GetNvramID(NVRAM_EF_GT_SPEED_THR_LID), 1, &gps_tracker_config.speed_thr, sizeof(gps_tracker_config.speed_thr), &error);
			//zt_trace(TPROT,"%s,EN_GT_DT_SPEED_THR,speed_thr=%d",__func__,gps_tracker_config.speed_thr);
			break;
		case EN_GT_DT_LANG:
			break;
		case EN_GT_DT_TIME_ZONE:
			break;
		case EN_GT_DT_ALARM_SWITCH:
			if(data->value[1]== 1)
			{
				*(U8*)&gps_tracker_config.alarm_switch |= (1<<data->value[0]);
			}
			else
			{
				*(U8*)&gps_tracker_config.alarm_switch &= ~(1<<data->value[0]);
			}
			WriteRecord(GetNvramID(NVRAM_EF_GT_ALARM_SWITCH_LID), 1, &gps_tracker_config.alarm_switch, sizeof(gps_tracker_config.alarm_switch), &error);
			//zt_trace(TPROT,"%s,EN_GT_DT_ALARM_SWITCH,alarm_switch=%d",__func__,gps_tracker_config.alarm_switch);
			break;
		case EN_GT_DT_SMS_ALARM_SWITCH:
			break;
		case EN_GT_DT_LOC:	
			break;
		case EN_GT_DT_IGNORE_ALARM:	
			break;
		case EN_GT_DT_LOG_LEVEL:	
			break;
		case EN_GT_DT_SET_TIME:	
			break;
		case EN_GT_DT_RESTORE:
			break;
		case EN_GT_DT_SMS:	
			break;
		case EN_GT_DT_DEFENCE:	
			gps_tracker_config.defence =  data->value[0];
			WriteRecord(GetNvramID(NVRAM_EF_GT_DEFENCE_LID), 1, &gps_tracker_config.defence, sizeof(gps_tracker_config.defence), &error);
			//zt_trace(TPROT,"%s,EN_GT_DT_DEFENCE,defence=%d",__func__,gps_tracker_config.defence);
			break;
		case EN_GT_DT_SERVER:
			break;
		case EN_GT_DT_SMS_CENTER:
			break;
		case EN_GT_DT_PWR_OIL_SWITCH:
			break;
		case EN_GT_DT_IO:
			break;
		default:
			break;
	}	
}
/*****************************************************************************
 * FUNCTION
 *  kfd_calibration_time
 * DESCRIPTION
 * 鏍″噯绯荤粺鏃堕棿
 * PARAMETERS
 * RETURNS
 *  void
 *****************************************************************************/
 void kfd_calibration_time(kal_uint8* buf)
{
	kal_uint8* date_time;
	applib_time_struct date;
	static kal_bool has_calibration = KAL_FALSE;

	date_time = ((kal_uint8*)buf + sizeof(gps_tracker_msg_head_struct) - 6);
	date.nYear= date_time[0] + YEARFORMATE;
	date.nMonth= date_time[1];
	date.nDay= date_time[2];
	date.nHour= date_time[3];
	date.nMin= date_time[4];
	date.nSec= date_time[5];

	mmi_dt_set_rtc_dt((MYTIME *)&date);
}
void kfd_dev_data_proc(gps_tracker_data_content_struct* data)
{
	S16 error;
	S32 ret;

	if (data == NULL)
		return;
	
	switch(data->type)
	{					
		case EN_GT_DT_VER:
			parse_ver_package(data->value,data->value_len);
			break;
		default:
			break;
	}
}

/*****************************************************************************
 * FUNCTION
 *  kfd_protocol_proc
 * DESCRIPTION
 * 包的处理
 * PARAMETERS
 * RETURNS
 * 错误码
 *****************************************************************************/
kal_int32 kfd_protocol_proc(kal_uint8* buf ,kal_uint16 len)
{	
	gps_tracker_msg_head_struct* head;
	kal_uint8 out[256];
	kal_uint16 crc1,crc2;

	if (buf == NULL)
		return KAL_FALSE;

	head = (gps_tracker_msg_head_struct*)buf;

	crc2 = buf[2]*0x100+buf[3];
	crc1 = get_crc16(buf+4, len-6);
	zt_hex_convert_str(buf,len, out);
	zt_trace(TPROT,"crc=%x, Recv=%s",crc1,out);
	if(crc1 != crc2)
	{
		zt_trace(TPROT,"check sum error");
		return KAL_FALSE;
	}

	kfd_hb_send_times = 0;
	switch(head->prot_type)
	{
		case EN_GT_PT_LOGIN:						
		{	
			zt_trace(TPROT, "login rsp sn ok");
			kfd_work_state = EN_WORKING_STATE;
			kfd_connect_times = 0;
			StopTimer(GetTimerID(ZT_ONLINE_CHECK_PROTECT_TIMER));

			kfd_calibration_time(buf);
			kfd_upload_ver_package();
			kfd_upload_imsi_package();
			zt_led_open_gsm_led();
			StartTimer(GetTimerID(ZT_HB_TIMER), HB_INTERVAL, kfd_upload_hb_package);
			StartTimer(GetTimerID(ZT_UPLOAD_TIMER), DATA_INTERVAL, kfd_upload_data_package);	
			break;																	
		}		
		case EN_GT_PT_GPS: 			
		{	
			zt_trace(TPROT, "gps rsp sn ok ");
			break;
		}
		case EN_GT_PT_STATUS: 			
		{	
			//zt_trace(TPROT, "status rsp sn ok ");
			break;
		}
		case EN_GT_PT_HB: 
		{	
			zt_trace(TPROT, "HB rsp sn ok");
			break; 
		}
		case EN_GT_PT_ALARM: 					
		{	
			//zt_trace(TPROT, "alarm rsp sn ok ");
			break;
		}		
		case EN_GT_PT_DEV_DATA:			
		{		
			//zt_trace(TPROT, "dev_data rsp sn ok ");
			gps_tracker_data_content_struct* data;
			data = (gps_tracker_data_content_struct*)((kal_uint8*)head + sizeof(gps_tracker_msg_head_struct));	
			kfd_dev_data_proc(data);
			break;						
		}			
		case EN_GT_PT_CONTROL: 					
		{	
			kfd_controller_data_struct* control_data;
			kal_uint8 *currPos=NULL;
			currPos = (kal_uint8*)head + sizeof(gps_tracker_msg_head_struct);
			if(*currPos==0x0d && *(currPos+1)==0x0a)
			{
				//zt_trace(TPROT, "control rsp sn ok" );
			}
			else
			{
				control_data = (kfd_controller_data_struct*)((kal_uint8*)head + sizeof(gps_tracker_msg_head_struct));
				if(control_data)
				{
				//zt_trace(TPROT, "server send to control, control_data->addr=%x,control_data->value_len=%d,control_data->value=%d",control_data->addr, control_data->value_len,control_data->value);
				}
				switch(control_data->addr)
				{
					case 0x1a:
					case 0x1b:
					{
						break;
					}
					case 0x1c:	
					case 0x1d:	
					{
						zt_smart_proc_network_data(control_data->value_len,control_data->value);
						break;
					}	
					default:
						break;
				}
			}
			break;
		}
		case EN_GT_PT_LBS:
		{
			gps_tracker_lbs_gps_struct* loc;
			float lat,lon;
			loc = (gps_tracker_lbs_gps_struct*)((kal_uint8*)head + sizeof(gps_tracker_msg_head_struct));
			lat = (float)(loc->lat[0]+loc->lat[1]*0x100+loc->lat[2]*0x10000+loc->lat[3]*0x1000000)/1800000;
			lon = (float)(loc->lon[0]+loc->lon[1]*0x100+loc->lon[2]*0x10000+loc->lon[3]*0x1000000)/1800000;
			zt_trace(TPROT,"AGPS经纬度lat=%f,lon=%f",lat,lon);
			break;
		}
		case EN_GT_PT_SRV_DATA:
		{
			gps_tracker_data_content_struct* data;
			data = (gps_tracker_data_content_struct*)((kal_uint8*)head + sizeof(gps_tracker_msg_head_struct));	
			kfd_srv_data_req_proc(data);
			kfd_upload_command_rsp_package((kal_uint8*)head);
			break;	
		}		
		default:
			break;			
	}						

	return KAL_TRUE;
}

/*****************************************************************************
 * FUNCTION
 *  kfd_protocol_parse
 * DESCRIPTION
 *  解析包
 * PARAMETERS
 * RETURNS
 *  void
 *****************************************************************************/
void kfd_protocol_parse(kal_int8 socket_id,RcvDataPtr GetRcvData)
{
	#define MAX_READ_COUNT 1024
	
	kal_int32 len;
	kal_uint8* head,*pBuf;
	kal_uint8* tail = NULL;
	kal_uint16 i = 0;
	kal_uint8 req[MAX_READ_COUNT] = {0};

	pBuf = (kal_uint8*)zt_Malloc(MAX_READ_COUNT);
	memset(pBuf, 0, MAX_READ_COUNT);
	len = GetRcvData(socket_id,pBuf,MAX_READ_COUNT);
	
	for(i = 0; i<len-1; i++)
	{
		memset(req, 0, sizeof(req));
		//找包头 0xffff
		if(pBuf[i]==0xff && pBuf[i+1]==0xff)
		{
			head = pBuf+i;
		}
		else if(pBuf[i]==0x0d && pBuf[i+1]==0x0a)
		{
			tail = pBuf+i+2;//结尾

			//验证长度合法性
			if(tail - head == head[4] + PACKET_FRAME_LEN)
			{
				//合法
				memcpy(req, head, tail-head);
				 kfd_protocol_proc(req,tail-head);

				//找到合法结尾
				head = tail;
			}
			//继续找结尾
			i +=1;	//代码加1，for循环自加1 ，等效于向后偏移2个字节（0d0a）			
		}
	}

	zt_Free(pBuf);
}

/*****************************************************************************
 * FUNCTION
 *  kfd_connect_service
 * DESCRIPTION
 * 连接服务器入口
 * PARAMETERS
 * RETURNS
 *  
 *****************************************************************************/
void kfd_connect_service(void)
{
	//zt_trace(TPROT, "%s", __func__);
	kfd_network_para.login_callback = kfd_upload_login_package;
	kfd_network_para.destory_callback = kfd_free_connect;
	kfd_network_para.parse_callback = kfd_protocol_parse;
	
	kfd_soc_app_id = zt_socket_get_app_id();
	zt_socket_launch(kfd_soc_app_id,&kfd_network_para);
}

/*****************************************************************************
 * FUNCTION
 *  kfd_protocol_init
 * DESCRIPTION
 *  系统全局变量初始化
 * PARAMETERS
 *  void  
 *  
 * RETURNS
 *  void
 *****************************************************************************/
void kfd_protocol_init(void)
{
	S16 error;
	network_info_struct network={0};

	kfd_get_gps_data_per_period();
	kfd_work_state = EN_INIT_STATE;	

	ReadRecord(GetNvramID(NVRAM_EF_GT_DEV_TYPE_LID), 1, &gps_tracker_config.dev_type, sizeof(gps_tracker_config.dev_type),&error);
	if(gps_tracker_config.dev_type==0)
	{
		gps_tracker_config.dev_type = 0x10;
		gps_tracker_config.defence = 1;
		gps_tracker_config.vibr2_thr = 0; 
		gps_tracker_config.vibr_thr = 10; 
		gps_tracker_config.speed_thr = 40;
		*(U8*)&gps_tracker_config.alarm_switch = 0xff;
		WriteRecord(GetNvramID(NVRAM_EF_GT_TEMP_THR_LID), 1, &gps_tracker_config.vibr2_thr, sizeof(gps_tracker_config.vibr2_thr),&error);
		WriteRecord(GetNvramID(NVRAM_EF_GT_VIBR_THR_LID), 1, &gps_tracker_config.vibr_thr, sizeof(gps_tracker_config.vibr_thr),&error);		
		WriteRecord(GetNvramID(NVRAM_EF_GT_SPEED_THR_LID), 1, &gps_tracker_config.speed_thr, sizeof(gps_tracker_config.speed_thr),&error);
		WriteRecord(GetNvramID(NVRAM_EF_GT_ALARM_SWITCH_LID), 1, &gps_tracker_config.alarm_switch, sizeof(gps_tracker_config.alarm_switch),&error);
		WriteRecord(GetNvramID(NVRAM_EF_GT_DEFENCE_LID), 1, &gps_tracker_config.defence, sizeof(gps_tracker_config.defence),&error);	
		WriteRecord(GetNvramID(NVRAM_EF_GT_DEV_TYPE_LID), 1, &gps_tracker_config.dev_type, sizeof(gps_tracker_config.dev_type),&error);		
	}
	else
	{
		ReadRecord(GetNvramID(NVRAM_EF_GT_TEMP_THR_LID), 1, &gps_tracker_config.vibr2_thr, sizeof(gps_tracker_config.vibr2_thr),&error);
		ReadRecord(GetNvramID(NVRAM_EF_GT_VIBR_THR_LID), 1, &gps_tracker_config.vibr_thr, sizeof(gps_tracker_config.vibr_thr),&error);
		ReadRecord(GetNvramID(NVRAM_EF_GT_SPEED_THR_LID), 1, &gps_tracker_config.speed_thr, sizeof(gps_tracker_config.speed_thr),&error);
		ReadRecord(GetNvramID(NVRAM_EF_GT_ALARM_SWITCH_LID), 1, &gps_tracker_config.alarm_switch, sizeof(gps_tracker_config.alarm_switch),&error);
		ReadRecord(GetNvramID(NVRAM_EF_GT_DEFENCE_LID), 1, &gps_tracker_config.defence, sizeof(gps_tracker_config.defence),&error);
	}

	if(zt_read_config_in_fs(NETWORK_FILE, (kal_uint8 *)&network, sizeof(network_info_struct)))
	{
		memcpy(&kfd_network_para.network_info, &network,sizeof(network_info_struct));
		zt_trace(TPROT,"Read FS network info");
	}
	
	gps_tracker_config.time_zone = 8*60;
	sprintf(gps_tracker_config.ver,"%s%s",GT_VER,(kal_uint8*)GetHWVersion());
	//zt_trace(TMAIN,"%d %d %d %d %d %d",gps_tracker_config.dev_type,gps_tracker_config.vibr2_thr,gps_tracker_config.vibr_thr,
//		gps_tracker_config.speed_thr,gps_tracker_config.alarm_switch,gps_tracker_config.defence);
		zt_trace(TPROT,"Ver=%s",gps_tracker_config.ver);

}

