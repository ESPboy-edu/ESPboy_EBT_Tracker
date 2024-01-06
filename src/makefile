CPPFLAGS=-I/usr/include/SDL2 -I/usr/include -std=c++11 -fpermissive -static -fno-strict-aliasing -fPIC
CXXFLAGS=-O2
LDFLAGS=-lSDL2 -lpthread

%.o: %.cpp
	g++ -c $(CPPFLAGS) $(CXXFLAGS) -o $@ $^

ebtsdl: main.o

	g++ -o $@ $^ $(LDFLAGS)

main.o: main.cpp

.PHONY: clean

clean:
	rm *.o
