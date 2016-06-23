include ../config.mk

web.o:web.c web.h ../devmgr/dev.h ../devmgr/io.h ../devmgr/485.h 
	$(CC) $(INC) -c web.c 
	cp 	*.o $(OUTPUT_DIR)
	rm *.o
clean:
	@rm -vf *.o
