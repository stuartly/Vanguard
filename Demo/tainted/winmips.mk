###############################################################
#Title:		tainted data project makefile
#Author:	
#Date:		
###############################################################
SRC_DIR	       = $(SOURCE_BASE)

CC     =clang
CPLUS  =clang++ 
LD     =
VXPOFILE_PRJ_DIR        = $(SRC_DIR)
VXWORKS68=$(WIND_HOME)
VSB_DIR = $(VXWORKS68)/MBSC_VSB_LIB_MIPSI64R2
VSB_CONFIG_FILE = $(VSB_DIR)/h/config/vsbConfig.h
			 
DRV_CFLAGS = -DCPU=MIPSI64R2 -DTOOL_FAMILY=gnu -DTOOL=sfgnu -D_WRS_KERNEL -D_WRS_MIPS_N32_ABI -DMIPSEB -DSOFT_FLOAT -DCPU_VARIANT=_xlp -D__GNUC__\
             -DMIPSXX_xlp -D_WRS_MIPS_NETLOGIC_CPU -DINCLUDE_RMI_XLP -DVCPU_OS_TYPE=RUN_VXWORKS -DIP_PORT_VXWORKS=68 -DINET -msoft-float \
			 -D_VSB_CONFIG_FILE=\"$(VSB_CONFIG_FILE)\" -fno-zero-initialized-in-bss -G4 -fno-common -fno-builtin -Wall -fno-strict-aliasing \
			 -fsigned-char -Wno-unused  -Wpointer-arith -O2 -emit-ast
			 

VXCFLAGS         =  $(DRV_CFLAGS) 	\
		 -I$(VXPOFILE_PRJ_DIR)/include 	\
		 -I. \
         -I$(VXWORKS68)/vxworks-6.8/target/h \
		 -I$(VXWORKS68)/vxworks-6.8/target/h/types \
		 -I$(VXWORKS68)/vxworks-6.8/target/src/config \
		 -I$(VXWORKS68)/vxworks-6.8/target/src/drv \
		 -I$(VXWORKS68)/vxworks-6.8/host/x86-win32/lib/gcc-lib/mips-wrs-vxworks/2.96-MIPS3264-010729/include \
		 -I$(VXWORKS68)/vxworks-6.8/target/config/all \
		 -I$(VXWORKS68)/vxworks-6.8/target/h/arch/mips

VXPROFILE_OBJS = tainted_array.o 	\
                 buffer2struct.o       \
                 tainted_for.o 		\
                 tainted_mod.o	\
                 tainted_div.o		\
				 tainted_mem.o \
				 mem_entry.o \
				 mod_entry.o \
				 array_entry.o \
				 for_entry.o \
				 div_entry.o \
				 cluster.o \
				 routine_ret.o \
				 recursive_call.o \
				 dangling_pointer.o \
                 routine_outarg.o

image.o:$(VXPROFILE_OBJS)
	

tainted_array.o:$(VXPOFILE_PRJ_DIR)/src/tainted_array.c
	$(CC)   $(VXCFLAGS)  -c $(VXPOFILE_PRJ_DIR)/src/tainted_array.c


buffer2struct.o:$(VXPOFILE_PRJ_DIR)/src/buffer2struct.c
	$(CC)   $(VXCFLAGS)  -c $(VXPOFILE_PRJ_DIR)/src/buffer2struct.c

tainted_for.o:$(VXPOFILE_PRJ_DIR)/src/tainted_for.c
	$(CC)   $(VXCFLAGS)  -c $(VXPOFILE_PRJ_DIR)/src/tainted_for.c

tainted_mod.o:$(VXPOFILE_PRJ_DIR)/src/tainted_mod.c
	$(CC)   $(VXCFLAGS)  -c $(VXPOFILE_PRJ_DIR)/src/tainted_mod.c

tainted_mem.o:$(VXPOFILE_PRJ_DIR)/src/tainted_mem.c
	$(CC)   $(VXCFLAGS)  -c $(VXPOFILE_PRJ_DIR)/src/tainted_mem.c
	
tainted_div.o:$(VXPOFILE_PRJ_DIR)/src/tainted_div.c
	$(CC)   $(VXCFLAGS)  -c $(VXPOFILE_PRJ_DIR)/src/tainted_div.c
	
mod_entry.o:$(VXPOFILE_PRJ_DIR)/src/mod_entry.c
	$(CC)   $(VXCFLAGS)  -c $(VXPOFILE_PRJ_DIR)/src/mod_entry.c

div_entry.o:$(VXPOFILE_PRJ_DIR)/src/div_entry.c
	$(CC)   $(VXCFLAGS)  -c $(VXPOFILE_PRJ_DIR)/src/div_entry.c

for_entry.o:$(VXPOFILE_PRJ_DIR)/src/for_entry.c
	$(CC)   $(VXCFLAGS)  -c $(VXPOFILE_PRJ_DIR)/src/for_entry.c

mem_entry.o:$(VXPOFILE_PRJ_DIR)/src/mem_entry.c
	$(CC)   $(VXCFLAGS)  -c $(VXPOFILE_PRJ_DIR)/src/mem_entry.c
	
array_entry.o:$(VXPOFILE_PRJ_DIR)/src/array_entry.c
	$(CC)   $(VXCFLAGS)  -c $(VXPOFILE_PRJ_DIR)/src/array_entry.c

cluster.o:$(VXPOFILE_PRJ_DIR)/src/cluster.cpp
	$(CPLUS)   $(VXCFLAGS)  -c $(VXPOFILE_PRJ_DIR)/src/cluster.cpp
	
routine_ret.o:$(VXPOFILE_PRJ_DIR)/src/routine_ret.c
	$(CC)   $(VXCFLAGS)  -c $(VXPOFILE_PRJ_DIR)/src/routine_ret.c

routine_outarg.o:$(VXPOFILE_PRJ_DIR)/src/routine_outarg.c
	$(CC)   $(VXCFLAGS)  -c $(VXPOFILE_PRJ_DIR)/src/routine_outarg.c

recursive_call.o:$(VXPOFILE_PRJ_DIR)/src/recursive_call.c
	$(CC)   $(VXCFLAGS)  -c $(VXPOFILE_PRJ_DIR)/src/recursive_call.c

dangling_pointer.o:$(VXPOFILE_PRJ_DIR)/src/dangling_pointer.c
	$(CC)   $(VXCFLAGS)  -c $(VXPOFILE_PRJ_DIR)/src/dangling_pointer.c
	
clean :
		

