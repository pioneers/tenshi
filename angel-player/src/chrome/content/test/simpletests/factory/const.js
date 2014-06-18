var buffer = require ( 'tenshi/angelic/buffer.js' );
var misc = require ( 'tenshi/angelic/misc.js' );
var factory = require ( 'tenshi/common/factory.js' );
var url = require ( 'jetpack/sdk/url' );

exports.run = function() {
  var fact = factory.make ( );
  fact.set_target_type ( 'ARM' );
  fact.load_type_file ( url.toFilename ( 
        'chrome://angel-player/content/common_defs/ngl_types.yaml' ) );

  misc.assert ( fact.get_const ( 'NGL_PACKAGE_MAGIC' ) === 0x008fa5e5,
                'NGL_PACKAGE_MAGIC should be 0x008fa5e5.' );
  return true;
};
