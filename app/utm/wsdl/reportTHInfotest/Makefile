GSOAP_ROOT=/usr/local/gSOAP
WSNAME0=soap
WSNAME=reportTHInfo
#CC=g++ -g -DWITH_NONAMESPACES
CC=arm-none-linux-gnueabi-g++ -g -DWITH_NONAMESPACES
INCLUDE=-I $(GSOAP_ROOT)/include
SERVER_OBJS=$(WSNAME0)C.o $(WSNAME0)Server.o stdsoap2.o
CLIENT_OBJS=$(GSOAP_ROOT)/env/envC.o $(WSNAME0)ClientLib.o stdsoap2.o
#CLIENT_OBJS=../link/envC.o $(WSNAME0)ClientLib.o stdsoap2.o
#ALL_OBJS=${WSNAME}server.o $(WSNAME0)C.o $(WSNAME0)Server.o ${WSNAME}test.o $(WSNAME0)ClientLib.o
ALL_OBJS=$(WSNAME0)C.o $(WSNAME0)Server.o ${WSNAME}test.o $(WSNAME0)ClientLib.o
#GSOAP_SRC=/usr/local/gsoap-2.7/gsoap

all:client

${WSNAME}.wsdl:${WSNAME}.h
	$(GSOAP_ROOT)/bin/soapcpp2 -c -ay  $(GSOAP_ROOT)/share/gsoap/import/ ${WSNAME}.h

stdsoap2.o:$(GSOAP_ROOT)/src/stdsoap2.c
	$(CC) -c $? $(INCLUDE)

$(ALL_OBJS):%.o:%.c
	$(CC) -c $? $(INCLUDE)

server:Makefile ${WSNAME}.wsdl ${WSNAME}server.o $(SERVER_OBJS)
	$(CC) ${WSNAME}server.o $(SERVER_OBJS) -o ${WSNAME}server

client:Makefile ${WSNAME}.wsdl ${WSNAME}test.c $(ALL_OBJS) stdsoap2.o
	$(CC) ${WSNAME}test.o $(CLIENT_OBJS) -o ${WSNAME}test
	cp $(WSNAME)test ~/project/HTH_EMAC/tftpboot/

clean:
	rm -f *.o *.xml *.a *.wsdl *.nsmap $(WSNAME0)H.h $(WSNAME0)C.c $(WSNAME0)Server.c $(WSNAME0)Client.c $(WSNAME0)Stub.* $(WSNAME)$(WSNAME)Proxy.* $(WSNAME)$(WSNAME)Object.* $(WSNAME0)ServerLib.c $(WSNAME0)ClientLib.c reportTHInfo.xsd ns1.xsd
