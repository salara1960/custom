
NAME=custom
proc=main
elib=functions

CC=gcc
#STRIP=strip
RM=rm

CFLAG =-std=gnu99 -O0 -Wall -g -D_GNU_SOURCE

INC=/usr/include

MACHINE := $(shell uname -m)
ifeq ($(MACHINE), x86_64)
	LIB_DIR = /usr/lib
else
	LIB_DIR = /usr/lib32
endif

$(NAME): $(elib).o $(proc).o
	$(CC) -o $(NAME) $(elib).o $(proc).o libiconv_hook.a
#-L$(LIB_DIR) -liconv_hook -ldl
#	$(STRIP) $(NAME)
$(proc).o: $(proc).c
	$(CC) -c $(proc).c $(CFLAG) -I$(INC)
$(elib).o: $(elib).c
	$(CC) -c $(elib).c $(CFLAG) -I$(INC)

clean:
	$(RM) -f *.o $(NAME)

