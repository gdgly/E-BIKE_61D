#include "zt_mtk_type.h"
#include "zt_smart_control.h"
#include "zt_trace.h"
#include "zt_gsensor.h"
#include "zt_interface.h"
#include "time.h"
#include "zt_agps.h"
#include "kfd_protocol.h"

#define DIANMEN_PIN (44|0x80)
#ifdef __CHAOWEI__
#define KEY_DETECT_PIN (50|0x80)
#else
#define KEY_DETECT_PIN (8|0x80)
#endif
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
config_struct controller={
	{0,0,LOW_SPEED,HIGH,HUNHE,VOT48V,XF_INVALID},
	{0,0,LOW_SPEED,HIGH,HUNHE,VOT48V,XF_INVALID}};
battery_info_struct curr_bat;
kal_uint32 pre_time=0;
kal_uint8 alarm_flag = 0;
#ifdef __MEILING__
default_setting_struct default_set={1};
#else
default_setting_struct default_set={0};
#endif
#ifdef __HW_2018__
kal_uint32 zuche_stamptime=0;
kal_uint8 bt_connect_status=0;
kal_uint32 zhendong_count_1sec=0;
kal_uint8 bt_heart_rsp_times=0;
#endif

#ifdef __BAT_PROT__
bat_cw_struct g_bat_cw;
void zt_bat_send(void);
#endif

void restartSystem(void);
kal_uint32 GetTimeStamp(void);
void zt_smart_write_hall(void);
void zt_smart_write_lundong(void);
kal_uint32 adc_convert_mv(kal_uint16 adc_mv);
void zt_controller_send(kal_uint8 addr,cmd_enum cmd, kal_uint8 data1,kal_uint8 data2);
kal_bool zt_smart_check_lundong_is_run(void);
kal_bool zt_smart_check_hall_is_run(void);
void bt_prepare_send_data(kal_uint8 operate, kal_uint8 param_len, kal_uint8* param);

