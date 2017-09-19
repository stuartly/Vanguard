###############################################################
#Title:		tainted data project makefile runnint on linux
#Author:	
#Date:		
###############################################################
SRC_DIR=$(SOURCE_BASE)
CC=clang
CPLUS=clang++
LD=ld
VXPOFILE_PRJ_DIR = $(SRC_DIR)
			 
DRV_CFLAGS =   -msoft-float  -fno-zero-initialized-in-bss  -fno-common -fno-builtin -Wall -fno-strict-aliasing \
	       -fsigned-char -Wno-unused  -Wpointer-arith -O2 -DOS_LINUX -emit-ast
			 

VXCFLAGS         =  $(DRV_CFLAGS) 	\
		 -I$(VXPOFILE_PRJ_DIR)/include 	\
		 -I. \


VXPROFILE_OBJS = tainted_array.o \
                tainted_for.o 	\
                tainted_mod.o	\
                tainted_div.o	\
                tainted_mem.o	\
                mem_entry.o	\
                mod_entry.o	\
                array_entry.o	\
                for_entry.o	\
                div_entry.o	\
                cluster.o	\
                routine_ret.o	\
                routine_outarg.o

image.o:$(VXPROFILE_OBJS)
	$(LD) -o image.o -r $(VXPROFILE_OBJS)

tainted_array.o:$(VXPOFILE_PRJ_DIR)/src/tainted_array.c
	$(CC)   $(VXCFLAGS)  -c $(VXPOFILE_PRJ_DIR)/src/tainted_array.c

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
	$(CC)   $(VXCFLAGS)  -c $(VXPOFILE_PRJ_DIR)/src/cluster.cpp

routine_ret.o:$(VXPOFILE_PRJ_DIR)/src/routine_ret.c
	$(CC)   $(VXCFLAGS)  -c $(VXPOFILE_PRJ_DIR)/src/routine_ret.c

routine_outarg.o:$(VXPOFILE_PRJ_DIR)/src/routine_outarg.c
	$(CC)   $(VXCFLAGS)  -c $(VXPOFILE_PRJ_DIR)/src/routine_outarg.c

clean :
		echo $(VXPROFILE_OBJS)
		- $(RM) $(VXPROFILE_OBJS)












