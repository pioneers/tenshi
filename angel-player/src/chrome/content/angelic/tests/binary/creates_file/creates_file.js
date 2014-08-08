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

var root = process.argv[2];
var binary = require ( root + '/binary.js' );
var misc = require ( root + '/misc.js' );
var buffer = require ( 'buffer' );

var b = new buffer.Buffer ( 0 );

binary.write_buffer ( b, 'creates.bin', function ( err ) {
  misc.assert ( false, 'Should not receive error when writing file!' );
  } );
