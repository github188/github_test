include ../config.mk

alarm.o:alarm.c alarm.h ../inc/atypes.h
	$(CC) $(INC) -c alarm.c
	cp 	*.o $(OUTPUT_DIR)
	rm *.o
clean:
	@rm -vf *.o
