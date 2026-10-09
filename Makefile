.PHONY: run

nearest_station.exe: nearest_station_src/nearest_station.c libstationmapper.dll
	gcc -L. -o nearest_station.exe nearest_station_src/nearest_station.c -lstationmapper

run: nearest_station.exe
	./nearest_station.exe

stationmapper.o: src/stationmapper.c
	gcc -c -fpic src/stationmapper.c -lm

libstationmapper.dll: stationmapper.o
	gcc -shared -o libstationmapper.dll stationmapper.o