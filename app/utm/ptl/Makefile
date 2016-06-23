include ../../config.mk

ptl.o:ptl.c protocol.h
	$(CC) $(INC) -c ptl.c crc.c
	cp 	*.o $(OUTPUT_DIR)
	rm *.o
clean:
	@rm -vf *.o
