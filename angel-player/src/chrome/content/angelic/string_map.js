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

//
// This module provides a simple string -> obj map.
//

var make = function make ( obj ) {
  var storage = {};

  if ( obj !== undefined ) {
    for ( var k in obj ) {
      storage[k + '$'] = obj[k];
      }
    }
  var table = {
    // Get the value, or set it to alternative.
    'get': function ( key, alternative ) {
      var val = storage[key + '$'];
      if ( val === undefined ) {
        this.set ( key, alternative );
        }
      return storage[key + '$'];
      },
    'set': function ( key, val ) {
      storage[key + '$'] = val;
      return this;
      },
    'has': function ( key ) {
      return key + '$' in storage;
      },
    'delete': function ( key ) {
      return delete storage [ key + '$' ];
      },
    // Update values in place using a function.
    'map': function ( func ) {
      var self = this;

      this.each ( function ( key, val ) {
        self.set ( key, func ( key, val ) );
        } );
      return this;
      },
    // Run a function on every key, value pair.
    'each': function ( func ) {
      var key;
      var val;

      for ( key in storage ) {
        val = storage[key];
        key = key.substr ( 0, key.length - 1 );
        func ( key, val );
        }
      return this;
      },
    'toString': function ( val_str ) {
      var out = '{';


      if ( val_str === undefined ) {
        val_str = function ( val ) {
          return '' + val;
          //return JSON.stringify ( val, null, '  ' );
          };
        }
      this.each ( function ( key, val ) {
        out += '' + key + ' : ' + val_str ( val ) + ', ';
        } );

      out += '}';
      return out;
      },
    };
  return misc.obj_or ( Object.create ( table ), {
    storage: storage,
    } );
  };

exports.make = make;
