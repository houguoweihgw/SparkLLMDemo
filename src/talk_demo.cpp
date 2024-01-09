#include "../include/sparkchain.h"
#include <iostream>
#include <string>
#include <atomic>
#include <unistd.h>
#include <regex>
#include "json.hpp"

#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define RED "\033[31m"
#define RESET "\033[0m"

using json = nlohmann::json;
using namespace SparkChain;
using namespace std;

// async status tag
static atomic_bool finish(false);

class SparkCallbacks : public LLMCallbacks
{
    void onLLMResult(LLMResult *result, void *usrContext)
    {
        int status = result->getStatus();
        printf(GREEN "%d:%s:%s:%s \n" RESET, status, result->getRole(), result->getContent(), usrContext);
        final_result += string(result->getContent());
        if (status == 2)
        {
            printf(GREEN "tokens:%d + %d = %d\n" RESET, result->getCompletionTokens(), result->getPromptTokens(), result->getTotalTokens());
            finish = true;
        }
    }

    void onLLMEvent(LLMEvent *event, void *usrContext)
    {
        printf(YELLOW "onLLMEventCB\n  eventID:%d eventMsg:%s\n" RESET, event->getEventID(), event->getEventMsg());
    }

    void onLLMError(LLMError *error, void *usrContext)
    {
        printf(RED "onLLMErrorCB\n errCode:%d errMsg:%s \n" RESET, error->getErrCode(), error->getErrMsg());
        finish = true;
    }

public:
    string final_result = "";
};

int initSDK()
{
    // 全局初始化
    SparkChainConfig *config = SparkChainConfig::builder();
    config->appID("XXX")                            // 你的appid
        ->apiKey("XXX")     // 你的apikey
        ->apiSecret("XXX"); // 你的apisecret
    // ->logLevel(0)
    // ->logPath("./aikit.log");
    int ret = SparkChain::init(config);
    printf(RED "\ninit SparkChain result:%d \n" RESET, ret);
    return ret;
}

string asyncLLMTest(LLM *asyncllm, SparkCallbacks *cbs, const char *input, char *usrContext)
{
    // 异步请求
    int i = 0;
    finish = false;
    int ret = asyncllm->arun(input, usrContext);
    if (ret != 0)
    {
        printf(RED "\narun failed: %d\n\n" RESET, ret);
        finish = true;
    }

    int times = 0;
    while (!finish)
    { // 等待结果返回退出
        sleep(1);
        if (times++ > 10) // 等待十秒如果没有最终结果返回退出
            break;
    }

    // 打印结果
    string result = cbs->final_result;
    cbs->final_result.clear();

    return result;
}

void uninitSDK()
{
    // 全局逆初始化
    SparkChain::unInit();
}

int main(int argc, char const *argv[])
{
    cout << "\n######### llm Demo #########" << endl;
    // 全局初始化
    int ret = initSDK();
    if (ret != 0)
    {
        cout << "initSDK failed:" << ret << endl;
        return -1;
    }

    json jsonArray;

    // 配置大模型参数
    LLMConfig *llmConfig = LLMConfig::builder();
    llmConfig->domain("generalv3");

    Memory *window_memory = Memory::WindowMemory(5);
    LLM *asyncllm = LLM::create(llmConfig, window_memory);

    // Memory* token_memory = Memory::TokenMemory(500);
    // LLM *asyncllm = LLM::create(llmConfig,token_memory);

    if (asyncllm == nullptr)
    {
        printf(RED "\nLLMTest fail, please setLLMConfig before" RESET);
        return 0;
    }
    // 注册监听回调
    SparkCallbacks *cbs = new SparkCallbacks();
    asyncllm->registerLLMCallbacks(cbs);

    // 对话
    while (true)
    {
        // 输入问题
        const int bufferSize = 100;
        char question[bufferSize];
        fputs("请问(quit结束): ", stdout);
        fgets(question, 100, stdin);
        // 将 char 数组转换为 std::string 对象
        std::string questionString(question);

        // 如果输入quit则退出循环
        if (questionString == "quit\n" || questionString == "quit\r\n" || questionString == "quit")
            break;

        // 添加问题到JSON数组
        jsonArray.push_back({{"role", "user"}, {"content", questionString}});

        // 将JSON数组转换为字符串
        string jsonString = jsonArray.dump();
        const char *input = jsonString.c_str();
        char *usrContext = "myContext";

         // 异步调用
        string ans = asyncLLMTest(asyncllm, cbs, input, usrContext);

        // 添加用户和助手的对话历史数据
        jsonArray.push_back({{"role", "assistant"}, {"content", ans}});
    }

    // 垃圾回收
    if (asyncllm != nullptr)
    {
        LLM::destroy(asyncllm);
    }
    if (cbs != nullptr)
        delete cbs;

    // 退出
    uninitSDK();
}
