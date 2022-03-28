#!bash

clean() {
    rm -Rfv ./curl/ > /dev/null;
    rm -Rfv ./OpenSSL/ > /dev/null;
    rm -Rfv ./log.c/ > /dev/null;
    rm -Rfv ./map/ > /dev/null;
    rm -Rfv ./CsvParser/ > /dev/null;
    rm -Rfv ./progressbar/ > /dev/null;
    rm -Rfv ./termcap-lib/ > /dev/null;
    rm -Rfv ./termcap-bin/ > /dev/null;
    rm -Rfv ./dmt/ > /dev/null;
    rm -f *.res rcedit-x86.exe *.dll > /dev/null;
}

ARCH="-m32";
DEBUG="";
CC="x86_64-w64-mingw32-gcc.exe";
AR="x86_64-w64-mingw32-gcc-ar.exe";

while getopts 'cd' OPTION; do
    case "$OPTION" in
        c)
            echo "Starting project cleaning...";
            clean;
            echo "Finish";
            exit 0;
            ;;
        d)
            DEBUG="-g";
            ;;
    esac
done

echo "Starting Windows build for cliFuel...";

BUILD_DIR="./bin/windows";
echo "Build dir: $BUILD_DIR";

rm -f *.o > /dev/null;
rm -Rf $BUILD_DIR;
mkdir -p $BUILD_DIR;

if [ ! -f "rcedit-x86.exe" ]; then
    wget https://github.com/electron/rcedit/releases/download/v1.1.1/rcedit-x86.exe;
fi

if [ ! -d "./curl" ]; then
    wget https://curl.se/windows/dl-7.82.0_1/curl-7.82.0_1-win32-mingw.zip;
    unzip -d curl curl-7.73.0_1-win32-mingw.zip > /dev/null;
    rm -f curl-7.73.0_1-win32-mingw.zip > /dev/null;
fi

if [ ! -d "./OpenSSL" ]; then
    wget https://downloads.sourceforge.net/project/openssl-for-windows/OpenSSL-1.1.1h_win32.zip;
    unzip -d OpenSSL OpenSSL-1.1.1h_win32.zip > /dev/null;
    rm -f OpenSSL-1.1.1h_win32.zip > /dev/null;
fi

if [ ! -d "./log.c/" ]; then
    git clone https://github.com/rxi/log.c.git;
fi

if [ ! -d "./map/" ]; then
    git clone https://github.com/rxi/map.git;
fi

if [ ! -d "./CsvParser/" ]; then
    wget https://downloads.sourceforge.net/project/cccsvparser/2016-12-06/CsvParser_2016_12_06.zip;
    unzip CsvParser_2016_12_06.zip > /dev/null;
    rm -f CsvParser_2016_12_06.zip > /dev/null;
fi

if [ ! -d "./termcap-lib/" ]; then
    wget https://downloads.sourceforge.net/project/gnuwin32/termcap/1.3.1/termcap-1.3.1-lib.zip;
    unzip -d termcap-lib termcap-1.3.1-lib.zip > /dev/null;
    rm -f termcap-1.3.1-lib.zip > /dev/null;
fi

if [ ! -d "./termcap-bin/" ]; then
    wget https://downloads.sourceforge.net/project/gnuwin32/termcap/1.3.1/termcap-1.3.1-bin.zip;
    unzip -d termcap-bin termcap-1.3.1-bin.zip > /dev/null;
    rm -f termcap-1.3.1-bin.zip > /dev/null;
fi

if [ ! -d "./progressbar/" ]; then
    git clone https://github.com/doches/progressbar.git;
fi

if [ ! -d "./dmt/" ]; then
    git clone https://github.com/rxi/dmt.git;
fi

if [ ! -f "./map/librximap.a" ]; then
    $CC $ARCH $DEBUG -I./map/src/ -c ./map/src/map.c -o ./map/map.o;
    $AR -rcs ./map/librximap.a ./map/map.o;
