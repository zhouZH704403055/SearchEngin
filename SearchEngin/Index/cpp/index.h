#pragma once

#include"index.pb.h"
#include<vector>
#include<string>
#include<unordered_map>
#include<base/base.h>
#include<cppjieba/Jieba.hpp>
#include"../../Common/util.hpp"

namespace doc_index
{
    typedef doc_index_proto::DocInfo DocInfo;
    typedef doc_index_proto::KwdInfo KwdInfo;
    typedef doc_index_proto::Weight Weight;
    //正排索引
    typedef std::vector<DocInfo> ForwardIndex;
    //倒排拉链（文档id列表）
    typedef std::vector<Weight> InvertedList;
    //倒排索引，利用unordered_map方便查找
    typedef std::unordered_map<std::string,InvertedList> InvertedIndex;

    struct WordCount
    {
        int title_count = 0;
        int content_count = 0;
        //记录关键词在正文中第一次出现的位置，方便构造描述信息
        int first_pos=-1;
    };

    typedef std::unordered_map<std::string,WordCount> WordCoutMap;
    //包含实现索引的数据结构和索引所需要提供的API接口
    class Index
    {
    public:
        Index();
        static Index* GetInstance()
        {
            if(_inst == nullptr)
            {
                _inst = new Index();
            }
            return _inst;
        }
        //1.制作索引：读取raw_input 文件，分析生成内存中的索引结构
        bool Build(const std::string& input_path);
        //2.保存索引：基于protobuf格式把内存中的索引结构写到文件中
        bool Save(const std::string& output_path);
        //3.加载索引：加载文件中的索引结构到内存中
        bool Load(const std::string& index_path);
        //4.反解索引：将内存中的索引结构按照可读性较好的格式写入文件
        bool Dump(const std::string& forward_dump_path,const std::string& inverted_dump_path);
        //5.查询正排索引：根据文档id，获取文档内容（取vector的下标）
        const DocInfo* GetDocInfo(uint64_t doc_id) const;
        //6.查询倒排索引：根据关键词，获取倒排拉链（unordered_map中寻找映射）
        const InvertedList* GetInvertedList(const std::string& key) const;
    private:
        //私有成员函数
        const DocInfo* BuildForward(const std::string& line);
        void BuildInverted(const DocInfo& doc_info);
        void SortInverted();
        void SplitTitle(const std::string& title, DocInfo* doc_info);
        void SplitContent(const std::string& content, DocInfo* doc_info);
        int CalcWeight(int title_count,int content_count);
        static bool CmpWeight(const Weight& weight_1,const Weight& weight_2);
        bool ConvertToProto(std::string* proto_data);
        bool ConvertFromProto(const std::string& proto_data);

        //私有成员变量
        ForwardIndex forward_index;
        InvertedIndex inverted_index;
        cppjieba::Jieba _jieba;
        common::DictUtil stop_word_dict;

        //单例模式，封死拷贝构造和赋值
        // Index() = default;
        Index(const Index&) = delete;
        Index& operator=(const Index&) = delete;
        //单例模式的唯一对象
        static Index* _inst;
    };
}// end doc_index
