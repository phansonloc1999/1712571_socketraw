all:
	gcc src/ping.c -o output/ping -lm
	sudo setcap cap_net_admin,cap_net_raw=eip output/ping

run: all
	./output/ping 192.168.1.0/24

clean:
	rm -f output/*