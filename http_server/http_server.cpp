#include<iostream>
#include<vector>
#include<string>
#include<unordered_map>
#include"httplib.h"
#include"../searcher/searcher.h"
using std::cout;
using std::endl;
using std::string;
using std::unordered_map;
using std::vector;

int main(){
    using namespace httplib;
    Server server;
    server.set_base_dir("./www","/");
    //创建Searcher对象
    searcher::Searcher searcher;
    bool ret=searcher.Init("/home/hedenghui/search/data/tmp/raw_input");
    if(!ret){
        std::cout<<"Searcher初始化失败"<<std::endl;
        return 1;
    }
    //创建server对象
    server.Get("/searcher",[&searcher](const Request& req,Response& resp){
        if(!req.has_param("query")){
            resp.set_content("您发的请求参数错误","text/plain; charset=utf-8");
            return;
        }
        string query=req.get_param_value("query");
        string results;
        searcher.Search(query,&results);
        resp.set_content(results,"application/json; charset=utf-8");
        
    });
    //启动服务器
    std::cout<<"jinru"<<std::endl;
    server.listen("172.24.3.182",9000);
    return 0;
}
