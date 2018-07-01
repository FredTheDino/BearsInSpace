# -nostdinc
#  -Werror
make:
	g++ -ggdb -Wall -std=c++11 -O0 -D__DEBUG=1 -DLINUX -fPIC src/bear_main.cpp -shared -o bin/libgame.so 
	g++ -ggdb -Wall -std=c++11 -O0 -D__DEBUG=1 -DLINUX src/bear_linux.cpp -o bin/game -Iinc -L./lib/linux -lSDL2 -lSDL2main -ldl -lpthread

run: make
	./bin/game

debug: make
	gdb ./bin/game

valgrind: make
	valgrind --suppressions=sup --tool=memcheck --show-reachable=yes --num-callers=20 --track-fds=yes --leak-check=full -v ./bin/game 2> temp

