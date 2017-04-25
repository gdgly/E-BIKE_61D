#include "zt_mtk_type.h"
#include "zt_smart_control.h"
#include "zt_trace.h"
#include "zt_gsensor.h"
#include "kfd_protocol.h"

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
kal_uint8 gps_delay_off_flag = 0;
controller_struct controller_data;
battery_info_struct curr_bat;
kal_uint32 pre_time=0;

#define MAX_BAT 10
battery_struct battery_array[MAX_BAT];
void zt_smart_write_hall(void);
void zt_controller_send(kal_uint8 cmd, kal_uint8 data);


kal_uint16 zt_adc_get_value(void)
{
	return curr_bat.sum_vol;
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

void zt_smart_delay_gps_off(void)
{
	gps_delay_off_flag = 0;
}

void zt_smart_proc_network_data(kal_uint8 value_len, kal_uint8* value_data)
{
	S16 error;
	command_struct* cmd; 
	kal_uint8 result =1;
	if(!value_data)
		return;
	cmd = (command_struct*)value_data;
	zt_trace(TPERI,"%s,len=%d,type=%d,para=%d",__func__,value_len,cmd->type,cmd->para[0]);
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
					}
				}
				break;
			case 0x08:	//租车成功指令
				if(cmd->para[0]==1)
				{
					if(!zt_gps_get_pwr_status())
					{
						gps_delay_off_flag = 1;
						zt_gps_power_on();
						StartTimer(ZT_GPS_DELAY_OFF_TIMER, 900*1000, zt_smart_delay_gps_off);
					}
				}
				break;
			case 0x09:	//报警开关指令
				gps_tracker_config.vibr2_thr = cmd->para[0];
				WriteRecord(GetNvramID(NVRAM_EF_GT_TEMP_THR_LID), 1, &gps_tracker_config.vibr2_thr, sizeof(gps_tracker_config.vibr2_thr),&error);
				zt_lbs_req();
				break;
			case 0x0a:	//调速
				if(cmd->para[0]==0)
				{
					controller_data.tiaosu = HIGH_SPEED;
					zt_trace(TPROT,"调高速");
				}
				else
				{
					controller_data.tiaosu = LOW_SPEED;
					zt_trace(TPROT,"调低速");
				}
				zt_controller_send(1,controller_data.tiaosu);
				break;
			case 0x0b:	//调欠压值
				if(cmd->para[0]==0)
				{
					controller_data.qianya = HIGH;
					zt_trace(TPROT,"调高欠压");
				}
				else
				{
					controller_data.qianya = LOW;
					zt_trace(TPROT,"调低欠压");
				}
				zt_controller_send(2,controller_data.qianya);
				break;
			case 0x0c:	//助力切换
				if(cmd->para[0]==0)
				{
					controller_data.zhuli = DIANDONG;
					zt_trace(TPROT,"纯电动");
				}
				else if(cmd->para[0]==1)
				{
					controller_data.zhuli = ZHULI;
					zt_trace(TPROT,"助力1:1");
				}
				else if(cmd->para[0]==2)
				{
					controller_data.zhuli = ZHULI2;
					zt_trace(TPROT,"助力1:2");
				}
				else 
				{
					controller_data.zhuli = RENLI; 
					zt_trace(TPROT,"纯人力");
				}
				zt_controller_send(3,controller_data.zhuli);
				break;
			case 0x0d:	//故障修复
				if(cmd->para[0]==1)
				{
					controller_data.xiufu = cmd->para[0];
					zt_trace(TPROT,"故障修复");
					zt_controller_send(4,cmd->para[0]);
				}
				else if(cmd->para[0]==0)
				{
					controller_data.xiufu = cmd->para[0];
					zt_trace(TPROT,"故障清除");
					zt_controller_send(4,cmd->para[0]);
				}
				break;	
			case 0x0e:	//切换电源电压
				if(cmd->para[0]==0)//36V
				{
					controller_data.dy = VOT36V;
					zt_trace(TPROT,"切换电源36V");
				}
				else if(cmd->para[0]==1)//48V
				{
					controller_data.dy = VOT48V;
					zt_trace(TPROT,"切换电源48V");
				}
				zt_controller_send(5,controller_data.dy);
				break;		
			case 0x20:	//切换服务器
				if(cmd->para[0]==1)
				{//现网转测试服务器
					
				}
				else if(cmd->para[0]==2)
				{//测试服务器转现网服务器
					zt_trace(TPROT,"测试切换现网服务器");
				}
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
	if(update_data)
	{
		ebike_struct ebike={0};

		ebike.fault = controller_data.fault;
		if(controller_data.tiaosu==HIGH_SPEED)
			ebike.status.tiaosu = 0;
		else if(controller_data.tiaosu==LOW_SPEED)
			ebike.status.tiaosu = 1;
		
		if(controller_data.qianya==HIGH)
			ebike.status.qianya = 0;
		else if(controller_data.qianya==LOW)
			ebike.status.qianya = 1;

		if(controller_data.zhuli==DIANDONG)
			ebike.status.zhuli = 0;
		else if(controller_data.zhuli==ZHULI)
			ebike.status.zhuli = 1;
		else if(controller_data.zhuli==RENLI)
			ebike.status.zhuli = 2;

		if(controller_data.dy==VOT36V)
			ebike.status.dy = 0;
		else if(controller_data.dy==VOT48V)
			ebike.status.dy = 1;
		
		if(who_open_electric_gate)
			ebike.status.lock = 0;
		else
			ebike.status.lock = 1;

		if(gps_tracker_config.vibr2_thr == 1)
			ebike.status.alarm= 1;	
		else
			ebike.status.alarm = 0;

		ebike.hall = curr_hall/8;
		ebike.bat.sum_vol = curr_bat.sum_vol;
		ebike.bat.currnt = curr_bat.currnt;
		ebike.bat.des_cap= curr_bat.des_cap;
		ebike.bat.des_vol= curr_bat.des_vol;
		ebike.bat.fcc= curr_bat.fcc;
		ebike.bat.residual_cap = curr_bat.residual_cap;
		ebike.bat.per_cap = curr_bat.per_cap;
		ebike.bat.cycle_count = curr_bat.cycle_count;
		ebike.bat.temperatue = curr_bat.temperatue;
		ebike.bat.status = curr_bat.status;
		ebike.bat.date = curr_bat.date;
		ebike.bat.id = curr_bat.id;
		memcpy(update_data,&ebike,sizeof(ebike_struct));
		zt_smart_write_hall();
		zt_trace(TPERI,"%s,len=%d",__func__,sizeof(ebike_struct));
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

	if(gps_tracker_config.vibr2_thr==1)
	{
		if(zt_gsensor_check_is_shake_sharp()&& !get_electric_gate_status())
		{
			zt_voice_play(VOICE_ALARM);
		}
	}
	StartTimer(GetTimerID(ZT_SMART_LUNDONG_CHECK_TIMER), 1000, zt_smart_check_lundong);
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
kal_bool zt_gps_valid(void)
{
	gps_info_struct* gps_info = (gps_info_struct*)zt_gps_get_curr_gps();

	if(gps_info->state=='A')
		return KAL_TRUE;
	else
		return KAL_FALSE;
}
kal_bool zt_time_expiry(void)
{
	kal_uint32 real_time = (kal_uint32)GetTimeSec();
	if((real_time-pre_time)>3600*2)	// 2H
		return KAL_TRUE;
	else
		return KAL_FALSE;
}
void zt_agps_process(void)
{
	if(zt_time_expiry()&&(kal_bool)zt_gps_get_pwr_status()&&!zt_gps_valid())
	{
		pre_time = (kal_uint32)GetTimeSec();
		zt_trace(TPERI,"LBS REQ, 时间:%d",pre_time);
		zt_lbs_req();
	}
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
		}
	}
	else if(zt_gsensor_check_is_motionless() && !gps_delay_off_flag)
	{
		if(zt_gps_get_pwr_status())
		{
			//zt_trace(TPERI,"gps pwr off");
			zt_gps_power_off();
		}
	}
