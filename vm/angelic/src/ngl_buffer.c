// Licensed to Pioneers in Engineering under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  Pioneers in Engineering licenses
// this file to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
//  with the License.  You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License

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
                                         bytes + sizeof(ngl_buffer));
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
