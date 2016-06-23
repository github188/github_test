include ../config.mk
ARCHFLAGS	=	
OPTIM		= -g -O0 
#INC         = -I../inc/ -I../devmgr/ -I../../mxml-$(MXML_VERSION)/ -I../log/ -I../wsdl/ -I../web/
CFLAGS      = $(OPTIM) $(INC) $(ARCHFLAGS) -Wall   -D_THREAD_SAFE -D_REENTRANT
OBJS         =dev.o io.o 485.o hth.o eth.o
PRIVATE     =./private/

devmgr.a:$(OBJS)
	$(AR) rc devmgr.a $(OBJS)
	cp devmgr.a $(LIB_DIR)
	rm *.o
	rm devmgr.a
dev.o:dev.c dev.h
	$(CC) $(CFLAGS) -c dev.c
io.o:io.c io.h
	$(CC) $(CFLAGS) -c io.c
485.o:485.c 485.h
	$(CC) $(CFLAGS) -c 485.c
eth.o:eth.c eth.h
	$(CC) $(CFLAGS) -c eth.c
hth.o:
	cd ./private && $(MAKE)
	
clean:
	@rm -vf *.o
	@rm -vf *.a
