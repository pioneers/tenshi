#include <ngl_utf8.h>
#include <ngl_error.h>
#include <ngl_compatibility.h>
#include <stdint.h>

const int8_t utf8_length[256] = {
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 
  4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 0, 0, 
  };

ngl_error ngl_invalid_utf8;

ngl_error * ngl_utf8_get (uint32_t * dst, const char ** c, const char * past_end) {
  const uint8_t * b = (uint8_t *) *c;
  int length = utf8_length[*b];
  if (*c + length > past_end) {
    return &ngl_invalid_utf8;
    }
  switch (length) {
  case 0:
    return &ngl_invalid_utf8;
  case 1:
    *dst = b[0];
    *c += 1;
    return ngl_ok;
  case 2:
    if (b[1] & 0x40) {
      return &ngl_invalid_utf8;
      }
    *dst = (b[0] & 0x1f) << 6 | (b[1] & 0x3f);
    *c += 2;
    return ngl_ok;
  case 3:
    if (b[1] & 0x40 || b[2] & 0x40) {
      return &ngl_invalid_utf8;
      }
    *dst = ((b[0] & 0x0f) << 12) |
           ((b[1] & 0x3f) <<  6) |
           ((b[2] & 0x3f));
    *c += 3;
    return ngl_ok;
  case 4:
    if (b[1] & 0x40 || b[2] & 0x40 || b[3] & 0x40) {
      return &ngl_invalid_utf8;
      }
    *dst = ((b[0] & 0x07) << 18) |
           ((b[1] & 0x3f) << 12) |
           ((b[2] & 0x3f) <<  6) |
           ((b[3] & 0x3f));
    *c += 4;
    return ngl_ok;
  case 5:
    if (b[1] & 0x40 || b[2] & 0x40 || b[3] & 0x40 || b[4] & 0x40) {
      return &ngl_invalid_utf8;
      }
    *dst = ((b[0] & 0x03) << 24) |
           ((b[1] & 0x3f) << 18) |
           ((b[2] & 0x3f) << 12) |
           ((b[3] & 0x3f) <<  6) |
           ((b[4] & 0x3f));
    *c += 5;
    return ngl_ok;
  case 6:
    if (b[1] & 0x40 || b[2] & 0x40 || b[3] & 0x40 || b[4] & 0x40 ||
        b[5] & 0x40) {
      return &ngl_invalid_utf8;
      }
    *dst = ((b[0] & 0x01) << 30) |
           ((b[1] & 0x3f) << 24) |
           ((b[2] & 0x3f) << 18) |
           ((b[3] & 0x3f) << 12) |
           ((b[4] & 0x3f) <<  6) |
           ((b[5] & 0x3f));
    *c += 6;
    return ngl_ok;
  default:
    unreachable();
    }
  }

ngl_error * ngl_utf8_valid_code_point (const char ** c, const char * past_end) {
  const uint8_t * b = (uint8_t *) *c;
  int length = utf8_length[*b];
  if (*c + length > past_end) {
    return &ngl_invalid_utf8;
    }
  switch (length) {
  case 0:
    return &ngl_invalid_utf8;
  case 1:
    *c += 1;
    return ngl_ok;
  case 2:
    if (b[1] & 0x40) {
      return &ngl_invalid_utf8;
      }
    *c += 2;
    return ngl_ok;
  case 3:
    if (b[1] & 0x40 || b[2] & 0x40) {
      return &ngl_invalid_utf8;
      }
    *c += 3;
    return ngl_ok;
  case 4:
    if (b[1] & 0x40 || b[2] & 0x40 || b[3] & 0x40) {
      return &ngl_invalid_utf8;
      }
    *c += 4;
    return ngl_ok;
  case 5:
    if (b[1] & 0x40 || b[2] & 0x40 || b[3] & 0x40 || b[4] & 0x40) {
      return &ngl_invalid_utf8;
      }
    *c += 5;
    return ngl_ok;
  case 6:
    if (b[1] & 0x40 || b[2] & 0x40 || b[3] & 0x40 || b[4] & 0x40 ||
        b[5] & 0x40) {
      return &ngl_invalid_utf8;
      }
    *c += 6;
    return ngl_ok;
  default:
    unreachable();
    }
  }

bool ngl_utf8_valid (ngl_str * str) {
  const char * c = str->start;
  while (c != str->past_end) {
    if (ngl_utf8_valid_code_point (&c, str->past_end) != ngl_ok) {
      return false;
      }
    }
  return true;
  }
