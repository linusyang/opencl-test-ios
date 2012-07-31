CC = /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/usr/bin/clang
LD = /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/usr/bin/ld
LDID = /usr/local/bin/ldid
CFLAGS = -arch armv7 -Os -I/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS5.1.sdk/usr/include/ -I./include -framework OpenCL -miphoneos-version-min=3.0 -L/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS5.1.sdk/usr/lib/ -F/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS5.1.sdk/System/Library/PrivateFrameworks -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS5.1.sdk
APP1 = transmission
APP2 = endianness

all: $(APP1) $(APP2)

$(APP1):
	$(CC) $(CFLAGS) -o $(APP1) $(APP1).c; \
	$(LDID) -S $(APP1)

$(APP2):
	$(CC) $(CFLAGS) -o $(APP2) $(APP2).c; \
	$(LDID) -S $(APP2)

clean:
	rm -f $(APP1) $(APP2)

.PHONY: all clean
