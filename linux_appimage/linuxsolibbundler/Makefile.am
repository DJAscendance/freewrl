DESTDIR=
PREFIX=/usr/local
CXXFLAGS = -O2

all: solibbundler

solibbundler:
	$(CXX) $(CXXFLAGS) -c -I./src ./src/Settings.cpp -o ./Settings.o
	$(CXX) $(CXXFLAGS) -c -I./src ./src/SoBundler.cpp -o ./SoBundler.o
	$(CXX) $(CXXFLAGS) -c -I./src ./src/Dependency.cpp -o ./Dependency.o
	$(CXX) $(CXXFLAGS) -c -I./src ./src/main.cpp -o ./main.o
	$(CXX) $(CXXFLAGS) -c -I./src ./src/Utils.cpp -o ./Utils.o
	$(CXX) $(CXXFLAGS) -o ./solibbundler ./Settings.o ./SoBundler.o ./Dependency.o ./main.o ./Utils.o

clean:
	rm -f *.o
	rm -f ./solibbundler

install: solibbundler
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp ./dylibbundler $(DESTDIR)$(PREFIX)/bin/solibbundler
	chmod 775 $(DESTDIR)$(PREFIX)/bin/solibbundler

.PHONY: all clean install
