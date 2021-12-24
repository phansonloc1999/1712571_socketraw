all:
	g++ src/main.cpp -o output/1712571

run: all
	./output/1712571 192.168.1.024