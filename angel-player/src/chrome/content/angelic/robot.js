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

var misc = require ( './misc.js' );
var angelic = require ( './main.js' );

//
// This is the main Angelic module
//

function compile_and_run ( text, robot ) {
  var vm = angelic.make ( );
  function set_motor ( port, val ) {
    robot.motors [ port ] = val;
    robot.version += 1;
    }
  function get_sensor ( port ) {
    return robot.sensors [ port ];
    }
  vm.add_library ( 'core', [
    vm.make_exfn ( 0, 'print', misc.print ),
    vm.make_exfn ( 1, 'set_motor', set_motor ),
    vm.make_exfn ( 2, 'get_sensor', get_sensor ),
    ] );
  vm.load_text ( text );
  vm.start_main ( );
  vm.set_evaluation_rate ( 100, 0.01 );
  return vm;
  }

exports.compile_and_run = compile_and_run;
exports.make = angelic.make;