kal_uint16 zt_adc_get_aver_value(void)
{
	return aver_adc;
}
kal_bool zt_get_bat_connect_status(void)
{
	if(curr_bat.voltage>6000)	//大于6V
		return KAL_TRUE;
	else if(adc_convert_mv(curr_adc)>6000)	//大于6V
		return KAL_TRUE;
	else
		return KAL_FALSE;
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
	zt_start_timer(clear_tangze_lock, /*KAL_TICKS_500_MSEC*/195);
	StartTimer(GetTimerID(ZT_TANGZE_LOCK_TIMER), 900, clear_tangze_lock_flag);
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

	StartTimer(GetTimerID(ZT_LOCK_DELAY_TIMER),6000,controller_lock_bike_callback);
}
kal_bool lock_bike(void)
{
	kal_bool result = KAL_FALSE;
	S16 error;
	
	if(!(who_open_electric_gate&KEY_OPEN)&&(who_open_electric_gate&(BT_OPEN|GPRS_OPEN))&&!IsMyTimerExist(GetTimerID(ZT_DIANMEN_UNLOCK_TIMER)))
	{
	#ifdef __MEILING__
		if(!zt_smart_check_lundong_is_run())
	#else
		if((!zt_smart_check_hall_is_run()&&default_set.motor==0)||(!zt_smart_check_lundong_is_run()&&default_set.motor==1))
	#endif
		{
			close_dianmen();
			tangze_is_locking = 1;
			tangze_lock_bike();
			who_open_electric_gate = 0;
			WriteRecord(GetNvramID(NVRAM_EF_ZT_DIANMEN_LID), 1, &who_open_electric_gate, 1, &error);
			result = KAL_TRUE;
		}
	}

	zt_trace(TPERI,"gate=%d,timer=%d",who_open_electric_gate,IsMyTimerExist(GetTimerID(ZT_DIANMEN_UNLOCK_TIMER)));
	return result;
}
void unlock_bike(void)
{
	//zt_trace(TPERI,"%s",__func__);
	GPIO_WriteIO(0, LOCK_A_PIN);
	GPIO_WriteIO(1, LOCK_B_PIN);
	zt_start_timer(clear_tangze_lock, /*KAL_TICKS_500_MSEC*/195);

	StartTimer(GetTimerID(ZT_TANGZE_LOCK_TIMER),900,clear_tangze_lock_flag);
}
void gprs_open_dianmen(void)
{
	S16 error;

	unlock_bike();
	who_open_electric_gate |= GPRS_OPEN;
	WriteRecord(GetNvramID(NVRAM_EF_ZT_DIANMEN_LID), 1, &who_open_electric_gate, 1, &error);
	StartTimer(GetTimerID(ZT_DIANMEN_UNLOCK_TIMER), 1000,open_dianmen);
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

void zt_smart_proc_network_data(kal_uint8 value_len, kal_uint8* value_data,kal_uint32 timestamp)
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
					if(lock_bike())
					{
						kfd_upload_ebike_package();
						zt_voice_play(VOICE_LOCK);
					}
				}
				else if(cmd->para[0]==0)	//解锁
				{
				#ifndef __CHAOWEI__
					if(abs(timestamp-GetTimeStamp())<=10)
				#endif		
					{
						if(!who_open_electric_gate)
						{
							gprs_open_dianmen();
							kfd_upload_ebike_package();
							zt_voice_play(VOICE_UNLOCK);
						}
					}
				}
				break;
			case 0x03:	//静音
				if(cmd->para[0]==1)
				{
					default_set.zd_alarm = 1;
				}
				else if(cmd->para[0]==0)
				{
					default_set.zd_alarm = 0;
				}
				else	//震动灵敏度
				{
					default_set.zd_sen = cmd->para[0];
				}
				zt_write_config_in_fs(SETTING_FILE,(kal_uint8*)&default_set,sizeof(default_setting_struct));
				break;
			case 0x04:	//开启电池锁
				if(cmd->para[0]==1)
				{
					open_dianchi_lock();
					StartTimer(GetTimerID(ZT_CLOSE_DIANCHI_LOCK_TIMER),500,close_dianchi_lock);
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
					if(lock_bike())
					{
						zt_voice_play(VOICE_LOCK);
					}
					kfd_upload_give_back_package(who_open_electric_gate);
				}
				else if(cmd->para[0]==2)	//服务器判断还车成功之后下发指令
				{
					if(zt_gps_get_pwr_status())
					{
		//				zt_gps_power_off();
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
						StartTimer(GetTimerID(ZT_GPS_DELAY_OFF_TIMER), 900*1000, zt_smart_delay_gps_off);
					}
				}
				break;
			case 0x09:	//报警开关指令
				gps_tracker_config.vibr2_thr = cmd->para[0];
				WriteRecord(GetNvramID(NVRAM_EF_GT_TEMP_THR_LID), 1, &gps_tracker_config.vibr2_thr, sizeof(gps_tracker_config.vibr2_thr),&error);
				break;
			case 0x0a:	//调速
				if(cmd->para[0]==0)
				{
					zt_controller_send(ADDR_CONTROL, CMD_CONTROL,1,HIGH_SPEED);
					controller.require.tiaosu = HIGH_SPEED;
					zt_trace(TPROT,"HIGH Speed");
				}
				else
				{
					zt_controller_send(ADDR_CONTROL, CMD_CONTROL,1,LOW_SPEED);
					controller.require.tiaosu = LOW_SPEED;
					zt_trace(TPROT,"LOW Speed");
				}
				zt_write_config_in_fs(CONTROLLER_FILE,(kal_uint8*)&controller,sizeof(config_struct));
				break;
			case 0x0b:	//调欠压值
				if(cmd->para[0]==0)
				{
					zt_controller_send(ADDR_CONTROL,CMD_CONTROL,2,HIGH);
					controller.require.qianya = HIGH;
					zt_trace(TPROT,"HIGH voltage");
				}
				else
				{
					zt_controller_send(ADDR_CONTROL,CMD_CONTROL,2,LOW);
					controller.require.qianya = LOW;
					zt_trace(TPROT,"LOW voltage");
				}
				zt_write_config_in_fs(CONTROLLER_FILE,(kal_uint8*)&controller,sizeof(config_struct));
				break;
			case 0x0c:	//助力切换
				if(cmd->para[0]==0)
				{
					zt_controller_send(ADDR_CONTROL,CMD_CONTROL,3,DIANDONG);
					controller.require.zhuli = DIANDONG;
					zt_trace(TPROT,"CHUN diandong");
				}
				else if(cmd->para[0]==1)
				{
					zt_controller_send(ADDR_CONTROL,CMD_CONTROL,3,ZHULI);
					controller.require.zhuli = ZHULI;
					zt_trace(TPROT,"ZHULI 1:1");
				}
				else if(cmd->para[0]==2)
				{
					zt_controller_send(ADDR_CONTROL,CMD_CONTROL,3,ZHULI2);
					controller.require.zhuli = ZHULI2;
					zt_trace(TPROT,"ZHULI 1:2");
				}
				else if(cmd->para[0]==3)
				{
					zt_controller_send(ADDR_CONTROL,CMD_CONTROL,3,RENLI); 
					controller.require.zhuli = RENLI;
					zt_trace(TPROT,"CHUN RENLI");
				}
				else
				{
					zt_controller_send(ADDR_CONTROL,CMD_CONTROL,3,HUNHE); 
					controller.require.zhuli = HUNHE;
					zt_trace(TPROT,"DIANDONG+1:2 ZHULI");
				}
				zt_write_config_in_fs(CONTROLLER_FILE,(kal_uint8*)&controller,sizeof(config_struct));
				break;
			case 0x0d:	//故障修复
				if(cmd->para[0]==1)
				{
					zt_controller_send(ADDR_CONTROL,CMD_CONTROL,4,XF_OK);
					controller.require.xf = XF_OK;
					zt_trace(TPROT,"GUZHANG XIUFU");
				}
				else if(cmd->para[0]==0)
				{
					zt_controller_send(ADDR_CONTROL,CMD_CONTROL,4,XF_INVALID);
					controller.require.xf = XF_INVALID;
					zt_trace(TPROT,"GUZHANG QINGCHU");
				}
				zt_write_config_in_fs(CONTROLLER_FILE,(kal_uint8*)&controller,sizeof(config_struct));				
				break;	
			case 0x0e:	//切换电源电压
				if(cmd->para[0]==0)//36V
				{
					zt_controller_send(ADDR_CONTROL,CMD_CONTROL,5,VOT36V);
					controller.require.dy = VOT36V;
					zt_trace(TPROT,"QIEHUAN 36V");
				}
				else if(cmd->para[0]==1)//48V
				{
					zt_controller_send(ADDR_CONTROL,CMD_CONTROL,5, VOT48V);
					controller.require.dy = VOT48V;
					zt_trace(TPROT,"QIEHUAN 48V");
				}
				zt_write_config_in_fs(CONTROLLER_FILE,(kal_uint8*)&controller,sizeof(config_struct));				
				break;	
			case 0x0f:
				if(cmd->para[0]==0)	//普通电机
				{
					default_set.motor = 0;
					zt_trace(TPROT,"General Motor");
				}
				else if(cmd->para[0]==1)//高速电机
				{
					default_set.motor = 1;
					zt_trace(TPROT,"High Speed Motor");
				}
				zt_write_config_in_fs(SETTING_FILE,(kal_uint8*)&default_set,sizeof(default_setting_struct));				
				break;
		#ifdef __CHAOWEI__		
			case 0x10:	//电门
				if(cmd->para[0]==0)//ACC ON
				{
					zt_controller_send(ADDR_CONTROL,CMD_CONTROL,6,ACC_ON);
					controller.require.acc= ACC_ON;
					zt_trace(TPROT,"ACC_ON");
				}
				else if(cmd->para[0]==1)//ACC_OFF
				{
					zt_controller_send(ADDR_CONTROL,CMD_CONTROL,6, ACC_OFF);
					controller.require.acc= ACC_OFF;
					zt_trace(TPROT,"ACC_OFF");
				}
				zt_write_config_in_fs(CONTROLLER_FILE,(kal_uint8*)&controller,sizeof(config_struct));	
				break;
			case 0x11:	//设置防御
				if(cmd->para[0]==0)//设防
				{
					zt_controller_send(ADDR_CONTROL,CMD_CONTROL,7,SHEFANG);
					controller.require.fy = SHEFANG;
					zt_trace(TPROT,"SHEFANG");
				}
				else if(cmd->para[0]==1)//撤防
				{
					zt_controller_send(ADDR_CONTROL,CMD_CONTROL,7, CHEFANG);
					controller.require.fy = CHEFANG;
					zt_trace(TPROT,"CHEFANG");
				}
				zt_write_config_in_fs(CONTROLLER_FILE,(kal_uint8*)&controller,sizeof(config_struct));	
				break;	
		#endif		
			case 0x20:	//切换服务器
				if(cmd->para[0]==1)
				{//现网转测试服务器
					
				}
				else if(cmd->para[0]==2)
				{//测试服务器转现网服务器
				}
				break;
		#ifdef __HW_2018__		
			case 0x21:	//蓝牙名称
				{
					char bt_name[6]={0};
					char param[16]={0};

					memcpy(bt_name,cmd->para,6);
					sprintf(param,"CC90%s",bt_name);

					zt_trace(TPERI,"bt name=%s",bt_name);
					bt_prepare_send_data(BT_UART_NAME,strlen(param),param);
				}
				break;
			case 0x22:	//rssi值
				{
					char param[2];
					param[0]=cmd->para[0];	//开锁rssi
					param[1]=cmd->para[1];	//锁车rssi

					zt_trace(TPERI,"开锁rssi:%d,锁车rssi:%d",param[0],param[1]);
					bt_prepare_send_data(BT_UART_RSSI,2,param);
				}
				break;
			case 0x23:
				{
					char param[4];
					
					memcpy(param,cmd->para,4);
					zuche_stamptime = param[3]*0x1000000+param[2]*0x10000+param[1]*0x100+param[0];
					default_set.timestamp = zuche_stamptime;
					zt_trace(TPERI,"租车有效期%d,%d %d %d %d",zuche_stamptime,cmd->para[0],cmd->para[1],cmd->para[2],cmd->para[3]);
					zt_write_config_in_fs(SETTING_FILE,(kal_uint8*)&default_set,sizeof(default_setting_struct));				
					break;
				}
		#endif		
			default:
				break;
		}	
	}
}

