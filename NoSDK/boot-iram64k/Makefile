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
	src/main.c		\
	src/check_image.c

##############################################################################



##############################################################################
##  Directories and file names
##############################################################################
# Output file name
OUT=boot
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

CC=xtensa-lx106-elf-gcc
INCLUDES=-I$(SRCDIR)
CFLAGS=-Os -Wall -Wl,-EL -fno-inline-functions -nostdlib -mlongcalls -mtext-section-literals  -D__ets__ -DICACHE_FLASH $(INCLUDES)
LIBS=
LD_SCRIPT=$(SRCDIR)/eagle.app.v6.ld
LDFLAGS=-nostdlib -Wl,--no-check-sections -u call_user_start -Wl,-static $(LIBS)
ESPTOOL=esptool.py
ESPTOOL2=/home/heavy/KLAD/x-tools/esp8266/esptool2/esptool2
##############################################################################



# Target ALL
all: $(OUTDIR)/$(OUT).bin


$(OUTDIR)/$(OUT).bin: $(OUTDIR)/$(OUT)
	@echo "Making $(OUT).bin..."; \
	$(ESPTOOL2) -quiet -bin -boot0 -4096 -qio -40 $^ $@ .text .rodata

# Target for linker
$(OUTDIR)/$(OUT): $(subst $(SRCDIR)/,$(OBJDIR)/$(SRCDIR)/,$(C_SOURCES:.c=.o))
	@echo "Linking..."; \
	$(CC) $(CFLAGS) -o $@ -Wl,-Map,$(OUTDIR)/$(OUT).map $(subst $(SRCDIR)/,$(OBJDIR)/$(SRCDIR)/,$(C_SOURCES:.c=.o)) -T$(LD_SCRIPT) $(LDFLAGS)

# Target for flashing
flash: $(OUTDIR)/$(OUT).bin
	@echo "Flashing..."; \
	$(ESPTOOL) write_flash \
	    --flash_size 8m \
	    --flash_freq 20m \
	    --flash_mode dio \
	    0x00000 $(OUTDIR)/$(OUT).bin && \
	$(ESPTOOL) run

# Target for flashing
flash2: $(OUTDIR)/$(OUT).bin
	@echo "Flashing..."; \
	$(ESPTOOL) write_flash \
	    --flash_size 8m \
	    --flash_freq 20m \
	    --flash_mode dio \
	    0x00000 $(OUTDIR)/$(OUT).bin \
	    0x01000 $(OUTDIR)/app.bin

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
	rm -f $(OUTDIR)/$(OUT) $(OUTDIR)/$(OUT).bin $(OUTDIR)/$(OUT).map
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
