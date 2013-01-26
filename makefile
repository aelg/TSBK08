

all : comp entropy

comp : comp.cc
	g++ -Wall -Wextra -pedantic -o comp comp.cc

entropy : entropy.cc
	g++ -Wall -Wextra -pedantic -o entropy entropy.cc

clean : 
	rm entropy comp
