#include <iostream>
#include <map>
#include <string>
#include <vector>
// C++ program to implement a custom container class template with basic operations.
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <array>
#include <atomic>

// Custom container class template
template <class T, size_t capacity> class lockFreeSPSC{
	static_assert((capacity & (capacity - 1)) == 0, "due to bit operations, capacity has to be a power of 2");
	public:
		// basic operations
		lockFreeSPSC() = default;
		~lockFreeSPSC() = default;

		//no copy or duplication
		lockFreeSPSC& operator=(const lockFreeSPSC&) = delete;
		lockFreeSPSC(const lockFreeSPSC&) = delete; // duplication


		bool try_enqueue(const T& item) noexcept;
    		bool try_dequeue(T& out_item) noexcept;


	

	private:
		std::array<T,capacity> ringbuffer_;
		alignas(64) std::atomic<size_t>  producer_pos_;
		alignas(64) std::atomic<size_t> consumer_pos_;

};
template<class T, size_t capacity>
bool lockFreeSPSC<T,capacity>::try_enqueue(const T &item) noexcept {
	const size_t cur_prod = producer_pos_.load(std::memory_order_relaxed);
	const size_t cur_cons = consumer_pos_.load(std::memory_order_relaxed);

	if((cur_prod ^ cur_cons) == capacity){return false;}

	ringbuffer_[cur_prod & (capacity - 1)] = item;
	producer_pos_.store(cur_prod + 1,std::memory_order_release);

	return true;
		
}

template<class T, size_t capacity>
bool lockFreeSPSC<T,capacity>::try_dequeue(T &item) noexcept {
	const size_t cur_prod = producer_pos_.load(std::memory_order_acquire );
	const size_t cur_cons = consumer_pos_.load(std::memory_order_relaxed);

	if((cur_prod - cur_cons) == 0){return false;}

	item =	std::move(ringbuffer_[cur_cons & (capacity - 1)]);
	consumer_pos_.store(cur_cons + 1,std::memory_order_release);

	return true;
}
