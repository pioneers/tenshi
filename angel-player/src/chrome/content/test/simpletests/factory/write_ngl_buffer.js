var buffer = require ( 'tenshi/angelic/buffer.js' );
var misc = require ( 'tenshi/angelic/misc.js' );
var factory = require ( 'tenshi/common/factory.js' );
var url = require ( 'jetpack/sdk/url' );

exports.run = function() {
  var fact = factory.make ( );
  fact.set_target_type ( 'ARM' );
  fact.load_type_file ( url.toFilename ( 
        'chrome://angel-player/content/common_defs/ngl_types.yaml' ) );

  var b = fact.create ( 'ngl_buffer' );
  b.set_slot ( 'header', { 'refc' : 1, 'type' : 0xff } );
  b.set_slot ( 'size', 500 );

  var sub_buf = buffer.Buffer ( 500 );
  sub_buf.fill ( 0xbb );
  b.set_slot ( 'bytes', sub_buf );

  var buf = buffer.Buffer ( fact.get_size ( 'ngl_buffer' ) + sub_buf.length );
  b.write ( buf );

  misc.assert ( buf.readUInt32LE ( 0 ) === 1, 'refc should be 1' );
  misc.assert ( buf.readUInt32LE ( 4 ) === 0xff, 'type should be 0xff' );
  misc.assert ( buf.readUInt32LE ( 8 ) === 500, 'size should be 500' );

  for ( var i = 12; i < buf.length; i++ ) {
    misc.assert ( buf[i] === 0xbb, 'bytes should have been written correctly' );
  }

  return true;
};
