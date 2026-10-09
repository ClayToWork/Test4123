run: example
	./example map_1.bmp map_1.csv stations_1.csv

stationmapper.o: src/stationmapper.c
	gcc -c -fpic src/stationmapper.c

libstationmapper.so: stationmapper.o
	gcc -shared -o libstationmapper.so stationmapper.o

example: example_src/example.c libstationmapper.so
	gcc -L. -o example example_src/example.c -lstationmapper
