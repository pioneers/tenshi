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

var unwrapped = pkg_header.unwrap ( );

misc.assert ( unwrapped.version === 1,
              'version should be set correctly.' );
misc.assert ( unwrapped.magic === magic,
              'magic should be set correctly.' );
misc.assert ( unwrapped.fixup_table_offset === 0,
              'fixup table offset should be set correctly.' );
misc.assert ( unwrapped.patch_table_offset === 0,
              'patch table offset should be set correctly.' );
