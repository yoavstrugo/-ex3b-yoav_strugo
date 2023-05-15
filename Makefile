SNAKES_PROGRAM_NAME = snakes_and_ladders
TWEETS_PROGRAM_NAME = tweets_generator
CC = gcc
CCFLAGS = -Wall -Wextra -Wvla -std=c99

SOURCES = $(wildcard *.c)
OBJECTS = $(patsubst %.c, %.o, $(SOURCES))
HEADERS = $(wildcard *.h)

all: $(SNAKES_PROGRAM_NAME) $(TWEETS_PROGRAM_NAME)

# main program rule
$(SNAKES_PROGRAM_NAME) $(TWEETS_PROGRAM_NAME): $(OBJECTS)
	$(CC) $(CCFLAGS) $^ -o $@

# rule for object files
%.o: %.c
	$(CC) $(CCFLAGS) $< -c

clean:
	rm -f *.o
	rm -f $(PROGRAM_NAME)

# rules for sources and headers
depend: .depend

.depend: $(SOURCES)
	rm -f $@
	$(CC) $(CFLAGS) -MM $^>>./$@

snake: $(SNAKES_PROGRAM_NAME)
tweets: $(TWEETS_PROGRAM_NAME)

-include .depend

.PHONY: clean, all, depend, snake, tweets