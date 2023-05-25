#include "network_interface.hh"

#include "arp_message.hh"
#include "ethernet_frame.hh"

using namespace std;

// ethernet_address: Ethernet (what ARP calls "hardware") address of the interface
// ip_address: IP (what ARP calls "protocol") address of the interface
NetworkInterface::NetworkInterface( const EthernetAddress& ethernet_address, const Address& ip_address )
  : ethernet_address_( ethernet_address ), ip_address_( ip_address )
{
  cerr << "DEBUG: Network interface has Ethernet address " << to_string( ethernet_address_ ) << " and IP address "
       << ip_address.ip() << "\n";
}

// dgram: the IPv4 datagram to be sent
// next_hop: the IP address of the interface to send it to (typically a router or default gateway, but
// may also be another host if directly connected to the same network as the destination)

// Note: the Address type can be converted to a uint32_t (raw 32-bit IP address) by using the
// Address::ipv4_numeric() method.
void NetworkInterface::send_datagram( const InternetDatagram& dgram, const Address& next_hop )
{
  const uint32_t next_ip_address = next_hop.ipv4_numeric();
  // arp表中存在并且还未过期, 组装成帧发送出去
  if(arp_table_.contains(next_ip_address) && (time_ - arp_table_[next_ip_address].second < 30*1000)){
    EthernetFrame frame;
    frame.header={arp_table_[next_ip_address].first,ethernet_address_,EthernetHeader::TYPE_IPv4};
    frame.payload=serialize(dgram);
    frames_out_.push_back(frame);
  }
  // 发送arp帧, 并保存一份数据报
  else{
    EthernetFrame arp_frame;
    arp_frame.header={ETHERNET_BROADCAST,ethernet_address_,EthernetHeader::TYPE_ARP};
    ARPMessage arp_message;
    arp_message.opcode = ARPMessage::OPCODE_REQUEST;
    arp_message.sender_ip_address=ip_address_.ipv4_numeric();
    arp_message.sender_ethernet_address=ethernet_address_;
    arp_message.target_ip_address = next_hop.ipv4_numeric();
    arp_frame.payload= serialize(arp_message);
    frames_out_.push_back(arp_frame);
    datagrams_out_.push_back({dgram,next_hop});
  }
}

// frame: the incoming Ethernet frame
optional<InternetDatagram> NetworkInterface::recv_frame( const EthernetFrame& frame )
{

  if(frame.header.dst != ETHERNET_BROADCAST && frame.header.dst != ethernet_address_){
    return{};
  }
  if(frame.header.type == EthernetHeader::TYPE_IPv4){
    InternetDatagram datagram;
    if(parse(datagram,frame.payload)){
      return datagram;
    }
  }
  else if(frame.header.type==EthernetHeader::TYPE_ARP){
    ARPMessage arp_message;
    if( parse(arp_message,frame.payload)){

        arp_table_[arp_message.sender_ip_address] = {arp_message.sender_ethernet_address,time_};

        // arp表更新了，看看有没有需要发的数据报
        for(auto it=datagrams_out_.begin();it!=datagrams_out_.end();){

          if(it->second.ipv4_numeric() == arp_message.sender_ip_address){

            send_datagram(it->first,it->second);
            it=datagrams_out_.erase(it);
          }
          else{
            ++it;
          }
        }

      //}
      if(arp_message.opcode==ARPMessage::OPCODE_REQUEST && arp_message.target_ip_address==ip_address_.ipv4_numeric()){
        EthernetFrame arp_frame;
        arp_frame.header={frame.header.src,ethernet_address_,EthernetHeader::TYPE_ARP};
        ARPMessage reply_message;
        reply_message.sender_ip_address = ip_address_.ipv4_numeric();
        reply_message.sender_ethernet_address = ethernet_address_;
        reply_message.target_ip_address = arp_message.sender_ip_address;
        reply_message.target_ethernet_address = arp_message.sender_ethernet_address;
        reply_message.opcode = ARPMessage::OPCODE_REPLY;
        arp_frame.payload= serialize(reply_message);
        frames_out_.push_back(arp_frame);
      }
    }
  }

  return {};

}

// ms_since_last_tick: the number of milliseconds since the last call to this method
void NetworkInterface::tick( const size_t ms_since_last_tick )
{
  time_+=ms_since_last_tick;
}

optional<EthernetFrame> NetworkInterface::maybe_send()
{
  if(frames_out_.empty()){
    return {};
  }
  EthernetFrame frame = frames_out_.front();
  // 数据帧或者是arp响应帧, 直接发就行了
  if(frame.header.type == EthernetHeader::TYPE_IPv4 || (frame.header.type == EthernetHeader::TYPE_ARP && frame.header.dst!=ETHERNET_BROADCAST)){
    frames_out_.pop_front();
    return frame;
  }
  // 如果是arp请求帧
  else if(frame.header.type == EthernetHeader::TYPE_ARP && frame.header.dst==ETHERNET_BROADCAST){
    // 检查前5秒内有没有发送过该ip的arp请求
    // 先解析出要发的ip
    ARPMessage arp_message;
    if( parse(arp_message,frame.payload)){
      if(!send_arp_time_.contains(arp_message.target_ip_address) || time_ - send_arp_time_[arp_message.target_ip_address] > 5*1000 ){
        // 可以发送arp帧
        frames_out_.pop_front();
        send_arp_time_[arp_message.target_ip_address] = time_;
        return frame;

      }
    }
  }
  return {};
}
