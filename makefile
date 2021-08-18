FLAG=-std=c++11

demo:demo.cpp
	g++ $^ -o $@ $(FLAG) -I /home/hedenghui/search/cppjieba/include -I /home/hedenghui/search/cppjieba/deps

.PHONY:clean
clean:
	rm demo
