#include "OrderBook2.hpp"
#include "RingBuffer.hpp"
#include "parser.hpp"
#include <benchmark/benchmark.h>
#include <thread>
#include <atomic>
#include <print>


void producer(lockFreeSPSC<InboundJob,65536>& r,std::atomic<bool>& done,std::vector<InboundJob>& orders){
		
	for( auto& o : orders){
		
		while(!r.try_enqueue(o)){ }		
	}
	done.store(true,std::memory_order_release);
}
	

void consumer(lockFreeSPSC<InboundJob,65536>& r,LOB& lob,std::atomic<bool>& done){
	InboundJob arriving;

	while(true){
		if(r.try_dequeue(arriving)){

			
		lob.processOrder(arriving.quantity,arriving.orderId,arriving.price,arriving.side);	}
		else if(done.load(std::memory_order_acquire)){ break;}
		else{ continue;}
	}

	

}

static void pipeline_endToEnd(benchmark::State &state){
	LOB lob;
	auto orders = fileReaderParser("incomingOrders.txt");
	
	for (auto _ : state) {
        state.PauseTiming();
	//new clean setup for new googble benchmark test
        lob.reset(); 
	lockFreeSPSC<InboundJob,65536> ring;
	std::atomic<bool> done{false};

        state.ResumeTiming();

        std::thread t1(producer,std::ref(ring),std::ref(done),std::ref(orders));
	std::thread t2(consumer,std::ref(ring),std::ref(lob),std::ref(done));

	t1.join(); t2.join();
    }
    state.SetItemsProcessed(state.iterations() * orders.size());
}
	
BENCHMARK(pipeline_endToEnd)->Iterations(500)->Unit(benchmark::kNanosecond);

	
	//queue the orders
	
		
	

	



