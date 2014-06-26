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

/* jshint globalstrict: true */
"use strict";
const buffer = require('sdk/io/buffer');

// Allocates memory in an emscripten runtime instance for a
// buffer, and copies the buffer contents into that memory.
// free must be used to free the ptr returned by this function, or
// memory will be leaked in that emscripten instance.
exports.buffer_to_ptr = function (emcc, buf, length) {
  if (length === undefined) {
    length = buf.length;
  }
  var mem = emcc.ccall('malloc', 'number', ['number'], [length]);
  if ( mem === 0 ) {
    throw 'Could not allocate memory for buffer copy.';
  }
  for (var i = 0; i < length; i++) {
    emcc.setValue ( mem + i, buf [ i ], 'i8' );
  }
  return mem;
};

// Copy a block of memory starting at <ptr> with length <length>
// into <buf>. If <buf> is not given, a buffer of the appropriate
// length will be constructed.
// Returns the buffer populated with the contents from memory.
// ptr is not freed in the process.
exports.ptr_to_buffer = function (emcc, ptr, length, buf, offset) {
  if (buf === undefined) {
    buf = buffer.Buffer(length);
  }
  if (offset === undefined) {
    offset = 0;
  }
  if (ptr === 0) {
    throw 'Attempt to copy from NULL ptr to buffer.';
  }
  for (var i = 0; i < length; i++) {
    buf[i + offset] = emcc.getValue(ptr + i, 'i8');
  }
  return buf;
};

// Read a ptr at address <ptr_ptr> from an emscripten instance
// <emcc>.
exports.get_ptr = function (emcc, ptr_ptr) {
  return emcc.getValue(ptr_ptr, 'i32');
};

// Write a ptr at address <ptr_ptr> from an emscripten instance
// <emcc>.
exports.set_ptr = function (emcc, ptr_ptr, ptr) {
  return emcc.setValue(ptr_ptr, ptr, 'i32');
};

// Size of a pointer in bytes in emscripten. Useful for allocating
// arrays of (pointer) pointers.
exports.PTR_SIZE = 4;
