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

var buffer = require ( 'tenshi/angelic/buffer.js' );
var misc = require ( 'tenshi/angelic/misc.js' );
var factory = require ( 'tenshi/common/factory.js' );
var url = require ( 'sdk/url' );

exports.run = function() {
  var fact = factory.make ( );
  fact.set_target_type ( 'ARM' );
  fact.load_type_file ( url.toFilename ( 
        'chrome://angel-player/content/common_defs/ngl_types.yaml' ) );

  var func = fact.create ( 'ngl_vm_func' );
  func.set_slot ( 'header', { 'refc' : 1, 'type' : 0xff } );
  func.set_slot ( 'code', 0xbee );
  func.set_slot ( 'stack_space', 10 );

  var buf = buffer.Buffer ( fact.get_size ( 'ngl_vm_func' ) );
  func.write ( buf );

  misc.assert ( buf.readUInt32LE ( 0 ) === 1, 'refc should be 1' );
  misc.assert ( buf.readUInt32LE ( 4 ) === 0xff, 'type should be 0xff' );
  misc.assert ( buf.readUInt32LE ( 8 ) === 0xbee, 'code should be 0xbee' );
  misc.assert ( buf.readUInt32LE ( 12 ) === 10, 'stack_space should be 10' );

  return true;
};
