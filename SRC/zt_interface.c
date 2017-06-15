#include "zt_interface.h"

kal_bool zt_write_config_in_fs(UI_character_type*filename,kal_uint8* buf, kal_uint16 len)
{
	FS_HANDLE file_handle;
	kal_uint32 write_len;
	kal_bool result=KAL_FALSE;
	
	file_handle = FS_Open(filename, FS_CREATE|FS_READ_WRITE);
	if(file_handle>=FS_NO_ERROR)
	{
		FS_Seek(file_handle, 0, FS_FILE_BEGIN);
		if(FS_Write(file_handle, buf, len,&write_len)>=FS_NO_ERROR)
		{
			result = KAL_TRUE;
		}
		FS_Close(file_handle);	
	}

	return result;
}

kal_bool zt_read_config_in_fs(UI_character_type*filename,kal_uint8* buf, kal_uint16 size)
{
	FS_HANDLE file_handle;
	kal_uint32 read_len;
	kal_bool result = KAL_FALSE;
	
	file_handle = FS_Open(filename, FS_READ_ONLY);
	if(file_handle>=FS_NO_ERROR)
	{
		FS_Seek(file_handle, 0, FS_FILE_BEGIN);
		if(FS_Read(file_handle, buf, size, &read_len)>=FS_NO_ERROR)
		{
			result = KAL_TRUE;
		}
		FS_Close(file_handle);	
	}
	return result;
}

