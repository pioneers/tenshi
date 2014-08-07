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

/* jshint globalstrict: true */
"use strict";

let $scope;

const url = require ('sdk/url');
const fs = require('sdk/io/fs');
const yaml = require('js-yaml/lib/js-yaml.js');

const APIHELP_FILE = "chrome://angel-player/content/apihelp/api.yaml";

exports.apiHelpController = function(angular_app) {
  angular_app.controller('ApiHelpCtrl', ['$scope', function($scope_) {
    $scope = $scope_;

    let help_yaml_data = fs.readFileSync(url.toFilename(APIHELP_FILE));
    $scope.help_data = yaml.safeLoad(help_yaml_data.toString());
    dump($scope.help_data);

    $scope.fullname = function(item) {
      switch (item.type) {
        case 'method':
          return item.containing_type + ":" + item.name;
        case 'bound_field':
          return item.containing_type + "." + item.name;
        case 'function':
        case 'field':
          return item.module + "." + item.name;
        default:
          return item.name;
      }
    };
  }]);
};