void zt_smart_pre_uart_data(void)
{
	static kal_uint8 index=0;

	if(index==0)
	{
		zt_controller_send(ADDR_CONTROL, CMD_READ, 0x00, 0x00);
	}

	if(controller.require.tiaosu != controller.actual.tiaosu && index==1)
	{
		zt_controller_send(ADDR_CONTROL, CMD_CONTROL,1,controller.require.tiaosu);
	}

	if(controller.require.qianya != controller.actual.qianya && index==2)
	{
		zt_controller_send(ADDR_CONTROL, CMD_CONTROL,2,controller.require.qianya);
	}

	if(controller.require.zhuli != controller.actual.zhuli && index==3)
	{
		zt_controller_send(ADDR_CONTROL, CMD_CONTROL,3,controller.require.zhuli);
	}

	if(controller.require.xf != controller.actual.xf && index==4)
	{
//		zt_controller_send(ADDR_CONTROL, CMD_CONTROL,4,controller.require.xf);
	}

	if(controller.require.dy != controller.actual.dy && index==5)
	{
		zt_controller_send(ADDR_CONTROL, CMD_CONTROL,5,controller.require.dy);
	}
#ifdef __CHAOWEI__		
	if(controller.require.acc != controller.actual.acc && index==6)
	{
		zt_controller_send(ADDR_CONTROL, CMD_CONTROL,6,controller.require.acc);
	}

	if(controller.require.fy!= controller.actual.fy&& index==7)
	{
		zt_controller_send(ADDR_CONTROL, CMD_CONTROL,7,controller.require.fy);
	}
#elif defined(__BAT_PROT__)	
	if(index==6)
	{
		zt_bat_send();
	}
#endif	
/*	zt_controller_send(ADDR_BAT, bat_temp, 0x00, 0x00);
	zt_controller_send(ADDR_BAT, bat_vol, 0x00, 0x00);
	zt_controller_send(ADDR_BAT, bat_curr, 0x00, 0x00);
	zt_controller_send(ADDR_BAT, bat_cap, 0x00, 0x00);
	zt_controller_send(ADDR_BAT, bat_total_cap, 0x00, 0x00);
	zt_controller_send(ADDR_BAT, bat_cycle, 0x00, 0x00);
	zt_controller_send(ADDR_BAT, bat_interval, 0x00, 0x00);
	zt_controller_send(ADDR_BAT, bat_max_interval, 0x00, 0x00);*/
#ifdef __CHAOWEI__	
	if(index>=7)
#else
	if(index>=6)
#endif		
	{
		index=0;
		StopTimer(GetTimerID(ZT_UART_INTERVAL_SEND_TIMER));
	}
	else
	{
		index++;
		StartTimer(GetTimerID(ZT_UART_INTERVAL_SEND_TIMER),500,zt_smart_pre_uart_data);
	}
}
kal_uint16 zt_convert_adc(kal_uint16 adc)
{
	kal_uint16 result;
	result = (((adc*1024/2800)*34*28)/(31*33))&0xffff;
	
	return result;
}
kal_uint32 adc_convert_mv(kal_uint16 adc_mv)
{
	kal_uint32 result;

	result = adc_mv*34;
//	zt_trace(TPERI,"adc_mv=%d,Vol=%dmv",adc_mv,result);
	return result;
}

void zt_smart_update_network_data(gps_tracker_control_data_struct* package)
{
	gps_info_struct* curr_gps_data = (gps_info_struct* )zt_gps_get_curr_gps();
	ebike_struct ebike={0};

	zt_trace(TPERI,"Get pwr data");

	package->addr = 0x1d;
	package->value_len = sizeof(ebike_struct);

	ebike.fault = controller.actual.fault;
	if(controller.actual.tiaosu==HIGH_SPEED)
		ebike.status.tiaosu = 0;
	else if(controller.actual.tiaosu==LOW_SPEED)
		ebike.status.tiaosu = 1;
	
	if(controller.actual.qianya==HIGH)
		ebike.status.qianya = 0;
	else if(controller.actual.qianya==LOW)
		ebike.status.qianya = 1;

	if(controller.actual.zhuli==DIANDONG)
		ebike.status.zhuli = 0;
	else if(controller.actual.zhuli==ZHULI)
		ebike.status.zhuli = 1;
	else if(controller.actual.zhuli==ZHULI2)
		ebike.status.zhuli = 2;
	else if(controller.actual.zhuli==RENLI)
		ebike.status.zhuli = 3;
	else if(controller.actual.zhuli==HUNHE)
		ebike.status.zhuli = 4;
	
	if(controller.actual.dy==VOT36V)
		ebike.status.dy = 0;
	else if(controller.actual.dy==VOT48V)
		ebike.status.dy = 1;
	
	if(controller.actual.xf==XF_INVALID)
		ebike.status.xf = 0;
	else if(controller.actual.xf==XF_OK)
		ebike.status.xf = 1;	
#ifdef __CHAOWEI__		
	if(controller.actual.acc==ACC_ON)
		ebike.status.acc = 0;
	else if(controller.actual.acc==ACC_OFF)
		ebike.status.acc = 1;
	
	if(controller.actual.fy==SHEFANG)
		ebike.status.fy = 0;
	else if(controller.actual.fy==CHEFANG)
		ebike.status.fy= 1;
#endif

	if(who_open_electric_gate)
		ebike.status.lock = 0;
	else
		ebike.status.lock = 1;

	if(gps_tracker_config.vibr2_thr == 1)
		ebike.status.alarm= 1;	
	else
		ebike.status.alarm = 0;

	if(default_set.motor == 1)
		ebike.status.motor = 1;
	else
		ebike.status.motor = 0;
#ifndef __CHAOWEI__
	if(default_set.zd_alarm == 1)
		ebike.status.zd_alarm = 1;
	else
		ebike.status.zd_alarm = 0;
#endif

#ifdef __MEILING__
	ebike.hall = curr_lundong/8;
#else
	if(default_set.motor==1)
		ebike.hall = curr_lundong/8;
	else
		ebike.hall = curr_hall/8;
#endif
	ebike.bat.temp = curr_bat.temp;
	if(curr_bat.voltage>0)	
		ebike.bat.voltage = curr_bat.voltage/10;
	else
		ebike.bat.voltage = (kal_uint16)(adc_convert_mv(zt_adc_get_aver_value())/10);

	ebike.bat.current= curr_bat.current;
	ebike.bat.residual_cap= curr_bat.residual_cap;
	ebike.bat.total_cap= curr_bat.total_cap;
	ebike.bat.cycle_count= curr_bat.cycle_count;
	ebike.bat.interval= curr_bat.interval;
	ebike.bat.max_interval= curr_bat.max_interval;

	ebike.sig.gsm_signal = (U8)srv_nw_info_get_signal_strength_in_percentage(MMI_SIM1);
	ebike.sig.gps_viewd = curr_gps_data->sat_view;
	ebike.sig.gps_used = curr_gps_data->sat_uesd;
	
	memcpy(package->value,&ebike,sizeof(ebike_struct));
	zt_smart_write_hall();
#ifdef __MEILING__	
	zt_smart_write_lundong();
#else
	if(default_set.motor==1)
	{
		zt_smart_write_lundong();
	}
#endif
	zt_trace(TPERI,"%s,len=%d",__func__,sizeof(ebike_struct));

}


/*****************************************************************************
 * FUNCTION
 *  bt_prepare_send_data
 * DESCRIPTION
 * 准备数据发送给蓝牙app
 * PARAMETERS
 operate:
 0x01	锁车
 0x02	解锁
 0x03	寻车
 0x04 	读取数据
 param_len:
 参数长度
 param:
 参数
 * RETURNS
 *  
 *****************************************************************************/
void bt_prepare_send_data(kal_uint8 operate, kal_uint8 param_len, kal_uint8* param)
{
	kal_uint8 buffer[128]={0};
	kal_uint32 ts = GetTimeStamp();
	kal_uint16 crc;
	kal_uint8 i;

#ifdef __CHAOWEI__	
	buffer[0] = 0x5b;
#else	
	buffer[0] = 0x3a;
#endif

#ifdef __WAIMAI__
	buffer[1] = 0x01;
#else	
	buffer[1] = 0x02;
#endif
	buffer[2] = operate;
	buffer[3] = param_len;
	if(param&&param_len)
		memcpy(buffer+4, param, param_len);

	buffer[4+param_len]=ts&0xff;
	buffer[4+param_len+1]=(ts>>8)&0xff;
	buffer[4+param_len+2]=(ts>>16)&0xff;
	buffer[4+param_len+3]=(ts>>24)&0xff;
#ifdef __CHAOWEI__	
	crc = CRC16_CCITT(buffer+1, 3+param_len+4);
#else	
	crc = get_crc16(buffer+1, 3+param_len+4);
#endif
	buffer[4+param_len+4]=crc&0xff;
	buffer[4+param_len+5]=(crc>>8)&0xff;

#ifdef __CHAOWEI__	
	buffer[4+param_len+6]=0x5d;	
#else	
	buffer[4+param_len+6]=0x0d;	
#endif
	buffer[4+param_len+7]=0x0a;	

/*	for(i=0;i<12+param_len;i++)
		zt_trace(TPERI,"%x",buffer[i]);*/

#ifdef __WAIMAI__
	zt_uart_write_data(uart_port1,buffer,12+param_len);
#else	
	bt_send_data(buffer,12+param_len);
#endif
}

