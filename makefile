
all : comp entropy base64 huffman huffman_memory

comp : comp.cc
	g++ -Wall -Wextra -pedantic -O2 -o comp comp.cc

entropy : entropy.cc
	g++ -Wall -Wextra -pedantic -O2 -o entropy entropy.cc

base64 : base64.cc
	g++ -Wall -Wextra -pedantic -O2 -o base64 base64.cc

huffman : huffman.cc
	g++ -Wall -Wextra -pedantic -O2 -o huffman huffman.cc

huffman_memory : huffman_memory.cc
	g++ -Wall -Wextra -pedantic -O2 -o huffman_memory huffman_memory.cc

clean : 
	rm entropy comp huffman huffman_memory base64
