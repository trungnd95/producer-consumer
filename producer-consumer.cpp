#include <iostream>
#include <thread>
#include <deque>
#include <mutex>
#include <chrono>
#include <condition_variable>

using std::deque;


class Buffer
{
public:
    void add(int num) {
        while (true) {
            std::unique_lock<std::mutex> locker(mu);
            cond.wait(locker, [this](){return buffer_.size() < size_;});
            buffer_.push_back(num);
            locker.unlock();
            cond.notify_all();
            return;
        }
    }
    int remove() {
        while (true)
        {
            std::unique_lock<std::mutex> locker(mu);
            cond.wait(locker, [this](){return buffer_.size() > 0;});
            int back = buffer_.back();
            buffer_.pop_back();
            locker.unlock();
            cond.notify_all();
            return back;
        }
    }
    Buffer() {}

    int size(){buffer_.size();}
private:
    deque<int> buffer_;
    const unsigned int size_ = 100;
    std::mutex mu,cout_mu;
    std::condition_variable cond;
};

std::mutex mtx_print;

class Producer
{
public:
    Producer(Buffer* buffer)
    {
        this->buffer_ = buffer;
    }
    void run() {
        while (true) {
            int num = std::rand() % 100;
            buffer_->add(num);
            mtx_print.lock();
            std::cout << "Produced: size = " << buffer_->size() << std::endl;
            mtx_print.unlock();
        }
    }
private:
    Buffer *buffer_;
};

class Consumer
{
public:
    Consumer(Buffer* buffer)
    {
        this->buffer_ = buffer;
    }
    void run() {
        while (true) {
            int num = buffer_->remove();
            mtx_print.lock();
            std::cout << "Consumed: size = " << buffer_->size() << std::endl;
            mtx_print.unlock();
        }
    }
private:
    Buffer *buffer_;
};

int main() {
    Buffer b;
    Producer p(&b);
    Consumer c(&b);

    std::thread producer_thread(&Producer::run, &p);
    std::thread consumer_thread(&Consumer::run, &c);

    producer_thread.join();
    consumer_thread.join();
    getchar();
    return 0;
}




