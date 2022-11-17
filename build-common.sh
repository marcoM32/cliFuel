#!/bin/bash

RCEDIT_V="v1.1.1"
CURL_V="7.86.0"
OPENSSL_V="1.1.1h"
TERMCAP_V="1.3.1"
CCCSVPARSER_V="2016-12-06"

clean() {
    rm -Rfv ./curl/ > /dev/null
    rm -Rfv ./termcap-lib/ > /dev/null
    rm -Rfv ./log.c/ > /dev/null
    rm -Rfv ./map/ > /dev/null
    rm -Rfv ./strsplit.c/ > /dev/null
    rm -Rfv ./CsvParser/ > /dev/null
    rm -Rfv ./progressbar/ > /dev/null
    rm -Rfv ./dmt/ > /dev/null
    rm -Rfv *.res *.dll *.so *.o > /dev/null
}

build() {
    for build in "${BUILDS[@]}" ; do
        KEY="${build%%:*}"
        VALUE="${build##*:}"

        if [ -n "${DEBUG}" ]; then
            echo "Active debug build..."
            VALUE+=' -DDEBUG'
        fi

        echo "Start build for ${KEY} with ${VALUE}"

        if [ -v WINDRES ]; then
            $WINDRES resource.rc -O coff -o resource.res
        fi;
        $CC $ARCH -Wall $VALUE $DEBUG $COMPILER_DIR -c main.c -o main.o
        $CC $ARCH -Wall $VALUE $DEBUG $COMPILER_DIR -c opendata.c -o opendata.o
        if [ -v WINDRES ]; then
            $CC $ARCH $LINKER_DIR -o "$BUILD_DIR/$KEY" main.o opendata.o resource.res -O3 $LIBRARIES
        else
            $CC $ARCH $LINKER_DIR -o "$BUILD_DIR/$KEY" main.o opendata.o -O3 $LIBRARIES
        fi;
    done
}

finalize() {
    cp cliFuel-demon.sh $BUILD_DIR
    chmod +x $BUILD_DIR/cliFuel-demon.sh

    cp LICENSE $BUILD_DIR
    cp README.md $BUILD_DIR

    for build in "${BUILDS[@]}" ; do
        KEY="${build%%:*}"
        VALUE="${build##*:}"
        if [ -f "${BUILD_DIR}/${KEY}" ]; then
            chmod +x $BUILD_DIR/"${build%%:*}"
            "${BUILD_DIR}/${KEY}" -h
        else
            echo "${build%%:*}"
            echo "Build error... "${BUILD_DIR}/${KEY}" not exist"
        fi
    done
}