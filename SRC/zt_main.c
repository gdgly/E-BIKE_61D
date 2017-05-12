#include "zt_mtk_type.h"
#include "zt_trace.h"
#include "kfd_protocol.h"
#include "zt_gsensor.h"
#include "zt_smart_control.h"
 
/*****************************************************************************
 * FUNCTION
 *  zt_main_full_service
 * DESCRIPTION
 * 已有网络服务,开始加载网络模块
 * PARAMETERS
 *  void  
 *  
 * RETURNS
 *  void
 *****************************************************************************/
void zt_main_full_service(void)
{
#ifdef ENABLE_LOG		
	zt_trace(TMAIN, "%s",__func__);
#endif
	kfd_connect_service();
} 

/*****************************************************************************
 * FUNCTION
 *  zt_main_wait_full_service
 * DESCRIPTION
 * 等待网络全服务
 * PARAMETERS
 *  void  
 *  
 * RETURNS
 *  void
 *****************************************************************************/
void zt_main_wait_full_service(void)
{
#ifdef ENABLE_LOG		
	zt_trace(TMAIN, "%s",__func__);
#endif

	if(srv_nw_info_get_service_availability(MMI_SIM1) == SRV_NW_INFO_SA_FULL_SERVICE)
	{
		StartTimer(GetTimerID(ZT_NETWORK_CHECK_TIMER),2000,zt_main_full_service);
	}
	else
	{
		StartTimer(GetTimerID(ZT_NETWORK_CHECK_TIMER),1000,zt_main_wait_full_service);
	}
}

/*****************************************************************************
 * FUNCTION
 *  zt_ota_main
 * DESCRIPTION
 * 程序入口
 * PARAMETERS
 *  void  
 *  
 * RETURNS
 *  void
 *****************************************************************************/
void zt_ota_main(void) 
{ 
	zt_trace_set(DEBUG_DLEVEL);   
#ifdef ENABLE_LOG			
	zt_trace(TMAIN, "%s",__func__); 
#endif	  
	StartTimer(GetTimerID(ZT_ONLINE_CHECK_PROTECT_TIMER),120*1000,kfd_reconnect_service);
	kfd_protocol_init(); 
	zt_gsensor_init();
	zt_smart_init();
	zt_get_imsi_request();
	
	zt_main_wait_full_service(); 
}
