SimpleFileTransferProtocol
Cameron Smick, Ryan Smick, Connor Higgins

File Hierarchy:

	-client
		-client.cpp
		-client.h
		-main.cpp
		-Makefile
	-README.md
	-server
		-main.cpp
		-Makefile
		-server.cpp
		-server.h

Commands:

	Start the server on port 41019:
		make
		./myftpd 41019

	Start the client (assume the server is on student02.ce.nd.edu and port 41019):
		make
		./myftpd student02.cse.nd.edu 41019
