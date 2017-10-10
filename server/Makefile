all: myftpd

myftpd: server.cpp main.cpp
	g++ -g -Wall -std=c++11 $^ -o $@

clean:
	rm -rf myftpd
