# Some FastCGI web services

## installation

`make && make install` (requires a sudo password) to build and isntall
the code. Then edit /etc/nginx/sites-available/default (or any other
site you want to add the services to). Add `include
/etc/nginx/fastcgi.conf` inside a server definition, it adds a /api/
location. Start the service, e.g. `service <servicename> start` and
test it from a browser (see below).

## counter

a web counter? TBD

## ennodb

A simple key-value store.

    # Start the service:
    sudo service ennodb start
    # store a value for key "foo":
    curl --data "Hello World" http://localhost/api/ennodb.cgi/foo
    # retrieve the value for key "foo":
    curl http://localhost/api/ennodb.cgi/foo

This should print "Hello World"


## prefix

prefix search in a dictionary, for example as an auto-complete service to
back a web form. See html/prefix.html for an example implementation.
