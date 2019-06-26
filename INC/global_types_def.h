#ifndef __POLLEX_GLOBAL_TYPES_DEF_H__
#define __POLLEX_GLOBAL_TYPES_DEF_H__

#ifndef NULL
#ifdef __cplusplus
#define NULL    (0)
#else
#define NULL    ((void *)0)
#endif
#endif // NULL
#ifndef null
#define null NULL
#endif


	
										  
typedef unsigned char					  U8;
typedef char 					  S8;
typedef unsigned short					  U16;
typedef signed short					  S16;
typedef unsigned int					  U32;
typedef signed int						  S32;
typedef unsigned long long	 U64;
typedef long long			 S64;


typedef unsigned char           kal_uint8;
typedef signed char             kal_int8;
typedef char                    kal_char;
typedef unsigned short          kal_wchar;
typedef unsigned short int      kal_uint16;
typedef signed short int        kal_int16;
typedef unsigned int            kal_uint32;
typedef signed int              kal_int32;



typedef U16              module_type;
typedef U16              msg_type;
typedef U16              sap_type;


typedef int             MMI_BOOL;

typedef enum 
{
  KAL_FALSE,
  KAL_TRUE
} kal_bool;

										  

typedef void (*oslTimerFuncPtr)(void*);
typedef void (*oslTaskFuncPtr)(void*);

typedef void (*mmi_void_funcptr_type)(void);
typedef void (*mmi_int_funcptr_type)(S32);
typedef void (*FuncPtr) (void);
typedef void (*FuncPtrShort) (U16);
typedef void (*PsFuncPtr) (void *);
typedef U8(*PsIntFuncPtr) (void *);
typedef void (*PsFuncPtrU32) (void *, U32);
typedef void (*PsExtFuncPtr) (void *, int);
typedef U8(*PsExtIntFuncPtr) (void *, int);
typedef void (*PsExtPeerFuncPtr) (void *, int, void *);
typedef U8(*PsExtPeerIntFuncPtr) (void *, int, void *);













#endif 
