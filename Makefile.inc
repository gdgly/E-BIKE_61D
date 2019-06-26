#******************************************************************************
#  Filename     :  Makefile
#  Project      :  8800H_ChipTest
#  Language     :  CEVAX-ASM CEVAX-C
#  Description  :  ChipeTest is created to test 8800H chip
#  Edit History :  Created by Leo.Feng at 2007-03-14
#******************************************************************************


#*****************************************************************************
# List of suffixes
#*****************************************************************************

.SUFFIXES:
.SUFFIXES:  .asm .c .o .lib .a
	


#*****************************************************************************
# Include path define
#*****************************************************************************
#ASM_INC = -I INC,../COMMON/INC,../../../COMMON/INC
#C_INC   = -I. -I./INC -I../../INC -I../COMMON/INC -I../../../COMMON/INC\

include wmp.inc


#*****************************************************************************
# C Compile option
#*****************************************************************************
PREPROC_FLAGS = -m -n -i
C_FLAGS       = -c -O2 -D__MTK__ -apcs /ropi/rwpi/interwork $(C_INC) -D_MAP_BROWSE_SUPPORT_ 
#C_FLAGS       = -c -O2 -D__MTK__ -apcs /ropi/rwpi/interwork $(C_INC) -D_MAP_BROWSE_SUPPORT_ -D_RELEASE_
#C_FLAGS       = -c -O2 -D__MTK__ -apcs /ropi/rwpi/interwork $(C_INC) -D_MAP_BROWSE_SUPPORT_ -D_FEE_SUPPORT_ -D_RELEASE_


#*****************************************************************************
# ASM Compile option
#*****************************************************************************
ASM_FLAGS     =  -32 -g -apcs /ropi/rwpi/interwork $(ASM_INC)


#*****************************************************************************
# LIB Compile option
#*****************************************************************************
LIB_FLAGS     = -a

#*****************************************************************************
# LINK Compile option
#*****************************************************************************
LINK_FLAGS    = -xref -remove -ropi -ro-base 0x8000 -rwpi -rw-base 0x1000000 -map -callgraph -symbols -info sizes,totals,unused -list


MAKEFILE = makefile

#*****************************************************************************
# Local Source File Directories
#*****************************************************************************

INC_DIR       = INC
SRC_DIR       = SRC

#VPATH = MainFrame/SRC MainFrame/SRC1
include wmp.pth

OBJ_DIR       = OBJ
LIB_DIR       = LIB

SUB_DIRS = $(OBJ_DIR)

#*****************************************************************************
# Source File Definitions
#*****************************************************************************
LIB_OBJS_ASM = $(LIB_SRCS_ASM:.s=.o)
LIB_OBJS_C   = $(LIB_SRCS_C:.c=.o)
Frame_OBJS_C  := $(Frame_SRCS_DIR %.c,%.o,$(wildcard *.c))
#*****************************************************************************
# Generate objects files full path
#*****************************************************************************
LIB_OBJS = $(addprefix $(OBJ_DIR)/,$(LIB_OBJS_ASM))	$(addprefix $(OBJ_DIR)/,$(LIB_OBJS_C))
ALL_OBJS = $(addprefix $(OBJ_DIR)/,$(LIB_OBJS_ASM))	$(addprefix $(OBJ_DIR)/,$(LIB_OBJS_C))
#*****************************************************************************
# Generate source files full path
#*****************************************************************************
ALL_SRCS_C = $(addprefix $(SRC_DIR)/,$(LIB_SRCS_C))
	
#*****************************************************************************
# Object creation
#*****************************************************************************

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.s
	$(ASM) $(ASM_FLAGS) $< $(OBJ_DIR)/$*.o

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(C_FLAGS) $< -o $(OBJ_DIR)/$*.o

$(OBJ_DIR)/%.o: %.c
	$(CC) $(C_FLAGS) $< -o $(OBJ_DIR)/$*.o