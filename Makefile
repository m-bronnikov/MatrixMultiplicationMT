all: test example generate

test: test.cpp
	g++ -std=c++14 -Wall -pedantic test.cpp -lpthread -o test

example: example.cpp
	g++ -std=c++14 -Wall -pedantic example.cpp -lpthread -o example

generate: generator.py
	python3 generator.py

clean:
	rm -rf temp/* tests/* test example
