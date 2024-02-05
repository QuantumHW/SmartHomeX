
CFLAGS = -Wall -o2 -g -std=gnu99 -pthread -I./include -I./lib/inc -DMYDBG
LDFLAGS = -lpthread 


APP = server
SRCS	 = $(wildcard ./src/*.c ./lib/src/*.c)
OBJS	 = $(patsubst %.c, %.o,$(SRCS))

CC = arm-linux-gcc
all: $(APP)

$(APP): $(OBJS)
	$(CC) -o $(APP) $^ $(LDFLAGS)
	rm -f $(OBJS)
	
clean:
	rm -rf $(OBJS) $(APP) ./*~
