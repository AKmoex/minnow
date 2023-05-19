#include "wrapping_integers.hh"
#include <iostream>
using namespace std;


Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{

  uint32_t seqno = (uint32_t)((n + zero_point.raw_value_) % ((uint64_t)1<<32));

  return Wrap32 { seqno };
}

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{
  uint64_t offset = raw_value_ - zero_point.raw_value_;
  if(offset < checkpoint){
    cout<<offset<<endl;
    uint64_t mod_num = (checkpoint - offset + ((uint64_t)1<<31)) / ((uint64_t)1<<32);
    offset += mod_num*((uint64_t)1<<32);
  }
  return offset;
}
