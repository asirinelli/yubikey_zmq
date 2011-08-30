#!/bin/sh
ver=$1
archive=yubi-zeromq_$ver.orig.tar
dir=yubi-zeromq-$ver
git archive --prefix=$dir/ -o /tmp/$archive HEAD
cd /tmp
gzip -9 $archive
rm -rf $dir
tar xvf $archive.gz
cd $dir
debuild -i -us -uc
