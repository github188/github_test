BASE_DIR		=	/home/chenxu/proj/HTH_EMAC/src/app
AID_DIR			=	$(BASE_DIR)/aid/
ALARM_DIR		=	$(BASE_DIR)/alarm/
COMM_DIR		=	$(BASE_DIR)/comm/
DEVMGR_DIR		=	$(BASE_DIR)/devmgr/
INC_DIR			=	$(BASE_DIR)/inc/
LIB_DIR			=	$(BASE_DIR)/lib/
PTL_DIR			=	$(BASE_DIR)/utm/ptl/
LOG_DIR			=	$(BASE_DIR)/log/
SYSTEM_DIR		=	$(BASE_DIR)/system/
UTM_DIR			=	$(BASE_DIR)/utm/
WEB_DIR			=	$(BASE_DIR)/web/
WSDL_DIR		=	$(BASE_DIR)/utm/wsdl/
SHARE_DIR		=	$(BASE_DIR)/share/
OUTPUT_DIR		=	$(BASE_DIR)/output
SHELL			=	/bin/sh

CC			=	/usr/local/arm-2011.09/bin/arm-none-linux-gnueabi-gcc	
AR			=	/usr/local/arm-2011.09/bin/arm-none-linux-gnueabi-ar
INC 			= 	-I$(AID_DIR) -I$(ALARM_DIR) -I$(COMM_DIR) -I$(DEVMGR_DIR) \
					-I$(INC_DIR) -I$(PTL_DIR) -I$(LOG_DIR) -I$(SYSTEM_DIR) \
					-I$(UTM_DIR) -I$(WEB_DIR) -I$(WSDL_DIR) -I$(SHARE_DIR)
					
