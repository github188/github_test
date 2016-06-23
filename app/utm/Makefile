include ../config.mk

SRCS=$(wildcard *.c)
OBJS=$(SRCS:%.c=%.o)

.c.o:
	echo Compiling $<
	$(CC) $(CFLAGS) $(INC) -c -o $@ $<

all : ptl.modul wsdl.modul $(OBJS)
	cp 	*.o $(OUTPUT_DIR)
	rm *.o
ptl.modul:
	cd ./ptl && $(MAKE)
wsdl.modul:
	cd ./wsdl && $(MAKE)

clean:
	@rm -vf *.o
