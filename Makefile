CC = msp430-gcc
OO = msp430-objcopy
WW = mspdebug

# compiler flags:
CFLAGS = -Os -mmcu=msp430g2231
OFLAGS = -O ihex
WFLAGS = rf2500

# the build target executable:
TARGET = hexbingame

#all: $(TARGET)

$(TARGET): $(TARGET).c
	$(CC) $(CFLAGS) -o $(TARGET).elf $(TARGET).c
	$(OO) $(OFLAGS) $(TARGET).elf $(TARGET).hex

write: $(TARGET)
	$(WW) $(WFLAGS) "prog $(TARGET).hex"

