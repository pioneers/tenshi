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
var Int64 = require ( 'tenshi/vendor-js/Int64.js' );

exports.run = function() {
  var fact = factory.make ( );
  fact.set_target_type ( 'ARM' );
  fact.load_type_file ( url.toFilename ( 
        'chrome://angel-player/content/common_defs/xbee_typpo.yaml' ) );

  var addr = new Int64('0xfedcba8987654321');

  var xb_pay_out = fact.create ( 'xbee_payload' );
  xb_pay_out.set_slot ( 'tx64', {
    xbee_api_type : 0xa,
    frameId : 0xb,
    xbee_dest_addr : addr,
    options : 0xc,
    data : buffer.Buffer ( [ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 ] ),
  } );

  var size = xb_pay_out.get_size ( );

  var buf = buffer.Buffer ( size );
  buf.fill ( 0xff );

  xb_pay_out.write ( buf );

  var xb_pay_in = fact.create ( 'xbee_payload' );
  xb_pay_in.read ( buf );

  var res = xb_pay_in.unwrap ( );

  // TODO(kzentner): Figure out a way to make the new test system non-verbose 
  // by default.
  // misc.print ( res );

  misc.assert ( res.bytes.length === size, 'bytes should have length ' + size );

  misc.assert ( res.bytes [ 0 ] === 0xa, 'bytes [ 0 ] should be correct' );
  misc.assert ( res.bytes [ 1 ] === 0xb, 'bytes [ 1 ] should be correct' );
  misc.assert ( res.bytes [ 10 ] === 0xc, 'bytes [ 10 ] should be correct' );

  var i;
  for ( i = 0; i < 11; i++ ) {
    misc.assert ( res.bytes [ 11 + i ] === i, 'bytes [ ' + i + ' ] should be correct' );
  }

  misc.assert ( res.tx64.xbee_dest_addr.toString ( ) === addr.toString ( ), 'xbee_dest_addr should be correct' );

  return true;
};
