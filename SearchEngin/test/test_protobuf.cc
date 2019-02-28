#include"hello.pb.h"

int main()
{
    //1.序列化
    //按照二进制方式进行序列化
    //优点：效率高（压缩效率高，网络传输和数据存储时资源开销小）
    //缺点：可读性查，不方便调试
    Hello hello;
    hello.set_name("ZHOU");
    hello.set_score(100);
    std::string buf;
    hello.SerializeToString(&buf);
    std::cout << buf <<std::endl;

    //2.反序列化
    Hello hello_result;
    hello_result.ParseFromString(buf);
    std::cout<<hello_result.name()<<" , "<<hello_result.score()<<std::endl;
    return 0;
}
