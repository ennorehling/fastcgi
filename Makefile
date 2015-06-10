PREFIX = /opt/cgi
CFLAGS = -g -Wall -Werror -Wextra -Icritbit -std=c99
PROGRAMS = counter-cgi prefix-cgi keyval-cgi
TESTS = fastcgi-test
WEBSITE = /var/www/html

# http://www.thinkplexx.com/learn/howto/build-chain/make-based/prevent-gnu-make-from-always-removing-files-it-says-things-like-rm-or-removing-intermediate-files
.SECONDARY: prefix.o

all: $(PROGRAMS) $(TESTS)

test: $(TESTS)
	./fastcgi-test

%.o: %.c %.h
	$(CC) $(CFLAGS) -o $@ -c $< $(INCLUDES)

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $< $(INCLUDES)

counter-cgi: counter.o
	$(CC) $(CFLAGS) -o $@ $^ -lfcgi $(LDFLAGS)

cgiapp.a: cgiapp.o critbit/critbit.o
	$(AR) -q $@ $^

keyval-cgi: keyval.o nosql.o cgiapp.a
	$(CC) $(CFLAGS) -o $@ $^ -lfcgi $(LDFLAGS)

%-cgi: %.o cgiapp.a
	$(CC) $(CFLAGS) -o $@ $^ -lfcgi $(LDFLAGS)

fastcgi-test: tests.o nosql.o critbit/test_critbit.o critbit/CuTest.o critbit/critbit.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f *~ *.a *.o $(PROGRAMS)

install: $(PROGRAMS)
	mkdir -p $(PREFIX)/bin
	install html/*.* $(WEBSITE)
	install $(PROGRAMS) $(PREFIX)/bin

check: $(PROGRAMS)
	cd tests && runtests -l TESTS
