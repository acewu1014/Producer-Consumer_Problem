#include <pthread.h>

#include "item.hpp"
#include "thread.hpp"
#include "transformer.hpp"
#include "ts_queue.hpp"

#ifndef PRODUCER_HPP
#define PRODUCER_HPP

class Producer : public Thread {
   public:
    // constructor
    Producer(TSQueue<Item*>* input_queue, TSQueue<Item*>* worker_queue, Transformer* transformer);

    // destructor
    ~Producer();

    virtual void start();

   private:
    TSQueue<Item*>* input_queue;
    TSQueue<Item*>* worker_queue;

    Transformer* transformer;

    // the method for pthread to create a producer thread
    static void* process(void* arg);
};

Producer::Producer(TSQueue<Item*>* input_queue, TSQueue<Item*>* worker_queue, Transformer* transformer)
    : input_queue(input_queue), worker_queue(worker_queue), transformer(transformer) {
}

Producer::~Producer() {}

void Producer::start() {
    // TODO: starts a Producer thread
    pthread_create(&t, 0, Producer::process, (void*)this);
}

void* Producer::process(void* arg) {
    // TODO: implements the Producer's work
    Producer* producer = (Producer*) arg;

    while(producer->input_queue->get_size() != 0){
        Item* item = new Item;
        //dequeue to get the item
        item = producer->input_queue->dequeue();
        //Use transformer to get the new value of val, opcode and key are not changed
        item->val  = producer->transformer->producer_transform(item->opcode, item->val);
        //Enqueue the item with new value into the queue
        producer->worker_queue->enqueue(item);
    }

    return nullptr;
}

#endif  // PRODUCER_HPP
