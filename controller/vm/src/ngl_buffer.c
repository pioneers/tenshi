#include <ngl_buffer.h>
#include <ngl_alloc.h>

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

ngl_buffer *
ngl_buffer_alloc(ngl_uint bytes) {
  return (ngl_buffer *) ngl_alloc_simple(uint8_t,
                                         bytes + sizeof(ngl_buffer) - 1);
}

ngl_buffer *
ngl_buffer_from_file(FILE * file) {
  if (file == NULL) {
    return NULL;
  }
  int64_t file_size = ngl_get_file_size(file);
  ngl_buffer *out = ngl_buffer_alloc(file_size);
  if (!out) {
    return NULL;
  }
  void *buffer_data = NGL_BUFFER_DATA(out);
  int64_t bytes_read = fread(buffer_data, sizeof(char), file_size, file);
  if (bytes_read != file_size) {
    ngl_free(out);
    return NULL;
  }
  return out;
}

ngl_buffer *
ngl_buffer_from_filename(ngl_str filename) {
  FILE *file = fopen(filename.start, "rb");
  ngl_buffer *out = ngl_buffer_from_file(file);
  if (file) {
    fclose(file);
  }
  return out;
}
