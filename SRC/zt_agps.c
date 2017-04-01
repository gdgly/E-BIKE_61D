#include "zt_agps.h"

kal_int8 agps_soc_app_id=-1;

network_para_struct agps_network_para ={
	CONNECT_ONE,
	{
	2,	// 1 ip; 2 domain
	"agps.u-blox.com",	//"agps.co",	//domain
	{0,0,0,0},	//ip
	4,		//ip len
	46434	//80			//port
	},
	zt_agps_login_package,
	NULL,
	zt_agps_parse
};
agps_struct agps_info={
	"full",
	"wangbenwei@cnedc.com",
	"Ljkewo",
	22.543952,
	114.087942,
	3000,
};

/*****************************************************************************
 * FUNCTION
 *  FindString
 * DESCRIPTION
 * 查找ublox字段
 * PARAMETERS
 *  kal_uint8* in	被查找的字串
 *  kal_uint8* out 查出来的字串
 *  kal_uint8* find 查找的关键字
 * RETURNS
 *  kal_uint8
 *****************************************************************************/
kal_uint8 FindString(kal_uint8* in,kal_uint8* out,kal_uint8* find)
{
	kal_uint8* pHead,*pTail;
	kal_uint8 ret =0;
	
	pHead = (kal_uint8*)strstr(in,find);
	pTail = (kal_uint8*)strstr(pHead+strlen(find),"\r\n");
	if(pHead&&pTail)
	{
		pHead = pHead+strlen(find);
		memcpy(out, pHead, pTail-pHead);
		ret = 1;
	}

	return ret;
}
/*****************************************************************************
 * FUNCTION
 *  zt_agps_write
 * DESCRIPTION
 * AGPS 数据写入ublox
 * PARAMETERS
 *  void
 *  
 * RETURNS
 *  void
 *****************************************************************************/
void zt_agps_write(char* buf)
{
	kal_uint8 tmp[32]={0},*pContent;
	kal_uint16 len;

	if(FindString(buf, tmp, "Content-Type: "))
	{
		if(!strcmp(tmp,"application/ubx"))
		{
			memset(tmp,0,sizeof(tmp));
			if(FindString(buf, tmp, "Content-Length: "))
			{
				len = atoi(tmp);
				pContent = (kal_uint8*)strstr(buf,"\r\n\r\n");
				if(pContent)
				{
					zt_trace(TLBS,"Write AGPS data %d",len);
					zt_uart_write_data(uart_port2,pContent+4,len);
				}
			}
		}
	}
}
/*****************************************************************************
 * FUNCTION
 *  zt_agps_parse
 * DESCRIPTION
 * AGPS 解析
 * PARAMETERS
 *  void
 *  
 * RETURNS
 *  void
 *****************************************************************************/
void zt_agps_parse(RcvDataPtr GetRcvData)
{	
	#define LBS_BUF_SIZE 1024*4
	kal_int8 len;
	kal_uint8 *pAgps = NULL;

	pAgps = (kal_uint8*)zt_Malloc(LBS_BUF_SIZE);

	len = GetRcvData(pAgps,LBS_BUF_SIZE);
	zt_trace(TLBS,"Agps len=%d",len);	
	if(len>0)
	{
		zt_agps_write(pAgps);
		zt_socket_free(agps_soc_app_id);
	}
	zt_Free(pAgps);
}

/*****************************************************************************
 * FUNCTION
 *  zt_agps_login_package
 * DESCRIPTION
 * AGPS 服务器登陆
 * PARAMETERS
 *  void
 *  
 * RETURNS
 *  void
 *****************************************************************************/
void zt_agps_login_package(void)
{
	kal_uint8 len;
	kal_uint8 buffer[512] = {0};
	
//	len = sprintf(buffer,"GET /ub?x=%s%s&l=%f,%f&f=400 HTTP/1.1\r\nHost:agps.co\r\n\r\n",serverbuf,nbrbuf,gps_info->latitude, gps_info->longitude);	
	len = sprintf(buffer,"user=%s;pwd=%s;cmd=%s;lat=%.4f;lon=%.4f;pacc=%.0f",agps_info.user,agps_info.pwd,agps_info.cmd,
	agps_info.lat,agps_info.lon,agps_info.pacc);
	zt_trace(TLBS,"%s",buffer);
	
	zt_socket_send(agps_soc_app_id, buffer, len);
}

/*****************************************************************************
 * FUNCTION
 *  zt_agps_request
 * DESCRIPTION
 * AGPS开始
 * PARAMETERS
 *  void
 *  
 * RETURNS
 *  void
 *****************************************************************************/
void zt_agps_request(void)
{
	zt_trace(TLBS,"%s","AGPS REQ");
	agps_soc_app_id = zt_socket_get_app_id();
	zt_socket_launch(agps_soc_app_id, &agps_network_para);
}
/*****************************************************************************
 * FUNCTION
 *  zt_agps_set_location
 * DESCRIPTION
 * 设置经纬度数据
 * PARAMETERS
 *  void
 *  
 * RETURNS
 *  void
 *****************************************************************************/
void zt_agps_set_location(float lat,float lon)
{
	agps_info.lat = lat;
	agps_info.lon = lon;
}
