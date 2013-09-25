TOPDIR = $(PWD)

SDKVERSION = 7.0
DEVROOT = $(shell xcode-select --print-path)
TOOLCHAINROOT = $(DEVROOT)/Toolchains/XcodeDefault.xctoolchain/usr/bin
SDKROOT= $(DEVROOT)/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS$(SDKVERSION).sdk

CC = $(TOOLCHAINROOT)/clang -arch armv7
CXX = $(TOOLCHAINROOT)/clang++
LDID = $(TOPDIR)/ldid-host
CFLAGS = -Os -I$(SDKROOT)/usr/include/ -Iinclude
LDFLAGS = -L$(SDKROOT)/usr/lib/ -F/$(SDKROOT)/System/Library/PrivateFrameworks -framework OpenCL -miphoneos-version-min=5.0 -isysroot $(SDKROOT)

EXECUTABLE = transmission endianness ldid-host
T_SRC = transmission.c
T_OBJECTS = $(T_SRC:.c=.o)
E_SRC = endianness.c
E_OBJECTS = $(E_SRC:.c=.o)

LDID_DIR = ldid
S_CPPSRC = ldid.cpp
S_CSRC = lookup2.c sha1.c

all: ldid-host transmission endianness

ldid-host: $(S_OBJECTS)
	cd $(LDID_DIR); $(CXX) -O2 -o $(TOPDIR)/$@ $(S_CPPSRC) -I. -x c $(S_CSRC)

transmission: $(T_OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS)
	$(LDID) -S $@

endianness: $(E_OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS)
	$(LDID) -S $@

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(T_OBJECTS) $(E_OBJECTS) $(EXECUTABLE)
