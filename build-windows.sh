#!bash
echo "starting Windows build for cliFuel...";

BUILD_DIR="./bin/windows";
echo "Build dir: $BUILD_DIR";

rm -Rf $BUILD_DIR;
mkdir -p $BUILD_DIR;

wget https://github.com/electron/rcedit/releases/download/v1.1.1/rcedit-x86.exe;
wget https://curl.haxx.se/windows/dl-7.73.0_1/curl-7.73.0_1-win32-mingw.zip;
wget https://downloads.sourceforge.net/project/openssl-for-windows/OpenSSL-1.1.1h_win32.zip;
git clone https://github.com/rxi/log.c.git;
git clone https://github.com/rxi/map.git;
wget https://downloads.sourceforge.net/project/cccsvparser/2016-12-06/CsvParser_2016_12_06.zip;
wget https://downloads.sourceforge.net/project/gnuwin32/termcap/1.3.1/termcap-1.3.1-lib.zip;
wget https://downloads.sourceforge.net/project/gnuwin32/termcap/1.3.1/termcap-1.3.1-bin.zip;
git clone https://github.com/doches/progressbar.git;

unzip curl-7.73.0_1-win32-mingw.zip > /dev/null;
rm -f curl-7.73.0_1-win32-mingw.zip > /dev/null;

unzip OpenSSL-1.1.1h_win32.zip > /dev/null;
rm -f OpenSSL-1.1.1h_win32.zip > /dev/null;

unzip CsvParser_2016_12_06.zip > /dev/null;
rm -f CsvParser_2016_12_06.zip > /dev/null;

unzip -d termcap-1.3.1-lib termcap-1.3.1-lib.zip > /dev/null;
rm -f termcap-1.3.1-lib.zip > /dev/null;

unzip -d termcap-1.3.1-bin termcap-1.3.1-bin.zip > /dev/null;
rm -f termcap-1.3.1-bin.zip > /dev/null;

mingw32-gcc.exe -Wall -g -I./map/src/ -c ./map/src/map.c -o ./map/map.o;
mingw32-gcc.exe -Wall -g -I./map/src/ -O2 -shared ./map/src/map.c -o ./map/rximap.dll;

mingw32-gcc.exe -Wall -g -DLOG_USE_COLOR -I./log.c/src/ -c ./log.c/src/log.c -o ./log.c/log.o;
mingw32-gcc.exe -Wall -g -I./log.c/src/ -O2 -shared ./log.c/src/log.c -o ./log.c/rxilog.dll

mingw32-gcc.exe -Wall -g -I./CsvParser/include/ -c ./CsvParser/src/csvparser.c -o ./CsvParser/csvparser.o;
mingw32-gcc.exe -Wall -g -I./CsvParser/include/ -O2 -shared ./CsvParser/src/csvparser.c -o ./CsvParser/csvparser.dll;

mingw32-gcc.exe -Wall -g -I./termcap-1.3.1-lib/include/ -I./progressbar/include/progressbar/ -c ./progressbar/lib/progressbar.c -o ./progressbar/lib/progressbar.o;
mingw32-gcc.exe -Wall -g -I./termcap-1.3.1-lib/include/ -I./progressbar/include/progressbar/ -c ./progressbar/lib/statusbar.c -o ./progressbar/lib/statusbar.o;
mingw32-gcc.exe -Wall -g -I./termcap-1.3.1-lib/include/ -I./progressbar/include/progressbar/ -L./termcap-1.3.1-lib/lib/ -O2 -shared ./progressbar/lib/*.c -o ./progressbar/progressbar.dll -ltermcap;

mingw32-gcc.exe -Wall -DFILE_DOWNLOAD -DANIMATION -g -I./curl-7.73.0-win32-mingw/include -I./log.c/src -I./CsvParser/include -I./map/src -I./progressbar/include/progressbar/ -c main.c -o main.o;
mingw32-gcc.exe -Wall -DFILE_DOWNLOAD -DANIMATION -g -I./curl-7.73.0-win32-mingw/include -I./log.c/src -I./CsvParser/include -I./map/src -I./progressbar/include/progressbar/ -c opendata.c -o opendata.o;
mingw32-gcc.exe -L./curl-7.73.0-win32-mingw/bin/ -L./log.c/ -L./CsvParser/ -L./map/ -L./progressbar/ -o $BUILD_DIR/cliFuel.exe main.o opendata.o -O3 -lcurl -lrxilog -lcsvparser -lrximap -lprogressbar -lpthread;

./rcedit-x86.exe "./bin/windows/cliFuel.exe" --set-icon "icon.ico";

cp ./curl-7.73.0-win32-mingw/bin/libcurl.dll $BUILD_DIR;
cp ./OpenSSL-1.1.1h_win32/libcrypto-1_1.dll $BUILD_DIR;
cp ./OpenSSL-1.1.1h_win32/libssl-1_1.dll $BUILD_DIR;
cp ./map/rximap.dll $BUILD_DIR;
cp ./log.c/rxilog.dll $BUILD_DIR;
cp ./CsvParser/csvparser.dll $BUILD_DIR;
cp ./progressbar/progressbar.dll $BUILD_DIR;
cp ./termcap-1.3.1-bin/bin/termcap.dll $BUILD_DIR;

rm -Rfv ./curl-7.73.0-win32-mingw/ > /dev/null;
rm -Rfv ./OpenSSL-1.1.1h_win32/ > /dev/null;
rm -Rfv ./log.c/ > /dev/null;
rm -Rfv ./map/ > /dev/null;
rm -Rfv ./CsvParser/ > /dev/null;
rm -Rfv ./progressbar/ > /dev/null;
rm -Rfv ./termcap-1.3.1-lib/ > /dev/null;
rm -Rfv ./termcap-1.3.1-bin/ > /dev/null;
rm -f *.o *.res rcedit-x86.exe *.dll > /dev/null;

cp cliFuel-demon.sh $BUILD_DIR;

cp LICENSE $BUILD_DIR;
cp README.md $BUILD_DIR;

$BUILD_DIR/cliFuel -h;