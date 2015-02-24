FROM debian:stable
MAINTAINER enno.rehling@gmail.com
RUN apt-get update && apt-get install spawn-fcgi libfcgi
WORKDIR /
COPY bin/* /usr/local/bin/
#COPY lib/* /usr/local/lib/
CMD /usr/bin/spawn-fcgi -n -p 6543 /usr/local/bin/counter-cgi
EXPOSE 6543
