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

CPP_SOURCES+= \
	src/main.cpp		src/Thread.cpp		\
	src/cpp.cpp		src/ui.cpp		\
	src/TCPSocket.cpp	src/HTTPd.cpp		\
	src/Web.cpp

C_SOURCES+= \
	src/gpio_lib.c		src/gpio16.c		\
	src/timer0.c		src/i2s.c		\
	src/tv.c		src/tv_data.c		\
	src/ps2.c		src/ps2_codes.c		\
	src/ffs.c		src/crc8.c		\
	src/str.c		src/reboot.c

##############################################################################



##############################################################################
##  Directories and file names
##############################################################################
# Output file name
OUT=wifi
# Path for source files
SRCDIR=src
# Path for binary files
OUTDIR=out
# Path for builds
BUILDDIR=builds
# Path for object files
OBJDIR=.obj
# Path for dependencies information
DEPDIR=.dep
##############################################################################


##############################################################################
##  Additional flash sections
##############################################################################

FLASH_SECTIONS+= \
	0x70000 httpfs/httpfs.bin

##############################################################################


##############################################################################
##  Compiler information
##############################################################################
SDK=/home/heavy/KLAD/x-tools/esp8266/sdk
export PATH := /home/heavy/KLAD/x-tools/esp8266/xtensa-lx106-elf/bin:$(PATH)

BUILD_NUMBER=$(shell git rev-list --count HEAD)

CC=xtensa-lx106-elf-gcc
CPP=xtensa-lx106-elf-g++
OBJCOPY=xtensa-lx106-elf-objcopy
OBJDUMP=xtensa-lx106-elf-objdump
INCLUDES=-I$(SDK)/include -Isrc/
CFLAGS=-Wall -Wshadow -DICACHE_FLASH -O3 -mlongcalls \
    -mtext-section-literals \
    $(INCLUDES) -ffunction-sections -fdata-sections
CPPFLAGS=-Wall -Wshadow -DICACHE_FLASH -O3 -mlongcalls \
    -fno-common -fno-builtin -nostdinc++ -fno-rtti -fno-exceptions \
    -fno-enforce-eh-specs -fnothrow-opt \
    -mtext-section-literals \
    $(INCLUDES) -ffunction-sections -fdata-sections
LIBS=-L$(SDK)/lib -lmain -lnet80211 -lwpa -llwip -lpp -lphy -Wl,--end-group -lgcc -lcrypto
# -lsmartconfig -lssl -lupgrade
LD_SCRIPT=ld/eagle.app.v6.ld
LDFLAGS_BUILD_NUMBER=-Xlinker --defsym -Xlinker __BUILD_NUMBER__=$(BUILD_NUMBER)
LDFLAGS=-nostdlib -Wl,--start-group $(LIBS) $(LDFLAGS_BUILD_NUMBER) -Wl,--relax,--gc-sections 
ESPTOOL=esptool.py --port /dev/ttyUSB0
##############################################################################



# Target ALL
all: $(OUTDIR)/$(OUT).1.bin


$(OUTDIR)/$(OUT).1.bin: $(OUTDIR)/$(OUT).1
	@echo "Making user.bin..."; \
	$(ESPTOOL) elf2image --output $(OUTDIR)/$(OUT).1.bin --version 2 --flash_size 4m $(OUTDIR)/$(OUT).1

# Target for linker user1
$(OUTDIR)/$(OUT).1: $(subst $(SRCDIR)/,$(OBJDIR)/$(SRCDIR)/,$(C_SOURCES:.c=.o)) $(subst $(SRCDIR)/,$(OBJDIR)/$(SRCDIR)/,$(CPP_SOURCES:.cpp=.o)) $(LIB_DEPS)
	@echo "Linking..."; \
	$(CPP) $(CPPFLAGS) -o $(OUTDIR)/$(OUT).1 -Wl,-Map,out/$(OUT).1.map $(subst $(SRCDIR)/,$(OBJDIR)/$(SRCDIR)/,$(C_SOURCES:.c=.o)) $(subst $(SRCDIR)/,$(OBJDIR)/$(SRCDIR)/,$(CPP_SOURCES:.cpp=.o)) -T$(LD_SCRIPT) $(LDFLAGS)

