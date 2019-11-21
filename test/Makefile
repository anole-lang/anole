CC = clang++

test: tokenizer-tester.cpp parser-tester.cpp
	$(CC) -std=c++17 tokenizer-tester.cpp -o test
	./test
	$(CC) -std=c++17 parser-tester.cpp -o test
	./test
	rm test

.PHONY: clean
clean:
	rm test
