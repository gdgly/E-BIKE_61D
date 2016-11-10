#include "zt_mtk_type.h"
#include "zt_smart_control.h"
#include "zt_trace.h"
#include "zt_gsensor.h"


#define DIANMEN_PIN (44|0x80)
#define KEY_DETECT_PIN (8|0x80)
#define LOCK_A_PIN (28|0x80)
#define LOCK_B_PIN (27|0x80)
#define CONTROL_PIN (54|0x80)

#define KEY_OPEN (1 << 0)
#define BT_OPEN	(1 << 1)
#define GPRS_OPEN (1 << 2)

kal_uint8 who_open_electric_gate=0;
kal_uint8 lundong_is_locking = 0;
kal_uint8 tangze_is_locking = 0;
kal_uint32 lundong_count_1sec = 0;
kal_uint32 hall_count_1sec = 0;
kal_uint16 curr_adc;
kal_uint16 aver_adc;
kal_uint16 pre_adc = 0;
kal_uint32 curr_hall;
kal_uint32 curr_lundong;

#define MAX_BAT 10
battery_struct battery_array[MAX_BAT];
void zt_smart_write_hall(void);

kal_uint16 zt_adc_get_value(void)
{
	return curr_adc;
}
kal_uint16 zt_adc_get_aver_value(void)
{
	return aver_adc;
}
kal_uint8 get_electric_gate_status(void)
{
	return who_open_electric_gate;
}
void close_dianmen(void)
{
	//zt_trace(TPERI,"%s",__func__);
	GPIO_WriteIO(0, DIANMEN_PIN);	
}

void open_dianmen(void)
{
	//zt_trace(TPERI,"%s",__func__);
	GPIO_WriteIO(1, DIANMEN_PIN);	
}
void clear_tangze_lock(void)
{
	//zt_trace(TPERI,"%s",__func__);
	GPIO_WriteIO(0, LOCK_A_PIN);
	GPIO_WriteIO(0, LOCK_B_PIN);
}
void clear_tangze_lock_flag(void)
{
	tangze_is_locking = 0;
}
void tangze_lock_bike(void)
{
	//zt_trace(TPERI,"%s",__func__);
	GPIO_WriteIO(1, LOCK_A_PIN);
	GPIO_WriteIO(0, LOCK_B_PIN);
	zt_start_timer(clear_tangze_lock, /*KAL_TICKS_500_MSEC*/120);
	StartTimer(GetTimerID(ZT_TANGZE_LOCK_TIMER), 600, clear_tangze_lock_flag);
}
void controller_lock_bike_callback(void)
{
	//zt_trace(TPERI,"%s",__func__);
	close_dianmen();
	GPIO_WriteIO(0, CONTROL_PIN);
	lundong_is_locking = 0;
}
void controller_lock_bike(void)
{
	//zt_trace(TPERI,"%s",__func__);
	//先打开电门，给锁供电
	open_dianmen();
	ztDelayms(10);
	GPIO_WriteIO(1, CONTROL_PIN);

	StartTimer(GetTimerID(ZT_LOCK_DELAY_TIMER),4000,controller_lock_bike_callback);
}
void unlock_bike(void)
{
	//zt_trace(TPERI,"%s",__func__);
	GPIO_WriteIO(0, LOCK_A_PIN);
	GPIO_WriteIO(1, LOCK_B_PIN);
	zt_start_timer(clear_tangze_lock, /*KAL_TICKS_500_MSEC*/120);

	StartTimer(GetTimerID(ZT_TANGZE_LOCK_TIMER),600,clear_tangze_lock_flag);
}
void gprs_open_dianmen(void)
{
	S16 error;
	
	who_open_electric_gate |= GPRS_OPEN;
	WriteRecord(GetNvramID(NVRAM_EF_ZT_DIANMEN_LID), 1, &who_open_electric_gate, 1, &error);
	open_dianmen();
}
kal_bool zt_smart_check_hall_is_run(void)
{
	if(hall_count_1sec>=2)
	{
		return KAL_TRUE;
	}
	else
	{
		return KAL_FALSE;
	}
}
void open_dianchi_lock(void)
{
	open_led2();
}
void close_dianchi_lock(void)
{
	close_led2();
}


