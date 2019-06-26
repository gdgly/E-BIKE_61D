#******************************************************************************
#  Filename     :  Makefile
#  Project      :  
#  Language     :  
#  Description  :  
#  Edit History :  
#******************************************************************************

#*****************************************************************************
# Target name
#*****************************************************************************
TARGET = wmp

#*****************************************************************************
# Link File Definitions
#*****************************************************************************
LINK_FILES   = 

#*****************************************************************************
# Source File Definitions
#*****************************************************************************
LIB_SRCS_ASM = wmp.s
#LIB_SRCS_C   = main.c EKP_MainFrame.c
include wmp.lis


#*****************************************************************************
# Inclue platfrom information
#*****************************************************************************

include Makefile_Platform.inc

include Makefile.inc

PLATFORM_SYMBOL_FILE  = wmp.sym
PLATFORM_HEADER_FILE  = wmp.o
THIRDPARTY_LIB        = ThirdParty/LIB
#*****************************************************************************
# Directory Definition
#*****************************************************************************
ALL_DIRS = $(SUB_DIRS) LIB

#*****************************************************************************
# Depedency Library Definition
#*****************************************************************************
ALL_LIBS =  

#*****************************************************************************
# Depedency Library full path
#*****************************************************************************
ALL_LIB_FILES =  $(addsuffix .lib, $(addprefix $(LIB_DIR)/,$(ALL_LIBS)))              
ALL_LINK_LIB_FILES  = $(ALL_LIB_FILES)

#*****************************************************************************
# Object creation
#*****************************************************************************
default: $(ALL_DIRS) $(ALL_LIBS) $(TARGET).axf
$(TARGET).axf: $(ALL_OBJS) $(ALL_LIB_FILES)
	cp -f $(SRC_DIR)/$(PLATFORM_SYMBOL_FILE) $(OBJ_DIR)/$(PLATFORM_SYMBOL_FILE)
	#cp -f $(SRC_DIR)/$(PLATFORM_HEADER_FILE) $(OBJ_DIR)/$(PLATFORM_HEADER_FILE)	
	$(LINK) -o $@ $(LINK_FLAGS) $(TARGET).map -entry Service_Entry -first $(PLATFORM_HEADER_FILE)\(ServiceHeader\)\
	        $(OBJ_DIR)/$(PLATFORM_SYMBOL_FILE) $(OBJ_DIR)/$(PLATFORM_HEADER_FILE) $(ALL_OBJS)	        
	$(BIN_TOOL) $(TARGET).axf -bin -o $(TARGET).bin		
	cp -f $(TARGET).bin $(TARGET).ex

      
$(ALL_DIRS):
	@$(MKDIR) $@

$(ALL_LIBS):
	 @$(MAKE)  -C prj/$@

all_tools:
	 @$(MAKE)  -C $(TOOLS_DIR)


#*****************************************************************************
# Clean Rule
#*****************************************************************************
.PHONY: clean
clean:	
	-@rm -f $(OBJ_DIR)/*.*
	-@rm -r $(OBJ_DIR) 
	-@rm -rf OUT1 LIB 
	-@rm -f *.o *.a  *.bin *.ex *.map *.axf *.dl $(MAINEXE) kongzhongyan2.*

.PHONY: clean_sub
clean_sub:
	 for D in $(ALL_LIBS); do cd prj/$$D ; $(MAKE) clean;cd ../..;done

.PHONY: clean_all
clean_all:
	$(MAKE) clean
	$(MAKE) clean_sub
#	$(MAKE) -C $(COMMON_DIR) clean

#
# make / clean dependency list
#
MAKEFILENAME = makefile
CFLAGSDEPEND = -MM  $(C_FLAGS)                    # for make depend
DEPENDENT_START = \# DO NOT EDIT AFTER THIS LINE -- make depend will generate it.

depend:
	$(MAKE) -f $(MAKEFILENAME)  clean_depend
	$(ECHO) "$(DEPENDENT_START)" >> $(MAKEFILENAME)
	$(CC) $(CFLAGSDEPEND) $(ALL_SRCS_C) | sed -e 's/^\(.*\)\.o/OBJ\/\1.o/g' >> $(MAKEFILENAME)

clean_depend:
	chmod u+w $(MAKEFILENAME)
	(awk 'BEGIN{f=1}\
	/^$(DEPENDENT_START)/{f=0} \
	{if (f) print $0}'\
	< $(MAKEFILENAME) > .depend && 	mv .depend $(MAKEFILENAME)) || exit 1;	
