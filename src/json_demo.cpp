/*
 * @Author: guowei.hou guowei.hou@emdoor.com
 * @Date: 2024-01-09 09:42:13
 * @LastEditors: guowei.hou guowei.hou@emdoor.com
 * @LastEditTime: 2024-01-09 09:49:14
 * @FilePath: /Spark3.0_Linux_SDK_v1.1/Spark3.0_Linux_SDK_v1.1/src/json_demo.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include <iostream>
#include "json.hpp"

using json = nlohmann::json;
using namespace std;

int main() {
    // 创建一个JSON数组
    json jsonArray;

    // 添加用户和助手的对话历史数据
    jsonArray.push_back({{"role", "user"}, {"content", "上海有什么景点？"}});
    jsonArray.push_back({{"role", "assistant"}, {"content", "上海有很多著名的景点，其中排名前十的是：\n\n1. 上海迪士尼乐园\n2. 上海静安寺\n3. 南翔古镇"}});
    jsonArray.push_back({{"role", "user"}, {"content", "那帮我安排一份旅游计划吧。"}});

    // 将JSON数组转换为字符串
    string jsonString = jsonArray.dump();

    // 输出构建的JSON字符串
    cout << jsonString << endl;

    return 0;
}
