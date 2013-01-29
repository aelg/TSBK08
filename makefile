

all : comp entropy base64

comp : comp.cc
	g++ -Wall -Wextra -pedantic -o comp comp.cc

entropy : entropy.cc
	g++ -Wall -Wextra -pedantic -o entropy entropy.cc

base64 : base64.cc
	g++ -Wall -Wextra -pedantic -o base64 base64.cc

clean : 
	rm entropy comp
