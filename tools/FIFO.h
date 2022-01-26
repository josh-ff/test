#ifndef FIFO_H_
#define FIFO_H_

#include <stdint.h>
#include <deque>
#include <algorithm>
#include <vector>

struct FIFO {
  FIFO(uint16_t cap) : capacity_(cap) {}
  uint16_t size() {return queue_.size();}
  bool is_full() {return queue_.size() == capacity_;}
  uint16_t capacity() {return capacity_;}
  void set_capacity(uint16_t cap) {capacity_ = cap;}
  void add(int val) {
    if (is_full())
      queue_.pop_front();
    queue_.push_back(val);
  }
  int mean(){
    float m = 0;
    for (const auto& val : queue_){
      m += val;
    }
    m /= static_cast<float>(capacity_);
    return m; // cast bac to uint16_t probably overkill, vet
  }

  int median(){
    if (size() == 0 ) throw;
    std::vector<int> v(queue_.begin(), queue_.end());
    std::sort(v.begin(), v.end());
    auto ret = (v.size() % 2) ? v.at(v.size()/2) : (v.at((v.size()/2)) + v.at((v.size()/2 - 1)))/2 ;
    return ret;
  }

  protected:
    std::deque<int> queue_;
    uint16_t capacity_;
};

struct ToConvBuff : public FIFO {
    ToConvBuff(uint8_t cap) : FIFO(cap) {}
    // dots contents of FIFO buffer with unit step of equal widths
    // does so to the signal minus its mean
    int unit_step_dot(){
      int i = 0;
      int sum = 0;
      int me = mean();

      for (const auto& val : queue_){
        auto v = val - me; // subtract average val of array from val
        sum += (++i > capacity_/2) ? v : -1 * v;
      }
      return sum;
    }
};

#endif /* FIFO_H_ */
