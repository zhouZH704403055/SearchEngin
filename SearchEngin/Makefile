test:test.cpp
	#g++ $^ -o $@ -I ~/Code/SearchEngine/ThirdPartLibForCpp/el7.x86_64/third_part/include -L ~/Code/SearchEngine/ThirdPartLibForCpp/el7.x86_64/third_part/lib -lgflags -lpthread
	g++ $^ -o $@ -I ./third_part/include -L ./third_part/lib -lglog -lpthread
.PHONY:clean
clean:
	rm -f test
