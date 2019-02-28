#pragma once

#include"index.pb.h"
#include<vector>
#include<string>
#include<unordered_map>

namespace doc_index
{
    typedef doc_index_proto::DocInfo DocInfo;
    typedef doc_index_proto::KwdInfo KwdInfo;
    typedef doc_index_proto::Weight Weight;
    //正排索引
    typedef std::vector<Docinfo> ForwardIndex;
    //倒排拉链（文档id列表）
    typedef std::vector<Weight> InvertedList;
    //倒排索引，利用unordered_map方便查找
    typedef unordered_map<std::string,InvertedList> InvertedIndex;

    //包含实现索引的数据结构和索引所需要提供的API接口
    class Index
    {
    public:
        //1.制作索引：读取raw_input 文件，分析生成内存中的索引结构
        bool Build(const std::string& input_path);
        //2.保存索引：基于protobuf格式把内存中的索引结构写到文件中
        bool Save(const std::string& output_path);
        //3.加载索引：加载文件中的索引结构到内存中
        bool Load(const std::string& index_path);
        //4.反解索引：将内存中的索引结构按照可读性较好的格式写入文件
        bool Dump(const std::string& forward_dump_path,const std::string& inverted_dump_path);
        //5.查询正排索引：给定文档id，获取文档内容（取vector的下标）
        consts DocInfo* GetDocInfo(uint64_t doc_id) const;
        //6.查询倒排索引：给定关键词，获取倒排拉链（unordered_map中寻找映射）
        const InvertedList* GetInvertedList(const std::string& key) const;
    private:
        ForwardIndex forward_index;
        InvertedIndex inverted_inex;
    };
}// end doc_index
