#include "tcp_receiver.hh"

using namespace std;

void TCPReceiver::receive( TCPSenderMessage message, Reassembler& reassembler, Writer& inbound_stream )
{
  if(message.SYN){
    zero_point = message.seqno;
    SYN = true;
  }
  if(!SYN){
    return ;
  }
  if(message.FIN){
    FIN = true;
  }

  uint64_t checkpoint = inbound_stream.bytes_pushed() + 1;
  uint64_t index = message.seqno.unwrap(zero_point,checkpoint);

  // index+SYN-1, 如果这个包是 SYN 包, 那么 index 应该为 0
  reassembler.insert(index+message.SYN-1,message.payload,message.FIN,inbound_stream);

}

TCPReceiverMessage TCPReceiver::send( const Writer& inbound_stream ) const
{
  TCPReceiverMessage message{};
  const uint64_t cap = inbound_stream.available_capacity();

  message.window_size = cap > 65535 ? 65535 : cap;
  if (SYN){
    uint64_t next_index = inbound_stream.bytes_pushed() + 1;
    if(FIN && inbound_stream.is_closed()){
      next_index+=1;
    }
    message.ackno = Wrap32::wrap(next_index,zero_point);
  }
  else{
    message.ackno = nullopt;
  }
  return message;
}
