

all : comp

comp : comp.cc
	g++ -Wall -Wextra -pedantic -o comp comp.cc

