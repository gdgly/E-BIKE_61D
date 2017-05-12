;/******************************************************************************
; ** DATE:           10/29/2015                                                *
; ** Copyright:      2015 EKool-Tech, Incoporated. All Rights Reserved.        *
; ** Description:                                                              *
; ******************************************************************************


    PRESERVE8 
    AREA ServiceHeader, CODE, READONLY

    IMPORT    Service_Entry        
    IMPORT    |Image$$ER_RO$$Length|
    IMPORT    |Image$$ER_RW$$Length|
    IMPORT    |Image$$ER_ZI$$ZI$$Length|

    EXPORT service_feature




;******************************************************************************    
; Added by JasonMa for version checking
service_version
    DCD       0x00000002
service_feature
    DCD       0x00000001

service_entry_offset
    DCD       Service_Entry - {PC}
    
ro_size
    DCD       |Image$$ER_RO$$Length|    
rw_size    
    DCD       |Image$$ER_RW$$Length|
zi_size    
    DCD       |Image$$ER_ZI$$ZI$$Length|



;******************************************************************************    
;The Kernel Export function pointer table
        GBLL    Is_ARM
Is_ARM  SETL    {CODESIZE}=32

        MACRO
        Veneer  $name
        LCLS    subname
        LCLS    supname
subname SETS    "$name"
supname SETS    "|$$Super$$$$" :CC: subname :CC: "|"
subname SETS    "|$$Sub$$$$" :CC: subname :CC: "|"
        EXPORT  $subname
        IMPORT  $supname
        IF Is_ARM
            CODE32
$subname    LDR     pc, =$supname
            LTORG            
        ELSE
            CODE16
$subname    BX      pc
            CODE32
            LDR     ip, {PC}+8
            BX      ip
            DCD     $supname+1
        ENDIF
        MEND
        
;       /*Add other export api according to the format of above*/ 

	Veneer WriteLogFile
	Veneer zt_trace
	Veneer zt_trace_set
	Veneer zt_socket_launch
	Veneer zt_socket_get_app_id
	Veneer zt_socket_free
	Veneer zt_socket_send
	Veneer zt_lis3dh_get_acce_xyz
	Veneer zt_mma_get_acce_xyz
	Veneer zt_mma_acce_get_tilt
	Veneer zt_voice_play
	Veneer zt_reset_system
	Veneer ztDelayms
	Veneer ztDelayus
	Veneer ReadRecord
	Veneer WriteRecord
	Veneer GetNvramID
	Veneer StartTimer
	Veneer StopTimer
	Veneer GetTimerID
	Veneer srv_nw_info_get_service_availability
	Veneer zt_lbs_req
	Veneer zt_lbs_get_curr_lbs_info
	Veneer applib_dt_increase_time
	Veneer applib_dt_decrease_time
	Veneer RTC_GetTime_
	Veneer mmi_dt_set_rtc_dt
	Veneer zt_gps_get_curr_gps
	Veneer zt_gps_power_off
	Veneer zt_gps_power_on
	Veneer zt_gps_get_pwr_status
	Veneer srv_imei_get_imei
	Veneer open_led2
	Veneer close_led2
	Veneer zt_led_open_gsm_led
	Veneer zt_led_close_gsm_led
	Veneer zt_led_open_gps_led
	Veneer zt_led_close_gps_led
	Veneer GPIO_ModeSetup
	Veneer GPIO_InitIO
	Veneer GPIO_WriteIO
	Veneer GPIO_ReadIO
	Veneer GetMsgID
	Veneer ConstructMsgBuf
	Veneer SendMsg
	Veneer Get_MMI_MOD
	Veneer Get_BT_MOD
	Veneer mmi_frm_set_protocol_event_handler
	Veneer mmi_frm_clear_protocol_event_handler
	Veneer zt_trace_ota
	Veneer bt_send_data
	Veneer parse_ver_package	
	Veneer zt_start_timer
	Veneer zt_hall_get_curr_count
	Veneer zt_lundong_get_curr_count
	Veneer bluetooth_reset
	Veneer GetHWVersion
	Veneer FS_Open
	Veneer FS_Close
	Veneer FS_Read
	Veneer FS_Write
	Veneer FS_Seek
	Veneer FS_Commit
	Veneer FS_GetFilePosition
	Veneer FS_SetAttributes
	Veneer FS_GetAttributes
	Veneer FS_GetFileSize
	Veneer FS_Rename
	Veneer FS_Delete
	Veneer FS_CreateDir
	Veneer FS_FindFirst
	Veneer FS_FindNext
	Veneer FS_FindClose
	Veneer kal_wsprintf
	Veneer zt_uart_init_port
	Veneer zt_uart_write_data
	Veneer IsMyTimerExist
	Veneer zt_Malloc
	Veneer zt_Free
	Veneer GetTimeSec
	Veneer zt_battery_handshake_status
	Veneer srv_nw_info_get_signal_strength_in_percentage
	Veneer zt_get_imsi
	Veneer zt_get_imsi_request

;---------------------------------------------------------
;	adding new api under this line
;	for ex: Veneer xxxxxxx

;---------------------------------------------------------

	EXPORT TK_dll_set_sb

;Get_Image_Info
;        STMDB R13!, {R0-R1, LR}
;        LDR  R1, service_entry_offset
;        STR  R1, [R0]
;        LDR  R1, ro_size
;        STR  R1,[R0,#0x4]
;        LDMIA R13!, {R0-R1, PC}
;        END

;	STR lr, [sp, #-4]!
;	LDR pc, [sp], #4 


TK_dll_set_sb
	STMDB SP!, {R1, LR}
	MOV R1, SB
	MOV SB, R0
	MOV R0, R1
	LDMIA SP!, {R1, PC}
        END
