PROTOC=~/Code/SearchEngine/SearchEngin/third_part/bin/protoc

test_protobuf:test_protobuf.cc hello.pb.cc
	g++ $^ -o $@ -I ~/Code/SearchEngine/SearchEngin/third_part/include -L ~/Code/SearchEngine/SearchEngin/third_part/lib -lprotobuf

hello.pb.cc:hello.proto
	$(PROTOC) hello.proto --cpp_out=.
