include ../config.mk

system:system.c system.h
	$(CC) $(INC) -c system.c 
	cp 	*.o $(OUTPUT_DIR)
	rm *.o
clean:
	@rm -vf *.o
