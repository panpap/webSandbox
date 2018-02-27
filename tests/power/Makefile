LDLIBS = -lphidget21
BIN = power
OBJ = power.o ifkit.o hacks.o
FILE1=/usr/include/phidget21.h

ifeq ($(shell test -e $(FILE1) && echo -n yes),yes)
	RESULT1=$(FILE1) exists.
$(BIN): $(OBJ)
	$(CC) $(LDFLAGS) $(OBJ) $(LDLIBS) -o $@
endif


clean:
	rm -f $(BIN) $(OBJ)

.PHONY: clean
