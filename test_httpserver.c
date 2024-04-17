#include <stdio.h>
#include <string.h>
#include <curl/curl.h>
#include <CUnit/Basic.h>

static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    ((char *)userp)[size * nmemb] = '\0'; // 确保数据字符串结束
    strcat((char *)userp, (char *)contents);
    return size * nmemb;
}


/**
 * 函数名: test_http_get
 * 功能: 测试http_get方法
 * 参数:
 *   - int a: 第一个整数
 *   - int b: 第二个整数
 * 返回值: 返回两个整数的和
 * 示例:
 *   int result = calculateSum(5, 3); // result将会是8
 */
void test_http_get(void) {
    CURL *curl;
    CURLcode res;
    char response[4096] = ""; // 创建字符数组缓存接收到的http消息

    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "localhost:8080"); // 指定测试url
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback); // 指定回调函数
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)response); // 将返回数据存入response
        res = curl_easy_perform(curl); // 发送curl请求
        CU_ASSERT(res == CURLE_OK); // 使用宏断言请求成功执行
        CU_ASSERT_STRING_NOT_EQUAL(response, ""); // 判断数组是否非空
        curl_easy_cleanup(curl); // 清理CURL资源
    }
}

void test_http_post(void) {
    CURL *curl;
    CURLcode res;
    char response[4096] = ""; 
    char data[] = "name=test&project=cunit";

    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "http://httpbin.org/post");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)response);
        res = curl_easy_perform(curl);
        CU_ASSERT(res == CURLE_OK);
        CU_ASSERT_STRING_NOT_EQUAL(response, "");
        curl_easy_cleanup(curl);
    }
}

int main() {
    CU_pSuite pSuite = NULL;

    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    pSuite = CU_add_suite("HTTP Server Test Suite", NULL, NULL);
    if (NULL == pSuite) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if (NULL == CU_add_test(pSuite, "test of HTTP GET", test_http_get)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    // if ((NULL == CU_add_test(pSuite, "test of HTTP GET", test_http_get)) ||
    //     (NULL == CU_add_test(pSuite, "test of HTTP POST", test_http_post))) {
    //     CU_cleanup_registry();
    //     return CU_get_error();
    // }

    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    CU_cleanup_registry();
    return 0;
}
