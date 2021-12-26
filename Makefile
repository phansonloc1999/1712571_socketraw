all:
	gcc src/ping.c -o output/1712571 -lm
	sudo setcap cap_net_admin,cap_net_raw=eip output/1712571

run: all
	./output/1712571 192.168.1.0/24

clean:
	rm -f output/*