#include <arpa/inet.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <unistd.h>

#include <log4c.h>

#include "libhttp.h"
#include "cJSON.h"
#include "map.h"
#include "wq.h"

#define LOG_INFO(message, args...) \
  log4c_category_log(log4c_category_get("httpserver"), LOG4C_PRIORITY_INFO, message, ##args)

#define LOG_DEBUG(message, args...) \
  log4c_category_log(log4c_category_get("httpserver"), LOG4C_PRIORITY_DEBUG, message, ##args)

#define LOG_ERROR(message, args...) \
  log4c_category_log(log4c_category_get("httpserver"), LOG4C_PRIORITY_ERROR, message, ##args)

#define BUFFER_SIZE 2048

wq_t work_queue;  // Only used by poolserver
int num_threads;  // Only used by poolserver  线程数量
int server_address; // Default value: 127.0.0.1 默认服务地址
int server_port;  // Default value: 8000   默认服务端口
char *server_files_directory = "static"; // 服务器工作路径
char *server_proxy_address; // 服务器代理地址
int server_proxy_port; // 代理服务器端口
log4c_category_t* mylog; // 声明日志

int max_file_size = 100; 

// step1 实现http GET方法

/* api */
void list(int fd){
  LOG_INFO("process GET api list");
  http_start_response(fd, 200);
  char *filetype = "application/json";
  http_send_header(fd, "Content-Type", filetype);
  int src_fd = open("./data.json",O_RDONLY);
  char *file_size = (char *) malloc(max_file_size * sizeof(char));
  sprintf(file_size, "%d", get_file_size(src_fd));  
  http_send_header(fd, "Content-Length", file_size); // Change this too
  http_end_headers(fd);
  http_send_file(fd, src_fd);

  close(fd);
  close(src_fd);
  free(file_size);

  return;
}

void check(int fd, struct apiRequest *request){
  LOG_INFO("process GET api check");
  cJSON* cjson_data = NULL;

  // 打开文件
  int src_fd = open("./data.json", O_RDONLY);
  if (src_fd < 0) {
      perror("Error opening file");
      return;
  }

  // 读取文件内容
  char buffer[1024]; // 假设文件内容不超过 1024 字节
  ssize_t bytes_read = read(src_fd, buffer, sizeof(buffer));
  if (bytes_read < 0) {
      perror("Error reading file");
      close(src_fd);
      return;
  }

  // 关闭文件
  close(src_fd);
  

  // 解析 JSON 字符串
  cJSON *json = cJSON_Parse(buffer);
  if (json == NULL) {
      printf("Error parsing JSON: %s\n", cJSON_GetErrorPtr());
      return;
  }

  // 遍历 JSON 数组
  cJSON *item = NULL;
  cJSON_ArrayForEach(item, json) {
    // 提取 id 和 name 字段
    cJSON *id = cJSON_GetObjectItem(item, "id");
    cJSON *name = cJSON_GetObjectItem(item, "name");

    if (strcmp(id->valuestring, request->params[0].value)==0 && strcmp(name->valuestring, request->params[1].value)==0) {
      printf("ID: %s, Name: %s\n", id->valuestring, name->valuestring);
      http_start_response(fd, 200);
      http_end_headers(fd);
      close(fd);
      return;
    }
  }

  http_start_response(fd, 404);
  http_send_header(fd, "Content-Type", "application/json");
  http_end_headers(fd); 
  //返回文件的字节长度
  int src_fd2 = open("404.html",O_RDONLY);
  http_send_file(fd, src_fd2);
  close(src_fd);
  close(fd);

  // 释放 cJSON 结构
  cJSON_Delete(json);

  return;
}

int upload(int fd, struct apiRequest *request){
  LOG_INFO("process POST api upload");

  cJSON* cjson_data = NULL;

  // 打开文件
  int src_fd = open("./data.json", O_RDONLY);
  if (src_fd < 0) {
      perror("Error opening file");
      return;
  }

  // 读取文件内容
  char buffer[1024]; // 假设文件内容不超过 1024 字节
  ssize_t bytes_read = read(src_fd, buffer, sizeof(buffer));
  if (bytes_read < 0) {
      perror("Error reading file");
      close(src_fd);
      return 0;
  }

  // 关闭文件
  close(src_fd);
  
  // 解析 JSON 字符串
  cJSON *json = cJSON_Parse(buffer);
  if (json == NULL) {
      printf("Error parsing JSON: %s\n", cJSON_GetErrorPtr());
      return 0;
  }

  if (request->count==2){
    // 遍历 JSON 数组
    cJSON *item = NULL;
    cJSON_ArrayForEach(item, json) {
      // 提取 id 和 name 字段
      cJSON *id = cJSON_GetObjectItem(item, "id");
      cJSON *name = cJSON_GetObjectItem(item, "name");

      if (strcmp(id->valuestring, request->params[0].value)==0 && strcmp(name->valuestring, request->params[1].value)==0) {
        LOG_INFO("aready exist : {id:%s,name:%s}", id->valuestring, name->valuestring);

        // 将当前 JSON 对象复制到一个新的 cJSON 对象中
        cJSON *new_item = cJSON_Duplicate(item, 1);
        // 向当前 JSON 对象添加新的属性
        cJSON_AddStringToObject(new_item, "success", "true");

        char *jsonString = cJSON_Print(new_item);
        removeSpacesAndNewlines(jsonString);

        http_start_response(fd, 200);
        http_send_header(fd, "Content-Type", "application/json");
        char *file_size = (char *) malloc(max_file_size * sizeof(char));
        sprintf(file_size, "%d", strlen(jsonString));
        http_send_header(fd, "Content-Length", file_size);
        http_end_headers(fd);
        http_send_string(fd, jsonString);
        close(fd);

        // 释放 cJSON 结构
        cJSON_Delete(json);
        cJSON_Delete(new_item);
        
        return 0;
      }
    }

    LOG_INFO("not find : {id:%s,name:%s}", request->params[0].value, request->params[1].value);
    // 创建一个新的 cJSON 对象
    cJSON *new_cjson = cJSON_CreateObject();
    cJSON_AddStringToObject(new_cjson, "id", request->params[0].value);
    cJSON_AddStringToObject(new_cjson, "name", request->params[1].value);
    cJSON_AddStringToObject(new_cjson, "success", "false");
    char *jsonString = cJSON_Print(new_cjson);
    removeSpacesAndNewlines(jsonString);

    http_start_response(fd, 404);
    http_send_header(fd, "Content-Type", "application/json");
    char *file_size = (char *) malloc(max_file_size * sizeof(char));
    sprintf(file_size, "%d", strlen(jsonString));
    http_send_header(fd, "Content-Length", file_size);
    http_end_headers(fd);
    http_send_string(fd, jsonString);

    cJSON_Delete(new_cjson);
    close(fd);

  } else if (request->count==3 && strcmp(request->params[2].param,"extra")==0)
  {
    LOG_INFO("add extra : {id:%s,name:%s,extra:%s}", request->params[0].value, request->params[1].value, request->params[2].value);
    // 遍历 JSON 数组
    cJSON *item = NULL;
    LOG_DEBUG("start loop");
    cJSON_ArrayForEach(item, json) {
      // 提取 id 和 name 字段
      cJSON *id = cJSON_GetObjectItem(item, "id");
      cJSON *name = cJSON_GetObjectItem(item, "name");

      LOG_DEBUG("start loop");

      if (strcmp(id->valuestring, request->params[0].value)==0 && strcmp(name->valuestring, request->params[1].value)==0) {
        cJSON *extra_item = cJSON_GetObjectItem(item, "extra");
        if (extra_item==NULL){
          LOG_DEBUG("add extra to json");
          cJSON_AddStringToObject(item, "extra", request->params[2].value);
        } else {
          cJSON_SetValuestring(extra_item, request->params[2].value);
        }

        // 将当前 JSON 对象复制到一个新的 cJSON 对象中
        cJSON *new_item = cJSON_Duplicate(item, 1);
        // 向当前 JSON 对象添加新的属性
        cJSON_AddStringToObject(new_item, "success", "true");

        char *jsonString = cJSON_Print(new_item);
        removeSpacesAndNewlines(jsonString);

        http_start_response(fd, 200);
        http_send_header(fd, "Content-Type", "application/json");
        char *file_size = (char *) malloc(max_file_size * sizeof(char));
        sprintf(file_size, "%d", strlen(jsonString));
        http_send_header(fd, "Content-Length", file_size);
        http_end_headers(fd);
        http_send_string(fd, jsonString);
        close(fd);

        cJSON_Delete(new_item); 

        LOG_INFO("save data.json");
        jsonString = cJSON_Print(json);
        removeSpacesAndNewlines(jsonString);
        FILE *file = fopen("./data.json", "w");
        if (file == NULL) {
            perror("Error opening file");
            return 1;
        }

        // 将数据缓冲区中的内容写入文件
        size_t bytes_written = fwrite(jsonString, sizeof(char), strlen(jsonString), file);
  
        // 关闭文件
        fclose(file);

        // 释放 cJSON 结构
        cJSON_Delete(json);

        return 1;
      }
    }

    LOG_INFO("not find : {id:%s,name:%s}", request->params[0].value, request->params[1].value);
    // 创建一个新的 cJSON 对象
    cJSON *new_cjson = cJSON_CreateObject();
    cJSON_AddStringToObject(new_cjson, "id", request->params[0].value);
    cJSON_AddStringToObject(new_cjson, "name", request->params[1].value);
    cJSON_AddStringToObject(new_cjson, "extra", request->params[2].value);
    cJSON_AddStringToObject(new_cjson, "success", "false");
    char *jsonString = cJSON_Print(new_cjson);
    removeSpacesAndNewlines(jsonString);

    http_start_response(fd, 404);
    http_send_header(fd, "Content-Type", "application/json");
    char *file_size = (char *) malloc(max_file_size * sizeof(char));
    sprintf(file_size, "%d", strlen(jsonString));
    http_send_header(fd, "Content-Length", file_size);
    http_end_headers(fd);
    http_send_string(fd, jsonString);

    // 释放 cJSON 结构
    cJSON_Delete(json);
    cJSON_Delete(new_cjson);
    close(fd);
    return;
  }

  LOG_INFO("wrong: invalid url");
  // 释放 cJSON 结构
  cJSON_Delete(json);
  close(fd);
  return;
}

// 去除字符串中的空格和换行符
void removeSpacesAndNewlines(char *str) {
    int len = strlen(str);
    int index = 0;
    for (int i = 0; i < len; i++) {
        if (str[i] != ' ' && str[i] != '\n' && str[i] != '\r' && str[i] != '\t') {
            str[index++] = str[i];
        }
    }
    str[index] = '\0';
}

/* Get a the size of the file specified by file descriptor fd */
int get_file_size(int fd) {  // 获取文件尺寸
    int saved_pos = lseek(fd, 0, SEEK_CUR);
    off_t file_size = lseek(fd, 0, SEEK_END);
    lseek(fd, saved_pos, SEEK_SET);
    return file_size;
}

int http_send_file(int dst_fd, int src_fd) {  //将src_fd的内容发送到dst_fd
    const int buf_size = 4096;
    char buf[buf_size];
    ssize_t bytes_read;
    int status;
    while( (bytes_read=read(src_fd, buf, buf_size))!=0) {
        status = http_send_data(dst_fd, buf, bytes_read);
        if(status < 0) return status;
    }
    return 0;
}

int http_send_string(int fd, char *data) {
    return http_send_data(fd, data, strlen(data));
}

int http_send_data(int fd, char *data, size_t size) {
    ssize_t bytes_sent;
    while (size > 0) {
        bytes_sent = write(fd, data, size);
        if (bytes_sent < 0)
            return bytes_sent;
        size -= bytes_sent;
        data += bytes_sent;
    }
    return 0;
}

char* join_string(char *str1, char *str2, size_t *size) {
    char *ret = (char *) malloc(strlen(str1) + strlen(str2) + 1), *p = ret;
    for(; (*p=*str1); p++, str1++);
    for(; (*p=*str2); p++, str2++);
    if(size != NULL) *size = (p-ret)*sizeof(char);
    return ret;
}


/*
 * Serves the contents the file stored at `path` to the client socket `fd`.
 * It is the caller's reponsibility to ensure that the file stored at `path` exists.
 */
void serve_file(int fd, char *path) {

  http_start_response(fd, 200);
  http_send_header(fd, "Content-Type", http_get_mime_type(path));

  int src_fd = open(path,O_RDONLY);
  char *file_size = (char *) malloc(max_file_size * sizeof(char));
  sprintf(file_size, "%d", get_file_size(src_fd));  
  http_send_header(fd, "Content-Length", file_size); // Change this too

  http_end_headers(fd);

  /* TODO: PART 2 */
  http_send_file(fd, src_fd);
  close(src_fd);
  return;
}


/*
 * Opens a TCP stream socket on all interfaces with port number PORTNO. Saves
 * the fd number of the server socket in *socket_number. For each accepted
 * connection, calls request_handler with the accepted fd number.
 */

void handle_request(int fd) {

  // char buffer[BUFFER_SIZE];
  // int read_bytes;
  // int read_num = 0;

  // // 循环读取客户端发送的数据
  // while ((read_bytes = recv(fd, buffer, BUFFER_SIZE, 0)) > 0) {
  //     buffer[read_bytes] = '\0';  // Null-terminate to create a string
  //     printf("Received request %d: %s\n",++read_num, buffer);

  //     // 简单响应消息
  //     char *response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 12\r\n\r\nHello world!\n";
  //     send(fd, response, strlen(response), 0);

  //     // 检查是否需要关闭连接（非管线化请求）
  //     if (strstr(buffer, "Connection: close") != NULL) {
  //       break;
  //     }
  // }

  // close(fd); 

  struct http_request *request = http_request_parse(fd);

  if (request == NULL || request->path[0] != '/') {  
    http_start_response(fd, 400);  //处理语法错误
    http_send_header(fd, "Content-Type", "text/html");
    http_end_headers(fd);
    close(fd);
    return;
  }

  if (strstr(request->path, "..") != NULL) {
    http_start_response(fd, 403);  //检查访问父级目录，增加代码安全性
    http_send_header(fd, "Content-Type", "text/html");
    http_end_headers(fd);
    close(fd);
    return;
  }


  char *path = malloc(2 + strlen(request->path) + 1);  //malloc函数详解
  path[0] = '.';
  path[1] = '/';
  memcpy(path + 2, request->path, strlen(request->path) + 1);

  LOG_DEBUG("path= %s", path);

  if (strcmp(request->method, "GET")==0){
    if (strncmp(request->path, "/api", 4) == 0)
    {
      LOG_INFO("start process GET api");
      struct apiRequest api_request;
      initapiRequest(&api_request);
      parseQueryString(request->path, &api_request);

      if (strcmp(api_request.api_type, "list") == 0){
        list(fd);
      } else if (strcmp(api_request.api_type, "check") == 0)
      {
        if (api_request.count==2 && strcmp(api_request.params[0].param,"id")==0 
            && strcmp(api_request.params[1].param,"name")==0){
          check(fd, &api_request);
          close(fd);
          return;
        }else{
        LOG_INFO("wrong: check failure");
        http_start_response(fd, 404);
        http_send_header(fd, "Content-Type", "text/html");
        http_end_headers(fd);

        int src_fd = open("404.html",O_RDONLY);
        http_send_file(fd, src_fd);
        close(src_fd);
        close(fd);
        return;
        }
      }
    } else {
      LOG_INFO("start process file request");

      struct stat sb;
      normalize_url(path);
      
      if (strcmp(path,"./")==0){
        path = "./index.html";
      }

      int exist = stat(path, &sb);
      LOG_DEBUG("file exist: %d", exist);

      LOG_DEBUG("now path is %s", path);

      if (exist == 0) { 
        serve_file(fd, path);
      } else {
        LOG_INFO("wrong: invalid file url");
        http_start_response(fd, 404);
        http_send_header(fd, "Content-Type", "text/html");
        http_end_headers(fd);

        int src_fd = open("404.html",O_RDONLY);
        http_send_file(fd, src_fd);
        close(src_fd);
      }
      close(fd);
      return;
    }
  } else if (strcmp(request->method, "POST")==0)
  {
    if (strncmp(request->path, "/api", 4) == 0){
      LOG_INFO("start process POST api");
      struct apiRequest api_request;
      initapiRequest(&api_request);
      parseQueryString(request->path, &api_request);
      
      if (strcmp(api_request.api_type, "upload") == 0){
        parseParam(request->content, &api_request);
        if (api_request.count>=2 && strcmp(api_request.params[0].param,"id")==0 
            && strcmp(api_request.params[1].param,"name")==0){
          upload(fd, &api_request);
          return;
        } else
        {
          LOG_INFO("wrong: params problem");
        }
        
      } else {
        LOG_INFO("wrong: invalid api tpye");
        http_start_response(fd, 404);
        http_send_header(fd, "Content-Type", "text/html");
        http_end_headers(fd);

        int src_fd = open("404.html",O_RDONLY);
        http_send_file(fd, src_fd);
        close(src_fd);
        close(fd);
        return;
      }
    }
  }

  LOG_INFO("wrong: invalid url");
  http_start_response(fd, 404);
  http_send_header(fd, "Content-Type", "text/html");
  http_end_headers(fd);

  int src_fd = open("404.html",O_RDONLY);
  http_send_file(fd, src_fd);
  close(src_fd);
  close(fd);
  return;
}

/* Helper function for Proxy handler */

struct fd_pair {
    int *read_fd;
    int *write_fd;
    pthread_cond_t* cond;
    int *finished;
    char* type;
    unsigned long id;
};

void* relay_message(void* endpoints) {
  // 类型转换，将void*转换为更具体的struct fd_pair*，以便访问结构体成员
  struct fd_pair* pair = (struct fd_pair*)endpoints;   
  
  char buffer[4096]; // 创建一个字符数组作为缓冲区，用于存储从文件描述符读取的数据
  int read_ret, write_ret; // 定义两个整型变量来存储read和write函数的返回值
  LOG_INFO("%s thread %lu start to work", pair->type, pair->id); // 打印消息，表示线程开始工作，输出线程类型和ID

  while((read_ret = read(*pair->read_fd, buffer, sizeof(buffer)-1)) > 0) { // 循环读取数据，直到没有数据可读或发生错误
    write_ret = http_send_data(*pair->write_fd, buffer, read_ret); // 将读取的数据发送到写文件描述符
    if(write_ret < 0) break; // 如果写操作失败，退出循环
  }
  
  if(read_ret <= 0) // 如果读操作失败或读取到的数据长度为0
    LOG_ERROR("%s thread %lu read failed, status %d", pair->type, pair->id, read_ret); // 打印读操作失败的消息
  if(write_ret <= 0) // 如果写操作失败
    LOG_ERROR("%s thread %lu write failed, status %d", pair->type, pair->id, write_ret); // 打印写操作失败的消息

*pair->finished = 1; // 设置完成标志为1，表示当前线程的工作已经完成
pthread_cond_signal(pair->cond); // 发送条件变量信号，唤醒在此条件变量上等待的其他线程

LOG_INFO("%s thread %lu exited", pair->type, pair->id);
return NULL;
}


static unsigned long id;
pthread_mutex_t id_mutex = PTHREAD_MUTEX_INITIALIZER;

void handle_proxy_request(int fd) {
  LOG_INFO("start process proxy request");
  struct sockaddr_in target_address; // 定义目标服务器的socket地址结构
  memset(&target_address, 0, sizeof(target_address)); // 初始化地址结构，将其内存空间清零
  target_address.sin_family = AF_INET; // 设置地址类型为IPv4
  target_address.sin_port = htons(server_proxy_port); // 设置目标端口号，并转换为网络字节顺序

  // 创建一个IPv4的TCP套接字，用于与代理目标通信
  int target_fd = socket(PF_INET, SOCK_STREAM, 0);
  if (target_fd == -1) {
    fprintf(stderr, "Failed to create a new socket: error %d: %s", errno, strerror(errno));
    close(fd);
    exit(errno);
  }

  // 尝试连接到目标代理服务器
  int connection_status = connect(target_fd, (struct sockaddr*) &target_address, sizeof(target_address));
  if (connection_status < 0) {
    // 这里不确定是否需要
    http_request_parse(fd);

    // 开始发送HTTP响应，状态码为502（代理错误）
    http_start_response(fd, 502);
    http_send_header(fd, "Content-Type", "text/html"); // 发送响应头部，设置内容类型为text/html
    http_end_headers(fd); // 结束头部发送
    close(target_fd);
    close(fd);
    return;
  }

  // 使用mutex和condition变量来同步不同线程之间的操作
  unsigned long local_id; // 本地线程标识符
  pthread_mutex_lock(&id_mutex); // 上锁
  local_id = id++; // 获取并更新线程ID
  pthread_mutex_unlock(&id_mutex); // 解锁

  // 输出当前线程将处理的代理请求ID
  LOG_INFO("Thread %lu will handle proxy request %lu.", pthread_self(), local_id);

  // 定义用于读写操作的文件描述符对
  struct fd_pair pairs[2];
  pthread_mutex_t mutex; // 定义互斥锁
  pthread_cond_t cond; // 定义条件变量
  int finished = 0; // 定义完成标志
  pthread_mutex_init(&mutex, NULL); // 初始化互斥锁
  pthread_cond_init(&cond, NULL); // 初始化条件变量

  // 设置文件描述符对和其他相关参数
  pairs[0].read_fd = &fd;
  pairs[0].write_fd = &target_fd;
  pairs[0].finished = &finished;
  pairs[0].type = "request";
  pairs[0].cond = &cond;
  pairs[0].id = local_id;

  pairs[1].read_fd = &target_fd;
  pairs[1].write_fd = &fd;
  pairs[1].finished = &finished;
  pairs[1].type = "response";
  pairs[1].cond = &cond;
  pairs[1].id = local_id;

  // 创建两个线程，分别用于处理请求和响应的转发
  pthread_t threads[2];
  pthread_create(&threads[0], NULL, relay_message, &pairs[0]);
  pthread_create(&threads[1], NULL, relay_message, &pairs[1]);

  // 如果还未完成，则在条件变量上等待
  if(!finished) pthread_cond_wait(&cond, &mutex);

  // 关闭文件描述符，清理资源
  close(fd);
  close(target_fd);

  // 销毁互斥锁和条件变量
  pthread_mutex_destroy(&mutex);
  pthread_cond_destroy(&cond);

  // 输出代理请求完成的信息
  LOG_INFO("Socket closed, proxy request %lu finished.\n", local_id);
}


void serve_forever(int *socket_number, void (*request_handler)(int)) {

  struct sockaddr_in server_address, client_address;
  size_t client_address_length = sizeof(client_address);
  int client_socket_number;
  pthread_t thread_id;

  // Creates a socket for IPv4 and TCP.
  *socket_number = socket(PF_INET, SOCK_STREAM, 0);  // 为IPv4和TCP创建套接口
  if (*socket_number == -1) {
    perror("Failed to create a new socket");
    exit(errno);
  }

  int socket_option = 1;
  if (setsockopt(*socket_number, SOL_SOCKET, SO_REUSEADDR, &socket_option,  //设置套接字选项允许地址复用
        sizeof(socket_option)) == -1) {
    perror("Failed to set socket options");
    exit(errno);
  }

  // Setup arguments for bind()
  memset(&server_address, 0, sizeof(server_address));
  server_address.sin_family = AF_INET;
  server_address.sin_addr.s_addr = INADDR_ANY;
  server_address.sin_port = htons(server_port);

  /* 
   * TODO: PART 1 BEGIN
   *
   * Given the socket created above, call bind() to give it
   * an address and a port. Then, call listen() with the socket.
   * An appropriate size of the backlog is 1024, though you may
   * play around with this value during performance testing.
   */


  if (bind(*socket_number, (struct sockaddr *) &server_address,  // 绑定IP与端口
            sizeof(server_address)) == -1) {
    perror("Failed to bind on socket");
    exit(errno);
  }

  if (listen(*socket_number, 1024) == -1) {
      perror("Failed to listen on socket");
      exit(errno);
  }

  /* PART 1 END */
  printf("Listening on port %d...\n", server_port);

  int socket_num = 0;

  // 接受连接
  while (1) {
      client_socket_number = accept(*socket_number,
        (struct sockaddr *) &client_address,
        (socklen_t *) &client_address_length);
      if (client_socket_number < 0) {
        perror("Error accepting socket");
        free(client_socket_number);
        continue;
      }

      printf("accept socket %d \n", ++socket_num);

      // 创建一个新的线程来处理连接
      if (pthread_create(&thread_id, NULL, request_handler, client_socket_number) != 0) {
          perror("Failed to create thread");
          free(client_socket_number);
      }

      pthread_detach(thread_id); // 不需要主线程等待这个新线程
  }
  
  // while (1) {
  //   client_socket_number = accept(*socket_number,
  //       (struct sockaddr *) &client_address,
  //       (socklen_t *) &client_address_length);
  //   if (client_socket_number < 0) {
  //     perror("Error accepting socket");
  //     continue;
  //   }


  //   printf("Accepted connection from %s on port %d\n",
  //       inet_ntoa(client_address.sin_addr),
  //       client_address.sin_port);

  //   request_handler(client_socket_number);  // BASICSERVER
  // }

  shutdown(*socket_number, SHUT_RDWR);  // 先关闭其读写功能,防止数据丢失
  close(*socket_number);
  return;
}


int server_fd;
void signal_callback_handler(int signum) {
  printf("Caught signal %d: %s\n", signum, strsignal(signum));
  printf("Closing socket %d\n", server_fd);
  if (close(server_fd) < 0) perror("Failed to close server_fd (ignoring)\n");
  exit(0);
}

char *USAGE =
  "Usage: ./httpserver --files some_directory/ [--port 8000 --num-threads 5]\n"
  "       ./httpserver --proxy inst.eecs.berkeley.edu:80 [--port 8000 --num-threads 5]\n";

void exit_with_usage() {
  fprintf(stderr, "%s", USAGE);
  exit(EXIT_SUCCESS);
}

int main(int argc, char **argv) {
  signal(SIGINT, signal_callback_handler);
  signal(SIGPIPE, SIG_IGN);

  //初始化 Log4c
  log4c_init();

  // 获取日志分类，'root' 是默认分类
  mylog = log4c_category_get("httpserver");

  log4c_category_log(mylog, LOG4C_PRIORITY_INFO, "log init.");

  /* Default settings */
  server_port = 8080;
  void (*request_handler)(int) = NULL;

  request_handler = handle_request;

  int i;
  for (i = 1; i < argc; i++) {  // 解析参数
    if (strcmp("--files", argv[i]) == 0) {
      request_handler = handle_request;
      server_files_directory = argv[++i];
      if (!server_files_directory) {
        fprintf(stderr, "Expected argument after --files\n");
        exit_with_usage();
      }
    } else if (strcmp("--proxy", argv[i]) == 0) {
      request_handler = handle_proxy_request;

      char *proxy_target = argv[++i];
      if (!proxy_target) {
        fprintf(stderr, "Expected argument after --proxy\n");
        exit_with_usage();
      }

      char *colon_pointer = strchr(proxy_target, ':');
      if (colon_pointer != NULL) {
        *colon_pointer = '\0';
        server_proxy_address = proxy_target;
        server_proxy_port = atoi(colon_pointer + 1);
      } else {
        server_proxy_address = proxy_target;
        server_proxy_port = 80;
      }
    } else if (strcmp("--port", argv[i]) == 0) {
      char *server_port_string = argv[++i];
      if (!server_port_string) {
        fprintf(stderr, "Expected argument after --port\n");
        exit_with_usage();
      }
      server_port = atoi(server_port_string);
    } else if (strcmp("--threads", argv[i]) == 0) {
      char *num_threads_str = argv[++i];
      if (!num_threads_str || (num_threads = atoi(num_threads_str)) < 1) {
        fprintf(stderr, "Expected positive integer after --threads\n");
        exit_with_usage();
      }
    } else if (strcmp("--help", argv[i]) == 0) {
      exit_with_usage();
    } else if (strcmp("--ip", argv[i]) == 0) {
      char *server_address_string = argv[++i];
    } else {
      fprintf(stderr, "Unrecognized option: %s\n", argv[i]);
      exit_with_usage();
    }
  }


  chdir(server_files_directory);  // 改变当前进程的工作目录为 server_files_directory 所指定的目录。
  serve_forever(&server_fd, request_handler);

  // 清理 Log4c 资源
  log4c_fini();

  return EXIT_SUCCESS;
}

