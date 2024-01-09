#include "../include/sparkchain.h"
#include <iostream>
#include <string>
#include <atomic>
#include <unistd.h>
#include <regex>

#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define RED "\033[31m"
#define RESET "\033[0m"

using namespace SparkChain;
using namespace std;

// async status tag
static atomic_bool finish(false);
// result cache
string final_result = "";

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
};

int initSDK()
{
    // 全局初始化
    SparkChainConfig *config = SparkChainConfig::builder();
    config->appID("ee858089")                            // 你的appid
        ->apiKey("5b2e7188403a9f1552e49ab78ff6464b")     // 你的apikey
        ->apiSecret("YzNlOWNkNWY0ZDk1M2Q0ZjhlZDMzNmI4"); // 你的apisecret
    // ->logLevel(0)
    // ->logPath("./aikit.log");
    int ret = SparkChain::init(config);
    printf(RED "\ninit SparkChain result:%d \n" RESET, ret);
    return ret;
}

void syncLLMTest()
{
    cout << "\n######### 同步调用 #########" << endl;
    // 配置大模型参数
    LLMConfig *llmConfig = LLMConfig::builder();
    llmConfig->domain("generalv3");
    Memory *window_memory = Memory::WindowMemory(5);
    LLM *syncllm = LLM::create(llmConfig, window_memory);

    // Memory* token_memory = Memory::TokenMemory(500);
    // LLM *syncllm = LLM::create(llmConfig,token_memory);

    char *input = "你好用英语怎么说？";
    int i = 0;
    while (i++ < 2)
    {
        // 同步请求
        LLMSyncOutput *result = syncllm->run(input);
        if (result->getErrCode() != 0)
        {
            printf(RED "\nsyncOutput: %d:%s\n\n" RESET, result->getErrCode(), result->getErrMsg());
            continue;
        }
        else
        {
            printf(GREEN "\nsyncOutput: %s:%s\n" RESET, result->getRole(), result->getContent());
        }
        input = "那日语呢？";
    }

    // 垃圾回收
    if (syncllm != nullptr)
    {
        LLM::destroy(syncllm);
    }
}

void asyncLLMTest(char *input, char *usrContext)
{
    // cout << "\n######### 异步调用 #########" << endl;
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
        return;
    }
    // 注册监听回调
    SparkCallbacks *cbs = new SparkCallbacks();
    asyncllm->registerLLMCallbacks(cbs);

    // 异步请求
    int i = 0;
    finish = false;
    int ret = asyncllm->arun(input, usrContext);
    if (ret != 0)
    {
        printf(RED "\narun failed: %d\n\n" RESET, ret);
        finish = true;
        // continue;
    }

    int times = 0;
    while (!finish)
    { // 等待结果返回退出
        sleep(1);
        if (times++ > 10) // 等待十秒如果没有最终结果返回退出
            break;
    }

    // 垃圾回收
    if (asyncllm != nullptr)
    {
        LLM::destroy(asyncllm);
    }
    if (cbs != nullptr)
        delete cbs;
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

    // 同步调用和异步调用选择一个执行
    while (true)
    {
        // syncLLMTest(); // 同步调用
        const int bufferSize = 100;
        char input[bufferSize];
        fputs("请问：",stdout);
        fgets(input,100,stdin);
        char *usrContext = "myContext";
        asyncLLMTest(input,usrContext); // 异步调用
    }

    // 退出
    uninitSDK();
}
