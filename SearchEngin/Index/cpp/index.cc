#include"index.h"
#include<fstream>

DEFINE_string(dict_path,"../../third_part/data/jieba_dict/jieba.dict.utf8","词典路径");
DEFINE_string(hmm_path,"../../third_part/data/jieba_dict/hmm_model.utf8","hmm词典路径");
DEFINE_string(user_dict_path,"../../third_part/data/jieba_dict/user.dict.utf8","用户自定义词典路径");
DEFINE_string(idf_path,"../../third_part/data/jieba_dict/idf.utf8","idf词典路径");
DEFINE_string(stop_word_path,"../../third_part/data/jieba_dict/stop_words.utf8","暂停词词典路径");

namespace doc_index
{
    Index* Index::_inst = nullptr;

    Index::Index():_jieba(FLAGS_dict_path,fLS::FLAGS_hmm_path,fLS::FLAGS_user_dict_path,fLS::FLAGS_idf_path,fLS::FLAGS_stop_word_path)
    {
        CHECK(stop_word_dict.Load(fLS::FLAGS_stop_word_path));
    }

    bool Index::Build(const std::string& input_path)
    {
        LOG(INFO) << "Index Build";
        //1.打开文件并且按行读取文件，针对每行进行解析
        //打开文件raw_input，文件中是预处理后的行文本文件，每行对应一个HTML
        //每行是一个三元组（jump_url,title,content）
        std::ifstream file(input_path.c_str());
        CHECK(file.is_open());
        std::string line;
        while(std::getline(file,line))
        {
            //2.针对读取的当前行，更新正排索引
            //构造一个DocInfo对象，并把这个对象push_back到forward_index中
            const DocInfo* doc_info = BuildForward(line);
            CHECK(doc_info != nullptr);
            //3.跟读当前的DocInfo更新倒排索引
            BuildInverted(*doc_info);
        }
        //4.更新完HTML所有的正排索引和倒排索引后
        //将倒排索引按照权重进行排序
        SortInverted();
        file.close();
        LOG(INFO) << "Index Build Done";
        return true;
    }

    const DocInfo* Index::BuildForward(const std::string& line)
    {
        //1.切分line，分出三元组
        std::vector<std::string> tokens;
        common::StringUtil::Split(line,&tokens,"\3");
        if(tokens.size() != 3)
        {
            LOG(FATAL) << "line split not 3 tokens! tokens.size()="<<tokens.size();
            return nullptr;
        }
        //2.构造一个DocInfo结构，将切分的结果保存到DocInfo(除了分词结果)
        DocInfo doc_info;
        doc_info.set_doc_id(forward_index.size());
        doc_info.set_title(tokens[1]);
        doc_info.set_content(tokens[2]);
        doc_info.set_jump_url(tokens[0]);
        doc_info.set_show_url(tokens[0]);
        //3.对标题和正文进行分词，将分词结果保存到DocInfo中
        SplitTitle(tokens[1],&doc_info);
        SplitContent(tokens[2],&doc_info);
        //4.将构造出的DocInfo插入到正排索引中
        forward_index.push_back(doc_info);
        return &forward_index.back();
    }

    void Index::SplitTitle(const std::string& title,DocInfo* doc_info)
    {
        std::vector<cppjieba::Word> words;
        _jieba.CutForSearch(title,words);
        //words中的分词结果中包含一个offset（当前次在文档中起始位置的下标）
        //我们所需要的是一个前闭后开区间
        if(words.size()<=1)
        {
            LOG(FATAL)<<"SplitTitle Error!!";
            return;
        }
        for(size_t i=0;i<words.size();++i)
        {
            auto* token = doc_info->add_title_token();
            token->set_begin(words[i].offset);
            if((i+1)<words.size())
            {
                token->set_end(words[i+1].offset);
            }
            else
            {
                token->set_end(title.size());
            }
        }
    }

    void Index::SplitContent(const std::string& content,DocInfo* doc_info)
    {
        std::vector<cppjieba::Word> words;
        _jieba.CutForSearch(content,words);
        if(words.size()<=1)
        {
            LOG(FATAL)<<"SplitContent Error!!";
            return;
        }
        for(size_t i=0;i<words.size();++i)
        {
            auto* token = doc_info->add_content_token();
            token->set_begin(words[i].offset);
            if((i+1)<words.size())
            {
                token->set_end(words[i+1].offset);
            }
            else
            {
                token->set_end(content.size());
            }
        }
    }

