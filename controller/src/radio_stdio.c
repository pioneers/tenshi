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

#include <stdio.h>
#include <sys/iosupport.h>

ssize_t radio_write(struct _reent *r, int fd, const char *ptr, size_t len) {
  // send_buf(ptr, len);
  return len;
}

static const devoptab_t dotab_stdout_radio = {
  "con_radio",    // name
  0,              // structSize

  NULL,           // open
  NULL,           // close
  radio_write,    // write
  NULL,           // read
  NULL,           // seek
  NULL,           // fstat
  NULL,           // stat
  NULL,           // link
  NULL,           // unlink
  NULL,           // chdir
  NULL,           // rename
  NULL,           // mkdir

  0,              // dirStateSize
  NULL,           // diropen
  NULL,           // dirreset
  NULL,           // dirnext
  NULL,           // dirclose
  NULL,           // statvfs
  NULL,           // ftruncate
  NULL,           // fsync

  NULL,           // deviceData

  NULL,           // chmod
  NULL,           // fchmod
};

void radio_stdio_init(void) {
  devoptab_list[STD_OUT] = &dotab_stdout_radio;

  // TODO(rqou): What does this do?
  setvbuf(stdout, NULL , _IONBF, 0);
}