void bt_prepare_send_data_ext(kal_uint8 operate, kal_uint8 param_len, kal_uint8* param)
{
	kal_uint8 buffer[128]={0};
	kal_uint32 ts = GetTimeStamp();
	kal_uint16 crc;
	kal_uint8 i;

#ifdef __CHAOWEI__
	buffer[0] = 0x5b;
#else
	buffer[0] = 0x3a;
#endif
	buffer[1] = 0x02;
	buffer[2] = operate;
	if(param&&param_len)
		memcpy(buffer+3, param, param_len);

	buffer[3+param_len]=ts&0xff;
	buffer[3+param_len+1]=(ts>>8)&0xff;
	buffer[3+param_len+2]=(ts>>16)&0xff;
	buffer[3+param_len+3]=(ts>>24)&0xff;
#ifdef __CHAOWEI__
	crc = CRC16_CCITT(buffer+1, 2+param_len+4);
#else	
	crc = get_crc16(buffer+1, 2+param_len+4);
#endif
	buffer[3+param_len+4]=crc&0xff;
	buffer[3+param_len+5]=(crc>>8)&0xff;
#ifdef __CHAOWEI__
	buffer[3+param_len+6]=0x5d;	
#else	
	buffer[3+param_len+6]=0x0d;	
#endif
	buffer[3+param_len+7]=0x0a;	

//	for(i=0;i<11+param_len;i++)
//		zt_trace(TPERI,"%x",buffer[i]);
	
	bt_send_data(buffer,11+param_len);
}

void read_data(kal_uint8 operate)
{
	read_data_struct data;

	if(curr_bat.voltage>0)	
		data.volt  = curr_bat.voltage/10;
	else
		data.volt  = (kal_uint16)(adc_convert_mv(zt_adc_get_aver_value())/10);
#ifdef __MEILING__
	data.hall = curr_lundong/8;
#else
	if(default_set.motor==1)
		data.hall = curr_lundong/8;
	else
		data.hall = curr_hall/8;
#endif
	if(who_open_electric_gate)
		data.lock = 0;
	else
		data.lock = 1;

	bt_prepare_send_data(operate, 0x07, (kal_uint8*)&data);
}
void send_ok_cmd(kal_uint8 operate)
{
	char param[6]={0}; 

	param[0] = 0;
	bt_prepare_send_data(operate, 1, param);
}
void send_error_cmd(kal_uint8 operate,kal_uint8 type)
{
	char param[6]={0};

	param[0] = type;
	bt_prepare_send_data(operate, 1, param);
}
void bt_open_dianmen(void)
{
	S16 error;
	
	unlock_bike();
	who_open_electric_gate |= BT_OPEN;
	WriteRecord(GetNvramID(NVRAM_EF_ZT_DIANMEN_LID), 1, &who_open_electric_gate, 1, &error);
	StartTimer(GetTimerID(ZT_DIANMEN_UNLOCK_TIMER), 1000,open_dianmen);	
}

kal_uint32 GetTimeStamp(void)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    kal_uint32 timeSecs=0;
    t_rtc rtc;
    struct tm when;

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/

    RTC_GetTime_(&rtc);
    when.tm_sec = rtc.rtc_sec;
    when.tm_min = rtc.rtc_min;
    when.tm_hour = rtc.rtc_hour;
    when.tm_mday = rtc.rtc_day;
    when.tm_mon =rtc.rtc_mon-1;
    when.tm_year = rtc.rtc_year+100;
    when.tm_isdst = 0;
    timeSecs = (kal_uint32) mktime(&when);	
    return timeSecs-60*60*8;
}

void bt_giveback_package(kal_uint8 operate)
{
	bt_giveback_struct bt_giveback_data={0};
	gps_tracker_gps_struct gps_tracker_gps;
	gps_info_struct *gps_info=kfd_get_best_hdop_gps_data();
	kal_uint8 len;

/*增加还车时判断主电源如没插上就提示失败*/
	if(zt_get_bat_connect_status())	
	{
		if(who_open_electric_gate)
		{
			bt_giveback_data.lock_state = 0;
		}
		else
		{
			bt_giveback_data.lock_state = 1;
		}
	}
	else
	{
		bt_giveback_data.lock_state = 0;
	}
	
	if(gps_info)
	{
		kfd_convert_gps_data_for_protocol(gps_info,&gps_tracker_gps);
		
		bt_giveback_data.gps.latitude = gps_tracker_gps.latitude;
		bt_giveback_data.gps.longitude = gps_tracker_gps.longitude;
	}	
	len = sizeof(bt_giveback_struct);

	bt_prepare_send_data_ext(operate, len, (kal_uint8*)&bt_giveback_data);
}

#ifdef __HW_2018__
kal_bool judge_zuche_valid(void)
{
	kal_uint32 stamp=GetTimeStamp();
	if(stamp<zuche_stamptime)
		return KAL_TRUE;
	else
		return KAL_FALSE;
}

void bt_prepare_send_heart_data(kal_uint8 operate, kal_uint8 param_len, kal_uint8* param)
{
	kal_uint8 buffer[128]={0};
	kal_uint32 ts = GetTimeStamp();
	kal_uint16 crc;
	kal_uint8 i;
	
	buffer[0] = 0x3a;
	buffer[1] = 0x01;
	buffer[2] = operate;
	buffer[3] = param_len;
	if(param&&param_len)
		memcpy(buffer+4, param, param_len);

	buffer[4+param_len]=ts&0xff;
	buffer[4+param_len+1]=(ts>>8)&0xff;
	buffer[4+param_len+2]=(ts>>16)&0xff;
	buffer[4+param_len+3]=(ts>>24)&0xff;
	crc = get_crc16(buffer+1, 3+param_len+4);
	buffer[4+param_len+4]=crc&0xff;
	buffer[4+param_len+5]=(crc>>8)&0xff;
	
	buffer[4+param_len+6]=0x0d;	
	buffer[4+param_len+7]=0x0a;	

/*	for(i=0;i<12+param_len;i++)
		zt_trace(TPERI,"%x",buffer[i]);*/

	zt_uart_write_data(uart_port1,buffer,12+param_len);
}

void bt_uart_send_heart(void)
{
	zt_trace(TPERI,"send BT heart,bt_heart_rsp_times=%d",bt_heart_rsp_times);
	bt_prepare_send_heart_data(BT_UART_HEART, 0, NULL);
#ifdef __WAIMAI__
	if(bt_heart_rsp_times>=3)
	{
		bluetooth_reset();
		bt_heart_rsp_times=0;
	}
	else
	{
		bt_heart_rsp_times++;
	}
#endif
	StartTimer(GetTimerID(ZT_BT_UART_HEART_TIMER),10*1000,bt_uart_send_heart);
}

void bt_disconnect_callback(void)
{
	if(!bt_connect_status)
	{
		if(lock_bike())
		{
			zt_voice_play(VOICE_LOCK);
		}
	}
}

