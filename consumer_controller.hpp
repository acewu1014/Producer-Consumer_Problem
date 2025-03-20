#include <pthread.h>
#include <chrono>
#include <unistd.h>
#include <vector>
#include <iostream>
#include <thread>
#include <functional>
#include "consumer.hpp"
#include "ts_queue.hpp"
#include "item.hpp"
#include "transformer.hpp"

#ifndef CONSUMER_CONTROLLER
#define CONSUMER_CONTROLLER

class ConsumerController : public Thread {
public:
	// constructor
	ConsumerController(
		TSQueue<Item*>* worker_queue,
		TSQueue<Item*>* writer_queue,
		Transformer* transformer,
		int check_period,
		int low_threshold,
		int high_threshold
	);

	// destructor
	~ConsumerController();

	virtual void start();

private:
	std::vector<Consumer*> consumers;

	TSQueue<Item*>* worker_queue;
	TSQueue<Item*>* writer_queue;

	Transformer* transformer;

	// Check to scale down or scale up every check period in microseconds.
	//Ace change to static
	int check_period;
	// When the number of items in the worker queue is lower than low_threshold,
	// the number of consumers scaled down by 1.
	int low_threshold;
	// When the number of items in the worker queue is higher than high_threshold,
	// the number of consumers scaled up by 1.
	int high_threshold;

	static void* process(void* arg);

	static void timer_start(std::function<void(void)> func, unsigned int interval);

	static void check(ConsumerController* cc);

	void create_consumer();

	void delete_consumer();
};

// Implementation start

ConsumerController::ConsumerController(
	TSQueue<Item*>* worker_queue,
	TSQueue<Item*>* writer_queue,
	Transformer* transformer,
	int check_period,
	int low_threshold,
	int high_threshold
) : worker_queue(worker_queue),
	writer_queue(writer_queue),
	transformer(transformer),
	check_period(check_period),
	low_threshold(low_threshold),
	high_threshold(high_threshold) {
}

ConsumerController::~ConsumerController() {}

void ConsumerController::start() {
	// TODO: starts a ConsumerController thread
	pthread_create(&t, 0, ConsumerController::process, (void*)this);
}

void* ConsumerController::process(void* arg) {
	// TODO: implements the ConsumerController's work
	ConsumerController* cc = (ConsumerController*) arg;
	timer_start(std::bind(check, cc), cc->check_period);
	
	while(true);
	return nullptr;
}


void ConsumerController::timer_start(std::function<void(void)> func, unsigned int interval)
{
  std::thread([func, interval]()
  { 
    while (true)
    { 
      auto x = std::chrono::steady_clock::now() + std::chrono::microseconds(interval);
      func();
      std::this_thread::sleep_until(x);
    }
  }).detach();
}
void ConsumerController::check(ConsumerController* c)
{
	ConsumerController* cc = c;
	static bool flag = false;
	float percentage = 100*cc->worker_queue->get_size()/cc->worker_queue->get_buff_size();
	//check whether the queue size is bigger than threshold
	if(percentage > cc->high_threshold){
		//if yes create a consumer
		cc->create_consumer();
		flag = true;
	}
	else if(percentage < cc->low_threshold){
		//check the flag and make sure that at least two consumer, then delete a consumer 
		if(flag && cc->consumers.size()>1){
			cc->delete_consumer();
		}
	}
}

void ConsumerController::create_consumer()
{
	Consumer* c = new Consumer(worker_queue, writer_queue, transformer);
	c->start();
	consumers.push_back(c);
	cout<< "Scaling up consumers from " << consumers.size()-1 << " to " << consumers.size() << endl;
}

void ConsumerController::delete_consumer()
{
	Consumer* c = new Consumer(worker_queue, writer_queue, transformer);
	c = consumers[consumers.size()-1];
	consumers.pop_back();
	c->cancel();
	cout<< "Scaling up consumers from " << consumers.size()+1 << " to " << consumers.size() << endl;
}

#endif // CONSUMER_CONTROLLER_HPP
