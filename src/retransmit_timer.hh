#pragma once

#include <cstdint>


class RetransmitTimer{
private:
  // 当前的RTO
  uint64_t RTO_ ;
  // 重传计时器是否运行
  bool is_running_;
  // 已计时
  uint64_t time_;
public:
  RetransmitTimer(uint64_t rto, bool is_running = false, uint64_t time = 0);
  void set_RTO(uint64_t rto);
  uint64_t get_RTO();
  bool is_running();
  void start();
  void stop();
  bool is_expired(uint64_t ms_since_last_tick);
  uint64_t get_time();



};
