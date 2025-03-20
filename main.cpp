#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include "ts_queue.hpp"
#include "item.hpp"
#include "reader.hpp"
#include "writer.hpp"
#include "producer.hpp"
#include "consumer_controller.hpp"

#define READER_QUEUE_SIZE 20
#define WORKER_QUEUE_SIZE 200
#define WRITER_QUEUE_SIZE 4000
#define CONSUMER_CONTROLLER_LOW_THRESHOLD_PERCENTAGE 20
#define CONSUMER_CONTROLLER_HIGH_THRESHOLD_PERCENTAGE 80
#define CONSUMER_CONTROLLER_CHECK_PERIOD 1000000

int main(int argc, char** argv) {
	assert(argc == 4);

	int n = atoi(argv[1]);
	std::string input_file_name(argv[2]);
	std::string output_file_name(argv[3]);
	//keep track of the time needed
	double START,END;
	START = clock();

	// TODO: implements main function
	TSQueue<Item*>* input_queue = new TSQueue<Item*>(READER_QUEUE_SIZE);
	TSQueue<Item*>* worker_queue = new TSQueue<Item*>(WORKER_QUEUE_SIZE);
	TSQueue<Item*>* output_queue = new TSQueue<Item*>(WRITER_QUEUE_SIZE);

	// //calculate the number of lines

	// std::ifstream myfile(input_file_name);
    // // new lines will be skipped unless we stop it from happening:    
    // myfile.unsetf(std::ios_base::skipws);
    // // count the newlines with an algorithm specialized for counting:
    // int line_count = std::count(
    //     std::istream_iterator<char>(myfile),
    //     std::istream_iterator<char>(), 
    //     '\n');

	//Instantiate transformer
	std::cout<< n << endl;
	Transformer* transformer = new Transformer;
	//instantiate a reader, a writer, 4 producers, and a consumer_controller
	Reader* reader = new Reader(n, input_file_name, input_queue);
	Writer* writer = new Writer(n, output_file_name, output_queue);
	Producer* p1 = new Producer(input_queue, worker_queue, transformer);
	Producer* p2 = new Producer(input_queue, worker_queue, transformer);
	Producer* p3 = new Producer(input_queue, worker_queue, transformer);
	Producer* p4 = new Producer(input_queue, worker_queue, transformer);
	ConsumerController* cc = new ConsumerController(worker_queue, output_queue, transformer, CONSUMER_CONTROLLER_CHECK_PERIOD, CONSUMER_CONTROLLER_LOW_THRESHOLD_PERCENTAGE, CONSUMER_CONTROLLER_HIGH_THRESHOLD_PERCENTAGE);
	//start the threads
	reader->start();
	writer->start();

	p1->start();
	p2->start();
	p3->start();
	p4->start();

	cc->start();
	


	//use join to make sure that all the thread is ended
	reader->join();
	writer->join();
	//delete all queue and thread
	delete reader;
	delete writer;
	delete p1;
	delete p2;
	delete p3;
	delete p4;
	delete cc;
	delete transformer;
	delete input_queue;
	delete worker_queue;
	delete output_queue;
	END = clock();
	cout << endl << "程式執行所花費：" << (double)clock()/CLOCKS_PER_SEC << " S";
	cout << endl << "進行運算所花費的時間：" << (END - START) / CLOCKS_PER_SEC << " S" << endl;
	return 0;
}
