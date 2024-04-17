#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "libhttp.h"

#define LIBHTTP_REQUEST_MAX_SIZE 8192
#define IS_DEBUG 1

void http_fatal_error(char *message) {
  fprintf(stderr, "%s\n", message);
  exit(ENOBUFS);
}

struct http_request *http_request_parse(int fd) {
  struct http_request *request = malloc(sizeof(struct http_request));
  if (!request) http_fatal_error("Malloc failed");

  char *read_buffer = malloc(LIBHTTP_REQUEST_MAX_SIZE + 1);  // read缓冲区
  if (!read_buffer) http_fatal_error("Malloc failed");

  int bytes_read = read(fd, read_buffer, LIBHTTP_REQUEST_MAX_SIZE);
  read_buffer[bytes_read] = '\0'; /* Always null-terminate. */

  if(IS_DEBUG)
  {
  printf("%s", read_buffer);
  }

  char *read_start, *read_end;
  size_t read_size;

  do {
    /* Read in the HTTP method: "[A-Z]*" */
    read_start = read_end = read_buffer;
    while (*read_end >= 'A' && *read_end <= 'Z') read_end++;
    read_size = read_end - read_start;
    if (read_size == 0) break;
    request->method = malloc(read_size + 1);
    memcpy(request->method, read_start, read_size);
    request->method[read_size] = '\0';

    /* Read in a space character. */
    read_start = read_end;
    if (*read_end != ' ') break;
    read_end++;

    /* Read in the path: "[^ \n]*" */
    read_start = read_end;
    while (*read_end != '\0' && *read_end != ' ' && *read_end != '\n') read_end++;
    read_size = read_end - read_start;
    if (read_size == 0) break;
    request->path = malloc(read_size + 1);
    memcpy(request->path, read_start, read_size);
    request->path[read_size] = '\0';

    /* Read in HTTP version and rest of request line: ".*" */
    read_start = read_end;
    while (*read_end != '\0' && *read_end != '\n') read_end++;
    if (*read_end != '\n') break;
    read_end++;

    read_start = read_end;
    char *content_length_str = strstr(read_start, "Content-Length: ");
    if (content_length_str == NULL) {
      // 如果没有找到 Content-Length 头字段，直接返回
      free(read_buffer);  // 通过使用缓冲区，可以灵活地处理不同大小的输入数据。
      return request;
    }else{
      // 如果找到 Content-Length 头字段,开始解析content
      content_length_str += strlen("Content-Length: ");
      // 解析 Content-Length 字段值
      request->content_length = 0;
      while (*content_length_str >= '0' && *content_length_str <= '9') {
          request->content_length = request->content_length * 10 + (*content_length_str - '0');
          content_length_str++;
      }
      
      // 定位到内容部分的起始位置
      char *content_start = strstr(read_start, "\r\n\r\n");
      if (content_start == NULL) {
          // 如果没有找到内容部分的起始位置，返回错误
          free(request);
          return NULL;
      }
      content_start += strlen("\r\n\r\n");

      // 分配内存保存内容部分
      request->content = malloc(request->content_length + 1);
      if (request->content == NULL) {
          // 内存分配失败，返回错误
          free(request);
          return NULL;
      }

      // 复制内容部分到 request->content
      strncpy(request->content, content_start, request->content_length);
      request->content[request->content_length] = '\0';

      free(read_buffer); 
      return request;
    }


    free(read_buffer);  // 通过使用缓冲区，可以灵活地处理不同大小的输入数据。
    return request;
  } while (0);

  /* An error occurred. */
  free(request);
  free(read_buffer);
  return NULL;

}


char* http_get_response_message(int status_code) {
  switch (status_code) {
    case 100:
      return "Continue";
    case 200:
      return "OK";
    case 301:
      return "Moved Permanently";
    case 302:
      return "Found";
    case 304:
      return "Not Modified";
    case 400:
      return "Bad Request";
    case 401:
      return "Unauthorized";
    case 403:
      return "Forbidden";
    case 404:
      return "Not Found";
    case 405:
      return "Method Not Allowed";
    default:
      return "Internal Server Error";
  }
}

void http_start_response(int fd, int status_code) {
  dprintf(fd, "HTTP/1.0 %d %s\r\n", status_code,
      http_get_response_message(status_code));
}

void http_send_header(int fd, char *key, char *value) {
  dprintf(fd, "%s: %s\r\n", key, value);
}

void http_end_headers(int fd) {
  dprintf(fd, "\r\n");
}

char *http_get_mime_type(char *file_name) {
  char *file_extension = strrchr(file_name, '.');
  if (file_extension == NULL) {
    return "text/plain";
  }

  if (strcmp(file_extension, ".html") == 0 || strcmp(file_extension, ".htm") == 0) {
    return "text/html";
  } else if (strcmp(file_extension, ".jpg") == 0 || strcmp(file_extension, ".jpeg") == 0) {
    return "image/jpeg";
  } else if (strcmp(file_extension, ".png") == 0) {
    return "image/png";
  } else if (strcmp(file_extension, ".css") == 0) {
    return "text/css";
  } else if (strcmp(file_extension, ".js") == 0) {
    return "application/javascript";
  } else if (strcmp(file_extension, ".pdf") == 0) {
    return "application/pdf";
  } else if (strcmp(file_extension, ".json") == 0) {
    return "application/json";
  } else {
    return "text/plain";
  }
}

/*
 * Puts `<a href="/path/filename">filename</a><br/>` into the provided buffer.
 * The resulting string in the buffer is null-terminated. It is the caller's
 * responsibility to ensure that the buffer has enough space for the resulting string.
 */
void http_format_href(char *buffer, char *path, char *filename) {
  int length = strlen("<a href=\"//\"></a><br/>") + strlen(path) + strlen(filename)*2 + 1;
  snprintf(buffer, length, "<a href=\"/%s/%s\">%s</a><br/>", path, filename, filename);
}

/*
 * Puts `path/index.html` into the provided buffer.
 * The resulting string in the buffer is null-terminated.
 * It is the caller's responsibility to ensure that the
 * buffer has enough space for the resulting string.
 */
void http_format_index(char *buffer, char *path) {
  int length = strlen(path) + strlen("/index.html") + 1;
  snprintf(buffer, length, "%s/index.html", path);
}


void normalize_url(char *url) {
    char *src = url;
    char *dest = url;

    // 初始化状态为非斜杠
    int prev_char_slash = 0;

    // 遍历 URL 中的每个字符
    while (*src) {
        // 如果当前字符是斜杠
        if (*src == '/') {
            // 如果前一个字符不是斜杠，则将当前字符复制到目标位置
            if (!prev_char_slash) {
                *dest++ = *src;
            }
            prev_char_slash = 1;
        } else {
            // 如果当前字符不是斜杠，则将当前字符复制到目标位置
            *dest++ = *src;
            prev_char_slash = 0;
        }
        src++;
    }
    *dest = '\0'; // 添加字符串结束符
}