void uart1_parse_proc(kal_uint8* buf, kal_uint16 len)
{
	S16 error;
	kal_uint8 out[32]={0};
	kal_uint16 i,crc1,crc2;
	kal_uint8 cmd = buf[2];
	kal_uint8 para_len = buf[3];
	kal_uint32 timestamp1 = GetTimeStamp();
	kal_uint32 timestamp2 = buf[len-8]+buf[len-7]*0x100+buf[len-6]*0x10000+buf[len-5]*0x1000000;

	 zt_hex_convert_str(buf,len,out);
	 zt_trace(TPERI,"bt_recv_uart1=%s,len=%d",out,len);
	 	
	crc1 = get_crc16(buf+1,len-5);
	crc2 = buf[len-4]+buf[len-3]*0x100;
	zt_trace(TPERI,"BT crc1=%x,crc2=%x,timestamp1=%d,timestamp2=%d",crc1,crc2,timestamp1,timestamp2);
	if(crc1 != crc2)
	{
		zt_trace(TPERI,"BT check sum error");
		send_error_cmd(cmd,2);
		return;
	}
	if(abs(timestamp1-timestamp2)>300)
	{
		zt_trace(TPERI,"BT cmd more than 5 minute,return");
//		send_error_cmd(cmd,3);
//		return;
	}

	switch(cmd)
	{
		case BT_UART_NAME:
			if(buf[4]==0)
				zt_trace(TPERI,"BT_UART_NAME success");
			else
				zt_trace(TPERI,"BT_UART_NAME fail");
			break;
		case BT_UART_RSSI:
			if(buf[4]==0)
				zt_trace(TPERI,"BT_UART_RSSI success");
			else
				zt_trace(TPERI,"BT_UART_RSSI fail");
			break;
		case BT_UART_HEART:
			if(buf[4]==0)
			{
				bt_heart_rsp_times=0;
				zt_trace(TPERI,"BT_UART_HEART success,bt_heart_rsp_times=0");
			}
			else
			{
				zt_trace(TPERI,"BT_UART_HEART fail");
			}
			break;
		case BT_UART_LOCK:
			zt_trace(TPERI,"BT_UART_LOCK, buf[4]=%d,who_open_electric_gate=%d",buf[4],who_open_electric_gate);
			if(buf[4]==0)	//锁车
			{
				if(who_open_electric_gate & KEY_OPEN)
				{//error
					send_error_cmd(cmd,1);
				}
				else if(who_open_electric_gate &(GPRS_OPEN|BT_OPEN))
				{
					if(lock_bike())
					{
						zt_voice_play(VOICE_LOCK);
						send_ok_cmd(cmd);	
			  		}
			  		else
			  		{
						send_error_cmd(cmd,1);
			  		}
				}
				else
				{
					send_error_cmd(cmd,1);
				}
			}
			else if(buf[4]==1)	//解锁
			{
				if(!who_open_electric_gate/* && judge_zuche_valid()*/)
				{
					bt_open_dianmen();
					zt_voice_play(VOICE_UNLOCK);
					send_ok_cmd(cmd);
				}
				else
				{
					send_error_cmd(cmd,1);
				}
			}
			break;
		case BT_UART_STATUS:
			zt_trace(TPERI,"BT_UART_STATUS buf[4]=%d",buf[4]);
			if(buf[4]==1)	//蓝牙连接
			{
				bt_connect_status = 1;
				StopTimer(GetTimerID(ZT_BT_DISCONNECT_TIMER));
		//		zt_voice_play(VOICE_SEARCH);
			}
			else if(buf[4]==0)	//蓝牙断开
			{
				bt_connect_status = 0;
				StartTimer(GetTimerID(ZT_BT_DISCONNECT_TIMER),3*1000,bt_disconnect_callback);
		//		zt_voice_play(VOICE_ALARM);
			}
			break;
		default:
			break;
	}
	
}
#endif

void bt_parse_proc(kal_uint8* buf, kal_uint16 len)
{
	S16 error;
	kal_uint8 out[32]={0};
	kal_uint16 i,crc1,crc2;
	kal_uint8 cmd = buf[2];
	kal_uint8 para_len = buf[3];
	kal_uint32 timestamp1 = GetTimeStamp();
	kal_uint32 timestamp2 = buf[len-8]+buf[len-7]*0x100+buf[len-6]*0x10000+buf[len-5]*0x1000000;

	 zt_hex_convert_str(buf,len,out);
	 zt_trace(TPERI,"bt_recv=%s,len=%d",out,len);
#ifdef __CHAOWEI__
	crc1 = CRC16_CCITT(buf+1,len-5);
#else
	crc1 = get_crc16(buf+1,len-5);
#endif
	crc2 = buf[len-4]+buf[len-3]*0x100;
	zt_trace(TPERI,"crc1=%x,crc2=%x,timestamp1=%d,timestamp2=%d",crc1,crc2,timestamp1,timestamp2);
	if(crc1 != crc2)
	{
		zt_trace(TPERI,"check sum error");
		send_error_cmd(cmd,2);
		return;
	}

#ifndef __CHAOWEI__	
	/*1433088000-20150601, 1420041600-20150101*/
	if(abs(timestamp1-timestamp2)>300 && (timestamp1>1433088000||timestamp1<1420041600))
	{
		zt_trace(TPERI,"cmd more than 5 minute,return");
		send_error_cmd(cmd,3);
		return;
	}
#endif

	switch(cmd)
	{
		case BT_LOCK:
		{
			if(who_open_electric_gate & KEY_OPEN)
			{//error
				send_error_cmd(cmd,1);
			}
			else if(who_open_electric_gate &(GPRS_OPEN|BT_OPEN))
			{
				if(lock_bike())
				{
					kfd_upload_ebike_package();
					zt_voice_play(VOICE_LOCK);
					send_ok_cmd(cmd);	
		  		}
		  		else
		  		{
					send_error_cmd(cmd,1);
		  		}
			}
			else
			{
				send_error_cmd(cmd,1);
			}
			break;
		}
		case BT_UNLOCK:
		{
			zt_trace(TPERI,"bluetooth unlock");
			if(!who_open_electric_gate)
			{
				bt_open_dianmen();
				kfd_upload_ebike_package();
				zt_voice_play(VOICE_UNLOCK);
				send_ok_cmd(cmd);
			}
			else
			{
				send_error_cmd(cmd,1);
			}
			break;
		}
		case BT_SEARCH:
		{
			zt_voice_play(VOICE_SEARCH);
			send_ok_cmd(cmd);
			break;
		}
		case BT_READ_DATA:
		{
			read_data(cmd);
			break;
		}
		case BT_DIANCHI:
		{
			open_dianchi_lock();
			StartTimer(GetTimerID(ZT_CLOSE_DIANCHI_LOCK_TIMER),500,close_dianchi_lock);
			send_ok_cmd(cmd);
			break;
		}
		case BT_GIVEBACK:
		{
			if(lock_bike())
			{
				zt_voice_play(VOICE_LOCK);
			}
			bt_giveback_package(cmd);
			break;
		}
		case BT_GIVEBACK_SUCCESS:
		{
			if(zt_gps_get_pwr_status())
			{
//				zt_gps_power_off();
			}
			send_ok_cmd(cmd);
			break;
		}
		case BT_RESET:
		{
			send_ok_cmd(cmd);
			StartTimer(GetTimerID(ZT_DELAY_RESTART_TIMER),1000,restartSystem);
			break;
		}
		case BT_SIGNAL:
		{
			gps_info_struct* curr_gps_data = (gps_info_struct* )zt_gps_get_curr_gps();
			char param[6]={0}; 

			param[0] = (U8)srv_nw_info_get_signal_strength_in_percentage(MMI_SIM1);
			param[1] = curr_gps_data->sat_view;
			param[2] = curr_gps_data->sat_uesd;
			param[3] = GetConnectTimes();
			zt_trace(TPERI,"sig=%d,view=%d,used=%d,connect_times=%d",param[0],param[1],param[2],param[3]);
			bt_prepare_send_data(cmd, 4, param);
			break;
		}
		default:
			send_error_cmd(cmd,1);
			break;
	}
	
}

