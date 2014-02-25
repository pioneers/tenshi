var string_map = require ( './string_map.js' );

var make = function make ( name ) {
  if ( name === undefined ) {
    name = '#obj';
    }
  return {
    name : name,
    data : [],
    relocations : [],
    id : 0,
    last_label_id : 0,
    last_stack_id : 0,
    labels : string_map.make ( ),
    };
  };

exports.make = make;
