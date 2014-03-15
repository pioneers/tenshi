var scope = require ( './scope.js' );

function make ( name ) {
  return {
    name: name || '',
    globals: scope.make ( ),
    exports: scope.make ( ),
    objects: [ ],
    imports: scope.make ( ),
    ast: null,
    };
  }

exports.make = make;
