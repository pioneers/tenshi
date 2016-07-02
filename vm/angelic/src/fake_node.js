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

// If we're actually inside XULRunner.
if (typeof process === 'undefined') {
  var fake_process = require('sdk/system/process');
  var fake_require = (function (real_require) {
    return function (path) {
        // Put the original arguments into an Array we can slice.
        var args = Array.prototype.slice.call(arguments);

        // These are the node module replacements
        var replacements = {
          'fs': 'sdk/io/fs',
          'path': 'sdk/fs/path',
        };

        // If there's a replacement, use it.
        var replacement = replacements[path];
        if (replacement !== undefined) {
          return real_require.apply(null, [replacement].concat(args.slice(1)));
        }
        else {
          // Otherwise, pass all our arguments to the real require.
          return real_require.apply(null, args);
        }
      };
    })(require);
  // The host (Node.js or XULRunner) will not even create the needed real
  // variables if there is a normal declaration of the variable in this if
  // statement.
  // So hide the declaration inside an eval.
  eval('var process = fake_process;');
  eval('var require = fake_require;');
}
