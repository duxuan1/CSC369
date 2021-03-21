CFLAGS=-std=gnu99 -Wall

libext2fsal:  e2fs.o ext2fsal.o ext2fsal_cp.o ext2fsal_rm.o ext2fsal_ln_hl.o ext2fsal_ln_sl.o ext2fsal_mkdir.o
	gcc $(CFLAGS) -shared -fPIC -o libext2fsal.so $^

%.o : %.c ext2.h e2fs.h
	gcc $(CFLAGS) -g -c -fPIC $<

clean : 
	rm -f *.o libext2fsal.so *~
