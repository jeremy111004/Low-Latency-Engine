#include "OrderBook2.hpp"
#include "RingBuffer.hpp"
#include "parser.hpp"

#include <thread>
#include <atomic>
#include <print>


void producer(lockFreeSPSC<InboundJob,65536>& r,std::atomic<bool>& done){
	auto orders = fileReaderParser("incomingOrders.txt");
	
	for( auto& o : orders){
		
		while(!r.try_enqueue(o)){ }		
	}
	done.store(true,std::memory_order_release);
}
	

void consumer(lockFreeSPSC<InboundJob,65536>& r,LOB& lob,std::atomic<bool>& done){
	InboundJob arriving;
	size_t processed = 0;
	while(true){
		if(r.try_dequeue(arriving)){
			processed++;
			
		lob.processOrder(arriving.quantity,arriving.orderId,arriving.price,arriving.side);	}
		else if(done.load(std::memory_order_acquire)){ break;}
		else{ continue;}
	}

	std::cout << (processed);

}

int main(){
	LOB lob;
	lockFreeSPSC<InboundJob,65536> ring;
	std::atomic<bool> done{false};

	std::thread t1(producer,std::ref(ring),std::ref(done));
	std::thread t2(consumer,std::ref(ring),std::ref(lob),std::ref(done));

	t1.join(); t2.join();

		
	//queue the orders
	
		
	

	



}
