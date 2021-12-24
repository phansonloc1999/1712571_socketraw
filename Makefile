all:
	gcc src/ping.c -o output/ping

run: all
	sudo ./output/ping 192.168.1.1

clean:
	rm -f output/*