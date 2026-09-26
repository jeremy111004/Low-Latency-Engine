#include "Order2.hpp"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstring>
#include <memory>
#include <list>

 struct Freelist {
    Order* head;
    Order* tail;
  };

class Arena {
  static constexpr size_t alignment = alignof(Order);

public:
  Arena(size_t N)
      : buffer_(std::unique_ptr<char[]>(new char[N])), ptr_(buffer_.get()),
       Bsize(N) {
    // Force the OS to allocate physical backing for every page.
    // This is called "pre-faulting" or "warming" the memory.
    std::memset(buffer_.get(), 0, Bsize);
  };
  Arena(const Arena &) = delete;
  Arena &operator=(const Arena &) = delete;

  auto reset() noexcept -> void { 
	ptr_ = buffer_.get();
	freelist.head = nullptr;
  
  };
  size_t size() const noexcept { return Bsize; }
  auto used() const noexcept -> size_t {
    assert(ptr_ >= buffer_.get() && "Arena pointer underflow error");
    return static_cast<size_t>(ptr_ - buffer_.get());
  };
  template <typename T>
  T* allocate(size_t n=1) {
    auto size = align_up(n*sizeof(T));
    if constexpr(std::is_same_v<T,Order>){
    	if (n==1 && freelist.head) {
      		T* address = freelist.head; 
      		freelist.head = freelist.head->next;
      		return address;
    	} 
    }
      auto remainingSpace =
          static_cast<decltype(size)>(buffer_.get() + Bsize - ptr_);
      if (size <= remainingSpace) {
        T *address = reinterpret_cast<T*>(ptr_);
        ptr_ += size;
        return address;
      } else {
        return nullptr;
      }
  };

  auto deallocate(Order *p) noexcept -> void {
	p->prev = nullptr;
    	p->next = freelist.head;
	freelist.head = p;

		
	
    	


  };

private:
  static auto align_up(size_t n) noexcept -> size_t {
    return (n + (alignment - 1)) & ~(alignment - 1);
  }

  auto pointer_in_buffer(const char *p) const noexcept -> bool {
    return buffer_.get() <= p && p <= buffer_.get() + Bsize;
  };

  const std::unique_ptr<char[]> buffer_;
  char* ptr_; 
  const size_t Bsize;
 
  Freelist freelist;
};
