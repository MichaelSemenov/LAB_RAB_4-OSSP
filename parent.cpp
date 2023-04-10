#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>
#include <string>

using namespace std;
struct Message {
    int id;
    string text;
};
class MessageQueue {
public:
    MessageQueue(int size) :                         //Максимальный размер очереди
        messages(size), head(0), tail(0), 
        added(0), extracted(0) {}
    
    void addMessage(Message* message) {              //Добавление 
         unique_lock<mutex> lock(mtx);
        while (added - extracted == messages.size()) {
            not_full.wait(lock);
        }
        messages[tail] = message;
        tail = (tail + 1) % messages.size();
        added++;
        not_empty.notify_one();
    }
    
    Message* getMessage() {
        unique_lock<mutex> lock(mtx);
        while (added == extracted) {
            not_empty.wait(lock);                                    //Приостановка потока!
        }
        Message* message = messages[head];
        head = (head + 1) % messages.size();
        extracted++;
        not_full.notify_one();
        return message;
    }
    ~MessageQueue()
    {
        cout << "Вызван деструктор, программа успешно завершена!" << endl;
    }
private:
    vector<Message*> messages;
    int head;
    int tail;
    int added;
    int extracted;
    mutex mtx;
    condition_variable not_empty;
    condition_variable not_full;
};

void producer(MessageQueue* queue, int id) {
    for (int i = 0; i < 10; i++) {
        Message* message = new Message();
        message->id = i;
        message->text = "Message from producer " + to_string(id);
        queue->addMessage(message);
    }
}

void consumer(MessageQueue* queue, int id) {
    for (int i = 0; i < 10; i++) {
        Message* message = queue->getMessage();
        cout << "Consumer " << id << " got message #" << message->id 
             << " with text: " << message->text << endl;
        delete message;
    }
}

int main() {
    MessageQueue queue(10);
    thread producers[2];
    thread consumers[2];
    for (int i = 0; i < 2; i++) {
        producers[i] = thread(producer, &queue, i);
        consumers[i] = thread(consumer, &queue, i);
    }    
    for (int i = 0; i < 2; i++) 
    {
        producers[i].join();

        consumers[i].join();
    }
    return 0;
}