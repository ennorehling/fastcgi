CFLAGS = -g -Wall -Werror -Wextra -std=c11 -Icritbit
PROGRAMS = counter-cgi complete-cgi

# http://www.thinkplexx.com/learn/howto/build-chain/make-based/prevent-gnu-make-from-always-removing-files-it-says-things-like-rm-or-removing-intermediate-files
.SECONDARY: complete.o

all: $(PROGRAMS)

%.o: %.c %.h
	$(CC) $(CFLAGS) -o $@ -c $< $(INCLUDES)

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $< $(INCLUDES)

counter-cgi: counter.o
	$(CC) $(CFLAGS) -o $@ $^ -lfcgi $(LDFLAGS)

cgiapp.a: cgiapp.o critbit/critbit.o
	$(AR) -q $@ $^

%-cgi: %.o cgiapp.a
	$(CC) $(CFLAGS) -o $@ $^ -lfcgi $(LDFLAGS)

clean:
	rm -f *~ *.a *.o $(PROGRAMS)
