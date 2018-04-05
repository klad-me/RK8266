##############################################################################
##
##  Makefile with automatic dependencies generation
##
##  (C) 2008, Alexey Presniakov (a@ooo-klad.ru)
##
##############################################################################



##############################################################################
##  Source files with paths
##############################################################################

C_SOURCES+= \
	src/startup.c		src/timer0.c		\
	src/i2s.c		src/gpio_lib.c		\
	src/tv.c		src/vg75.c		\
	src/zkg.c		src/xlat.c		\
	src/vv55_i.c		src/i8080_hal.c		\
	src/i8080.c		src/main.c		\
	src/ps2.c		src/ps2_codes.c		\
	src/keymap.c		\
	src/ui.c		src/menu.c		\
	src/xprintf.c		src/str.c		\
	src/tape.c		src/reboot.c		\
	src/crc8.c		src/ffs.c		\
	src/files.c		src/align4.c		\
	src/fileman.c

##############################################################################



##############################################################################
##  Directories and file names
##############################################################################
# Output file name
OUT=emu
# Path for source files
SRCDIR=src
# Path for binary files
OUTDIR=out
# Path for object files
OBJDIR=.obj
# Path for dependencies information
DEPDIR=.dep
##############################################################################



##############################################################################
##  Compiler information
##############################################################################
export PATH := /home/heavy/KLAD/x-tools/esp8266/xtensa-lx106-elf/bin:$(PATH)
BUILD_NUMBER=$(shell git rev-list --count HEAD)

CC=xtensa-lx106-elf-gcc
INCLUDES=-I$(SRCDIR)
CFLAGS=-O3 -Wall -Wl,-EL -fno-inline-functions -nostdlib -mlongcalls -mtext-section-literals -D__ets__ $(INCLUDES)
LIBS=-lgcc
LD_SCRIPT=ld/eagle.app.v6.ld
LDFLAGS_BUILD_NUMBER=-Xlinker --defsym -Xlinker __BUILD_NUMBER__=$(BUILD_NUMBER)
LDFLAGS=-nostdlib -Wl,--no-check-sections -u call_user_start -Wl,-static $(LIBS) $(LDFLAGS_BUILD_NUMBER)
ESPTOOL=esptool.py
##############################################################################



# Target ALL
all: $(OUTDIR)/$(OUT)-0x00000.bin


$(OUTDIR)/$(OUT)-0x00000.bin: $(OUTDIR)/$(OUT)
	@echo "Making $(OUT).bin..."; \
	$(ESPTOOL) elf2image -e 1 -o $(OUTDIR)/$(OUT)- $^

# Target for linker
$(OUTDIR)/$(OUT): $(subst $(SRCDIR)/,$(OBJDIR)/$(SRCDIR)/,$(C_SOURCES:.c=.o))
	@echo "Linking..."; \
	$(CC) $(CFLAGS) -o $@ -Wl,-Map,$(OUTDIR)/$(OUT).map $(subst $(SRCDIR)/,$(OBJDIR)/$(SRCDIR)/,$(C_SOURCES:.c=.o)) -T$(LD_SCRIPT) $(LDFLAGS)

# Target for flashing
flash: $(OUTDIR)/$(OUT)-0x00000.bin
	@echo "Flashing..."; \
	$(ESPTOOL) write_flash \
	    --flash_size 8m \
	    --flash_freq 20m \
	    --flash_mode dio \
	    0x01000 $(OUTDIR)/$(OUT)-0x00000.bin
	$(ESPTOOL) run

# Target for resetting
reset:
	@echo "Resetting..."; \
	$(ESPTOOL) run

# Target for terminal
term:
	@echo "Terminal..."; \
	telnet localhost 60485

# Target for clean
clean:
	rm -f $(OUTDIR)/$(OUT) $(OUTDIR)/$(OUT).map
	rm -rf $(DEPDIR)
	rm -rf $(OBJDIR)

# Target for distclean
distclean:
	rm -f $(OUTDIR)/$(OUT) $(OUTDIR)/$(OUT)-0x*.bin $(OUTDIR)/$(OUT).map
	rm -rf $(DEPDIR)
	rm -rf $(OBJDIR)


# PHONY
.PHONY: all


# Rule for generation of dependency information
$(DEPDIR)/%.d: %.c
	@set -e; \
	echo "Making dependencies for $*.c"; \
	mkdir -p `dirname "$@"`; \
	mkdir -p `dirname "$(OBJDIR)/$*.o"`; \
	ONAME=`echo "$(OBJDIR)/$*.o" | sed -e 's/\\//\\\\\\//g' | sed -e 's/\\./\\\\\\./g'`; \
	DNAME=`echo "$@" | sed -e 's/\\//\\\\\\//g' | sed -e 's/\\./\\\\\\./g'`; \
	$(CC) -MM $(CFLAGS) $< \
	| sed "s/.*:/$$ONAME $$DNAME : /g" > $@; \
	[ -s $@ ] || rm -f $@

# Rule for compiling C files
$(OBJDIR)/%.o: %.c
	@echo "Compiling $<"; \
	$(CC) $(CFLAGS) -c -o $@ $<


# Including dependencies infomation
-include $(subst $(SRCDIR)/,$(DEPDIR)/$(SRCDIR)/,$(C_SOURCES:.c=.d))
