#pragma once

#include "byte_stream.hh"
#include "tcp_receiver_message.hh"
#include "tcp_sender_message.hh"
#include "retransmit_timer.hh"
#include<algorithm>
class TCPSender
{
  Wrap32 isn_;
  uint64_t initial_RTO_ms_;

public:
  /* Construct TCP sender with given default Retransmission Timeout and possible ISN */
  TCPSender( uint64_t initial_RTO_ms, std::optional<Wrap32> fixed_isn );

  /* Push bytes from the outbound stream */
  void push( Reader& outbound_stream );

  /* Send a TCPSenderMessage if needed (or empty optional otherwise) */
  std::optional<TCPSenderMessage> maybe_send();

  /* Generate an empty TCPSenderMessage */
  TCPSenderMessage send_empty_message() const;

  /* Receive an act on a TCPReceiverMessage from the peer's receiver */
  void receive( const TCPReceiverMessage& msg );

  /* Time has passed by the given # of milliseconds since the last time the tick() method was called. */
  void tick( uint64_t ms_since_last_tick );

  /* Accessors for use in testing */
  uint64_t sequence_numbers_in_flight() const;  // How many sequence numbers are outstanding?
  uint64_t consecutive_retransmissions() const; // How many consecutive *re*transmissions have happened?

private:
  // SYN包是否已发送
  bool syn_{ false };
  // FIN包是否已发送
  bool fin_{ false };
  // 窗口大小
  uint16_t window_size_{0};
  // 连续重传次数
  uint64_t retransmissions_num_{0};
  // 发送方已发送的,[0,1...recv_seqno_,...,next_seqno_-1]为已发送的, next_seqno_还未发送, 也表示发送方即将发送的报文序号
  uint64_t next_seqno_{0};
  // 已确认的序号, [0,1...recv_seqno_-1] 都确认了, 但是recv_seqno_这个还未确认
  uint64_t recv_seqno_{0};
  // 已经发送但是还未得到确认的字节数
  uint64_t outstanding_bytes_{0};
  // 已发送但是还未被确认的报文段
  std::deque<TCPSenderMessage>outstanding_messages_{};
  // 准备发送的报文段
  std::deque<TCPSenderMessage>messages_{};
  // 定时器
  RetransmitTimer timer{initial_RTO_ms_};

};
