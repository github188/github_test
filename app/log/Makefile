include ../config.mk

trace.o:trace.c trace.h ../inc/atypes.h
	$(CC) $(INC) -c trace.c
	cp 	*.o $(OUTPUT_DIR)
	rm *.o
clean:
	@rm -vf *.o
