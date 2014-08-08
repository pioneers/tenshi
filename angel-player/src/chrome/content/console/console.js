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

exports.consoleController = function(angular_app) {
  angular_app.controller('ConsoleCtrl', ['$scope', function($scope_) {
    $scope = $scope_;
    $scope.text = "";

    $scope.aceLoaded = function(_editor) {
      _editor.setReadOnly(true);
      _editor.setHighlightActiveLine(false);
      _editor.setDisplayIndentGuides(false);
      _editor.renderer.setPrintMarginColumn(false);
    };
  }]);
};

exports.report_error = function(str) {
  if (!$scope) return;

  $scope.text += "\nERROR: " + str;
};

exports.report_warning = function(str) {
  if (!$scope) return;

  $scope.text += "\nWARN: " + str;
};

exports.report_message = function(str) {
  if (!$scope) return;

  $scope.text += "\n" + str;
};
