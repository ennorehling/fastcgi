NAME=$1-cgi
USR=/usr
TMPDIR=$(tempfile)
rm $TMPDIR && mkdir $TMPDIR
mkdir $TMPDIR/bin
mkdir $TMPDIR/lib
install $1/Dockerfile $TMPDIR
install ../$NAME $TMPDIR/bin
#install $USR/lib/libfcgi.so.0 $TMPDIR/lib
#install $USR/bin/spawn-fcgi $TMPDIR/bin
docker build -t badgerman/$NAME $TMPDIR
rm -rf $TMPDIR
