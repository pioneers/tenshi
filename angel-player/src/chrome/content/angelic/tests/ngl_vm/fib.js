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

function compile_and_run ( text ) {
  var vm = angelic.make ( );
  vm.set_common_defs_path ( root + '/../common_defs' );
  vm.add_library ( 'core', [
    vm.make_exfn ( 0, 'print_float', misc.print ),
    ] );
  vm.load_text ( text );
  vm.save_pkg ( 'x86_64', 'fib64.ngl_pkg', misc.print );
  vm.save_pkg ( 'ARM', 'fib32.ngl_pkg', misc.print );
  return vm;
  }

var code = '' +
'fib = fn (n):\n' +
'    n = n - 1\n' +
'    a = 0\n' +
'    b = 1\n' +
'    while n != 0:\n' +
'        temp = a + b\n' +
'        a = b\n' +
'        b = temp\n' +
'        n = n - 1\n' +
'    return b\n' +
'main = fn:\n' +
'    x = 50\n' +
'    print_float (x)\n' +
'    fib50 = fib(x)\n' +
'    print_float (fib50)\n' +
'';

compile_and_run ( code );
