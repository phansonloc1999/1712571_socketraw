all:
	gcc src/1712571.c -o output/1712571

run: all
	sudo ./output/1712571 192.168.1.1

clean:
	rm -f output/*