void zt_smart_proc_network_data(kal_uint8 value_len, kal_uint8* value_data)
{
	S16 error;
	command_struct* cmd; 
	kal_uint8 result =1;
	if(!value_data)
		return;
	cmd = (command_struct*)value_data;
//	zt_trace(TPERI,"%s,len=%d,type=%d,para=%d",__func__,value_len,cmd->type,cmd->para[0]);
	if(value_len != 0)
	{
		switch(cmd->type)
		{
			case 0x01:	//寻车
				zt_voice_play(VOICE_SEARCH);
				break;
			case 0x02:	
				if(cmd->para[0]==1)	//锁车
				{
					if(!(who_open_electric_gate&KEY_OPEN)&&(who_open_electric_gate&(BT_OPEN|GPRS_OPEN)))
					{
						if(!zt_smart_check_hall_is_run())
						{
							close_dianmen();
							tangze_is_locking = 1;
							StartTimer(GetTimerID(ZT_DIANMEN_LOCK_TIMER), 1000,tangze_lock_bike);	
							who_open_electric_gate = 0;
							WriteRecord(GetNvramID(NVRAM_EF_ZT_DIANMEN_LID), 1, &who_open_electric_gate, 1, &error);
							zt_voice_play(VOICE_LOCK);
						}
					}
				}
				else if(cmd->para[0]==0)	//解锁
				{
					if(!who_open_electric_gate)
					{
						unlock_bike();
						StartTimer(GetTimerID(ZT_DIANMEN_UNLOCK_TIMER), 1000,gprs_open_dianmen);
						zt_voice_play(VOICE_UNLOCK);
					}
				}
				break;
			case 0x03:	//静音
				
				break;
			case 0x04:	//开启电池锁
				if(cmd->para[0]==1)
				{
					open_dianchi_lock();
					StartTimer(GetTimerID(ZT_CLOSE_DIANCHI_LOCK_TIMER),1000,close_dianchi_lock);
					pre_adc = 0;	//更换电池时，电量上报当前电池的电量值，所以把上一个电量清零
				}
				break;
			case 0x05:	//远程重启系统
				if(cmd->para[0]==1)
				{
					zt_reset_system();
				}
				break;
			case 0x06:
				if(cmd->para[0]==1)
				{
					bluetooth_reset();
				}
				break;
			case 0x07:	//还车
				if(cmd->para[0]==1)	//还车指令
				{
					if(!(who_open_electric_gate&KEY_OPEN)&&(who_open_electric_gate&(BT_OPEN|GPRS_OPEN)))
					{
						if(!zt_smart_check_hall_is_run())
						{
							close_dianmen();
							tangze_is_locking = 1;
							StartTimer(GetTimerID(ZT_DIANMEN_LOCK_TIMER), 1000,tangze_lock_bike);	
							who_open_electric_gate = 0;
							WriteRecord(GetNvramID(NVRAM_EF_ZT_DIANMEN_LID), 1, &who_open_electric_gate, 1, &error);
						}
					}
					kfd_upload_give_back_package(who_open_electric_gate);
				}
				else if(cmd->para[0]==2)	//服务器判断还车成功之后下发指令
				{
					if(zt_gps_get_pwr_status())
					{
						zt_gps_power_off();
						kfd_stop_gps_data_per_period();
					}
				}
				break;
			default:
				break;
		}	
	}
}

kal_uint16 zt_convert_adc(kal_uint16 adc)
{
	kal_uint16 result;
	result = (((adc*1024/2800)*34*28)/(31*33))&0xffff;
	
	return result;
}
void zt_smart_collect_battery_proc(void)
{
	kal_int8 i;

	for(i=MAX_BAT-1; i>0; i--)
	{
		battery_array[i].adc = battery_array[i-1].adc;
		battery_array[i].electric_gate = battery_array[i-1].electric_gate;
		//zt_trace(TPERI,"adc=%d,gate=%d", battery_array[i].adc,battery_array[i].electric_gate);
	}
	battery_array[0].adc = zt_convert_adc(zt_adc_get_aver_value());
	battery_array[0].electric_gate = who_open_electric_gate;
}

