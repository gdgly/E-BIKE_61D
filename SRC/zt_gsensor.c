#include "zt_gsensor.h"
#include "zt_trace.h"
#include "zt_mtk_type.h"

#define __ACCE_MMA_7660__
//#define __ACCE_LIS3DH__

#define MAX_SHAKE_NUM 60
kal_uint8 zt_gsensor_curr_shake_value_array[MAX_SHAKE_NUM];

#define SHAKE_BUF_LEN 2
kal_uint8 shake_buf[SHAKE_BUF_LEN] = {0};
kal_uint8 shake_buf_index = 0;

zt_acce_struct zt_acce = {0};

kal_uint8 abs_S8(kal_uint8 v)
{
	if(v<128) 
	{
		return v;
	}
	else
	{
		return 256-v;
	}
}

kal_uint8 zt_gsensor_get_curr_max_shake_value(kal_uint8 during_times)
{
	kal_uint8 i,max_value=0;

	for(i=0; i<during_times; i++)
	{
		if(zt_gsensor_curr_shake_value_array[i] > max_value)
		{
			max_value = zt_gsensor_curr_shake_value_array[i];
		}
	}
	return max_value;
}
/*****************************************************************************
 * FUNCTION
 * zt_gsensor_get_shake_num
 * DESCRIPTION
 * ��ȡһ��ʱ��gsensor��ֵ
 * PARAMETERS
 * kal_uint8 shake_value  ������ֵ
 * kal_uint8 during_times �����ʱ���ڣ���λ�룬���Ϊ60s
 * RETURNS
 *  kal_uint8  ���ش���shake_value�ĸ���
 *****************************************************************************/
kal_uint8 zt_gsensor_get_shake_num(kal_uint8 shake_value, kal_uint8 during_times)
{
	kal_uint8 i,num=0;

	for(i=0; i<during_times; i++)
	{
		if(zt_gsensor_curr_shake_value_array[i] > shake_value)
		{
			num++;
		}
	}
#ifdef ENABLE_LOG		
	zt_trace(TSEN,"%s,shake_value=%d,during_times=%d,num=%d",__func__,shake_value,during_times,num);
#endif
	return num;
}

/*****************************************************************************
 * FUNCTION
 *  zt_gsensor_check_is_shake_sharp
 * DESCRIPTION
 *  ����Ƿ�����˶�
 * PARAMETERS
 *  void  
 *  
 * RETURNS
 *  kal_bool
 *****************************************************************************/
kal_bool zt_gsensor_check_is_shake_sharp(void)
{
	kal_uint8 num;
#ifdef ENABLE_LOG	
	zt_trace(TSEN,"%s", __func__);
#endif
	
#ifdef __ACCE_MMA_7660__
	num = zt_gsensor_get_shake_num(20, 3);
#elif defined(__ACCE_LIS3DH__)
	num = zt_gsensor_get_shake_num(25, 3);
#endif

	if(num>=2)
		return KAL_TRUE;
	else
		return KAL_FALSE;
}

/*****************************************************************************
 * FUNCTION
 *  zt_gsensor_check_is_moving
 * DESCRIPTION
 *  ����Ƿ��ƶ�
 * PARAMETERS
 *  void  
 *  
 * RETURNS
 *  kal_bool
 *****************************************************************************/
kal_bool zt_gsensor_check_is_moving(void)
{
	kal_uint8 num;
#ifdef ENABLE_LOG	
	zt_trace(TSEN,"%s", __func__);
#endif
#ifdef __ACCE_MMA_7660__
	num = zt_gsensor_get_shake_num(5, 5);
#elif defined(__ACCE_LIS3DH__)
	num = zt_gsensor_get_shake_num(8, 5);
#endif

	if(num>=2)
		return KAL_TRUE;
	else
		return KAL_FALSE;
}
/*****************************************************************************
 * FUNCTION
 *  zt_gsensor_check_is_motionless
 * DESCRIPTION
 *   ����Ƿ�ֹ
 * PARAMETERS
 *  void  
 *  
 * RETURNS
 *  kal_bool
 *****************************************************************************/
