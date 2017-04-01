/******************************************************************************
 ** DATE:           10/30/2014                                                *
 ** Description:                                                              *
 ******************************************************************************
    
 ******************************************************************************
 **                        Edit History                                       *
 ** --------------------------------------------------------------------------*
 ** DATE           NAME             DESCRIPTION                               *
 ** 10/30/2014     wongshan         Create.                                   *
 *****************************************************************************/
/**---------------------------------------------------------------------------*
 **                         Dependencies                                      *
**----------------------------------------------------------------------------*/

/**---------------------------------------------------------------------------*
 **                         Compiler Flag                                     *
 **--------------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif
#include "Main.h"

#define OTA_RELEASE_HW_VER "UMEOX61D_WIFI_11C_HW"
#define OTA_RELEASE_BRANCH "11CW1352MP UMEOX61D_WIFI_11C"


extern const S32 service_feature;
extern S32 TK_dll_set_sb(S32 val);
extern void kal_prompt_trace(module_type, const kal_char *fmt,...);
extern void zt_ota_main(void);

//extern void wmp_main_entry(void);
U32 dll_version=0;
U32 dll_feature=0;
U32 dll_edition=0;
void DLL_init_SB(void);

U32 exportfunc[5]={0};

void dll_test_entry(void)
{ 
//	wmp_main_entry(); 
//	WriteLogFile("dll_test_entry begin...");
	zt_ota_main();
//	WriteLogFile("dll_test_entry complate...");

}
//do not modiy 
void Service_Entry(dll_struct* dll)
{
	TK_dll_set_sb(service_feature);
	dll_version = dll->header.version;
	dll_edition = dll->header.feature;
	dll->export_func_count = sizeof(exportfunc)/4;
	dll->export_funcs = exportfunc;
	dll->export_funcs[0] = (S32)DLL_init_SB;
	dll->export_funcs[1] = (S32)dll_test_entry;
	dll->export_funcs[2] = (S32)NULL;
	dll->export_funcs[3] = (S32)NULL;
	dll->export_funcs[4] = (S32)NULL;
	
}

void DLL_init_SB(void)
{
	TK_dll_set_sb(service_feature);
}


#ifdef __cplusplus

}
#endif

