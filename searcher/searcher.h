#pragma once
#include "cppjieba/Jieba.hpp"
#include "limonp/Logging.hpp"
#include<iostream>
#include<string>
#include<unordered_map>
#include<vector>
#include<stdint.h>
using std::string;
using std::cout;
using std::endl;
using std::unordered_map;
using std::vector;

//索引模块的内容
namespace searcher{
    //这个结构是正排索引的基础
    struct DocInfo{
        int64_t doc_id;
        string title;
        string url;
        string content;
    };
    //倒排索引是给定词,映射到包含该词的文档id列表
    struct Weight{
        //在哪个文档出现
        int64_t doc_id;
        //对应权重是多少
        int weight;
        //词是啥 
        string word;
    };
    typedef vector<Weight> InvertedList;//倒排拉链

    class Index{
    private:
        //索引结构
        //正排索引
        vector<DocInfo> forward_index;
        //倒排索引
        unordered_map<string,InvertedList> inverted_index;



        DocInfo* BuildForward(const string &line);//建立正排
        void BuildInverted(const DocInfo &doc_info);//建立倒排
        cppjieba::Jieba jieba;
    public:
        Index();
        //提供一些对外调用的函数
        //1.查正排
        const DocInfo* GetDocInfo(int64_t doc_id);
        //2.查倒排
        const InvertedList* GetInvertedList(const string &key);
        //3.构建索引
        bool Build(const string& input_path);
        //分词
        void CutWord(const string& input,vector<string>* output);

    };


    //搜索模块
    class Searcher{
    private:
        Index* index;
        string GenerateDesc(const string& content,const string& word);
    public:
        Searcher()
            :index(new Index())
        {}
        bool Init(const string& input_path);
        bool Search(const string& query,string* results);

    };
}

