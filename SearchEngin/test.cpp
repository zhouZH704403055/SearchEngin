//#include<iostream>
//#include<gflags/gflags.h>

//DEFINE_string(ip,"127.0.0.1","IP地址");
//DEFINE_int32(port,80,"端口号");

//int main(int argc,char* argv[])
//{
//    gflags::ParseCommandLineFlags(&argc,&argv,true);
//    std::cout<<FLAGS_ip<<std::endl;
//    std::cout<<FLAGS_port<<std::endl;
//    return 0;
//}

#include <iostream>
#include <glog/logging.h>

int main(int argc, char* argv[])
{
    (void) argc;//参数未使用的话，编译器会报警告
    google::InitGoogleLogging(argv[0]);
    fLS::FLAGS_log_dir = "./log";
    LOG(INFO)<<"hello info";//括号中代表日志级别
    LOG(WARNING)<<"hello warning";
    LOG(ERROR)<<"hello error";
    //LOG(FATAL)<<"hello fatal";
    return 0;
}
