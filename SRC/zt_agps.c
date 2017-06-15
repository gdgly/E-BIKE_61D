#include "stdio.h"
#include "zt_agps.h"
#include "zt_trace.h"


kal_int8 agps_soc_app_id=-1;

#define __MINI_AGPS__

network_para_struct agps_network_para ={
#ifdef __MINI_AGPS__
	CONNECT_ONE,
	{
	2,	// 1 ip; 2 domain
	"agps.co",	//domain
	{0,0,0,0},	//ip
	4,		//ip len
	80			//port
	},
	NULL,	//zt_agps_login_package,
	NULL,
	NULL,	//zt_agps_parse
#else
	CONNECT_ONE,
	{
	2,	// 1 ip; 2 domain
	"agps.u-blox.com",	//"agps.co",	//domain
	{0,0,0,0},	//ip
	4,		//ip len
	46434	//80			//port
	},
	NULL,	//zt_agps_login_package,
	NULL,
	NULL,	//zt_agps_parse
#endif	
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
	if(FindString(buf,tmp,"HTTP/1.1 "))
	{
		if(strstr(tmp,"200"))
		{
			memset(tmp,0,sizeof(tmp));
			if(FindString(buf, tmp, "Content-Type: "))
			{
				if(strstr(tmp,"application/ubx"))
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
void zt_agps_parse(kal_int8 socket_id,RcvDataPtr GetRcvData)
{	
	#define LBS_BUF_SIZE 1024*4
	kal_uint16 len;
	kal_uint8 *pAgps = NULL;
	
	pAgps = (kal_uint8*)zt_Malloc(LBS_BUF_SIZE);

	len = GetRcvData(socket_id,pAgps,LBS_BUF_SIZE);
	zt_trace(TLBS,"Agps parse len=%d,pAgps=%s",len,pAgps);	
	if(len>0)
	{
		zt_agps_write(pAgps);
		zt_socket_free(agps_soc_app_id);
	}
	zt_Free(pAgps);
}
void ftoa(char*out, float in)
{
	kal_uint8* pFind=NULL;

	sprintf(out,"%f",in);
	pFind = (kal_uint8*)strstr(out,"F");
	*(pFind)='.';
}

kal_uint16 zt_agps_req_package(kal_uint8* outbuf,lbs_info_struct* lbs)
{
	kal_uint8 i;
	kal_uint8 lbsbuf[128] ={0};
	char nbronebuf[64]={0};
	kal_uint16 len;
	kal_uint8 lat[32]={0},lon[32]={0};

	sprintf(lbsbuf, "%x-%x-%x-%x-%x", lbs->lbs_server.mcc,lbs->lbs_server.mnc,lbs->lbs_server.lac_sid,lbs->lbs_server.cellid_nid,lbs->lbs_server.sig_stren);
	for(i=0; i<lbs->lbs_nbr_num; i++)
	{
		memset(nbronebuf,0,sizeof(nbronebuf));
		sprintf(nbronebuf,"%x-%x-%x",lbs->lbs_nbr[i].lac_sid,lbs->lbs_nbr[i].cellid_nid,lbs->lbs_nbr[i].sig_stren);
		strcat(lbsbuf,"-");
		strcat(lbsbuf,nbronebuf);
	}
	zt_trace(TLBS,"num=%d,%s",lbs->lbs_nbr_num,lbsbuf);
	ftoa(lat,agps_info.lat);
	ftoa(lon,agps_info.lon);
	len = sprintf(outbuf,"GET /ub?x=%s&l=%s,%s&f=400 HTTP/1.1\r\nHost:agps.co\r\n\r\n",lbsbuf, lat, lon);	
	zt_trace(TLBS,"%s",outbuf);

	return len;
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
	kal_uint16 len;
	kal_uint8 buffer[512] = {0};
	
	len = zt_agps_req_package(buffer,(lbs_info_struct*)zt_lbs_get_curr_lbs_info());
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
	agps_network_para.login_callback = zt_agps_login_package;
	agps_network_para.parse_callback = zt_agps_parse;
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
