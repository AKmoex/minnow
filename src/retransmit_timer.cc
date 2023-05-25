#include "retransmit_timer.hh"
#include <iostream>

RetransmitTimer::RetransmitTimer(uint64_t rto, bool is_running, uint64_t time):RTO_(rto),is_running_(is_running),time_(time)
{
}

bool RetransmitTimer::is_running()
{
  return is_running_;
}

void RetransmitTimer::set_RTO(uint64_t rto)
{
  RTO_ = rto;
}

void RetransmitTimer::start()
{
  is_running_ = true;
  time_ = 0;
}

void RetransmitTimer::stop()
{
  is_running_ = false;
}

bool RetransmitTimer::is_expired( uint64_t ms_since_last_tick )
{
  time_ += ms_since_last_tick;
  return is_running_ && time_ >= RTO_;
}

uint64_t RetransmitTimer::get_RTO()
{
  return RTO_;
}

uint64_t RetransmitTimer::get_time()
{
  return time_;
}
