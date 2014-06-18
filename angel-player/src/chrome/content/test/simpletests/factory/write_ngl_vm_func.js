var buffer = require ( 'tenshi/angelic/buffer.js' );
var misc = require ( 'tenshi/angelic/misc.js' );
var factory = require ( 'tenshi/common/factory.js' );
var url = require ( 'jetpack/sdk/url' );

exports.run = function() {
  var fact = factory.make ( );
  fact.set_target_type ( 'ARM' );
  fact.load_type_file ( url.toFilename ( 
        'chrome://angel-player/content/common_defs/ngl_types.yaml' ) );

  var func = fact.create ( 'ngl_vm_func' );
  func.set_slot ( 'header', { 'refc' : 1, 'type' : 0xff } );
  func.set_slot ( 'code', 0xbee );
  func.set_slot ( 'stack_space', 10 );

  var buf = buffer.Buffer ( fact.get_size ( 'ngl_vm_func' ) );
  func.write ( buf );

  misc.assert ( buf.readUInt32LE ( 0 ) === 1, 'refc should be 1' );
  misc.assert ( buf.readUInt32LE ( 4 ) === 0xff, 'type should be 0xff' );
  misc.assert ( buf.readUInt32LE ( 8 ) === 0xbee, 'code should be 0xbee' );
  misc.assert ( buf.readUInt32LE ( 12 ) === 10, 'stack_space should be 10' );

  return true;
};
