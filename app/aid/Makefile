include ../config.mk

%.o:%.c  
	$(CC) $(CFLAGS) $(INC) -o $@ -c $< 	
	
aid.o:aid.c aid.h ../inc/atypes.h 
	$(CC) $(INC) -c aid.c
	cp 	*.o $(OUTPUT_DIR)
	rm *.o
clean:
	@rm -vf *.o