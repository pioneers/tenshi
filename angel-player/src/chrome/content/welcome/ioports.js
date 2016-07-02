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

exports.ioportController = function(angular_app) {
  angular_app.controller('IoPortCtrl', ['$scope', function($scope_) {
    $scope = $scope_;
    $scope.xbeeAddr = "";
    $scope.serialPort = "";
  }]);
};

exports.get_xbee_addr = function() {
  if (!$scope) return undefined;

  // TODO(rqou): Validity isn't checked

  return $scope.xbeeAddr;
};

exports.set_xbee_addr = function(new_addr) {
  if (!$scope) return;

  // $apply is needed because we can be called in an async context and expect
  // changes to be visible immediately (on the welcome page).
  // TODO(rqou): Do we need to fix this? Do we need this on other pages?
  // In general other pages will refresh when they get shown.
  $scope.$apply(function() {
    $scope.xbeeAddr = new_addr;
  });
};

exports.get_serial_port = function() {
  if (!$scope) return undefined;

  // TODO(rqou): Validity isn't checked

  return $scope.serialPort;
};

exports.set_serial_port = function(new_port) {
  if (!$scope) return;

  // $apply is needed because we can be called in an async context and expect
  // changes to be visible immediately (on the welcome page).
  // TODO(rqou): Do we need to fix this? Do we need this on other pages?
  // In general other pages will refresh when they get shown.
  $scope.$apply(function() {
    $scope.serialPort = new_port;
  });
};
