#******************************************************************************
#  Filename     :  Makefile
#  Project      :  General Project
#  Language     :  GUN makefile
#  Description  :  Created to define make platform
#  Edit History :  Created by Leo.Feng at 2007-03-14
#******************************************************************************

#******************************************************************************
#PLATFORM
#******************************************************************************
#BUILD_TOOLS  = "C:/Program Files/ARM/ADSv1_2/Bin"
#BUILD_TOOLS  = "C:/Program Files/ARM/RVCT/Programs/3.1/569/win_32-pentium"
BUILD_TOOLS  = "D:\ARM\RVCT\Programs\3.1\569\win_32-pentium"
#*****************************************************************************
# Make file MACRO definitions
#*****************************************************************************
CC            = $(BUILD_TOOLS)/armcc
ASM           = $(BUILD_TOOLS)/armasm


LINK          = $(BUILD_TOOLS)/armlink
LIB           = $(BUILD_TOOLS)/armar

BIN_TOOL      = $(BUILD_TOOLS)/fromelf

RM            = rm
MKDIR         = mkdir
ECHO          = echo
