TSBK08
======

Laboration in TSBK08 Data Compression

Laboration tasks:

Entropy estimation
Make random models for the different test files and estimate entropies for them. 
You should at least estimate H(Xi), H(Xi|Xi-1) and H(Xi|Xi-1,Xi-2) for all sources.

Source coding
Implement two of the following compression methods and test them by coding all the test files. 
At least one of the implemented methods should utilize the memory of the sources.
Huffman coding (static or adaptive code tree)
Arithmetic coding (static or adaptive probability model)
Lempel-Ziv coding (LZ77, LZSS, LZ78, LZW or another variant)
Burrows-Wheeler block transform

Compare your results with the estimated entropies.

You are free to choose any programming language for your implementations. 
Matlab will work nicely for entropy estimation and for simple source coding, 
but might be too slow for some coding methods, especially those that involve searching (LZ) or sorting (BWT).

Test data files is the Canterbury Corpus found at http://corpus.canterbury.ac.nz/
The files used are the Canterbury Corpus and the Large Corpus.