kal_bool zt_gsensor_check_is_motionless(void)
{
	kal_uint8 num;

#ifdef ENABLE_LOG	
	zt_trace(TSEN,"%s", __func__);
#endif
#ifdef __ACCE_MMA_7660__
	num = zt_gsensor_get_shake_num(6, 60);
#elif defined(__ACCE_LIS3DH__)
	num = zt_gsensor_get_shake_num(8, 60);
#endif

	if(num < 5)
		return KAL_TRUE;
	else
		return KAL_FALSE;
}
/*****************************************************************************
 * FUNCTION
 *  zt_gsensor_get_shake_change
 * DESCRIPTION
 *   ��ȡ �����ݸı�ֵ
 * PARAMETERS
 *  void  
 *  
 * RETURNS
 *  kal_uint8
 *****************************************************************************/
kal_uint8 zt_gsensor_get_shake_change(void)
{ 
	kal_uint8 x=0,y=0,z=0;
	kal_uint8 a=0,b=0,c=0;
	kal_uint8 change = 0;
	kal_uint8 average_change = 0;
	kal_uint8 i;
	kal_uint16 sum = 0;

#ifdef __ACCE_MMA_7660__
	zt_mma_get_acce_xyz(&x, &y, &z);
#elif defined(__ACCE_LIS3DH__)
	zt_lis3dh_get_acce_xyz(&x, &y, &z);
#endif

	x &= 0x3f;
	y &= 0x3f;
	z &= 0x3f;

#ifdef ENABLE_LOG	
	zt_trace(TSEN,"binary: x %d; y %d; z %d", x, y, z);
#endif
	if(x>32) x-=64;
	if(y>32) y-=64;
	if(z>32) z-=64;
	
	a=x;b=y;c=z;
	
	a-=zt_acce.x;
	b-=zt_acce.y;
	c-=zt_acce.z;
#ifdef ENABLE_LOG	
	zt_trace(TSEN,"changed abs: x %d; y %d; z %d",abs_S8(a), abs_S8(b), abs_S8(c));
#endif
	
	change = abs_S8(a) + abs_S8(b) + abs_S8(c);
	
	zt_acce.x = x;
	zt_acce.y = y;
	zt_acce.z = z;  

	//��������ƽ��
	shake_buf_index++;
	shake_buf_index %= SHAKE_BUF_LEN;
	shake_buf[shake_buf_index] = change;

	for(i = 0; i< SHAKE_BUF_LEN; i++)
		sum += shake_buf[i];
	
	average_change = sum/SHAKE_BUF_LEN;
#ifdef ENABLE_LOG		
	zt_trace(TSEN,"%s average_change: %u", __func__,average_change);
#endif

	return average_change;
}

kal_uint8 zt_gsensor_get_curr_shake_value(void)
{
	return zt_gsensor_get_shake_change();
}
void zt_gsensor_shake_detect_timer_proc(void)
{			
	kal_int8 index;

	for(index=MAX_SHAKE_NUM-1;index>=1;index--)
	{
		zt_gsensor_curr_shake_value_array[index]=zt_gsensor_curr_shake_value_array[index-1];
	//	zt_trace(TSEN,"array[%d]=%d",index,zt_gsensor_curr_shake_value_array[index]);
	}
	zt_gsensor_curr_shake_value_array[0] = zt_gsensor_get_shake_change();

	StartTimer(GetTimerID(ZT_GSENSOR_CHECK_TIMER), 1000, zt_gsensor_shake_detect_timer_proc);
} 
/*****************************************************************************
 * FUNCTION
 *  zt_gsensor_acce_init
 * DESCRIPTION
 *  ��ʼ�� ���ٶȴ�����
 * PARAMETERS
 *  void  
 *  
 * RETURNS
 *  void
 *****************************************************************************/
void zt_gsensor_init(void)
{
	StartTimer(GetTimerID(ZT_GSENSOR_CHECK_TIMER), 1000, zt_gsensor_shake_detect_timer_proc);
#ifdef ENABLE_LOG		
	zt_trace(TSEN,"%s",__func__);  
#endif
}
