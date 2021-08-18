#include"searcher.h"
#include<fstream>
#include<string>
#include<algorithm>
#include<boost/algorithm/string/case_conv.hpp>
#include"/home/hedenghui/search/common/util.hpp"
#include<jsoncpp/json/json.h>
namespace searcher{
    const char* const DICT_PATH = "/home/hedenghui/search/cppjieba/dict/jieba.dict.utf8";
    const char* const HMM_PATH = "/home/hedenghui/search/cppjieba/dict/hmm_model.utf8";
    const char* const USER_DICT_PATH = "/home/hedenghui/search/cppjieba/dict/user.dict.utf8";
    const char* const IDF_PATH = "/home/hedenghui/search/cppjieba/dict/idf.utf8";
    const char* const STOP_WORD_PATH = "/home/hedenghui/search/cppjieba/dict/stop_words.utf8";

    Index::Index():jieba(DICT_PATH,HMM_PATH,USER_DICT_PATH,IDF_PATH,STOP_WORD_PATH){}
    
    //构建正排(对'\3'进行切分)
    DocInfo* searcher::Index::BuildForward(const string &line){
        //1.切分
        vector<string> tokens;
        common::Utill::Split(line,"\3",&tokens);
        if(tokens.size()!=3){
            return nullptr;
        }
        //2.填充
        DocInfo doc_info;
        doc_info.doc_id=forward_index.size();
        doc_info.title=tokens[0];
        doc_info.url=tokens[1];
        doc_info.content=tokens[2];
        forward_index.push_back(std::move(doc_info));//右值引用
        //3.返回结果
        //return &doc_info;临时变量,野指针问题
        return &forward_index.back();
    }
    //构建倒排
    //1.分词需要考虑标题和正文
    //2.构造Weight对象的时候考虑权重字段如何计算
    //权重=标题出现的次数*10+正文出现的次数
    void searcher::Index::BuildInverted(const DocInfo &doc_info){
        struct WordCnt
        {
            int title_cnt;
            int content_cnt;
            WordCnt()
                :title_cnt(0)
                ,content_cnt(0)
            {}
        };
        unordered_map<string,WordCnt> word_cnt_map;
        //1.针对标题进行分词
        vector<string> title_token;
        CutWord(doc_info.title,&title_token);
        //2.遍历分词结果,统计次数(大小写的问题?)
        //统计之前把所有单词转换为小写
        for(string word:title_token){
            boost::to_lower(word);
            word_cnt_map[word].title_cnt++;
        }

        //3.针对正文进行分词
        vector<string> content_token;
        CutWord(doc_info.content,&content_token);
        //4.遍历分词结果,统计次数
        for(string word:content_token){
            boost::to_lower(word);
            word_cnt_map[word].content_cnt++;
        }

        //5.根据统计结果整合出Weight对象,并把结果更新到倒排索引中即可
        for(const auto& word_pair:word_cnt_map){
            Weight weight;
            weight.doc_id=doc_info.doc_id;
            weight.weight=10*word_pair.second.title_cnt+word_pair.second.content_cnt;
            weight.word=word_pair.first;
            InvertedList& inverted_list=inverted_index[word_pair.first];
            //typedef vector<Weight> InvertedList;//倒排拉链
            inverted_list.push_back(weight);
        }
    }
    //根据id得到DocInfo
    const DocInfo* searcher::Index::GetDocInfo(int64_t doc_id){
        if(doc_id<0||doc_id>=forward_index.size()){
            return nullptr;
        }
        return &forward_index[doc_id];
    }
    //根据string 得到InvertedList
    const InvertedList* searcher::Index::GetInvertedList(const string &key){
        auto it=inverted_index.find(key);
        if(it==inverted_index.end()){
            return nullptr;
        }
        return &it->second;
    }
    //构建索引
    bool searcher::Index::Build(const string& input_path){
        std::cout<<"开始构建~~"<<std::endl;
        //1.按行读取输入文件内容raw_input
        std::ifstream file(input_path.c_str());
        if(!file.is_open()){
            std::cout<<"raw_input open error"<<std::endl;
            return false;
        }
        string line;
        while(std::getline(file,line)){
            //2.针对当前行,解析成DocInfo对象,并构造正排索引
            DocInfo *doc_info=BuildForward(line);
            if(doc_info==nullptr){
                std::cout<<"构建正排失败"<<std::endl;
                continue;
            }
            //3.根据DocInfo构建倒排索引
            BuildInverted(*doc_info);
            std::cout<<doc_info->doc_id<<std::endl;
        }
        file.close();
        std::cout<<"构建结束~~"<<std::endl;
        return true;
    }
    void Index::CutWord(const string& input,vector<string>*output){
        jieba.CutForSearch(input,*output);
    }


    //------------------------
    //searcher模块的代码
    bool searcher::Searcher::Init(const string& input_path){
        return index->Build(input_path);
    }
    //把查询词进行搜索,得到搜索结果
    bool searcher::Searcher::Search(const string& query,string* output){
        //1.分词(针对查询词进行分词)
        vector<string> tokens;
        index->CutWord(query,&tokens);
        //2.查倒排
        vector<Weight> all_token_result;
        for(string word:tokens){
            //忽略大小写,统一转换为小写
            boost::to_lower(word);
            auto* inverted_list=index->GetInvertedList(word);
            if(inverted_list==nullptr){
                continue;
            }
            all_token_result.insert(all_token_result.end(),inverted_list->begin(),inverted_list->end());


        }
        //3.排序:对查到的倒排进行排序(降序排序)
        std::sort(all_token_result.begin(),all_token_result.end(),[](const Weight& w1,const Weight& w2){return w1.weight>w2.weight;});
        //4.包装:去查正排,再把doc_info中内容构造成预期的格式(Json格式)
        Json::Value results;//包含若干个搜索结果,相当于json数组
        for(const auto& weight:all_token_result){
            const DocInfo* doc_info=index->GetDocInfo(weight.doc_id);
            Json::Value result;
            result["title"]=doc_info->title;
            result["url"]=doc_info->url;
            result["desc"]=GenerateDesc(doc_info->content,weight.word);

            results.append(result);

        }
        //把resultsJson序列化字符串
        Json::FastWriter writer;
        *output=writer.write(results);
        return true;
    }
    string searcher::Searcher::GenerateDesc(const string& content,const string& word){
        //1.根据正文,找到word出现的位置
        size_t first_pos=content.find(word);
        size_t desc_beg=0;
        if(first_pos==string::npos){
            //该词在正文中不存在(例如在标题中有,正文中没有)
            //直接从0开始
            if(content.size()<160){
                return content;
            }
            string desc=content.substr(0,160);
            desc[desc.size()-1]='.';
            desc[desc.size()-2]='.';
            desc[desc.size()-3]='.';
            return desc;
        }
        //2.以该位置为中心.往前找60个字节,作为描述的起始位置
        //注意边界问题:前面不够60个字节,从0开始
        //后面不够160个字节,可以使用...省略号
        desc_beg= first_pos<60?0:first_pos-60;
        //往后找160个字节,作为整个描述
        if(desc_beg+160>=content.size()){
            return content.substr(desc_beg);
        }

        string desc=content.substr(desc_beg,160);
        desc[desc.size()-1]='.';
        desc[desc.size()-2]='.';
        desc[desc.size()-3]='.';
        return desc;
    }
}