kal_uint8 bt_parse_actual_data_hdlr(void* info)
{
	bt_msg_struct* bt_msg = (bt_msg_struct*)info;
	kal_uint8 i,*buf,*head=NULL,*tail=NULL;
	kal_uint16 len;
	kal_uint8 req[64]={0};
	kal_uint8 head_first = 1;

	buf = bt_msg->data;
	len = bt_msg->dataLen;
	
	for(i=0; i<len-1; i++)
	{
	#ifdef __CHAOWEI__
		if(buf[i]==0x5b && head_first)
	#else
		if(buf[i]==0x3a && head_first)
	#endif		
		{
			head = buf+i;
			head_first = 0;
		}
	#ifdef __CHAOWEI__	
		else if(buf[i]==0x5d&&buf[i+1]==0x0a)
	#else
		else if(buf[i]==0x0d&&buf[i+1]==0x0a)
	#endif		
		{
			tail = buf+i+2;
			if(head && tail-head==buf[3]+12)
			{
				memcpy(req, head, tail-head);
			#ifdef __WAIMAI__
				uart1_parse_proc(req,tail-head);
			#else
				bt_parse_proc(req,tail-head);
			#endif
			}
		}
	}
	
}

kal_bool zt_smart_check_lundong_is_run(void)
{
//	//zt_trace(TPERI,"%s,lundong_count_1sec=%d",__func__,lundong_count_1sec);
	if(default_set.motor==1)
	{
		if(lundong_count_1sec>=2)
		{
			return KAL_TRUE;
		}
		else
		{
			return KAL_FALSE;
		}
	}
	else
	{
		if(lundong_count_1sec>=4 && lundong_count_1sec<=20)
		{
			return KAL_TRUE;
		}
		else
		{
			return KAL_FALSE;
		}
	}
}

#ifdef __HW_2018__
kal_bool check_zhendong(void)
{
	if(zhendong_count_1sec>100)
		return KAL_TRUE;
	else
		return KAL_FALSE;
}

kal_bool check_sharp_zhendong(void)
{
//	zt_trace(TPERI,"sharp=%d",default_set.zd_sen);
	if(zhendong_count_1sec>default_set.zd_sen)
		return KAL_TRUE;
	else
		return KAL_FALSE;
}
#endif

void zt_alarm_period_callback(void)
{
	alarm_flag = 0;
}

void zt_smart_check_lundong(void)
{
	 static kal_uint32 pre_count=0;
	 kal_uint32 curr_count;
	 static kal_uint32 pre_hall = 0;
	 kal_uint32 curr_hall_tmp = 0;
	 static kal_uint8 bat_flag = 0;

#ifdef __HW_2018__
	 static kal_uint32 pre_zhendong = 0;
	 kal_uint32 curr_zhendong_tmp = 0;

	 curr_zhendong_tmp = zt_lundong_get_curr_count();
	 zhendong_count_1sec = curr_zhendong_tmp-pre_zhendong;
	 pre_zhendong = curr_zhendong_tmp;

//	 zt_trace(TPERI,"zhendong=%d,count=%d,alarm_flag=%d,zd_alarm=%d",zhendong_count_1sec,curr_zhendong_tmp,alarm_flag,default_set.zd_alarm);
#endif	 

	curr_hall_tmp = curr_hall;
	curr_count = curr_lundong;

	hall_count_1sec = abs(curr_hall_tmp - pre_hall);
	pre_hall = curr_hall_tmp;
	
	lundong_count_1sec = curr_count-pre_count;
	pre_count = curr_count;

//	zt_trace(TPERI,"curr_count=%d,lundong_count_1sec=%d",curr_count,lundong_count_1sec);

/*美翎里程信号采用轮动脚，无需轮动锁车，而且霍尔有误信号触发轮动，导致电门时开时关，
而且经常触发会执行delayms(10),导致MMI挂死*/
#ifndef __MEILING__
	if(zt_smart_check_lundong_is_run() && !lundong_is_locking && !tangze_is_locking)
	{
		if(!who_open_electric_gate)
		{
			lundong_is_locking = 1;
			controller_lock_bike();
			if(gps_tracker_config.vibr2_thr==1)
			{
				zt_voice_play(VOICE_ALARM);
			}
		}
	}
#endif

	if(zt_gsensor_check_is_shake_sharp()&& !get_electric_gate_status() && alarm_flag==0)
	{
		kfd_upload_alarm_package();
	#ifdef __CHAOWEI__
		if(gps_tracker_config.vibr2_thr==1)
	#else
		if(default_set.zd_alarm == 1)
	#endif
		{
			zt_voice_play(VOICE_ALARM);
		}
		alarm_flag = 1;
		StartTimer(GetTimerID(ZT_ALARM_PERIOD_TIMER),6000,zt_alarm_period_callback);
	}
	

	if(!zt_get_bat_connect_status() && bat_flag==0)
	{
		kfd_upload_ebike_package();
		bat_flag = 1;
	}
	else if(zt_get_bat_connect_status() && bat_flag==1)
	{
		kfd_upload_ebike_package();
		bat_flag = 0;
	}

	StartTimer(GetTimerID(ZT_SMART_LUNDONG_CHECK_TIMER), 1000, zt_smart_check_lundong);
}

