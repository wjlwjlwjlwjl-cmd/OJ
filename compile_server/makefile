cp: compile_server.cc
	g++ compile_server.cc -o cs -lspdlog -lfmt -lgflags -ljsoncpp -g
.PHONY:clean
clean:
	rm -rf cp test_code/* tmp/* && make
