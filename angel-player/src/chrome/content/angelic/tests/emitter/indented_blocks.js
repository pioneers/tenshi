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
var emitter = require ( root + '/emitter.js' );

var emit = emitter.make ( );

emit.begin_obj ( 'test_obj' );
  var label = emit.emit_label ( );
    emit.emit_li ( 'int', 1 );
    var label2 = emit.emit_label ( );
      emit.emit_li ( 'int', 1 );
  emit.emit_label_end ( label );
  emit.emit_li ( 'int', 1 );
emit.end_obj ( );

function line ( i, out ) {
  if ( i === 0 ) {
    return out;
    }
  else {
    return line ( i - 1, out.substr ( out.indexOf ( '\n' ) + 1 ) );
    }
  }

var o = emit.to_asm ( '', '  ' );

misc.print ( o );

misc.assert ( /  [^ ]/.test ( line ( 1, o ).substr ( 0, 3 ) ),
              'Indent should be 2 spaces.' );
misc.assert ( /    [^ ]/.test ( line ( 2, o ).substr ( 0, 5 ) ),
              'Indent should be 4 spaces.' );
misc.assert ( /      [^ ]/.test ( line ( 6, o ).substr ( 0, 7 ) ),
              'Indent should be 6 spaces.' );

var o = emit.to_asm ( '  ', '    ' );

misc.print ( o );

misc.assert ( /  [^ ]/.test ( line ( 1, o ) ),
              'Indent should be 2 spaces.' );
misc.assert ( /      [^ ]/.test ( line ( 2, o ) ),
              'Indent should be 6 spaces.' );
misc.assert ( /          [^ ]/.test ( line ( 6, o ) ),
              'Indent should be 10 spaces.' );
