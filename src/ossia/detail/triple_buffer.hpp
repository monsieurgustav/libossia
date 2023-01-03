#pragma once
#include <atomic>

namespace ossia
{

template <typename T>
class triple_buffer
{
  T data[3] = {};

  std::atomic<T*> to_read{&data[0]};
  std::atomic<T*> buffer{&data[1]};
  std::atomic<T*> to_write{&data[2]};

  std::atomic_flag stale;

public:
  triple_buffer(T init)
  {
    // Store the initial data in the "ready" buffer:
    data[1] = std::move(init);
    stale.clear();
  }

  void produce(T& t)
  {
    using namespace std;

    // Load the data in the buffer
    auto& old = *to_write.load();
    swap(old, t);

    // Perform the buffer swap: ready <-> to_write
    auto p = buffer.exchange(to_write);
    to_write.store(p);

    // Notify the reader that new data is available
    stale.clear();
  }

  bool consume(T& res)
  {
    // Check if new data is available
    if(stale.test_and_set())
      return false;

    // Load the new data: ready <-> present:
    auto p = buffer.exchange(to_read);
    to_read.store(p);

    // Read back into our data
    res = std::move(*p);
    return true;
  }
};

}