/*****************************************************************************
 * FUNCTION
 *  zt_smart_calibrate_adc
 * DESCRIPTION
 *   adc 校准算法，获取最近10次采集的电门是打开状态下的平均值
 * PARAMETERS
 *  void  
 *  
 * RETURNS
 *  kal_uint16
 *****************************************************************************/
kal_uint16 zt_smart_calibrate_adc(void)
{
	kal_int8 i,num=0;
	kal_uint16 adc=0;
	kal_uint16 sum=0;

	for(i=0; i<MAX_BAT; i++)
	{
		if(battery_array[i].electric_gate && battery_array[i].adc>100)
		{
			num++;
			sum += battery_array[i].adc;
		}
	}

	if(num>2)
	{
		adc = sum/num;
	}
	else
	{
		adc = 0;
	}
	//zt_trace(TPERI,"%s,adc=%d,num=%d",__func__,adc,num);
	return adc;	
}


void zt_smart_update_network_data(kal_uint8* update_data)
{
	kal_uint32 hall;
	kal_uint16 curradc=0,cali_adc=0;
//	static kal_uint16 pre_adc = 0;

	/*电量获取均值数据*/
	zt_smart_collect_battery_proc();
	
	if(update_data)
	{
	//电量
		cali_adc = zt_smart_calibrate_adc();
		if(cali_adc)
		{
			*update_data = cali_adc&0xff;
			*(update_data+1) = (cali_adc>>8)&0xff;
			pre_adc = cali_adc;
		}
	/*	else if(pre_adc)
		{
			*update_data = pre_adc&0xff;
			*(update_data+1) = (pre_adc>>8)&0xff;
		}*/
		else
		{
			curradc = zt_convert_adc((kal_uint16)zt_adc_get_aver_value());
			*update_data = curradc&0xff;
			*(update_data+1) = (curradc>>8)&0xff;
		}
	//	zt_trace(TPERI,"%s,cali_adc=%d,pre_adc=%d,curr_adc=%d",__func__,cali_adc,pre_adc,curradc);

		
	//故障值
		*(update_data+2) = 0x00;
	//开关锁状态
		if(who_open_electric_gate)
			*(update_data+3) = 0;
		else
			*(update_data+3) = 1;
	//霍尔里程数
		hall = curr_hall/8;
		*(update_data+4) = hall&0xff;
		*(update_data+5) = (hall>>8)&0xff;
		*(update_data+6) = (hall>>16)&0xff;
		*(update_data+7) = (hall>>24)&0xff;
		zt_smart_write_hall();
	//	zt_trace(TPROT|TPERI,"%x %x %x %x %x %x %x %x",*update_data,*(update_data+1),*(update_data+2),*(update_data+3),*(update_data+4),*(update_data+5),*(update_data+6),*(update_data+7));
	}
}


/*****************************************************************************
 * FUNCTION
 *  bt_prepare_send_data
 * DESCRIPTION
 * 准备数据发送给蓝牙app
 * PARAMETERS
 reply:
 0x01:app->toshiba
 0x02:toshiba->app
 operate:
 0x01	普通命令回复
 0x02	密码命令回复
 0x03	电量
 0x04 	查询模式
 0x05	手机是否有校验
 param_len:
 参数长度
 param:
 参数
 * RETURNS
 *  
 *****************************************************************************/
