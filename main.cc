#include <iostream>
#include <assert.h>
#include <cstring>
#include <fstream>
#include <vector>
#include <thread>
#include <sys/time.h>
#include <sstream>
#include <mutex>
#include <condition_variable>
// #include "threadsafe_queue.cc"
#include "thread_pool.cc"
#include "sudoku.h"

using namespace std;

std::mutex mtx; 
const int MAX_QUEUE_SIZE = 100; // 最大队列大小
const int linesPerChunk = 5000; // 每次读取的行数，可以根据需要调整
std::condition_variable cv;
int current_task = 1; // 当前应该输出的任务编号

// 声明 split_file 函数
void split_file(std::string path, int& task_num, thread_pool& my_thread_pool);
void print_task(int task_num, const std::string& message);
int64_t now();
struct task; // 前向声明

struct task {
    int task_num; // 任务编号
    std::string task_data; // 任务数据
    std::string result_data; // 计算结果

    
    // 构造函数
    task(int num, const std::string& data) : task_num(num), task_data(data) {}
    
    // 重载函数调用运算符，用于执行任务
    void operator()() 
    {
        // 这里是task的具体工作
        init_neighbors();
        char puzzle[128];
        bool (*solve)(int) = solve_sudoku_dancing_links;
        int64_t start = now();

        // 使用 std::istringstream 将 std::string 分割成行
        std::istringstream iss(task_data);
        std::string line;
 
        // 逐行读取并输出
        while (std::getline(iss, line)) {
            if (line.length() < sizeof(puzzle)) { // 检查行长度是否小于字符数组大小
                std::strcpy(puzzle, line.c_str());
            
                if (strlen(puzzle) >= N) {
                input(puzzle);
                init_cache();
                if (solve(0)) {
                    if (!solved())
                    assert(0);
                }
                else {
                    printf("No: %s", puzzle);
                }
                }
            }
            
            // 保存计算结果
            for (int i = 0; i < 81; ++i) {
                result_data += std::to_string(board[i]);
            }
            result_data += '\n';
        }


        print_task(task_num, result_data); // 将计算结果按task顺序输出到标准输出
    }
};

int64_t now()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec * 1000000 + tv.tv_usec;
}


void receive_filename(thread_pool& my_thread_pool)  //接收控制台输入
{ 
    std::string test_filepath;
    int task_num{0};

    // 读取输入直到输入结束
    while (std::cin >> test_filepath) {
        // 处理输入
        split_file(test_filepath, task_num, my_thread_pool);
    }
    
    if (std::cin.eof()) {
        return ;
    } else {
        std::cout << "输入错误或遇到文件结束" << std::endl;
    }
    
    return ;
}

void split_file(std::string path, int& task_num, thread_pool& my_thread_pool)
{
    std::ifstream file(path); 
    if (!file.is_open()) {
        std::cerr << "无法打开文件！" << std::endl;
        return ;
    }
    
    
    std::string line;
    std::string chunk;

    // 循环读取文件内容，直到文件末尾
    int lineCount = 0;
    while (std::getline(file, line)) {
        chunk += line + "\n"; // 将读取的行添加到块中
        ++lineCount;

        // 如果达到了指定的行数，处理当前块并重置计数器和块内容
        if (lineCount >= linesPerChunk) {
            task_num += 1;
            task new_task{task_num, chunk};
            my_thread_pool.submit(new_task); // 向线程池添加一个任务
            lineCount = 0;
            chunk.clear();
        }

        // 使用循环等待，直到队列有足够的空间
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [&my_thread_pool]{
            return my_thread_pool.work_queue.size() < MAX_QUEUE_SIZE;
        });
    }

    // 处理最后一块不足 linesPerChunk 行的内容
    if (!chunk.empty()) {
        task_num += 1;
        task new_task{task_num, chunk};

        // 使用循环等待，直到队列有足够的空间
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [&my_thread_pool]{
            return my_thread_pool.work_queue.size() < MAX_QUEUE_SIZE;
        });

        my_thread_pool.submit(new_task); // 向线程池添加一个任务
    }

    file.close(); // 关闭文件
}

void print_task(int task_num, const std::string& message) {
    std::unique_lock<std::mutex> lock(mtx);
    
    // 如果当前线程编号不是自己，则等待
    cv.wait(lock, [&](){ return current_task == task_num; });
    
    // 输出信息
    std::cout << message << std::flush;
    
    // 更新当前线程编号，唤醒下一个线程
    current_task++;
    cv.notify_all();
}


int main()
{ 
    thread_pool my_thread_pool; // 初始化一个线程池
    receive_filename(my_thread_pool); //线程池开始工作
    return 0;
};