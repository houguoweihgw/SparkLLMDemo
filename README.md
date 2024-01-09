# 星火大模型demo

## 1.SDK地址
    https://www.xfyun.cn/doc/spark/LinuxSDK.html

## 2.免费购买token
    200万tokens免费领取
    地址：https://xinghuo.xfyun.cn/sparkapi?scr=price

## 3.demo
    /src/demo.cpp 实现了单次对话
    /src/talk_demo.cpp 通过封装历史对话记录实现了多轮对话
## 4.说明
    为了实现多轮对话，需要构建JsonArray字符串传入run方法中。故使用到了nlohmann/json库。地址：https://github.com/nlohmann/json
