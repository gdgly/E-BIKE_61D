#ifndef __KIEHFOLJDHFIKSDHFSIDHF__
#define __KIEHFOLJDHFIKSDHFSIDHF__
#include "ProtocolUtilityGPort.h"
#include "AnZhiXingSmsDef.h"
#include "AnZhiXingSocketDef.h"
#include "AnZhiXingUartDef.h"
#include "AnZhiXingListenDef.h"
#include "AnZhiXingEintDef.h"
#include "http_download.h"
typedef int ZFILE;

typedef enum
{
	ZFS_MODE_READ           =   1,	  //可读
	ZFS_MODE_WRITE          =   2,	  //可写
	ZFS_MODE_CREATE         =   4,  //文件不存在时新建
	ZFS_MODE_CREATE_ALWAYS  =   8, //总是新建
        ZFS_MODE_SHARE          = 0x10    //共享打开
}OPENX_FS_OPEN_MODE;

//文件指针移动方式
typedef enum
{
   ZFS_BEGIN=0,					//从文件头
   ZFS_CURRENT,					//从当前位置
   ZFS_END						//从文件尾
}OPENX_FS_SEEK_POS;

typedef enum
{
        DEVICE_SYSTEM,     //系统盘
	DEVICE_TCARD,       //T卡存储器
	DEVICE_NORMAL,    //手机内存盘
	DEVICE_ROOM    // room工作目录
}OPENX_DEVICE_DRIVE;

typedef enum
{
   ZFS_REDNONY = 0x1,	
   ZFS_HIDDEN    = 0x2,
   ZFS_SYSTEM    =0x4 ,
   ZFS_ARCHIVE   = 0x8
}OPENX_FS_ATTRIBUTE;

typedef enum
{
    OPENX_ATTR_READ_ONLY = 1,			//只读属性
    OPENX_ATTR_HIDDEN = 2,			//隐藏属性
    OPENX_ATTR_SYSTEM = 4,			//系统属性
    OPENX_ATTR_DIR = 8,				//目录属性
}OPENX_FS_ATTR;
typedef struct _OpenXSocket{

	int socket_id;
	int nwt_acount_id;


}OpenXSocket;
#define MAX_SOCK_ADDR_LEN           (28)
typedef enum
{
    OPENX_CMWAP,  /* cmwap */
    OPENX_CMNET,       /* cmnet */
    OPENX_WIFI      /*wifi*/
} OPENX_CONNECT_TYPE;
typedef enum
{
    OPENX_SOCK_STREAM = 0,  /* stream socket, TCP */
    OPENX_SOCK_DGRAM       /* datagram socket, UDP */
} OPENX_SOCK_TYPE;
typedef struct
{
    OPENX_SOCK_TYPE	sock_type;
    short	addr_len;
    unsigned short	port;
    unsigned char	addr[MAX_SOCK_ADDR_LEN]; 
} OPENX_SOCKADDR_STRUCT;




typedef struct
{
    int socket_id;
    int event_type;
    int result;
}OpenXSocketEvent,*OpenXSocketEventPtr;
typedef void (*OpenXSocketHandler) (OpenXSocketEventPtr OpenX_socket_evt);
typedef void (*OpenXDnsCb) (char *pIp);
#define OPENX_MAX_SOCKET 10
typedef struct
{
    int socket_id;
    OpenXSocketHandler cb;
} OPENX_SOCKITEM;
extern ZFILE OpenXFileOpen(char* file_name,int flag);
extern int OpenXFileClose(ZFILE hfile);
extern int OpenXFileRead(ZFILE hfile, char* data,int length);
extern int OpenXFileWrite(ZFILE hfile, char *data, int length);
extern int OpenXFileCommit(ZFILE hfile);
extern int OpenXFileSeek(ZFILE hfile,int offset,OPENX_FS_SEEK_POS pos);
extern int OpenXFileTell(ZFILE hfile);
extern BOOL OpenXFileExist(char* file_name);
extern int OpenXFileGetSize(ZFILE hfile);
extern int OpenXFileDelete(char* file_name);
extern int OpenXFileRename(char* old_name,char* new_name);
extern int OpenXFileSetAtrributes(char* file_name,OPENX_FS_ATTRIBUTE attrib);
extern int OpenXCreateDir(char* dir_name);
extern int OpenXRemoveDir(char* dir_name);

extern int OpenXGetDrive(OPENX_DEVICE_DRIVE drive);
extern int OpenXGetDriveInfo(OPENX_DEVICE_DRIVE drive, U64 *totalspace, U64 *freespace);

typedef void (*OpenXFileFindCb)(U16* ret_name,int size,OPENX_FS_ATTR attr);
extern int OpenXFileSearch(U16* name_pattern, OpenXFileFindCb cb);



extern int OpenXSocketCreate(OPENX_SOCK_TYPE protype, OPENX_CONNECT_TYPE connecttype ,OpenXSocketHandler cb);
extern int OpenXSocketConnect(int socket_id,U32 ip,U16 port);
extern int OpenXSocketClose(int socket_id);
extern void OpenXSocketExit();
extern int OpenXSocketSend(int socket_id,char* buf,int len);
extern int OpenXSocketRecv(int socket_id, char* buf,int len);

extern int OpenXSocketBind(int socket_id, OPENX_SOCKADDR_STRUCT *addr);
extern int OpenXSocketSendTo(int socket_id,char* buf,int len,int flag, OPENX_SOCKADDR_STRUCT *addr);
extern int OpenXSocketRecvFrom(int socket_id, char* buf,int len,int flag, OPENX_SOCKADDR_STRUCT *addr);
extern int OpenXSocketGetHostByName(char *host,char *ip_address,OpenXDnsCb cb);

extern int OpenXGetNetStatus(void);

extern int OpenXUCSStrlen(U16* str);
extern int OpenXUCSStrcmp(U16* str_src,U16* str_dst);
extern int OpenXUCSStrncmp(U16* str_src,U16* str_dst,int count);
extern U16* OpenXUCSStrcpy(U16* str_dst,U16* str_src);
extern U16* OpenXUCSStrncpy(U16* str_dst,U16* str_src,int count);
extern U16* OpenXUCSStrcat(U16* str_dst,U16* str_src);
extern U16* OpenXUCSStrstr(U16* str,U16* str_char_set);
#endif