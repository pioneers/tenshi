var make = function make ( name ) {
  if ( name === undefined ) {
    name = '#obj';
    }
  return {
    name: name,
    data: [],
    relocations: [],
    };
  };

exports.make = make;
