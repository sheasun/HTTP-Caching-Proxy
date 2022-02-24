all: main

main: main.cpp  server.cpp
	g++ -o main main.cpp server.cpp cache.cpp client.cpp proxy.cpp request.cpp response.cpp -lpthread

.PHONY:
	clean
	
clean:
	rm -rf *.o main