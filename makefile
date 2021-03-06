
all : comp entropy base64 huffman huffman_memory arithmetic

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

frequency.o : frequency.cc frequency.h
	g++ -Wall -Wextra -pedantic -O2 -c -o frequency.o frequency.cc

arithmetic.o : arithmetic.cc frequency.h
	g++ -Wall -Wextra -pedantic -O2 -c -o arithmetic.o arithmetic.cc

arithmetic : arithmetic.o frequency.o
	g++ -Wall -Wextra -pedantic -O2 -o arithmetic arithmetic.o frequency.o

test_huffman :
	@echo "No memory"
	@./test_compress "./huffman_memory -m 1" "./huffman_memory -m 1 -d" "huffman_memory"
	@echo "One symbol memory"
	@./test_compress "./huffman_memory -m 2" "./huffman_memory -m 2 -d" "huffman_memory"
	@echo "Two symbols memory"
	@./test_compress "./huffman_memory -m 3" "./huffman_memory -m 3 -d" "huffman_memory"

clean : 
	rm entropy comp huffman huffman_memory base64 arithmetic
