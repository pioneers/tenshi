//
// This module holds the object used to represent a type in the compiler.
//

var make = function make ( name ) {
  return {
    name: name,
    size: 0,
    };
  };

exports.make = make;