void zt_smart_key_detect_proc(void)
{
	char value;
	static kal_uint8 key_detect_num = 0;

#ifdef __CHAOWEI__
	value = !GPIO_ReadIO(KEY_DETECT_PIN);	// 1 open, 0 close
#else	
	value = GPIO_ReadIO(KEY_DETECT_PIN);	//0 open, 1 close
#endif

	if(!lundong_is_locking && !tangze_is_locking && !IsMyTimerExist(GetTimerID(ZT_DIANMEN_UNLOCK_TIMER)) && !GPIO_ReadIO(DIANMEN_PIN))
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
kal_bool zt_lbs_valid(void)
{
	lbs_info_struct* lbs_info = (lbs_info_struct*)zt_lbs_get_curr_lbs_info();

	if(lbs_info->lbs_server.cellid_nid != 0)
		return KAL_TRUE;
	else
		return KAL_FALSE;
}
kal_bool zt_time_expiry(void)
{
	kal_uint32 real_time = (kal_uint32)GetTimeSec();
//	zt_trace(TPERI,"time_expiry=%d",real_time-pre_time);
	if((real_time-pre_time)>3600)	// 1H
		return KAL_TRUE;
	else
		return KAL_FALSE;
}
void zt_agps_process(void)
{
	if(zt_time_expiry()&&(kal_bool)zt_gps_get_pwr_status()&&!zt_gps_valid()&&GetNetworkService()&&zt_lbs_valid())
	{
		pre_time = (kal_uint32)GetTimeSec();
		zt_trace(TPERI,"LBS REQ, time:%d",pre_time);
		zt_agps_request();
	}
}
void zt_smart_check_gps_pwr(void)
{
//	zt_trace(TPERI,"%s",__func__);
	if(zt_gsensor_check_is_moving()||get_electric_gate_status())
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
	//		zt_gps_power_off();
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
//		unlock_bike();
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
 * kal_uint8 addr  地址
 * cmd cmd_control 控制命令
 *        cmd_read  读取命令
 *  data 1:调速2:调欠压值3:助力切换4:故障修复5:电源电压
 *  data 2: 值
 * RETURNS
 *  void
 *****************************************************************************/
void zt_controller_send(kal_uint8 addr,cmd_enum cmd, kal_uint8 data1,kal_uint8 data2)
{
	kal_uint16 check_sum = 0;
	kal_uint8 i, send_data[] = {0x3a,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0d,0x0a};

	send_data[1] = addr;
	send_data[2] = cmd;
	send_data[3] = 0x02;	//长度
	send_data[4] = data1;
	send_data[5] = data2;

	for(i=0; i<5;i++)
		check_sum += send_data[i+1];
	
	send_data[6] =check_sum&0xff;
	send_data[7] =check_sum>>8;

	zt_trace(TPERI,"uart2 send addr=%x,cmd=%x,data1=%x,data2=%x",addr,cmd,data1,data2);
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

	if(buf[0]==ADDR_CONTROL && get_electric_gate_status())	//控制器地址
	{
		if(buf[1]==0x01)	//控制命令的响应
		{
			zt_trace(TPERI,"Recv Control RSP len = %d",len);
			switch(buf[3])
			{
				case 0x01:	//调速
					if(buf[4])
						zt_trace(TPERI,"Speed change Success");
					else
						zt_trace(TPERI,"Speed change Fail");
					break;
				case 0x02:	//调欠压值
					if(buf[4])
						zt_trace(TPERI,"Qianya Success");
					else
						zt_trace(TPERI,"Qianya Fail");
					break;
				case 0x03:	//助力切换
					if(buf[4])
						zt_trace(TPERI,"Zhuli Success");
					else
						zt_trace(TPERI,"Zhuli Fail");
					break;
				case 0x04:	//故障修复
					if(buf[4])
						zt_trace(TPERI,"Guzhang xiufu Success");
					else
						zt_trace(TPERI,"Guzhang xiufu Fail");
					break;
			#ifdef __CHAOWEI__			
				case 0x05:	//电源电压
					if(buf[4])
						zt_trace(TPERI,"dianya qiehuan Success");
					else
						zt_trace(TPERI,"dianya qiehuan Fail");
					break;
				case 0x06:	//开关电门
					if(buf[4])
						zt_trace(TPERI,"ACC Success");
					else
						zt_trace(TPERI,"ACC Fail");
					break;	
				case 0x07:	//设防/撤防
					if(buf[4])
						zt_trace(TPERI,"fangyu Success");
					else
						zt_trace(TPERI,"fangyu Fail");
					break;
			#endif	
				default:
					break;
			}
		}
		else if(buf[1]==0x02)	//读取命令
		{
			controller.actual.fault = buf[3];
			controller.actual.hall = buf[4]+buf[5]<<8+buf[6]<<16+buf[7]<<24;
			controller.actual.tiaosu = buf[8];
			controller.actual.qianya = buf[9];
			controller.actual.zhuli = buf[10];
			controller.actual.dy = buf[11];
			controller.actual.xf = buf[12];
		#ifdef __CHAOWEI__			
			controller.actual.acc = buf[13];
			controller.actual.fy = buf[14];
		#endif	
			zt_trace(TPERI,"Recv Upload tiaosu=%d,qy=%d,zhuli=%d,dianyuan=%d,xiufu=%d",controller.actual.tiaosu,controller.actual.qianya,
				controller.actual.zhuli,controller.actual.dy,controller.actual.xf);
			zt_write_config_in_fs(CONTROLLER_FILE,(kal_uint8*)&controller,sizeof(config_struct));
		}
	}
	else if(buf[0]==ADDR_BAT)
	{
		kal_uint16 data=buf[3]+buf[4]*0x100;
		switch(buf[1])
		{
			case bat_temp:
				curr_bat.temp = data;
				break;
			case bat_vol:
				curr_bat.voltage = data;
				zt_trace(TPERI,"voltage=%d",data);
				break;
			case bat_curr:
				curr_bat.current = data;
				break;
			case bat_cap:
				curr_bat.residual_cap = data;
				break;
			case bat_total_cap:
				curr_bat.total_cap = data;
				break;
			case bat_cycle:
				curr_bat.cycle_count = data;
				break;
			case bat_interval:
				curr_bat.interval = data;
				break;
			case bat_max_interval:
				curr_bat.max_interval = data;
				break;
		}
	}
	
	return KAL_TRUE;
}

void get_int_num(kal_uint8*buf,kal_uint8*ip1,kal_uint8*ip2,kal_uint8*ip3,kal_uint8*ip4)
{
	kal_uint8 *head,*tail,tmp[4];
	
	head = buf;
	tail = (kal_uint8*)strstr(head,".");
	memset(tmp,0,sizeof(tmp));
	memcpy(tmp,head,tail-head);
	*ip1 = atoi(tmp);

	head = tail+1;
	tail = (kal_uint8*)strstr(head,".");
	memset(tmp,0,sizeof(tmp));
	memcpy(tmp,head,tail-head);
	*ip2 = atoi(tmp);

	head = tail+1;
	tail = (kal_uint8*)strstr(head,".");
	memset(tmp,0,sizeof(tmp));	
	memcpy(tmp,head,tail-head);
	*ip3 = atoi(tmp);

	head = tail+1;
	tail = (kal_uint8*)strstr(head,",");
	memset(tmp,0,sizeof(tmp));	
	memcpy(tmp,head,tail-head);
	*ip4 = atoi(tmp);
}

void restartSystem(void)
{
	zt_reset_system();
}
void modify_service_address(kal_uint8* buf)
{
	//type:2,domain:www.liabar.com,port:9000#
	//type:1,ip:139.224.3.220,port:9000#
	network_info_struct network={0};
	kal_uint8 tmp[64]={0};
	kal_uint8 *head=NULL,*tail=NULL;
	S16 error;
	
	head = (kal_uint8*)strstr(buf,"type:");
	tail = (kal_uint8*)strstr(buf,"#");
	if(head&&tail)
	{
		zt_trace(TPERI,"%s",buf);
		head += strlen("type:");
		tail = (kal_uint8*)strstr(head,",");
		memcpy(tmp, head,tail-head);
		network.ym_type = atoi(tmp);
		zt_trace(TPERI,"network.ym_type=%d",network.ym_type);
		if(network.ym_type==2)
		{
			head = (kal_uint8*)strstr(buf,"domain:");
			if(head)
			{
				head+= strlen("domain:");
				tail = (kal_uint8*)strstr(head,",");
				memcpy(network.domain,head,tail-head);

				head = (kal_uint8*)strstr(buf,"port:");
				head+=strlen("port:");
				tail = (kal_uint8*)strstr(head,"#");
				memset(tmp,0,sizeof(tmp));
				memcpy(tmp,head,tail-head);
				network.port = atoi(tmp);

				sprintf(tmp,"OK type:%d,%s,%d\r\n",network.ym_type,network.domain,network.port);
				zt_uart_write_data(uart_port2, tmp,strlen(tmp));
				zt_write_config_in_fs(NETWORK_FILE, (kal_uint8*)&network,  sizeof(network_info_struct));
				StartTimer(GetTimerID(ZT_DELAY_RESTART_TIMER),1000,restartSystem);
			}
		}
		else if(network.ym_type==1)
		{
			head = (kal_uint8*)strstr(buf,"ip:");
			if(head)
			{
				head+= strlen("ip:");
				tail = (kal_uint8*)strstr(head,",")+1;
				memcpy(tmp, head, tail-head);
				get_int_num(tmp,&network.ip[0],&network.ip[1],&network.ip[2],&network.ip[3]);
							
				head = (kal_uint8*)strstr(buf,"port:");
				head+=strlen("port:");
				tail = (kal_uint8*)strstr(head,"#");
				memset(tmp,0,sizeof(tmp));
				memcpy(tmp,head,tail-head);
				network.port = atoi(tmp);

				sprintf(tmp,"OK type:%d,%d:%d:%d:%d,%d",network.ym_type,network.ip[0],network.ip[1],network.ip[2],network.ip[3],network.port);
				zt_uart_write_data(uart_port2, tmp,strlen(tmp));
				zt_trace(TPERI,"%s",tmp);
				zt_write_config_in_fs(NETWORK_FILE, (kal_uint8*)&network,  sizeof(network_info_struct));
				StartTimer(GetTimerID(ZT_DELAY_RESTART_TIMER),1000,restartSystem);
			}
		}
	}
}

#ifdef __BAT_PROT__
void zt_bat_send(void)
{
	kal_uint8 check_sum = 0;
	kal_uint8 i, send_data[] = {0x7e,0x03,0x01,0x01,0x01,0x00,0x7e};

	for(i=0; i<5;i++)
		check_sum += send_data[i];
	
	send_data[5] =check_sum&0xff;

	zt_trace(TPERI,"zt_bat_send check_sum=%x,data[5]=%x",check_sum,send_data[5]);
	zt_uart_write_data(uart_port2, send_data, sizeof(send_data));
}

void zt_bat_send_hand(void)
{
	kal_uint8 check_sum = 0;
	kal_uint8 i, send_data[] = {0x7e,0x05,0x01,0x00,0x00,0x00,0x7e};
	kal_uint8 key[11]={0};
	kal_uint16 tea;

	sprintf(key,"%s%02d%d%d","liabar",g_bat_cw.curr_vol,g_bat_cw.curr_current,g_bat_cw.rest_per);

	tea = get_crc16(key, 10);
	send_data[3]=tea&0xff;
	send_data[4]=(tea>>8)&0xff;

	for(i=0; i<5;i++)
		check_sum += send_data[i];
	
	send_data[5] =check_sum&0xff;

	zt_trace(TPERI,"zt_bat_send tea=%x,check_sum=%x",tea,check_sum);
	zt_uart_write_data(uart_port2, send_data, sizeof(send_data));
}

void parse_uart2_bat(kal_uint8* buf, kal_uint16 len)
{
	kal_uint16 i;
	kal_uint8 checksum=0;

//累加和校验，包头到校验的前一个字节累加
	for(i=0; i<len-2; i++)
	{
		checksum += buf[i];
	}

	zt_trace(TPERI,"checksum1=%d,checksum2=%d",checksum,buf[len-2]);
	if(checksum == buf[len-2])	
	{
	//通讯协议04
		if(buf[1]==0x02)
		{
			memcpy(&g_bat_cw,buf,len);
			zt_bat_send_hand();	//发送请求大电流的防盗协议
		}
	}
}

kal_bool zt_bat_parse(kal_uint8* buf, kal_uint16 len)
{
	kal_bool ret = KAL_FALSE;
	kal_uint16 i;
	kal_uint8* head=NULL, *tail=NULL;
	kal_uint8 req[64]={0};	

	for(i=0; i<len; i++)
	{
		if(buf[i]==0x7e)
		{
			if(!head)
			{
				head = buf+i;
			}
			else
			{
				tail = buf+i;
				if(tail-head==33)
				{
					parse_uart2_bat(head,tail-head+1);
					ret = KAL_TRUE;
				}
					
			}
		}
			
	}

	return ret;
}
#endif

kal_uint8 parse_uart2_hdlr(void* info)
{
	bt_msg_struct* uart2_msg = (bt_msg_struct*)info;
	kal_uint16 len,i,len2;
	kal_uint8 *buf,*head=NULL,*tail=NULL;
	kal_uint8 req[64]={0};
	kal_bool ret = KAL_FALSE;

	len = uart2_msg->dataLen;
	buf = uart2_msg->data;
	zt_trace(TPERI,"Uart 2 Recv len =%d",len);
	for(i=0; i<len; i++)
	{
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
				ret = zt_controller_proc(req,len2);
			}
		}
	}
#ifdef __BAT_PROT__
	if(!ret)
	{
		ret = zt_bat_parse(buf,len);
	}
#endif

	if(!ret)
	{
		modify_service_address(buf);
	}
	
}