# Binary build
build: $(OUTDIR)/$(OUT).1.bin
	@echo "Make build $(BUILD_NUMBER)...";	\
	mkdir -p $(BUILDDIR)/$(BUILD_NUMBER) || exit;	\
	cp $(SDK)/bin/fastboot.bin $(BUILDDIR)/$(BUILD_NUMBER)/0x00000.bin; \
	cp $(OUTDIR)/$(OUT).1.bin $(BUILDDIR)/$(BUILD_NUMBER)/0x01000.bin; \
	cp games/klad.bin $(BUILDDIR)/$(BUILD_NUMBER)/0x80000.bin

# Target for flashing
flash: $(OUTDIR)/$(OUT).1.bin
	@echo "Flashing fastboot, user1.bin..."; \
	$(ESPTOOL) write_flash \
	    --flash_size 8m \
	    0x00000 $(SDK)/bin/fastboot.bin	\
	    0x01000 $(OUTDIR)/$(OUT).1.bin	\
	    $(FLASH_SECTIONS)

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
	rm -f $(OUTDIR)/$(OUT).1 $(OUTDIR)/$(OUT).2 $(OUTDIR)/$(OUT).1.bin $(OUTDIR)/$(OUT).2.bin $(OUTDIR)/user1.bin $(OUTDIR)/user2.bin $(OUTDIR)/$(OUT).1.map $(OUTDIR)/$(OUT).2.map
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

# Rule for generation of dependency information
$(DEPDIR)/%.d: %.cpp
	@set -e; \
	echo "Making dependencies for $*.cpp"; \
	mkdir -p `dirname "$@"`; \
	mkdir -p `dirname "$(OBJDIR)/$*.o"`; \
	ONAME=`echo "$(OBJDIR)/$*.o" | sed -e 's/\\//\\\\\\//g' | sed -e 's/\\./\\\\\\./g'`; \
	DNAME=`echo "$@" | sed -e 's/\\//\\\\\\//g' | sed -e 's/\\./\\\\\\./g'`; \
	$(CPP) -MM $(CPPFLAGS) $< \
	| sed "s/.*:/$$ONAME $$DNAME : /g" > $@; \
	[ -s $@ ] || rm -f $@

# Rule for compiling C files
$(OBJDIR)/%.o: %.c
	@echo "Compiling $<"; \
	$(CC) $(CFLAGS) -c -o $@ $< && ( \
	for section in `$(OBJDUMP) -h $@|awk '{print $$2}'|grep -E '^(\.text\.|\.literal\.)'|grep -v .irom|grep -v .iram`; \
	do \
	    $(OBJCOPY) --rename-section $$section=.irom0$$section $@; \
	done; \
	$(OBJCOPY) --rename-section .text=.irom0.text --rename-section .literal=.irom0.literal $@ )

# Rule for compiling C++ files
$(OBJDIR)/%.o: %.cpp
	@echo "Compiling $<"; \
	$(CPP) $(CPPFLAGS) -c -o $@ $< && ( \
	for section in `$(OBJDUMP) -h $@|awk '{print $$2}'|grep -E '^(\.text\.|\.literal\.)'|grep -v .irom|grep -v .iram`; \
	do \
	    $(OBJCOPY) --rename-section $$section=.irom0$$section $@; \
	done; \
	$(OBJCOPY) --rename-section .text=.irom0.text --rename-section .literal=.irom0.literal $@ )


# Including dependencies infomation
-include $(subst $(SRCDIR)/,$(DEPDIR)/$(SRCDIR)/,$(C_SOURCES:.c=.d))
-include $(subst $(SRCDIR)/,$(DEPDIR)/$(SRCDIR)/,$(CPP_SOURCES:.cpp=.d))
