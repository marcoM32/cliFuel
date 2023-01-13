#!/bin/bash

source build-common.sh

ARCH="-m32"
DEBUG=""
CC="gcc.exe"
AR="gcc-ar.exe"
WINDRES="./windres.exe"

while getopts 'cd' OPTION; do
    case "$OPTION" in
        c)
            echo "Starting project cleaning..."
            clean
            echo "Finish"
            exit 0
            ;;
        d)
            DEBUG="-g"
            ;;
    esac
done

echo "Starting Windows build for cliFuel..."

BUILD_DIR="./bin/windows"
echo "Build dir: $BUILD_DIR"

rm -f *.o > /dev/null
rm -Rf $BUILD_DIR
mkdir -p $BUILD_DIR

if [ ! -d "./curl" ]; then
    wget https://curl.se/windows/dl-$CURL_V/curl-$CURL_V-win32-mingw.zip
    unzip -d curl -j curl-$CURL_V-win32-mingw.zip > /dev/null
    rm -f curl-$CURL_V-win32-mingw.zip > /dev/null
fi

if [ ! -d "./log.c/" ]; then
    git clone https://github.com/rxi/log.c.git
fi

if [ ! -d "./map/" ]; then
    git clone https://github.com/rxi/map.git
fi

if [ ! -d "./strsplit.c/" ]; then
    git clone https://github.com/mr21/strsplit.c.git
fi

if [ ! -d "./CsvParser/" ]; then
    wget https://downloads.sourceforge.net/project/cccsvparser/$CCCSVPARSER_V/CsvParser_${CCCSVPARSER_V//[-]/_}.zip
    unzip CsvParser_${CCCSVPARSER_V//[-]/_}.zip > /dev/null
    rm -f CsvParser_${CCCSVPARSER_V//[-]/_}.zip > /dev/null
fi

if [ ! -d "./termcap-lib/" ]; then
    wget https://downloads.sourceforge.net/project/gnuwin32/termcap/$TERMCAP_V/termcap-$TERMCAP_V-lib.zip
    unzip -d termcap-lib termcap-$TERMCAP_V-lib.zip > /dev/null
    rm -f termcap-$TERMCAP_V-lib.zip > /dev/null
fi

if [ ! -d "./progressbar/" ]; then
    git clone https://github.com/doches/progressbar.git
fi

if [ ! -d "./dmt/" ]; then
    git clone https://github.com/rxi/dmt.git
fi

if [ ! -f "./map/librximap.a" ]; then
    $CC $ARCH $DEBUG -I./map/src/ -c ./map/src/map.c -o ./map/map.o
    $AR -rcs ./map/librximap.a ./map/map.o
fi

if [ ! -f "./strsplit.c/libstrsplit.a" ]; then
    $CC $ARCH $DEBUG -I./strsplit.c/ -c ./strsplit.c/strsplit.c -o ./strsplit.c/strsplit.o
    $AR -rcs ./strsplit.c/libstrsplit.a ./strsplit.c/strsplit.o
fi

if [ ! -f "./log.c/librxilog.a" ]; then
    $CC $ARCH $DEBUG -I./log.c/src/ -c ./log.c/src/log.c -o ./log.c/log.o
    $AR -rcs ./log.c/librxilog.a ./log.c/log.o
fi

if [ ! -f "./CsvParser/libcsvparser.a" ]; then
    $CC $ARCH $DEBUG -I./CsvParser/include/ -c ./CsvParser/src/csvparser.c -o ./CsvParser/csvparser.o
    $AR -rcs ./CsvParser/libcsvparser.a ./CsvParser/csvparser.o
fi

if [ ! -f "./dmt/librxidmt.a" ]; then
    $CC $ARCH $DEBUG -I./dmt/src/ -c ./dmt/src/dmt.c -o ./dmt/dmt.o
    $AR -rcs ./dmt/librxidmt.a ./dmt/dmt.o
fi

if [ ! -f "./progressbar/libprogressbar.a" ]; then
    $CC $ARCH $DEBUG -L./termcap-lib/lib -I./progressbar/include/progressbar/ -I./termcap-lib/include -c ./progressbar/lib/progressbar.c -o ./progressbar/lib/progressbar.o -ltermcap
    $CC $ARCH $DEBUG -L./termcap-lib/lib -I./progressbar/include/progressbar/ -I./termcap-lib/include -c ./progressbar/lib/statusbar.c -o ./progressbar/lib/statusbar.o -ltermcap
    $AR -rcs ./progressbar/libprogressbar.a ./progressbar/lib/progressbar.o ./progressbar/lib/statusbar.o
fi

COMPILER_DIR="-I. -I./log.c/src -I./CsvParser/include -I./map/src -I./strsplit.c -I./progressbar/include/progressbar -I./dmt/src"
LINKER_DIR="-L./curl/ -L./log.c/ -L./CsvParser/ -L./map/ -L./strsplit.c/ -L./progressbar/ -L./dmt/"
LIBRARIES="-lcurl -lrxilog -lcsvparser -lrximap -lstrsplit -lprogressbar -lrxidmt -lpthread"

BUILDS=("cliFuel-basic.exe:-DFILE_DOWNLOAD" "cliFuel.exe:-DFILE_DOWNLOAD -DANIMATION" "cliFuel.exe:-DFILE_DOWNLOAD -DANIMATION -DHTML_OUTPUT")

build

if [ -f "./curl/libcurl.dll" ]; then
    cp ./curl/libcurl.dll $BUILD_DIR
fi

finalize