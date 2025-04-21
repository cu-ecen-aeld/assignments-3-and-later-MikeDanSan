
ifdef $(TARGET_CC)
	CC=$(TARGET_CC)
else
	CC=$(CROSS_COMPILE)gcc
endif

all: writer

writer: writer.c
	$(CC) -o $@ writer.c

.PHONY:	clean

clean:
	rm -f *~ writer