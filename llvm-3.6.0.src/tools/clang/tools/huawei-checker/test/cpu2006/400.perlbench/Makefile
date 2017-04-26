ifeq ($(OS),Windows_NT)
OS_CFLAGS      = -I. -Iwin32 -DWIN32 -D_CONSOLE -DPERLDLL
OS_LIBS        = advapi32.lib wsock32.lib user32.lib Shell32.lib
endif

# Include standard template for this suite
-include ../../../../Makefile.defaults
-include ../../../Makefile.defaults
-include ../../Makefile.defaults
-include ../Makefile.defaults

nomakefiles:
	@echo "*** Whoops!  I didn't find a Makefile.defaults file."
	@echo "*** Your distribution is probably corrupt."
	@echo
