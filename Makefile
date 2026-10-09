.PHONY: run

example.exe: example_src/example.c libstationmapper.dll
	gcc -L. -o example.exe example_src/example.c -lstationmapper

run: example.exe
	./example.exe

stationmapper.o: src/stationmapper.c
	gcc -c -fpic src/stationmapper.c -lm

libstationmapper.dll: stationmapper.o
	gcc -shared -o libstationmapper.dll stationmapper.o