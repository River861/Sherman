#if !defined(_RDMA_BUFFER_H_)
#define _RDMA_BUFFER_H_

#include "Common.h"

// abstract rdma registered buffer
class RdmaBuffer {

private:
  static const int kPageBufferCnt = 128;    // async, buffer safty  [DEBUG] 64
  static const int kSiblingBufferCnt = 16; // async, buffer safty
  static const int kCasBufferCnt = 128;     // async, buffer safty  [DEBUG] 16

  char *buffer;

  uint64_t *cas_buffer;
  uint64_t *unlock_buffer;
  uint64_t *zero_64bit;
  char *page_buffer;
  char *sibling_buffer;
  char *range_buffer;

  int page_buffer_cur;
  int sibling_buffer_cur;
  int cas_buffer_cur;

  int kPageSize;

public:
  RdmaBuffer(char *buffer) {
    set_buffer(buffer);

    page_buffer_cur = 0;
    sibling_buffer_cur = 0;
    cas_buffer_cur = 0;
  }

  RdmaBuffer() = default;

  void set_buffer(char *buffer) {

    // printf("set buffer %p\n", buffer);

    kPageSize = std::max(kLeafPageSize, kInternalPageSize);
    this->buffer = buffer;
    cas_buffer = (uint64_t *)buffer;
    unlock_buffer =
        (uint64_t *)((char *)cas_buffer + sizeof(uint64_t) * kCasBufferCnt);
    zero_64bit = (uint64_t *)((char *)unlock_buffer + sizeof(uint64_t));
    page_buffer = (char *)zero_64bit + sizeof(uint64_t);
    sibling_buffer = (char *)page_buffer + kPageSize * kPageBufferCnt;
    range_buffer = (char *)sibling_buffer + kPageSize * kSiblingBufferCnt;
    char * border = (char*) range_buffer + kPageSize * 16;
    *zero_64bit = 0;

    assert((char *)border - buffer < define::kPerCoroRdmaBuf);
  }

  uint64_t *get_cas_buffer() {
    cas_buffer_cur = (cas_buffer_cur + 1) % kCasBufferCnt;
    return cas_buffer + cas_buffer_cur;
  }

  uint64_t *get_unlock_buffer() const { return unlock_buffer; }

  uint64_t *get_zero_64bit() const { return zero_64bit; }

  char *get_page_buffer() {
    page_buffer_cur = (page_buffer_cur + 1) % kPageBufferCnt;
    return page_buffer + (page_buffer_cur * kPageSize);
  }

  char *get_range_buffer() {
    return range_buffer;
  }

  char *get_sibling_buffer() {
    sibling_buffer_cur = (sibling_buffer_cur + 1) % kSiblingBufferCnt;
    return sibling_buffer + (sibling_buffer_cur * kPageSize);
  }

};

#endif // _RDMA_BUFFER_H_
