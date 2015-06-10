PREFIX = /opt
#CFLAGS = -g -Wconversion -Wall -Werror -Wextra -Icritbit -std=c99
CFLAGS = -g -Wall -Werror -Wextra -Icritbit -std=c99
PROGRAMS = counter-cgi prefix-cgi ennodb-cgi
TESTS = fastcgi-test
WEBSITE = /var/www/html

#CC = clang
#CFLAGS += -Weverything

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

ennodb-cgi: ennodb.o nosql.o cgiapp.a
	$(CC) $(CFLAGS) -o $@ $^ -lfcgi $(LDFLAGS)

%-cgi: %.o cgiapp.a
	$(CC) $(CFLAGS) -o $@ $^ -lfcgi $(LDFLAGS)

fastcgi-test: tests.o nosql.o critbit/test_critbit.o critbit/CuTest.o critbit/critbit.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f *~ *.a *.o critbit/*.o $(PROGRAMS) $(TESTS)

install: $(PROGRAMS)
	sudo mkdir -p $(PREFIX)/bin
	install html/*.* $(WEBSITE)
	sudo install $(PROGRAMS) $(PREFIX)/bin
	sudo install etc/init.d/* /etc/init.d/
	sudo install etc/nginx/* /etc/nginx/

check: $(PROGRAMS)
	cd tests && runtests -l TESTS
