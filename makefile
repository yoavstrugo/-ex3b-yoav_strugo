SNAKES_PROGRAM_NAME := snakes_and_ladders
TWEETS_PROGRAM_NAME := tweets_generator
CC := gcc
CCFLAGS := -Wall -Wextra -Wvla -g

ALL_SOURCES := $(wildcard *.c)
SNAKES_SRCS := $(filter-out tweets_generator.c, $(ALL_SOURCES))
TWEETS_SRCS := $(filter-out snakes_and_ladders.c, $(ALL_SOURCES))

SNAKES_OBJS := $(patsubst %.c, %.o, $(SNAKES_SRCS))
TWEETS_OBJS := $(patsubst %.c, %.o, $(TWEETS_SRCS))

all: $(SNAKES_PROGRAM_NAME) $(TWEETS_PROGRAM_NAME)

# main program rule
$(SNAKES_PROGRAM_NAME): $(SNAKES_OBJS)
	$(CC) $(CCFLAGS) $^ -o $@

$(TWEETS_PROGRAM_NAME): $(TWEETS_OBJS)
	$(CC) $(CCFLAGS) $^ -o $@

# rule for object files
%.o: %.c
	$(CC) $(CCFLAGS) $< -c

clean:
	rm -f *.o
	rm -f $(PROGRAM_NAME)

# rules for sources and headers
depend: .depend

.depend: $(ALL_SOURCES)
	rm -f $@
	$(CC) $(CFLAGS) -MM $^>>./$@

snake: $(SNAKES_PROGRAM_NAME)
tweets: $(TWEETS_PROGRAM_NAME)

-include .depend

.PHONY: clean, all, depend, snake, tweets
