#!/bin/bash

clean() {
	rm -Rfv ./log.c/ > /dev/null;
	rm -Rfv ./map/ > /dev/null;
	rm -Rfv ./CsvParser/ > /dev/null;
	rm -Rfv ./progressbar/ > /dev/null;
	rm -Rfv ./dmt/ > /dev/null;
}

DEBUG="";

while getopts 'cdf' OPTION; do
    case "$OPTION" in
        c)
            echo "starting project cleaning...";
			clean;
			echo "finish";
            		exit 0;
			;;
		d)
			DEBUG="-g";
			;;
    esac
done

echo "starting Linux build for cliFuel...";

dpkg -s "libcurl4-openssl-dev" >/dev/null 2>&1 && {
        echo "libcurl is installed";
} || {
        echo "missing libcurl --> run sudo apt-get install libcurl4-openssl-dev";
        exit -1;
}

dpkg -s "libncurses5-dev" >/dev/null 2>&1 && {
        echo "libncurses is installed";
} || {
        echo "missing libncurses --> run sudo apt-get install libncurses5-dev";
        exit -1;
}

dpkg -s "astyle" >/dev/null 2>&1 && {
        eval "astyle --style=java --style=attach -A2 --suffix=none ./*.c,*.h";
} || {
        echo "formatting source is not possible";
}

BUILD_DIR="./bin/linux";
echo "Build dir: $BUILD_DIR";

rm -f *.o *.so > /dev/null;
rm -Rf $BUILD_DIR;
mkdir -p $BUILD_DIR;

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

if [ ! -d "./progressbar/" ]; then
  git clone https://github.com/doches/progressbar.git;
fi

if [ ! -d "./dmt/" ]; then
  git clone https://github.com/rxi/dmt.git;
fi

gcc -Wall $DEBUG -I./map/src/ -c ./map/src/map.c -o ./map/map.o;
ar -rcs ./map/librximap.a ./map/map.o;

gcc -Wall $DEBUG -DLOG_USE_COLOR -I./log.c/src/ -c ./log.c/src/log.c -o ./log.c/log.o;
ar -rcs ./log.c/librxilog.a ./log.c/log.o;

gcc -Wall $DEBUG -I./CsvParser/include/ -c ./CsvParser/src/csvparser.c -o ./CsvParser/csvparser.o;
gcc -Wall $DEBUG -I./CsvParser/include/ -O2 -shared ./CsvParser/src/csvparser.c -o ./CsvParser/csvparser.so;
ar -rcs ./CsvParser/libcsvparser.a ./CsvParser/csvparser.o;

gcc -Wall $DEBUG -I./progressbar/include/progressbar/ -c ./progressbar/lib/progressbar.c -o ./progressbar/lib/progressbar.o;
gcc -Wall $DEBUG -I./progressbar/include/progressbar/ -c ./progressbar/lib/statusbar.c -o ./progressbar/lib/statusbar.o;
ar -rcs ./progressbar/libprogressbar.a ./progressbar/lib/progressbar.o ./progressbar/lib/statusbar.o;

gcc -Wall $DEBUG -I./dmt/src/ -c ./dmt/src/dmt.c -o ./dmt/dmt.o;
ar -rcs ./dmt/librxidmt.a ./dmt/dmt.o;

COMPILER_DIR="-I./log.c/src -I./CsvParser/include -I./map/src -I./progressbar/include/progressbar/ -I./dmt/src";
SYMBOLS="-DFILE_DOWNLOAD -DANIMATION -DCOLOR";

if [ -n "${DEBUG}" ]; then
  echo "Active debug build...";
  SYMBOLS+=' -DDEBUG';
fi

LINKER_DIR="-L./log.c/ -L./CsvParser/ -L./map/ -L./progressbar/ -L./dmt/";
LIBRARIES="-lcurl -lrxilog -lcsvparser -lrximap -lprogressbar -lrxidmt -lpthread";

gcc -Wall $SYMBOLS $DEBUG $COMPILER_DIR -c main.c -o main.o;
gcc -Wall $SYMBOLS $DEBUG $COMPILER_DIR -c opendata.c -o opendata.o;
gcc $LINKER_DIR -o $BUILD_DIR/cliFuel main.o opendata.o -O3 $LIBRARIES;

chmod +x $BUILD_DIR/cliFuel;

cp cliFuel-demon.sh $BUILD_DIR;
chmod +x $BUILD_DIR/cliFuel-demon.sh;

cp LICENSE $BUILD_DIR;
cp README.md $BUILD_DIR;

$BUILD_DIR/cliFuel -h;