kal_uint8 parse_lbs_hdlr(void* info)
{
	lbs_msg_struct* lbs_msg = (lbs_msg_struct*)info;
	zt_trace(TLBS,"parse_lbs_hdlr num=%d",lbs_msg->lbs.lbs_nbr_num);
	if(lbs_msg->lbs.lbs_server.cellid_nid!=0)
	{
	}
}
/*kal_uint8 parse_battery_hdlr(void* info)
{
	battery_msg_struct* bat_msg = (battery_msg_struct*)info;
	memcpy(&curr_bat, &bat_msg->bat, sizeof(battery_info_struct));	
}*/
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

void zt_smart_write_lundong(void)
{
	S16 error;
	kal_uint8 lundong_array[4];
	kal_uint32 lundong = curr_lundong;
	
	lundong_array[0] = lundong&0xff;
	lundong_array[1] = (lundong>>8)&0xff;
	lundong_array[2] = (lundong>>16)&0xff;
	lundong_array[3] = (lundong>>24)&0xff;
	WriteRecord(GetNvramID(NVRAM_EF_ZT_LUNDONG_LID), 1, lundong_array, 4, &error);
}
void zt_smart_read_lundong(void)
{
	S16 error;
	kal_uint8 lundong_array[4];

	ReadRecord(GetNvramID(NVRAM_EF_ZT_LUNDONG_LID), 1, lundong_array, 4, &error);
	curr_lundong = lundong_array[3]*0x1000000+lundong_array[2]*0x10000+lundong_array[1]*0x100+lundong_array[0];
}

void zt_smart_init(void)
{
	zt_smart_dianmen_init();

	zt_read_config_in_fs(SETTING_FILE,(kal_uint8*)&default_set,sizeof(default_setting_struct));
	zt_trace(TPERI,"default_set.motor=%d",default_set.motor);
#ifdef __HW_2018__
	bluetooth_reset();
	zuche_stamptime = default_set.timestamp;
	if(default_set.zd_sen==0)
		default_set.zd_sen=100;

#ifdef __CHAOWEI__
	GPIO_ModeSetup(KEY_DETECT_PIN,0);
	GPIO_InitIO(0,KEY_DETECT_PIN);
	zt_smart_key_detect_proc();
#endif
#endif	
	zt_read_config_in_fs(CONTROLLER_FILE,(kal_uint8*)&controller,sizeof(config_struct));
	zt_trace(TPERI,"control req: %d %d %d %d %d",controller.require.tiaosu,controller.require.qianya,controller.require.zhuli,controller.require.dy,controller.require.xf);
	zt_trace(TPERI,"control staus: %d %d %d %d %d",controller.actual.tiaosu,controller.actual.qianya,controller.actual.zhuli,controller.actual.dy,controller.actual.xf);


/*总里程*/
	zt_smart_read_hall();
#ifdef __MEILING__
	zt_smart_read_lundong();
#else
	if(default_set.motor==1)
	{
		zt_smart_read_lundong();
	}
#endif
/*轮动报警*/	
	StartTimer(GetTimerID(ZT_SMART_LUNDONG_CHECK_TIMER), 8000, zt_smart_check_lundong);

/*GPS电源开启检测*/
	StartTimer(GetTimerID(ZT_GPS_PWR_CHECK_TIMER), 90000, zt_smart_check_gps_pwr);

/*注册消息的回调函数*/
	mmi_frm_set_protocol_event_handler((U16)GetMsgID(MSG_ID_BT_SEND_TO_MMI_REQ), (PsIntFuncPtr)bt_parse_actual_data_hdlr,  KAL_FALSE);
	mmi_frm_set_protocol_event_handler((U16)GetMsgID(MSG_ID_ADC_UEM_SEND_TO_MMI_REQ), (PsIntFuncPtr)parse_adc_hdlr,  KAL_FALSE);
	mmi_frm_set_protocol_event_handler((U16)GetMsgID(MSG_ID_HALL_EINT_SEND_MMI_REQ), (PsIntFuncPtr)parse_hall_hdlr,  KAL_FALSE);
	mmi_frm_set_protocol_event_handler((U16)GetMsgID(MSG_ID_LUNDONG_SEND_MMI_REQ), (PsIntFuncPtr)parse_lundong_hdlr,  KAL_FALSE);
	mmi_frm_set_protocol_event_handler((U16)GetMsgID(MSG_ID_UART2_SEND_TO_MMI_REQ), (PsIntFuncPtr)parse_uart2_hdlr,  KAL_FALSE);	
	mmi_frm_set_protocol_event_handler((U16)GetMsgID(MSG_ID_BAT_SEND_TO_MMI_REQ), /*(PsIntFuncPtr)parse_battery_hdlr*/NULL,  KAL_FALSE);
	mmi_frm_set_protocol_event_handler((U16)GetMsgID(MSG_ID_LBS_SEND_MMI_REQ), (PsIntFuncPtr)parse_lbs_hdlr,  KAL_FALSE);
}
