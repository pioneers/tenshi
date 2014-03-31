var root = process.argv[2];

var buffer = require ( root + '/buffer.js' );
var misc = require ( root + '/misc.js' );
var factory = require ( root + '/factory.js' );

var fact = factory.make ( );
fact.set_target_type ( 'ARM' );
fact.load_type_file ( root + '/../common_defs/ngl_types.yaml' );

var pkg_header = fact.create ( 'ngl_package' );
var magic = 0x008fa5e5;

pkg_header.set_slot ( 'version', 1 );
pkg_header.set_slot ( 'fixup_table_offset', 0 );
pkg_header.set_slot ( 'patch_table_offset', 0 );
pkg_header.set_slot ( 'magic', magic );

function check_buffer ( buf, base ) {
  var i;
  for ( i = 0; i < base; i++ ) {
    misc.assert ( buf[i] === 0xAA,
                  'Addresses before struct should not be written to.' );
    }
  misc.assert ( buf.readUInt32LE ( base + 0 ) === magic,
                'magic should be set correctly.' );
  misc.assert ( buf.readUInt32LE ( base + 4 ) === 1,
                'version should be set correctly.' );
  misc.assert ( buf.readUInt32LE ( base + 8 ) === 0,
                'fixup table offset should be set correctly.' );
  misc.assert ( buf.readUInt32LE ( base + 12 ) === 0,
                'patch table offset should be set correctly.' );

  for ( i = base + 16; i < 1024; i++ ) {
    misc.assert ( buf[i] === 0xAA,
                  'Addresses after struct should not be written to.' );
    }
  }

var buf = buffer.Buffer ( 1024 );
buf.fill ( 0xAA );

pkg_header.write ( buf );

check_buffer ( buf, 0 );

buf.fill ( 0xAA );
pkg_header.set_offset ( 16 );
pkg_header.write ( buf );

check_buffer ( buf, 16 );
