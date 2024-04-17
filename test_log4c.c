#include <log4c.h>
#include <stdlib.h>

int main(int argc, char** argv) {

  char cwd[1024];
  if (getcwd(cwd, sizeof(cwd)) != NULL) {
      printf("Current working dir: %s\n", cwd);
  } else {
      perror("getcwd() error");
  }
  // 初始化 log4c
  int init_status = log4c_init();

  if (init_status != 0) {
    fprintf(stderr, "log4c initialization failed with status: %d\n", init_status);
    return 1;
  }

  // 获取日志类别
  log4c_category_t* mycat = log4c_category_get("root");

  if (!mycat) {
    fprintf(stderr, "Failed to get log4c category.\n");
    log4c_fini();
    return 1;
  } 

  // 记录不同级别的日志消息
  log4c_category_log(mycat, LOG4C_PRIORITY_DEBUG, "This is a debug message.");
  log4c_category_log(mycat, LOG4C_PRIORITY_INFO, "This is an info message.");
  log4c_category_log(mycat, LOG4C_PRIORITY_WARN, "This is a warning message.");
  log4c_category_log(mycat, LOG4C_PRIORITY_ERROR, "This is an error message.");
  log4c_category_log(mycat, LOG4C_PRIORITY_FATAL, "This is a fatal message.");

  // 清理 log4c
  log4c_fini();

  return 0;
}