void bt_prepare_send_data(kal_uint8 reply, kal_uint8 operate, kal_uint8 param_len, kal_uint8* param)
{
	kal_uint8 buffer[16]={0};
	buffer[0] = 0x0b;
	buffer[1] = reply;
	buffer[2] = operate;
	buffer[3] = 0;	//param_len;
	if(param)
		memcpy(buffer+4, param, param_len);

	bt_send_data(buffer,buffer[0]);
}
void send_ok_cmd(kal_uint8 operate)
{
	char param[6]={0}; 

	param[0] = 1;
	bt_prepare_send_data(0x02, operate, 1, param);
}
void send_error_cmd(kal_uint8 operate)
{
	char param[6]={0};

	param[0] = 0;
	bt_prepare_send_data(0x02, 0x01, 1, param);
}
void bt_open_dianmen(void)
{
	S16 error;
	who_open_electric_gate |= BT_OPEN;
	open_dianmen();
	send_ok_cmd(0x01);
	WriteRecord(GetNvramID(NVRAM_EF_ZT_DIANMEN_LID), 1, &who_open_electric_gate, 1, &error);
}
kal_uint8 bt_parse_actual_data_hdlr(void* info)
{
	bt_msg_struct* bt_msg = (bt_msg_struct*)info;
	kal_uint8 result = 1;
	kal_uint8 i;
	S16 error;
	char param[6]={0};
	kal_uint16 adc = 0;

	//zt_trace(TPERI, "bt_parse_actual_data_hdlr,cmd=%d",*(bt_msg->data+2)); 
/*	for(i=0;i<bt_msg->dataLen;i++)
	{
	//zt_trace(TPERI, "[%d]=%x ",i,*(bt_msg->data+i));
	}*/
	
	if (0x0B != *(bt_msg->data + 0)) {
		send_error_cmd(0x01);
		result = 0;
		return result;
	}
	switch (*(bt_msg->data + 2)) {
	case CHECK_PHONE_WHETHER_VERIFIED_CMD:
		//zt_trace(TPERI, "CHECK_PHONE_WHETHER_VERIFIED_CMD");
		send_ok_cmd(0x05);
		break;
	case INQUIRE_PWD_STATUS_CMD:
		//zt_trace(TPERI, "INQUIRE_PWD_STATUS_CMD");
		send_ok_cmd(0x02);
	  	break;
	case SET_PWD_CMD:
		//zt_trace(TPERI, "SET_PWD_CMD");
	  	break;
	case INQUIRE_MODE_CMD:
		//zt_trace(TPERI, "INQUIRE_MODE_CMD");
		send_ok_cmd(0x04);
	  	break;
	case READ_VOLTAGE_VALUE:
		//zt_trace(TPERI, "READ_VOLTAGE_VALUE");
		adc = (kal_uint16)zt_convert_adc(zt_adc_get_aver_value());
		param[0]= adc&0xff;
		param[1]= (adc>>8)&0xff;
		bt_prepare_send_data(0x02, 0x03, 2, param);
	  	break;
	case SWITCH_TO_GANYING_MODE_CMD:
		//zt_trace(TPERI, "SWITCH_TO_GANYING_MODE_CMD");
		send_ok_cmd(0x01);
	  	break;
	case SWITCH_TO_YAOKONG_MODE_CMD:
		//zt_trace(TPERI, "SWITCH_TO_YAOKONG_MODE_CMD");
		send_ok_cmd(0x01);
	  	break;
	case PWD_VERIFY_CMD:
		//zt_trace(TPERI, "PWD_VERIFY_CMD");
		send_ok_cmd(0x01);
	  	break;
	case LOCK_CMD:
		//zt_trace(TPERI, "LOCK_CMD,who_open_electric_gate=%d",who_open_electric_gate);
		if(who_open_electric_gate & KEY_OPEN)
		{//error
			send_error_cmd(0x01);
		}
		else if(who_open_electric_gate &(GPRS_OPEN|BT_OPEN))
		{
			if(!zt_smart_check_hall_is_run())
			{
				who_open_electric_gate = 0; 
				tangze_is_locking = 1;
				close_dianmen();
				StartTimer(GetTimerID(ZT_DIANMEN_LOCK_TIMER), 1000, tangze_lock_bike);	
				send_ok_cmd(0x01);
				WriteRecord(GetNvramID(NVRAM_EF_ZT_DIANMEN_LID), 1, &who_open_electric_gate, 1, &error);
				zt_voice_play(VOICE_LOCK);
	  		}
	  		else
	  		{
				send_error_cmd(0x01);
	  		}
		}
		else
		{
			zt_voice_play(VOICE_LOCK);
			send_ok_cmd(0x01);
		}
	  	break;
	case UNLOCK_CMD:
		//zt_trace(TPERI, "UNLOCK_CMD,who_open_electric_gate=%d",who_open_electric_gate);
		if(!who_open_electric_gate)
		{
			unlock_bike();
			StartTimer(GetTimerID(ZT_DIANMEN_UNLOCK_TIMER), 1000,bt_open_dianmen);	//zzt.20160725
		}
		else
		{
			send_ok_cmd(0x01);
		}
		zt_voice_play(VOICE_UNLOCK);
	  	break;
	case SEARCH_CMD:
		//zt_trace(TPERI, "SEARCH_CMD");
		zt_voice_play(VOICE_SEARCH);
		send_ok_cmd(0x01);
	  	break;
	case LOW_VOLTAGE_NOTICE_CMD:
		//zt_trace(TPERI, "LOW_VOLTAGE_NOTICE_CMD");
	  	send_ok_cmd(0x01);
	  	break;
	case KEEP_QUIET_CMD:
	  	//zt_trace(TPERI, "KEEP_QUIET_CMD");
	  	break;
	case SET_SENSITIVITY:
	  	//zt_trace(TPERI, "SET_SENSITIVITY");
	 	break;
	case READ_WHETHER_PHONE_NEED_TO_ALERT:
	  	//zt_trace(TPERI, "READ_WHETHER_PHONE_NEED_TO_ALERT");
	  	break;
	default:
		send_error_cmd(0x01);
		error=0;
	  	break;
	}
	return result;
}

