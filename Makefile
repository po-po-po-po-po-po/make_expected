FLAGS=-std=c++11 -I "$(HOME)\Documents\Arduino\libraries\UdonLibrary\src"

main: main.cpp
	g++ -o main main.cpp $(FLAGS) && ./main
	rm -f main.exe main

# clean:
