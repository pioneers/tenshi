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
