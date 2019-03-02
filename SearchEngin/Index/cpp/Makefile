PROTOC=~/Code/SearchEngine/SearchEngin/third_part/bin/protoc

#FLAG=-std=c++11 -g -I ~/Code/SearchEngine/SearchEngin/third_part/include -L ~/Code/SearchEngine/SearchEngin/third_part/lib -lprotobuf -lgflags -lglog -lpthread

FLAG=-std=c++11 -g -I ~/Code/SearchEngine/SearchEngin/third_part/include -L ~/Code/SearchEngine/SearchEngin/third_part/lib -lglog -lprotobuf -lpthread -lgflags

.PHONY:all
all:index_builder index_dump

index_builder:index_build.cc libindex.a
	g++ $^ -o $@ $(FLAG)
	cp -f $@ ../bin/

index_dump:index_dump.cc libindex.a
	g++ $^ -o $@ $(FLAG)
	cp -f $@ ../bin/

libindex.a:index.pb.cc index.cc
	g++ -c index.pb.cc -o index.pb.o $(FLAG)
	g++ -c index.cc -o index.o $(FLAG)
	ar -rc libindex.a index.o index.pb.o

index.pb.cc:index.proto
	$(PROTOC) index.proto --cpp_out=.

.PHONY:clean
clean:
	rm -f index.o libindex.a index.pb.* index_builder index_dump