fi

if [ ! -f "./log.c/librxilog.a" ]; then
    $CC $ARCH $DEBUG -I./log.c/src/ -c ./log.c/src/log.c -o ./log.c/log.o;
    $AR -rcs ./log.c/librxilog.a ./log.c/log.o;
fi

if [ ! -f "./CsvParser/libcsvparser.a" ]; then
    $CC $ARCH $DEBUG -I./CsvParser/include/ -c ./CsvParser/src/csvparser.c -o ./CsvParser/csvparser.o;
    $AR -rcs ./CsvParser/libcsvparser.a ./CsvParser/csvparser.o;
fi

if [ ! -f "./dmt/librxidmt.a" ]; then
    $CC $ARCH $DEBUG -I./dmt/src/ -c ./dmt/src/dmt.c -o ./dmt/dmt.o;
    $AR -rcs ./dmt/librxidmt.a ./dmt/dmt.o;
fi

if [ ! -f "./progressbar/libprogressbar.a" ]; then
    $CC $ARCH $DEBUG -I./progressbar/include/progressbar/ -I./termcap-lib/include -c ./progressbar/lib/progressbar.c -o ./progressbar/lib/progressbar.o;
    $CC $ARCH $DEBUG -I./progressbar/include/progressbar/ -I./termcap-lib/include -c ./progressbar/lib/statusbar.c -o ./progressbar/lib/statusbar.o;
    $AR -rcs ./progressbar/libprogressbar.a ./progressbar/lib/progressbar.o ./progressbar/lib/statusbar.o;
fi

COMPILER_DIR="-I./curl/include -I./log.c/src -I./CsvParser/include -I./map/src -I./progressbar/include/progressbar -I./dmt/src";
LINKER_DIR="-L./curl/lib/ -L./log.c/ -L./CsvParser/ -L./map/ -L./progressbar/ -L./dmt/";
LIBRARIES="-lcurl -lrxilog -lcsvparser -lrximap -lprogressbar -lrxidmt -lpthread";

BUILDS=( "cliFuel-basic.exe:-DFILE_DOWNLOAD" "cliFuel.exe:-DFILE_DOWNLOAD -DANIMATION");

for build in "${BUILDS[@]}" ; do
    KEY="${build%%:*}";
    VALUE="${build##*:}";

    if [ -n "${DEBUG}" ]; then
        echo "Active debug build...";
        VALUE+=' -DDEBUG';
    fi

    echo "Start build for ${KEY} with ${VALUE}";

    $CC $ARCH -Wall $VALUE $DEBUG $COMPILER_DIR -c main.c -o main.o;
    $CC $ARCH -Wall $VALUE $DEBUG $COMPILER_DIR -c opendata.c -o opendata.o;
    $CC $ARCH $LINKER_DIR -o "$BUILD_DIR/$KEY" main.o opendata.o -O3 $LIBRARIES;

    ./rcedit-x86.exe "$BUILD_DIR/$KEY" --set-icon "icon.ico";
done

cp ./curl/bin/libcurl.dll $BUILD_DIR;
cp ./OpenSSL/libcrypto-1_1.dll $BUILD_DIR;
cp ./OpenSSL/libssl-1_1.dll $BUILD_DIR;

cp cliFuel-demon.sh $BUILD_DIR;
chmod +x $BUILD_DIR/cliFuel-demon.sh;

cp LICENSE $BUILD_DIR;
cp README.md $BUILD_DIR;

for build in "${BUILDS[@]}" ; do
    KEY="${build%%:*}";
    VALUE="${build##*:}";
    if [ -f "${BUILD_DIR}/${KEY}" ]; then
        chmod +x $BUILD_DIR/"${build%%:*}";
        "${BUILD_DIR}/${KEY}" -h;
    else
        echo "${build%%:*}";
        echo "Build error... "${BUILD_DIR}/${KEY}" not exist";
    fi
done
