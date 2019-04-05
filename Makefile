#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

PROJECT_NAME := reflow

#EXTRA_CFLAGS += 
EXTRA_CPPFLAGS += -fexceptions

include $(IDF_PATH)/make/project.mk

# Build a SPIFFS partition containing the javascript client files
$(eval $(call spiffs_create_partition_image,jsclient,jsclient-dist,FLASH_IN_PROJECT))



