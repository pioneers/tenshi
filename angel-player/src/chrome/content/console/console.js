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
