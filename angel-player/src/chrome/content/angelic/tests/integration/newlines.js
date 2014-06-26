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
var misc = require ( root + '/misc.js' );
var angelic = require ( root + '/main.js' );

var worked = 0;

function compile_and_run ( text ) {
  var vm = angelic.make ( );
  vm.set_common_defs_path(root + '/../common_defs');
  vm.add_library ( 'core', [
    vm.make_exfn ( 0, 'print', misc.print ),
    vm.make_exfn ( 1, 'set_worked', function ( val ) {
      worked = val;
      } ),
    ] );
  vm.load_text ( text );
  vm.start_main ( );
  vm.set_evaluation_rate ( 10000, 0.01 );
  return vm;
  }

var code = '' +
'\n' +
'\n' +
'test = fn:\n' +
'    print(1)\n' +
'\n' +
'\n' +
'main = fn:\n' +
'    set_worked(1)\n' +
'';

compile_and_run ( code );

setTimeout ( function ( ) {
  if ( worked == 1 ) {
    process.exit ( 0 );
    }
  else {
    process.exit ( 1 );
    }
  }, 1 );
