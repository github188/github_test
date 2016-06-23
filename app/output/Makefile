include ../config.mk

SHELL 				= 	/bin/bash
OPTIM				=	-fPIC -O0 -g
CFLAGS 				= 	$(OPTIM) $(INC) -Wall -std=c99 -D_THREAD_SAFE -D_REENTRANT
LIBS				=	-lpthread -lrt -lsqlite3 $(LIB_DIR)devmgr.a $(LIB_DIR)modbus_lib.a $(LIB_DIR)libmxml.a -lm
LDFLAGS 			= 	-L$(LIB_DIR) $(OPTIM) -Wl,-rpath,$(libdir)


exec_prefix			=	/usr/local
libdir				=	${exec_prefix}/lib
#OBJS				= 	web.o comm.o utm.o ptl.o system.o aid.o crc.o trace.o alarm.o wsdl.o share.o
OBJS				= *.o
app	:$(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)
	cp ./app /home/chenxu/tftpboot/
	rm app
clean:
	@rm -vf *.o
	@rm -vf app
