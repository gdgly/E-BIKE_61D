#ifndef HTTP_H
#define HTTP_H

#include "Http_port.h"


#define VALID_FILE(h) (h>0)
#define URL_MAX_LEN 300
#define HTTP_RECV_BUF_SIZE (5*1024)	//5K
#define VALID_SOCKET(s) (s>0)
#define INVALID_SOCKET -1
#define FILE_MAX_LEN	30
enum
{
	HTTP_POST,
	HTTP_GET
};
enum
{
	HTTP_STS_INIT = 0,//				0
	HTTP_STS_CREATE_SOCKET,//		1
	HTTP_STS_CONNECTING,//			2
	HTTP_STS_RECVING,//				3	//���ڵ�����������ڽ�������
	HTTP_STS_FINISH = 0xffffffff,//				0
};

//����http������
enum
{
	HTTP_EVENT_GETHOST = -100,
	HTTP_EVENT_CONNECTING,//				-101	//
	HTTP_EVENT_CONNECTOK = 300,//				-101	//
	HTTP_EVENT_HDROK,//					-102	//

	HTTP_EVENT_RELINK = -300,
	HTTP_EVENT_TIMEOUT = 502,
	HTTP_EVENT_CREATE_SOCKET_FAIL,//	0xfff3	//����socketʧ��
	HTTP_EVENT_CONNECTFAIL,//			0xfff4	//����ʧ��
	HTTP_EVENT_SAVE_ERR,//				0xfff7	//�޷��������ݣ������Ǵ洢�ռ���
};


typedef void (*TOnHttpEvent)(char* file,int code,int data);
typedef void (*THttpGetHostOK)(void *httpTsk);
typedef void (*TListCB)(void* pData);
typedef int (*TListCompare)(void* data1,void* data2);




//event type
enum
{
    OPENX_SOC_READ    = 0x01,  /* Notify for read */
    OPENX_SOC_WRITE   = 0x02,  /* Notify for write */
    OPENX_SOC_ACCEPT  = 0x04,  /* Notify for accept */
    OPENX_SOC_CONNECT = 0x08,  /* Notify for connect */
    OPENX_SOC_CLOSE   = 0x10   /* Notify for close */
};



typedef enum
{
	eNET_STS_ACTIVED,
	eNET_STS_ACTIVING,
	eNET_STS_DEACTIVING,	
	eNET_STS_DEACTIVED

}TOPENX_NET_STS;




#define OpenX_strncmp	strncmp
#define OpenX_strchr	strchr
#define	OpenX_strncpy	strncpy
#define OpenX_atoi		atoi
#define OpenX_strcat	strcat
#define OpenX_sprintf	sprintf
#define OpenX_strlen	strlen
#define OpenX_memcpy	memcpy
#define	OpenX_strstr	strstr
#define	OpenX_memmove	memmove
#define OpenX_memset	memset
#define	OpenX_strcpy	strcpy
#define	OpenX_strcmp	strcmp

typedef struct  //Լ6K
{
	char LocalFile[FILE_MAX_LEN];//
	char url[URL_MAX_LEN];
	int socketID;
	char RecvBuf[HTTP_RECV_BUF_SIZE];
	int BufDataPos;
	int netFileSize;
	int receivedSize;
	char recevieHTTPHdrOK;
	unsigned int Status;
	unsigned int OldStatus;
	char isGetByRange;
	TOnHttpEvent cb;//�����¼���֪ͨ������ɣ��������ؽ��ȣ��������ش����
	char hostIp[20];//ip����18.2.1.1
	char hostName[70];
	unsigned short port;
	char* netFile;	
	int httpHeaderLen;
	char retryTimes;
	int timeused;
	int para;
	char encoding[10];//��Ҫ���gzip�õ�
	char isChunk;//chuncked��ʽ����

}THttpTaskInfo;

typedef struct  
{
	char LocalFile[FILE_MAX_LEN];//
	char url[URL_MAX_LEN];
	
	TOnHttpEvent cb;
	int data;
}THTTPTaskItem;


typedef struct  
{
	THttpTaskInfo runningTsk;//
	//TList taskList;//THTTPTaskItem��������������������������10���ڵ�
	char netChoose;//CMWAP ,CMNET
	char requestType;//GET,POST
	char keepConnect;//�Ƿ�������0Ϊ����
	char timeOutDetectStarted;
	char lastHost[70];//�ϴ����������
}THttp;

extern THttp *pHttp;//&Http;


void HTTPInit(void);
void HTTPUnInit(void);
void HTTPDownFile(char*Url, char* FileName,TOnHttpEvent cb,int data);
void HTTPCancelTask(char* FileName);//�����ļ�����ȡ����������
void HTTPCancelAll(void);//ȡ�����������������˳�ʱ��������
void HTTPChooseNet(int net);//ѡ��CMNET or CMWAP

#endif






