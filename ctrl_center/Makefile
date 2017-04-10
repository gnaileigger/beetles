
TARGETS:=ctrl_center

.PHONY: all clean

all : $(TARGETS)

ctrl_center: ctrl_center.o
	$(CC) $(LDFLAGS) $^ -o $@ $(CLIENT_LDFLAGS)

%.o: %.c ../beetle.h
	$(CC) $(CFLAGS) -c $< -o $@ $(CLIENT_CFLAGS)

clean : 
	-rm -f *.o $(TARGETS)
