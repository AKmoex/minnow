#include <stdexcept>

#include "byte_stream.hh"

using namespace std;

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ) {}

void Writer::push( string data )
{
  // Your code here.
  size_t length = min(data.length(), capacity_-buffer_.size());
  for(size_t i = 0;i < length;i ++){
    buffer_.emplace_back(data[i]);
  }
  total_pushed_bytes_ += length;
}

void Writer::close()
{
  // Your code here.
  is_closed_ = true;
}

void Writer::set_error()
{
  // Your code here.
  is_error_ = true;
}

bool Writer::is_closed() const
{
  // Your code here.
  return is_closed_;
}

uint64_t Writer::available_capacity() const
{
  // Your code here.
  return capacity_ - buffer_.size();
}

uint64_t Writer::bytes_pushed() const
{
  // Your code here.
  return total_pushed_bytes_;
}

string_view Reader::peek() const
{
  // Your code here.
  return string_view(&buffer_.front(), 1);
}

bool Reader::is_finished() const
{
  // Your code here.
  return is_closed_ && buffer_.empty();
}

bool Reader::has_error() const
{
  // Your code here.
  return is_error_;
}

void Reader::pop( uint64_t len )
{
  // Your code here.
  const size_t length = min(len,buffer_.size());
  for(size_t i = 0; i < length; i ++){
    buffer_.pop_front();
  }
  total_popped_bytes_ += length;
}

uint64_t Reader::bytes_buffered() const
{
  // Your code here.
  return buffer_.size();
}

uint64_t Reader::bytes_popped() const
{
  // Your code here.
  return total_popped_bytes_;
}
