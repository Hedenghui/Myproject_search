#pragma once
#include<iostream>
#include<string>
#include<vector>
#include<unordered_map>
#include<fstream>
#include<boost/algorithm/string.hpp>//boost切分函数
using std::string;
using std::cout;
using std::endl;
using std::unordered_map;
using std::vector;
namespace common{
    class Utill{
    public:
        //负责从指定路径中，读取文件的整体内容，读到output这个string里
        static bool Read(const string& input_path,string* out_put){
            std::ifstream file(input_path.c_str());
            if(!file.is_open()){
                return false;
            }
            //getline读取文件的一行
            //读取失败返回false
            string line;
            while(std::getline(file,line)){
                *out_put+=(line+"\n");
            }
            file.close();
            return true;
        }
        //字符串切分函数
        static void Split(const string& input,const string& delimiter,vector<string>* output){
            boost::split(*output,input,boost::is_any_of(delimiter),boost::token_compress_off);
            
        }
    };
}