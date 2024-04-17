#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "map.h"

// 初始化参数结构体
void initapiRequest(struct apiRequest *request) {
    request->count = 0;
    request->api_type = NULL;
}

// 解析URL参数并保存到参数结构体中
void parseQueryString(char *queryString, struct apiRequest *request) {
    char *param, *value;
    char *query_start = strchr(queryString, '?'); // 查找查询字符串的起始位置

    if (query_start == NULL && queryString != NULL){
      query_start = queryString;
      request->api_type = queryString + 5;
      return;
    }

    if (query_start != NULL) {
        *query_start = '\0'; // 将路径截断，以便单独处理路径和查询字符串
        query_start++; // 指向查询字符串的开头

        request->api_type = queryString + 5;
    }

    if (query_start != NULL){
    char *queryCopy = strdup(query_start); // 复制查询字符串，以便修改它
    // 使用strtok函数分割查询字符串
    param = strtok(queryCopy, "&");
    while (param != NULL && request->count < MAX_PARAMS) {
        value = strchr(param, '='); // 查找参数值的等号
        if (value != NULL) {
            *value++ = '\0'; // 在等号处分割参数和值
            strcpy(request->params[request->count].param, param);
            strcpy(request->params[request->count].value, value);
            request->count++;
        }
        param = strtok(NULL, "&"); // 继续解析下一个参数
    }

    free(queryCopy); // 释放复制的内存
    }
}

void parseParam(char *ParamString, struct apiRequest *request) {
    char *param, *value;
    param = strtok(ParamString, "&");
    while (param != NULL && request->count < MAX_PARAMS) {
        value = strchr(param, '='); // 查找参数值的等号
        if (value != NULL) {
            *value++ = '\0'; // 在等号处分割参数和值
            strcpy(request->params[request->count].param, param);
            strcpy(request->params[request->count].value, value);
            request->count++;
        }
        param = strtok(NULL, "&"); // 继续解析下一个参数
    }
}

// 根据参数名检索参数值
const char* getValue(const char *param, const struct apiRequest *request) {
    for (int i = 0; i < request->count; i++) {
        if (strcmp(request->params[i].param, param) == 0) {
                printf("%s, %d\n", request->params[i].value, i);
            return request->params[i].value;
        }
    }

    printf("return \n");
    return NULL; // 如果找不到参数，则返回NULL
}
