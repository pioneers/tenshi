var module = require ( './module.js' );
var type = require ( './type.js' );
var fn = require ( './fn.js' );

function make ( ) {
  var core = module.make ( );
  var print = fn.make ( );
  print.external = true;
  core.exports.set_type ( 'number', type.make ( 'number' ) );
  core.exports.set_type ( 'bool', type.make ( 'bool' ) );
  core.exports.set_text ( 'print', print );
  core.objects.push ( print );
  return core;
  }

exports.make = make;
