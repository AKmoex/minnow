#include "tcp_sender.hh"
#include "tcp_config.hh"

#include <random>
#include <iostream>
#include <stdio.h>

using namespace std;

/* TCPSender constructor (uses a random ISN if none given) */
TCPSender::TCPSender( uint64_t initial_RTO_ms, optional<Wrap32> fixed_isn )
  : isn_( fixed_isn.value_or( Wrap32 { random_device()() } ) ), initial_RTO_ms_( initial_RTO_ms ){}

uint64_t TCPSender::sequence_numbers_in_flight() const
{
  return outstanding_bytes_;
}

uint64_t TCPSender::consecutive_retransmissions() const
{
  return retransmissions_num_;
}

optional<TCPSenderMessage> TCPSender::maybe_send()
{
  if(messages_.empty()){
    return {};
  }
  if(retransmissions_num_ && timer.is_running() && timer.get_time()>0){
    return {};
  }

  TCPSenderMessage message = messages_.front();
  messages_.pop_front();

  return message;
}

void TCPSender::push( Reader& outbound_stream )
{
  // 剩余窗口大小
  // 有可能出现了多次push(),但是没有新的receive()过来, 所以可以发送容量并不一定是window_size_
  uint64_t remain_window_size = (!window_size_?1:window_size_) - (next_seqno_-recv_seqno_);

  while(!fin_ && remain_window_size > 0){
    TCPSenderMessage message;
    if(!syn_){
      message.SYN = true;
      syn_ = true;
      remain_window_size--;
    }
    message.seqno = Wrap32::wrap(next_seqno_,isn_);
    Buffer &buffer = message.payload;
    uint64_t payload_size = min(TCPConfig::MAX_PAYLOAD_SIZE,remain_window_size);
    read(outbound_stream,payload_size,buffer);
    // 应该减去buffer.size(), payload_size不对
    remain_window_size -= buffer.size();
    // 如果发送完了, 并且还有空闲空间, 那就发送FIN
    if(outbound_stream.is_finished() && remain_window_size > 0){
      message.FIN = true;
      fin_ = true;
      remain_window_size--;
    }

    if(!message.sequence_length()){
      return;
    }
    next_seqno_ += message.sequence_length();
    messages_.push_back(message);
    outstanding_messages_.push_back(message);
    outstanding_bytes_+= message.sequence_length();

    if(message.sequence_length()>0 && !timer.is_running()){
      timer.set_RTO(initial_RTO_ms_);
      timer.start();
    }
  }
}

TCPSenderMessage TCPSender::send_empty_message() const
{
  TCPSenderMessage message;
  message.seqno = Wrap32::wrap(next_seqno_,isn_);
  return message;
}

void TCPSender::receive( const TCPReceiverMessage& msg )
{
  if(msg.ackno){
    recv_seqno_ = msg.ackno->unwrap(isn_,recv_seqno_);
  }
  if(recv_seqno_ > next_seqno_){
    return ;
  }
  window_size_ = msg.window_size;

  while(!outstanding_messages_.empty()){
    TCPSenderMessage message = outstanding_messages_.front();
    uint64_t msg_seqno = message.seqno.unwrap(isn_,recv_seqno_);
    if(msg_seqno + message.sequence_length() <= recv_seqno_){
      outstanding_messages_.pop_front();
      outstanding_bytes_ -= message.sequence_length();
      timer.set_RTO(initial_RTO_ms_);
      timer.start();
    }
    else{
      break;
    }
  }

//  if(outstanding_messages_.empty()){
//    timer.stop();
//  }
//  else{
//    timer.start();
//
//  }
  retransmissions_num_ = 0;
}

void TCPSender::tick( uint64_t ms_since_last_tick )
{

  if(timer.is_expired(ms_since_last_tick) && !outstanding_messages_.empty() ){
     messages_.push_back(outstanding_messages_.front());
     if(window_size_>0 || outstanding_messages_.front().SYN){
      retransmissions_num_++;
      timer.set_RTO(timer.get_RTO()<<1);
     }

     timer.start();

  }
}
