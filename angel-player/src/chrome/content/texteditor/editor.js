let $scope;

exports.texteditorController = function(angular_app) {
  angular_app.controller('TexteditCtrl', ['$scope', function($scope_) {
    $scope = $scope_;
  }]);
};

exports.get_text = function() {
  if (!$scope) return undefined;

  return $scope.text;
};

exports.set_text = function(new_text) {
  if (!$scope) return undefined;

  $scope.text = new_text;
};
