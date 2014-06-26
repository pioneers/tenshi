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

  var pkg_header = fact.create ( 'ngl_package' );
  var magic = 0x008fa5e5;

  pkg_header.set_slot ( 'version', 1 );
  pkg_header.set_slot ( 'fixup_table_offset', 0 );
  pkg_header.set_slot ( 'patch_table_offset', 0 );
  pkg_header.set_slot ( 'magic', magic );

  var unwrapped = pkg_header.unwrap ( );

  misc.assert ( unwrapped.version === 1,
                'version should be set correctly.' );
  misc.assert ( unwrapped.magic === magic,
                'magic should be set correctly.' );
  misc.assert ( unwrapped.fixup_table_offset === 0,
                'fixup table offset should be set correctly.' );
  misc.assert ( unwrapped.patch_table_offset === 0,
                'patch table offset should be set correctly.' );
  return true;
};