kal_bool zt_smart_check_lundong_is_run(void)
{
//	//zt_trace(TPERI,"%s,lundong_count_1sec=%d",__func__,lundong_count_1sec);
	if(lundong_count_1sec>=3)
	{
		return KAL_TRUE;
	}
	else
	{
		return KAL_FALSE;
	}
}

void zt_smart_check_lundong(void)
{
	 static kal_uint32 pre_count=0;
	 kal_uint32 curr_count;
	 static kal_uint32 pre_hall = 0;
	 kal_uint32 curr_hall_tmp = 0;

	curr_hall_tmp = curr_hall;
	curr_count = curr_lundong;

	hall_count_1sec = abs(curr_hall_tmp - pre_hall);
	pre_hall = curr_hall_tmp;
	
	lundong_count_1sec = curr_count-pre_count;
	pre_count = curr_count;

//	//zt_trace(TPERI,"%s,curr_count=%d,pre_count=%d",__func__,curr_count,pre_count);

	if(zt_smart_check_lundong_is_run() && !lundong_is_locking && !tangze_is_locking)
	{
		if(!who_open_electric_gate)
		{
			lundong_is_locking = 1;
			controller_lock_bike();
		//	zt_voice_play(VOICE_ALARM);
		}
	}
	
	StartTimer(GetTimerID(ZT_SMART_LUNDONG_CHECK_TIMER), 500, zt_smart_check_lundong);
}

void zt_smart_key_detect_proc(void)
{
	char value;
	static kal_uint8 key_detect_num = 0;
	
	value = GPIO_ReadIO(KEY_DETECT_PIN);	//0 open, 1 close

	if(!lundong_is_locking && !tangze_is_locking)
	{
		if(!value &&(!(who_open_electric_gate&BT_OPEN) && !(who_open_electric_gate&KEY_OPEN) && !(who_open_electric_gate&GPRS_OPEN)))
		{
			key_detect_num++;
			if(key_detect_num>2)
			{
				key_detect_num = 0;
				who_open_electric_gate |= KEY_OPEN;
				//unlock_bike();
				zt_voice_play(VOICE_UNLOCK);
			}
		}
		else if(value && who_open_electric_gate)
		{
			key_detect_num++;
			if(key_detect_num>2)
			{
				key_detect_num = 0;
				who_open_electric_gate = 0;
				zt_voice_play(VOICE_LOCK);
			}
		}
		else
		{
			key_detect_num = 0;
		}
	}
	
	StartTimer(GetTimerID(ZT_KEY_DETECT_TIMER),200,zt_smart_key_detect_proc);
}

