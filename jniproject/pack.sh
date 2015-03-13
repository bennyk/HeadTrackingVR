#!/bin/sh

if [ ! -d "lib" ]; then
	# create a link to libs if non exists
	ln -s libs lib
fi

jarout=liblynda-demo.jar
ndk-build NDK_DEBUG=1
zip -r $jarout lib

# making sure the destination path locates the libs directory in the app for the jar file.
mv $jarout ../app/libs/.

