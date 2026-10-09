example.exe: example_src/example.c libstationmapper.dll
	gcc -L. -o example.exe example_src/example.c -lstationmapper

run: example.exe
	./example.exe data/map_1.bmp data/map_1.csv data/stations_1.csv

stationmapper.o: src/stationmapper.c
	gcc -c -fpic src/stationmapper.c

libstationmapper.dll: stationmapper.o
	gcc -shared -o libstationmapper.dll stationmapper.o