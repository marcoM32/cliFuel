#!/bin/bash
echo "starting Linux build for cliFuel...";

cd ..;

dpkg -s "libcurl4-openssl-dev" >/dev/null 2>&1 && {
	echo "libcurl is installed";
} || {
	echo "missing libcurl --> run sudo apt-get install libcurl4-openssl-dev";
	exit -1;
}

git clone https://github.com/rxi/log.c.git;
git clone https://github.com/rxi/map.git;
wget https://downloads.sourceforge.net/project/cccsvparser/2016-12-06/CsvParser_2016_12_06.zip;

unzip CsvParser_2016_12_06.zip > /dev/null;
rm -f CsvParser_2016_12_06.zip > /dev/null;

gcc -Wall -g -Wall -I./map/src/ -c ./map/src/map.c -o ./map/map.o;
gcc -I./map/src/ -g -O2 -shared ./map/src/map.c -o ./map/rximap.so;
ar -rcs ./map/librximap.a ./map/map.o;

gcc -Wall -DLOG_USE_COLOR -g -Wall -I./log.c/src/ -c ./log.c/src/log.c -o ./log.c/log.o;
gcc -I./log.c/src/ -g -O2 -shared ./log.c/src/log.c -o ./log.c/rxilog.so;
ar -rcs ./log.c/librxilog.a ./log.c/log.o;

gcc -Wall -g -Wall -I./CsvParser/include/ -c ./CsvParser/src/csvparser.c -o ./CsvParser/csvparser.o;
gcc -I./CsvParser/include/ -g -O2 -shared ./CsvParser/src/csvparser.c -o ./CsvParser/csvparser.so;
ar -rcs ./CsvParser/libcsvparser.a ./CsvParser/csvparser.o;

cd cliFuel;

gcc -Wall -DFILE_DOWNLOAD -g -Wall -DFILE_DOWNLOAD -I../log.c/src -I../CsvParser/include -I../map/src -c main.c -o main.o;
gcc -Wall -DFILE_DOWNLOAD -g -Wall -DFILE_DOWNLOAD -I../log.c/src -I../CsvParser/include -I../map/src -c opendata.c -o opendata.o;
gcc -L../log.c/ -L../CsvParser/ -L../map/ -o cliFuel main.o opendata.o -O3  -lcurl -lrxilog -lcsvparser -lrximap;

cp ../map/rximap.so .;
cp ../log.c/rxilog.so .;
cp ../CsvParser/csvparser.so .;

rm -Rfv ../log.c/ > /dev/null;
rm -Rfv ../map/ > /dev/null;
rm -Rfv ../CsvParser/ > /dev/null;
rm -f *.o > /dev/null;

chmod +x cliFuel;
chmod +x cliFuel-demon.sh;

./cliFuel -h;
