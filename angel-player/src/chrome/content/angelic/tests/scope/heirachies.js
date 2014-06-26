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
var scope = require ( root + '/scope.js' );

function heirarchies_work ( ) {
  var p = scope.make ( );
  var p2 = scope.make ( p );
  var c = scope.make ( p2 );
  p.set_text ( 'a', 1 );
  misc.assert ( c.get_text ( 'a' ) === 1 );

  c.set_text ( 'b', 2 );
  misc.assert ( p.get_text ( 'b' ) === undefined );

  p.set_text ( 'c', 3 );
  c.set_text ( 'c', 4 );
  misc.assert ( p.get_text ( 'c' ) === 3 );

  misc.assert ( p.is_above ( c ) );
  misc.assert ( p2.is_above ( c ) );
  }

heirarchies_work ();
