all:
	g++ src/main.cpp -o output/1712571

run: output/1712571
	./output/1712571 192.168.1.0/24