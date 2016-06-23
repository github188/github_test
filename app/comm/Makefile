include ../config.mk

comm.o:comm.c ../system/system.c ../system/system.h comm.h ../inc/atypes.h ../utm/utm.h
	$(CC) $(INC) -c comm.c
	cp 	*.o $(OUTPUT_DIR)
	rm *.o
clean:
	@rm -vf *.o
