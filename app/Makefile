include ./config.mk

.c.o:
	echo Compiling $<
	$(CC) $(CFLAGS) $(INC) -c -o $@ $<
	cp *.o $(OUTPUT_DIR)
	rm *.o 
app	: share.modul devmgr.modul web.modul comm.modul utm.modul system.modul aid.modul log.modul alarm.modul main.o
	cd $(OUTPUT_DIR) && $(MAKE)
	@echo Compile Success !!!
devmgr.modul:
	cd ./devmgr && $(MAKE)
web.modul:
	cd ./web && $(MAKE)
comm.modul:
	cd ./comm && $(MAKE)
utm.modul:
	cd ./utm && $(MAKE)	
system.modul:
	cd ./system && $(MAKE)
aid.modul:
	cd ./aid && $(MAKE)
log.modul:
	cd ./log && $(MAKE)
alarm.modul:
	cd ./alarm && $(MAKE)
share.modul:
	cd ./share && $(MAKE)
clean:
	@rm -vf *.o

