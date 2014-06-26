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

// Possible block end caps:
// Flat face
exports.BLOCK_END_FLAT = 0;
// Arrow that points "into" the center of the block
exports.BLOCK_END_INNER_ARROW = 1;
// Arrow that points "out" away from the center of the block. This version is
// always visible.
exports.BLOCK_END_OUTER_ARROW_PERSISTENT = 2;
// Arrow that points "out" away from the center of the block. This version is
// not shown unless there is a nearby "inner arrow" being dragged at it.
exports.BLOCK_END_OUTER_ARROW_PROXIMITY = 3;

// A text comment
exports.BLOCK_TYPE_COMMENT = 0;
// Assignment lvalue, something like "foo = <>"
exports.BLOCK_TYPE_LVALUE = 1;
