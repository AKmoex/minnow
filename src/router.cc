#include "router.hh"

#include <iostream>
#include <limits>

using namespace std;

// route_prefix: The "up-to-32-bit" IPv4 address prefix to match the datagram's destination address against
// prefix_length: For this route to be applicable, how many high-order (most-significant) bits of
//    the route_prefix will need to match the corresponding bits of the datagram's destination address?
// next_hop: The IP address of the next hop. Will be empty if the network is directly attached to the router (in
//    which case, the next hop address should be the datagram's final destination).
// interface_num: The index of the interface to send the datagram out on.
void Router::add_route( const uint32_t route_prefix,
                        const uint8_t prefix_length,
                        const optional<Address> next_hop,
                        const size_t interface_num )
{
  cerr << "DEBUG: adding route " << Address::from_ipv4_numeric( route_prefix ).ip() << "/"
       << static_cast<int>( prefix_length ) << " => " << ( next_hop.has_value() ? next_hop->ip() : "(direct)" )
       << " on interface " << interface_num << "\n";

  route_table_.push_back({route_prefix, prefix_length, next_hop, interface_num});
}

void Router::route() {
  for(AsyncNetworkInterface &anInterface : interfaces_){


    optional<InternetDatagram> option_dgram = anInterface.maybe_receive();
    if(option_dgram.has_value()){
      InternetDatagram dgram = option_dgram.value();
      auto matched_route = route_table_.end();
      for(auto it = route_table_.begin();it!= route_table_.end();it++){
        if(it->prefix_length==0 ||(it->route_prefix ^ dgram.header.dst) >> (32 - it->prefix_length) ==0 ){
          if(matched_route == route_table_.end() || matched_route->prefix_length < it->route_prefix){
            matched_route = it;
          }
        }
      }
      if(matched_route!=route_table_.end()){
        if(dgram.header.ttl>1){
          dgram.header.ttl--;
          dgram.header.compute_checksum();
          const optional<Address> next_hop = matched_route->next_hop;
          if(next_hop.has_value()){
            interface(matched_route->interface_num).send_datagram(dgram,next_hop.value());
          }
          else{
            interface(matched_route->interface_num).send_datagram(dgram,Address::from_ipv4_numeric(dgram.header.dst));
          }
        }
      }
    }


//    optional<InternetDatagram> option_dgram = anInterface.maybe_receive();
//    while(option_dgram.has_value()){
//      InternetDatagram dgram = option_dgram.value();
//      auto matched_route = route_table_.end();
//      for(auto it = route_table_.begin();it!= route_table_.end();it++){
//        if(it->prefix_length==0 ||(it->route_prefix ^ dgram.header.dst) >> (32 - it->prefix_length) ==0 ){
//          if(matched_route == route_table_.end() || matched_route->prefix_length < it->route_prefix){
//            matched_route = it;
//          }
//        }
//      }
//      if(matched_route!=route_table_.end()){
//        if(dgram.header.ttl>1){
//          dgram.header.ttl--;
//          dgram.header.compute_checksum();
//
//          const optional<Address> next_hop = matched_route->next_hop;
//          if(next_hop.has_value()){
//            interface(matched_route->interface_num).send_datagram(dgram,next_hop.value());
//          }
//          else{
//            interface(matched_route->interface_num).send_datagram(dgram,Address::from_ipv4_numeric(dgram.header.dst));
//          }
//        }
//      }
//      option_dgram = anInterface.maybe_receive();
//    }

  }
}
