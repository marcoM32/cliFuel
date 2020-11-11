#!/bin/bash
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

git clone https://github.com/rxi/log.c.git;
git clone https://github.com/rxi/map.git;
wget https://downloads.sourceforge.net/project/cccsvparser/2016-12-06/CsvParser_2016_12_06.zip;
git clone https://github.com/doches/progressbar.git;

unzip CsvParser_2016_12_06.zip > /dev/null;
rm -f CsvParser_2016_12_06.zip > /dev/null;

gcc -Wall -g -I./map/src/ -c ./map/src/map.c -o ./map/map.o;
gcc -Wall -g -I./map/src/ -O2 -shared ./map/src/map.c -o ./map/rximap.so;
ar -rcs ./map/librximap.a ./map/map.o;

gcc -Wall -g -DLOG_USE_COLOR -I./log.c/src/ -c ./log.c/src/log.c -o ./log.c/log.o;
gcc -Wall -g -I./log.c/src/ -O2 -shared ./log.c/src/log.c -o ./log.c/rxilog.so;
ar -rcs ./log.c/librxilog.a ./log.c/log.o;

gcc -Wall -g -I./CsvParser/include/ -c ./CsvParser/src/csvparser.c -o ./CsvParser/csvparser.o;
gcc -Wall -g -I./CsvParser/include/ -O2 -shared ./CsvParser/src/csvparser.c -o ./CsvParser/csvparser.so;
ar -rcs ./CsvParser/libcsvparser.a ./CsvParser/csvparser.o;

gcc -Wall -g -I./progressbar/include/progressbar/ -c ./progressbar/lib/progressbar.c -o ./progressbar/lib/progressbar.o;
gcc -Wall -g -I./progressbar/include/progressbar/ -c ./progressbar/lib/statusbar.c -o ./progressbar/lib/statusbar.o;
gcc -Wall -g -I./progressbar/include/progressbar/ -O2 -shared ./progressbar/lib/*.c -o ./progressbar/progressbar.so -ltermcap;
ar -rcs ./progressbar/libprogressbar.a ./progressbar/lib/progressbar.o ./progressbar/lib/statusbar.o;

gcc -Wall -DFILE_DOWNLOAD -DANIMATION -g -I./log.c/src -I./CsvParser/include -I./map/src -I./progressbar/include/progressbar/ -c main.c -o main.o;
gcc -Wall -DFILE_DOWNLOAD -DANIMATION -g -I./log.c/src -I./CsvParser/include -I./map/src -I./progressbar/include/progressbar/ -c opendata.c -o opendata.o;
gcc -L./log.c/ -L./CsvParser/ -L./map/ -L./progressbar/ -o cliFuel main.o opendata.o -O3 -lcurl -lrxilog -lcsvparser -lrximap -lprogressbar -lpthread;

cp ./map/rximap.so .;
cp ./log.c/rxilog.so .;
cp ./CsvParser/csvparser.so .;
cp ./progressbar/progressbar.so .;

rm -Rfv ./log.c/ > /dev/null;
rm -Rfv ./map/ > /dev/null;
rm -Rfv ./CsvParser/ > /dev/null;
rm -Rfv ./progressbar/ > /dev/null;
rm -f *.o > /dev/null;

chmod +x cliFuel;
chmod +x cliFuel-demon.sh;

./cliFuel -h;