    void Index::BuildInverted(const DocInfo& doc_info)
    {
        WordCoutMap word_count_map;
        //1.统计title中每个关键词出现的次数
        for(int i=0;i<doc_info.title_token_size();++i)
        {
            const auto& token=doc_info.title_token(i);
            std::string word=doc_info.title().substr(token.begin(),token.end()-token.begin());
            boost::to_lower(word);
            //去除暂停词
            if(stop_word_dict.Find(word))
            {
                continue;
            }
            ++word_count_map[word].title_count;
        }
        //2.统计content中每个关键词出现的次数
        //统计完次数后，得到哈希表，key为关键词（分词结果）value为一个结构体
        for(int i=0;i<doc_info.content_token_size();++i)
        {
            const auto& token=doc_info.content_token(i);
            std::string word=doc_info.content().substr(token.begin(),token.end()-token.begin());
            boost::to_lower(word);
            if(stop_word_dict.Find(word))
            {
                continue;
            }
            ++word_count_map[word].content_count;
            //记录关键词在正文中第一次出现的位置
            if(word_count_map[word].content_count==1)
            {
                word_count_map[word].first_pos=token.begin();
            }
        }
        //结构体中保存关键词在title中出现的次数和在content中出现的次数
        //3.根据出现次数的统计结果，保存到倒排索引中
        //遍历哈希表，利用key在倒排索引中查询是否已经存在？
        //如果不存在则插入倒排索引中
        //如果不存在则根据改造好的Weight结构添加到倒排索引中对应的倒排拉链中
        for(const auto& word_pair : word_count_map)
        {
            Weight weight;
            weight.set_doc_id(doc_info.doc_id());
            weight.set_weight(CalcWeight(word_pair.second.title_count,word_pair.second.content_count));
            weight.set_title_count(word_pair.second.title_count);
            weight.set_content_count(word_pair.second.content_count);
            weight.set_first_pos(word_pair.second.first_pos);
            //先获取到关键词对应的倒排拉链
            InvertedList& inverted_list=inverted_index[word_pair.first];
            inverted_list.push_back(weight);
        }
    }

    int Index::CalcWeight(int title_count,int content_count)
    {
        return 10*title_count + content_count;
    }

    void Index::SortInverted()
    {
        for(auto& inverted_pair : inverted_index)
        {
            InvertedList& inverted_list = inverted_pair.second;
            std::sort(inverted_list.begin(),inverted_list.end(),CmpWeight);
        }
    }

    bool Index::CmpWeight(const Weight& weight_1,const Weight& weight_2)
    {
        //降序排序
        return weight_1.weight() > weight_2.weight();
    }

    bool Index::Save(const std::string& output_path)
    {
        //1.将内存中的结构构造成对应的protobuf结构
        std::string proto_data;
        CHECK(ConvertToProto(&proto_data));
        //2.基于protobuf结构进行序列化，将序列化的结果写到磁盘上
        CHECK(common::FileUtil::Write(output_path,proto_data));
        return true;
    }

    bool Index::ConvertToProto(std::string* proto_data)
    {
        doc_index_proto::Index index;
        //1.将正排索引构造到protobuf中
        for(const auto& doc_info : forward_index)
        {
            auto* proto_doc_info = index.add_forward_index();
            *proto_doc_info = doc_info;
        }
        //2.将倒排索引构造到protobuf中
        for(const auto& inverted_pair : inverted_index)
        {
            auto* kwd_info = index.add_inverted_index();
            kwd_info->set_key(inverted_pair.first);
            for(const auto& weight : inverted_pair.second)
            {
                auto proto_weight = kwd_info->add_doc_list();
                *proto_weight = weight;
            }
        }
        //3.对protobuf对象进行序列化
        index.SerializeToString(proto_data);
        return true;
    }

    bool Index::Load(const std::string& index_path)
    {
        //1.读取文件，基于protobuf进行反序列化
        std::string proto_data;
        CHECK(common::FileUtil::Read(index_path,&proto_data));
        //2.将protobuf结构转换回内存结构
        CHECK(ConvertFromProto(proto_data));
        return true;
    }

    bool Index::ConvertFromProto(const std::string& proto_data)
    {
        doc_index_proto::Index index;
        index.ParseFromString(proto_data);
        //1.将正排索引转换到内存中
        for(int i=0;i<index.forward_index_size();++i)
        {
            const auto& doc_info=index.forward_index(i);
            forward_index.push_back(doc_info);
        }
        //2.将倒排索引转换到内存中
        for(int i=0;i<index.inverted_index_size();++i)
        {
            const auto& kwd_info=index.inverted_index(i);
            InvertedList& inverted_list=inverted_index[kwd_info.key()];
            for(int j=0;j<kwd_info.doc_list_size();++j)
            {
                const auto& weight=kwd_info.doc_list(j);
                inverted_list.push_back(weight);
            }
        }
        return true;
    }

    //辅助测试，遍历内存中的索引结构，按照方便查看的格式打印到文件中
    bool Index::Dump(const std::string& forward_dump_path,const std::string& inverted_dump_path)
    {
        //处理正排索引
        std::ofstream forward_dump_file(forward_dump_path.c_str());
        CHECK(forward_dump_file.is_open());
        for(const auto& doc_info : forward_index)
        {
            forward_dump_file<<doc_info.Utf8DebugString()<<"\n====================================\n";
        }
        forward_dump_file.close();
        //处理倒排索引
        std::ofstream inverted_dump_file(inverted_dump_path.c_str());
        CHECK(inverted_dump_file.is_open());
        for(const auto& inverted_pair : inverted_index)
        {
            inverted_dump_file<<inverted_pair.first<<"\n";
            for(const auto& weight : inverted_pair.second)
            {
                inverted_dump_file<<weight.Utf8DebugString();
            }
            inverted_dump_file<<"\n=====================================\n";
        }
        inverted_dump_file.close();
        return true;
    }

    const DocInfo* Index::GetDocInfo(uint64_t doc_id) const
    {
        if(doc_id >= forward_index.size())
        {
            return nullptr;
        }
        return &forward_index[doc_id];
    }

    const InvertedList* Index::GetInvertedList(const std::string& key) const
    {
        auto it = inverted_index.find(key);
        if(it==inverted_index.end())
        {
            return nullptr;
        }
        return &(it->second);
    }

}//end doc_index

