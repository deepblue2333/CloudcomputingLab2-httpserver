#ifndef MAP_H
#define MAP_H

#define MAX_PARAMS 10
#define MAX_PARAM_LENGTH 10
#define MAX_VALUE_LENGTH 20

// 定义键值对结构体
struct KeyValuePair {
    char param[MAX_PARAM_LENGTH];
    char value[MAX_VALUE_LENGTH];
};

// 定义存储api类型和参数的结构体
struct apiRequest {
  char *api_type;
  struct KeyValuePair params[MAX_PARAMS];
  int count;
};

// 初始化参数结构体
void initapiRequest(struct apiRequest *request);

// 解析URL参数并保存到参数结构体中
void parseQueryString(char *queryString, struct apiRequest *request) ;

void parseParam(char *ParamString, struct apiRequest *request);

// 根据参数名检索参数值
const char* getValue(const char *param, const struct apiRequest *request);
#endif
