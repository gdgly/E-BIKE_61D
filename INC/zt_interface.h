#ifndef __ZT_INTERFACE_H__
#define __ZT_INTERFACE_H__
#include "zt_mtk_type.h"

#define CONTROLLER_FILE L"C:\\controller.txt"
#define NETWORK_FILE L"C:\\network.txt"
#define SETTING_FILE L"C:\\default_setting.txt"

kal_bool zt_write_config_in_fs(UI_character_type*filename,kal_uint8* buf, kal_uint16 len);
kal_bool zt_read_config_in_fs(UI_character_type*filename,kal_uint8* buf, kal_uint16 size);

#endif
