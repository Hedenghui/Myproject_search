FLAG=-std=c++11 -I /home/hedenghui/search/cppjieba/include -I /home/hedenghui/search/cppjieba/deps -lpthread -L. -lsearcher

http_server:http_server.cpp
	g++ $^ -o $@ $(FLAG)

.PHONY:clean
clean:
	rm http_server