void zt_smart_check_gps_pwr(void)
{
//	//zt_trace(TPERI,"%s",__func__);
	if(zt_gsensor_check_is_moving())
	{
		if(!zt_gps_get_pwr_status())
		{
			//zt_trace(TPERI,"gps pwr on");
			zt_gps_power_on();
			kfd_get_gps_data_per_period();
		}
	}
	else if(zt_gsensor_check_is_motionless())
	{
		if(zt_gps_get_pwr_status())
		{
			//zt_trace(TPERI,"gps pwr off");
			zt_gps_power_off();
			kfd_stop_gps_data_per_period();
		}
	}
	StartTimer(GetTimerID(ZT_GPS_PWR_CHECK_TIMER),3000,zt_smart_check_gps_pwr);
}

void zt_smart_dianmen_init(void)
{
	S16 error;
	kal_uint8 dianmen;
	
	ReadRecord(GetNvramID(NVRAM_EF_ZT_DIANMEN_LID), 1, &dianmen, 1, &error);
	//zt_trace(TPERI,"%s,dianmen=%d",__func__,dianmen);
	if(dianmen)
	{
		who_open_electric_gate = dianmen;
		open_dianmen(); 
		unlock_bike();
	}
}
kal_uint8 parse_adc_hdlr(void* info)
{
	adc_msg_struct* adc_msg = (adc_msg_struct*)info;

	curr_adc = adc_msg->adc_value;
	aver_adc = adc_msg->aver_value;
	return 1;  
}
kal_uint8 parse_hall_hdlr(void* info)
{
	eint_msg_struct* eint_msg = (eint_msg_struct*)info;

	curr_hall = eint_msg->data;
	return 1;
}
kal_uint8 parse_lundong_hdlr(void* info)
{
	eint_msg_struct* eint_msg = (eint_msg_struct*)info;
	
	curr_lundong = eint_msg->data;
	return 1;
}
void zt_smart_write_hall(void)
{
	S16 error;
	kal_uint8 hall_array[4];
	kal_uint32 hall = curr_hall;
	
	hall_array[0] = hall&0xff;
	hall_array[1] = (hall>>8)&0xff;
	hall_array[2] = (hall>>16)&0xff;
	hall_array[3] = (hall>>24)&0xff;
	WriteRecord(GetNvramID(NVRAM_EF_ZT_HALL_LID), 1, hall_array, 4, &error);
}
void zt_smart_read_hall(void)
{
	S16 error;
	kal_uint8 hall_array[4];

	ReadRecord(GetNvramID(NVRAM_EF_ZT_HALL_LID), 1, hall_array, 4, &error);
	curr_hall = hall_array[3]*0x1000000+hall_array[2]*0x10000+hall_array[1]*0x100+hall_array[0];
}
void zt_smart_init(void)
{
	//zt_trace(TPERI,"%s",__func__);
	zt_smart_dianmen_init();

	zt_smart_key_detect_proc();

/*总里程*/
	zt_smart_read_hall();

/*轮动报警*/	
	StartTimer(GetTimerID(ZT_SMART_LUNDONG_CHECK_TIMER), 5000, zt_smart_check_lundong);

/*GPS电源开启检测*/
	StartTimer(GetTimerID(ZT_GPS_PWR_CHECK_TIMER), 90000, zt_smart_check_gps_pwr);

/*注册消息的回调函数*/
	mmi_frm_set_protocol_event_handler((U16)GetMsgID(MSG_ID_BT_SEND_TO_MMI_REQ), (PsIntFuncPtr)bt_parse_actual_data_hdlr,  KAL_FALSE);
	mmi_frm_set_protocol_event_handler((U16)GetMsgID(MSG_ID_ADC_UEM_SEND_TO_MMI_REQ), (PsIntFuncPtr)parse_adc_hdlr,  KAL_FALSE);
	mmi_frm_set_protocol_event_handler((U16)GetMsgID(MSG_ID_HALL_EINT_SEND_MMI_REQ), (PsIntFuncPtr)parse_hall_hdlr,  KAL_FALSE);
	mmi_frm_set_protocol_event_handler((U16)GetMsgID(MSG_ID_LUNDONG_SEND_MMI_REQ), (PsIntFuncPtr)parse_lundong_hdlr,  KAL_FALSE);

}
