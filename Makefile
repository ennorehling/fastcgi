CFLAGS = -g -Wall -Werror -Wextra -std=c11
PROGRAMS = counter-cgi

all: $(PROGRAMS)

%.o: %.c %.h
	$(CC) $(CFLAGS) -o $@ -c $< $(INCLUDES)

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $< $(INCLUDES)

%-cgi: %.o
	$(CC) $(CFLAGS) -o $@ $^ -lfcgi $(LDFLAGS)

clean:
	rm -f *~ *.o $(PROGRAMS)