//	zt_agps_process();
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
/*****************************************************************************
 * FUNCTION
 *  zt_controller_send
 * DESCRIPTION
 * 控制器命令接口
 * PARAMETERS
 *  cmd 1:调速2:调欠压值3:助力切换4:故障修复
 *  data 值
 * RETURNS
 *  void
 *****************************************************************************/
void zt_controller_send(kal_uint8 cmd, kal_uint8 data)
{
	kal_uint16 check_sum = 0;
	kal_uint8 i, send_data[] = {0x3a,0x1d,0x01,0x00,0x00,0x00,0x00,0x00,0x0d,0x0a};

	send_data[3] = 2;
	send_data[4] = cmd;
	send_data[5] = data;

	for(i=0; i<5;i++)
		check_sum += send_data[i+1];
	
	send_data[6] =check_sum&0xff;
	send_data[7] =check_sum>>8;

	zt_trace(TPERI,"串口发送数据%d",cmd);
	zt_uart_write_data(uart_port2, send_data, sizeof(send_data));
}
kal_bool zt_controller_proc(kal_uint8* buf, kal_uint16 len)
{
	kal_uint16 i,checksum=0;

	for(i=0; i<len-2; i++)
	{
		checksum += buf[i];
	}
	zt_trace(TPERI,"checksum=%x",checksum);
	if(!((checksum&0x00ff)==(kal_uint16)buf[len-2] && (checksum&0xff00)>>8 == (kal_uint16)buf[len-1]))
		return KAL_FALSE;
	zt_trace(TPERI,"checksum ok");

	if(buf[0]==0x1d)	//控制器地址
	{
		if(buf[1]==0x01)	//控制命令的响应
		{
			zt_trace(TPERI,"接收到控制响应命令,长度%d",len);
			switch(buf[3])
			{
				case 0x01:	//调速
					if(buf[4])
						zt_trace(TPERI,"调速成功");
					else
						zt_trace(TPERI,"调速失败");
					break;
				case 0x02:	//调欠压值
					if(buf[4])
						zt_trace(TPERI,"调欠压值成功");
					else
						zt_trace(TPERI,"调欠压值失败");
					break;
				case 0x03:	//助力切换
					if(buf[4])
						zt_trace(TPERI,"助力切换成功");
					else
						zt_trace(TPERI,"助力切换失败");
					break;
				case 0x04:	//故障修复
					if(buf[4])
						zt_trace(TPERI,"故障修复成功");
					else
						zt_trace(TPERI,"故障修复失败");
					break;
				default:
					break;
			}
		}
		else if(buf[1]==0x02)	//上报命令
		{
			zt_trace(TPERI,"接收到上报命令,长度%d",len);

			controller_data.fault = buf[3];
			controller_data.hall = buf[4]+buf[5]<<8+buf[6]<<16+buf[7]<<24;
			controller_data.tiaosu = buf[8];
			controller_data.qianya = buf[9];
			controller_data.zhuli = buf[10];
		}
	}
}
kal_uint8 parse_uart2_hdlr(void* info)
{
	bt_msg_struct* uart2_msg = (bt_msg_struct*)info;
	kal_uint16 len,i,len2;
	kal_uint8 *buf,*head=NULL,*tail=NULL;
	kal_uint8 req[64]={0};

	len = uart2_msg->dataLen;
	buf = uart2_msg->data;
	zt_trace(TPERI,"串口数据长度%d",len);
	for(i=0; i<len; i++)
	{
		zt_trace(TPERI,"[%d] = %x",i,buf[i]);
		if(buf[i]==0x3a)
		{
			head = buf+i;
		}
		else if(buf[i]==0x0d&&buf[i+1]==0x0a)
		{
			tail = buf+i;
			if(head)
			{
				len2 = tail-(head+1);
				memset(req,0,sizeof(req));
				memcpy(req, head+1, len2);
				zt_controller_proc(req,len2);
			}
		}
	}
	
}

kal_uint8 parse_lbs_hdlr(void* info)
{
	lbs_msg_struct* lbs_msg = (lbs_msg_struct*)info;
	if(lbs_msg->lbs.lbs_server.cellid_nid!=0)
		kfd_upload_lbs_package(&lbs_msg->lbs);
}
kal_uint8 parse_battery_hdlr(void* info)
{
	battery_msg_struct* bat_msg = (battery_msg_struct*)info;
	memcpy(&curr_bat, &bat_msg->bat, sizeof(battery_info_struct));	
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
	mmi_frm_set_protocol_event_handler((U16)GetMsgID(MSG_ID_UART2_SEND_TO_MMI_REQ), (PsIntFuncPtr)parse_uart2_hdlr,  KAL_FALSE);
	mmi_frm_set_protocol_event_handler((U16)GetMsgID(MSG_ID_BAT_SEND_TO_MMI_REQ), (PsIntFuncPtr)parse_battery_hdlr,  KAL_FALSE);
	mmi_frm_set_protocol_event_handler((U16)GetMsgID(MSG_ID_LBS_SEND_MMI_REQ), (PsIntFuncPtr)parse_lbs_hdlr,  KAL_FALSE);
}
