//实现预处理模块
//读取分析boost文档的.html文件
//解析标题，url，正文
//输出一个行文本文件
#include<iostream>
#include<string>
#include<unordered_map>
#include<vector>
#include<boost/filesystem/path.hpp>
#include<boost/filesystem/operations.hpp>
#include"../common/util.hpp"
using std::cout;
using std::endl;
using std::string;
using std::unordered_map;
using std::vector;

//定义一些变量和结构体
//这个路径下获取boost的html
string g_input_path="/home/hedenghui/search/data/input/boost_1_76_0/doc/html";
//这个路径下保存输出结果
string g_output_path="/home/hedenghui/search/data/tmp/raw_input";
//创建一个重要的结构体，表示一个文档
struct DocInfo{
    string title;
    string url;
    string content;
};
bool EnumFile(const string& g_input_path,vector<string> *file_list){
    //枚举目录使用boost完成(boost中遍历路径有递归的功能)
    //需要包含相关头文件
    namespace fs=boost::filesystem;//定义别名
    fs::path root_path(g_input_path);
    if(!fs::exists(root_path)){
        std::cout<<"当前目录不存在~"<<std::endl;
        return false;
    }
    //递归遍历使用的一个核心类
    fs::recursive_directory_iterator end_iter;
    for(fs::recursive_directory_iterator iter(root_path);iter!=end_iter;++iter){
        //判断是不是一个文件，不是则跳过
        if(!fs::is_regular_file(*iter)){
            continue;
        }
        //判断是不是.html文件，不是则跳过
        if(iter->path().extension()!=".html"){
            continue;
        }
        file_list->push_back(iter->path().string());
    }


    return true;
}
bool ParseTitle(const string& html,string* title){
    size_t begin=html.find("<title>");
    if(begin==string::npos){
        std::cout<<"标题未找到"<<std::endl;
        return false;
    }
    size_t end=html.find("</title>");
    if(end==string::npos){
        std::cout<<"标题未找到"<<std::endl;
        return false;
    }
    begin+=string("<title>").size();
    if(begin>=end){
        std::cout<<"标题位置不合法"<<std::endl;
        return false;
    }
    *title=html.substr(begin,end-begin);
    return true;
}
//本地路径:../data/input/html/thread.html
//在线路径:https://www.boost.org/doc/libs/1_76_0/doc/html/thread.html
bool ParseUrl(const string& file_path,string* url){
    string url_head="https://www.boost.org/doc/libs/1_76_0/doc/html";
    string url_tail=file_path.substr(g_input_path.size());
    *url=url_head+url_tail;
    return true;
}
bool ParseContent(const string& html,string* content){
    bool is_content=true;
    for(auto c:html){
        if(is_content){
            if(c=='<'){
                is_content=false;
            }else{
                //写入结果,行文本格式需要注意'\n'
                if(c=='\n'){
                    c=' ';
                }
                content->push_back(c);
            }
        }else{
            //已经是标签
            if(c=='>'){
                //标签结束
                is_content=true;
            }
        }
    }
    return true;
}
bool ParseFile(const string& file_path,DocInfo* doc_info){
    //1.先读取文件内容
    string html;
    bool ret=common::Utill::Read(file_path,&html);
    if(!ret){
        std::cout<<"解析文件失败~"<<file_path<<std::endl;
        return false;
    }
    ret=ParseTitle(html,&doc_info->title);
    if(!ret){
        std::cout<<"解析标题失败"<<std::endl;
        return false;
    }
    ret=ParseUrl(file_path,&doc_info->url);
    if(!ret){
        std::cout<<"解析url失败"<<std::endl;
        return false;
    }
    ret=ParseContent(html,&doc_info->content);
    if(!ret){
        std::cout<<"解析正文失败"<<std::endl;
        return false;
    }
    return true;
}
void WriteOutput(const DocInfo& doc_info,std::ofstream& ofstream){
    ofstream<<doc_info.title<<"\3"<<doc_info.url<<"\3"<<doc_info.content<<std::endl;
}
//预处理过程的核心流程
//1.把input目录中html路径都枚举出来
//2.根据枚举出来的路依次读取每个文件内容，解析
//3.把解析结果写入到最终的输出文件中
int main(){
    //1.把input目录中html路径都枚举出来
    vector<string> file_list;
    bool ret=EnumFile(g_input_path,&file_list);
    if(!ret){
        std::cout<<"枚举路径失败~"<<std::endl;
        return 1;
    }
    //2.遍历枚举出来的路径，针对每个文件，单独进行处理
    std::ofstream output_file(g_output_path.c_str());
    if(!output_file.is_open()){
        std::cout<<"打开output文件失败"<<std::endl;
        return 1;
    }
    for(const auto& file_path:file_list){
        std::cout<<file_path<<std::endl;
        //先创建一个DocInfo对象
        DocInfo doc_info;
        //通过一个函数来负责这里的解析
        ret=ParseFile(file_path,&doc_info);
        if(!ret){
            std::cout<<"解析文件失败:"<<file_path<<std::endl;
            continue;
        }
        //把解析出来的结果写入到最终的输出文件
        WriteOutput(doc_info,output_file);

    }
    output_file.close();
    return 0;
}