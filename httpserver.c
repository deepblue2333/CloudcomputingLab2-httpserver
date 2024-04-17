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

#define LOG_INFO(message, args...) \
  log4c_category_log(log4c_category_get("httpserver"), LOG4C_PRIORITY_INFO, message, ##args)

#define LOG_DEBUG(message, args...) \
  log4c_category_log(log4c_category_get("httpserver"), LOG4C_PRIORITY_DEBUG, message, ##args)


#define IS_DEBUG 1

int num_threads;  // Only used by poolserver  线程数量
int server_port;  // Default value: 8000   默认服务端口
char *server_files_directory; // 服务器工作路径
char *server_proxy_hostname; // 服务器代理主机名
int server_proxy_port; // 代理服务器端口
log4c_category_t* mylog; // 声明日志

int max_file_size = 100; 

void handle_proxy_request(int fd) {}

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
        http_start_response(fd, 404);
        http_send_header(fd, "Content-Type", "text/html");
        http_end_headers(fd);

        int src_fd = open("404.html",O_RDONLY);
        http_send_file(fd, src_fd);
        close(src_fd);
        return;
        }
      }
    } else {
      LOG_INFO("start process file request");

      struct stat sb;
      normalize_url(path);
      int exist = stat(path, &sb);
      LOG_DEBUG("file exist: %d", exist);
      
      if (strcmp(path,"./")==0){
        path = "./index.html";
      }

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



void serve_forever(int *socket_number, void (*request_handler)(int)) {

  struct sockaddr_in server_address, client_address;
  size_t client_address_length = sizeof(client_address);
  int client_socket_number;

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
  
  while (1) {
    client_socket_number = accept(*socket_number,
        (struct sockaddr *) &client_address,
        (socklen_t *) &client_address_length);
    if (client_socket_number < 0) {
      perror("Error accepting socket");
      continue;
    }


    printf("Accepted connection from %s on port %d\n",
        inet_ntoa(client_address.sin_addr),
        client_address.sin_port);

    request_handler(client_socket_number);  // BASICSERVER
  }

  shutdown(*socket_number, SHUT_RDWR);  // 先关闭其读写功能,防止数据丢失
  close(*socket_number);
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
        server_proxy_hostname = proxy_target;
        server_proxy_port = atoi(colon_pointer + 1);
      } else {
        server_proxy_hostname = proxy_target;
        server_proxy_port = 80;
      }
    } else if (strcmp("--port", argv[i]) == 0) {
      char *server_port_string = argv[++i];
      if (!server_port_string) {
        fprintf(stderr, "Expected argument after --port\n");
        exit_with_usage();
      }
      server_port = atoi(server_port_string);
    } else if (strcmp("--num-threads", argv[i]) == 0) {
      char *num_threads_str = argv[++i];
      if (!num_threads_str || (num_threads = atoi(num_threads_str)) < 1) {
        fprintf(stderr, "Expected positive integer after --num-threads\n");
        exit_with_usage();
      }
    } else if (strcmp("--help", argv[i]) == 0) {
      exit_with_usage();
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

