#include <string.h>
#include <assert.h>

#include <ngl_str.h>
#include <ngl_type.h>
#include <ngl_alloc.h>
#include <ngl_hash.h>

size_t
ngl_str_size(const ngl_str str) {
  return str.past_end - str.start;
}

ngl_str
ngl_str_from_static(const char *str) {
  ngl_str out;
  ngl_obj_init(&out.header, ngl_type_ngl_str);
  out.start = str;
  out.past_end = str + strlen(str);
  out.alloc_type = ngl_str_static;
  return out;
}

ngl_str
ngl_str_from_dynamic(const char *dynamic) {
  ngl_str out;
  ngl_obj_init(&out.header, ngl_type_ngl_str);
  out.start = dynamic;
  out.past_end = dynamic + strlen(dynamic);
  out.alloc_type = ngl_str_dynamic;
  return out;
}

static int64_t
ngl_get_file_size(FILE * file) {
  fpos_t start_position;
  int64_t file_size;
  fgetpos(file, &start_position);
  fseek(file, 0, SEEK_END);
  file_size = ftell(file);
  fsetpos(file, &start_position);
  return file_size;
}

ngl_str
ngl_str_from_file(FILE * file) {
  ngl_str out;
  ngl_obj_init(&out.header, ngl_type_ngl_str);
  out.alloc_type = ngl_str_dynamic;
  if (file == NULL) {
    out.start = NULL;
    out.past_end = NULL;
  } else {
    int64_t to_read = ngl_get_file_size(file);
    uint8_t *buffer = malloc(to_read + 1);
    if (buffer) {
      int64_t bytes_read =
        fread((void *) buffer, sizeof(char), to_read, file);
      assert(bytes_read == to_read);
      out.past_end = (const char *) buffer + bytes_read;
      buffer[bytes_read] = '\0';
      out.start = (const char *) buffer;
    } else {
      out.start = NULL;
      out.past_end = NULL;
    }
  }
  return out;
}

ngl_str
ngl_str_from_filename(ngl_str filename) {
  FILE *file = fopen(filename.start, "r");
  ngl_str out = ngl_str_from_file(file);
  if (file) {
    fclose(file);
  }
  return out;
}

/* Count the number of UTF-8 code points in <str>. */
size_t
ngl_str_code_length(const ngl_str str) {
  size_t count = 0;
  const unsigned char *cur = (unsigned char *) str.start;
  const unsigned char *ascii_start = cur;
  const unsigned char *past_end = (unsigned char *) str.past_end;

  while (cur < past_end && *cur <= 127)
  ascii:++cur;

  count += cur - ascii_start;
  while (cur < past_end) {
    if (*cur <= 127) {
      ascii_start = cur;
      goto ascii;
    } else {
      switch (0xF0 & *cur) {
      case 0xE0:
        cur += 3;
        break;
      case 0xF0:
        cur += 4;
        break;
      default:
        cur += 2;
        break;
      }
    }
    ++count;
  }
  return count;
}

ngl_int
ngl_str_compare(ngl_str a, ngl_str b) {
  size_t a_size = ngl_str_size(a);
  size_t b_size = ngl_str_size(b);
  if (a_size == b_size) {
    return memcmp((const void *) a.start, (const void *) b.start, a_size);
  } else if (a_size > b_size) {
    int result = memcmp(a.start, b.start, b_size);
    if (result == 0) {
      return 1;
    } else {
      return result;
    }
  } else {                      /* if (b_size > a_size) */
    int result = memcmp(a.start, b.start, a_size);
    if (result == 0) {
      return -1;
    } else {
      return result;
    }
  }
}

void
ngl_str_init_dynamic(ngl_str * to_init) {
  ngl_obj_init(&to_init->header, ngl_type_ngl_str);
  to_init->alloc_type = ngl_str_dynamic;
}

void
ngl_str_print(ngl_str str) {
  for (const char *c = str.start; c != str.past_end; c++) {
    putc(*c, stdout);
  }
}

void
ngl_str_println(ngl_str str) {
  ngl_str_print(str);
  printf("\n");
}

ngl_hash
ngl_hash_ngl_str_ptr(ngl_val str) {
  ngl_str self = *(ngl_str *) str.pointer;
  return ngl_str_hash(self);
}

ngl_hash
ngl_str_hash(ngl_str self) {
  return siphash_24(ngl_hash_key, (uint8_t *) self.start, ngl_str_size(self));
}

ngl_int
ngl_compare_ngl_str_ptr(ngl_val v_str_a, ngl_val v_str_b) {
  ngl_str str_a = *(ngl_str *) v_str_a.pointer;
  ngl_str str_b = *(ngl_str *) v_str_b.pointer;
  ngl_int res = ngl_str_compare(str_a, str_b);
  return res;
}

ngl_table_elem_interface ngl_str_table_i = {
  &ngl_hash_ngl_str_ptr,
  &ngl_compare_ngl_str_ptr
} END

void
ngl_str_init_alloc(ngl_str * self, const char *start, const char *past_end,
                   enum ngl_str_alloc_type alloc_type) {
  if ((!past_end) && start) {
    past_end = start;
    while (*past_end) {
      past_end++;
    }
  }
  ngl_obj_init(&self->header, ngl_type_ngl_str);
  self->alloc_type = alloc_type;
  self->start = start;
  self->past_end = past_end;
}

void
ngl_str_init(ngl_str * self, const char *start, const char *past_end) {
  ngl_str_init_alloc(self, start, past_end, ngl_str_dynamic);
}

ngl_str *
ngl_str_new_alloc(const char *start, const char *past_end,
                  enum ngl_str_alloc_type alloc_type) {
  ngl_str *to_return = ngl_alloc_simple(ngl_str, 1);
  if (!to_return) {
    return NULL;
  }
  ngl_str_init_alloc(to_return, start, past_end, alloc_type);
  return to_return;
}

ngl_str *
ngl_str_new(const char *start, const char *past_end) {
  return ngl_str_new_alloc(start, past_end, ngl_str_dynamic);
}

ngl_str *
ngl_str_from_int64(int64_t input) {
  char *text = malloc(sizeof(char) * 40);
  snprintf(text, 40, "%" PRId64, input); /* NOLINT(runtime/printf) */
  return ngl_str_new(text, NULL);
}

ngl_str *
ngl_str_copy(ngl_str * original) {
  size_t size = ngl_str_size(*original);
  ngl_str *str = ngl_alloc_simple(ngl_str, 1);
  char *buf = malloc(size);
  if (!str || !buf) {
    free(str);
    free(buf);
    return NULL;
  }
  memcpy(buf, original->start, size);
  ngl_obj_init(&str->header, ngl_type_ngl_str);
  str->start = buf;
  str->past_end = buf + size;
  str->alloc_type = ngl_str_dynamic;
  return str;
}

ngl_str
ngl_str_empty(void) {
  ngl_str to_return;
  ngl_str_init_dynamic(&to_return);
  to_return.start = NULL;
  to_return.past_end = NULL;
  return to_return;
}

void
ngl_str_free(ngl_str * str) {
  if (str) {
    if (str->alloc_type == ngl_str_dynamic) {
      free((void *) str->start);
    }
    free(str);
  }
}
