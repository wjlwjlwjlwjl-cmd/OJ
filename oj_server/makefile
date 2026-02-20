oj:oj_server.cc ./mysql/problem-odb.cxx
	g++ $^ -o $@ -lpthread -ljsoncpp -lctemplate -lfmt -lspdlog -lodb -lodb-boost -lodb-mysql -lgflags -g
.PHONY:clean
clean:
	rm -rf oj; make
