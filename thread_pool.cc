#include <atomic>
#include <functional>
#include <vector> 
#include <thread>
#include <stdexcept>
#include "threadsafe_queue.cc" // 线程安全队列在此声明


class join_threads
{
    std::vector<std::thread>& threads;
public:
    explicit join_threads(std::vector<std::thread>& threads_):
        threads(threads_)
    {}
    ~join_threads()
    {
        for(unsigned long i=0;i<threads.size();++i)
        {
            if(threads[i].joinable())
                threads[i].join();
        }
    }
};

class thread_pool
{
    std::atomic_bool done;  // 原子操作布尔值
    public: threadsafe_queue<std::function<void()>> work_queue;   // 一个线程安全的队列，其中存储了 std::function<void()> 类型的任务（函数对象）
    std::vector<std::thread> threads;   // 一个存放线程的容器
    join_threads joiner;  // join_threads实现异常安全
    void worker_thread()
    {
        while(!done)   
        {
            std::function<void()> task;
            if(work_queue.try_pop(task))  
            {
                task();   
            }
            else
            {
                std::this_thread::yield(); //等待新任务
            }
        }
    }
public:
    thread_pool():
        done(false),joiner(threads)
    {
        unsigned const thread_count=std::thread::hardware_concurrency();  
        try
        {
            for(unsigned i=0;i<thread_count;++i)
            {
                threads.push_back(
                    std::thread(&thread_pool::worker_thread,this));   // 向新线程传入工作线程函数引用，this是隐式参数，传入新线程以指向worker_thread函数
            }
        }
        catch(...)
        {
            done=true;
            throw;
        }
    }
    ~thread_pool()
    {
        done=true;    
    }
    template<typename FunctionType>
    void submit(FunctionType f)
    {
        work_queue.push(std::function<void()>(f)); 
    }
};