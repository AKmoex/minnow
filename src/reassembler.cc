#include "reassembler.hh"
#include<iostream>

using namespace std;

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring, Writer& output )
{
  // Your code here.

  // first_unassembled_index 表示的是下一个期望被push进Stream的index
  // first_unassembled_index = 10, 表示下次推进去的数据应该从 index = 10 开始
  const uint64_t first_unassembled_index = output.bytes_pushed();

  // first_unacceptable_index = 10, 表示当 index 大于等于 10 时丢弃
  const uint64_t first_unacceptable_index = first_unassembled_index + output.available_capacity();

  if(is_last_substring){
    total_bytes_ = first_index + data.size();
  }

  if( first_index >= first_unacceptable_index || first_index + data.size() <= first_unassembled_index ){
    check_close(output);
    return ;
  }

  // 字符串裁剪
  uint64_t begin_index = first_index;
  uint64_t end_index = first_index + data.size()-1;

  if( begin_index < first_unassembled_index ){
    begin_index = first_unassembled_index;
  }
  if( end_index > first_unacceptable_index ){
    end_index = first_unacceptable_index;
  }

  update_buffer( begin_index, end_index, first_index, first_unassembled_index, data );

  string write_str = "";
  while( !flag_.empty() && flag_.front() ){
    write_str += buffer_.front();
    flag_.pop_front();
    buffer_.pop_front();
  }
  if( write_str.length() > 0 ){
    output.push(write_str);
    unassembled_bytes_ -= write_str.length();
  }

  check_close(output);
}

uint64_t Reassembler::bytes_pending() const
{
  // Your code here.
  return unassembled_bytes_;
}

void Reassembler::update_buffer( uint64_t begin_index, uint64_t end_index, uint64_t first_index, uint64_t first_unassembled_index, std::string data )
{

  while( first_unassembled_index + buffer_.size() < begin_index ){
    // 随便push点数据都行
    buffer_.push_back('\0');
    // 标记这块数据是无用的
    flag_.push_back( false);
  }

  for( uint64_t i = begin_index; i <= end_index; i++ ){
    if( i >= buffer_.size() + first_unassembled_index ){
      buffer_.push_back(data[i-first_index]);
      flag_.push_back( true);
      unassembled_bytes_ ++;
    }
    else if(!flag_[i - first_unassembled_index]){
      buffer_[i - first_unassembled_index] = data[i-first_index];
      flag_[i - first_unassembled_index] = true;
      unassembled_bytes_ ++;
    }
  }
}

void Reassembler::check_close( Writer& output )
{
  if( output.bytes_pushed() == total_bytes_){
    output.close();
  }
}