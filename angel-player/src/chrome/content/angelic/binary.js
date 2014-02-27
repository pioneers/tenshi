var misc = require ( './misc.js' );

var is_xulrunner = false;
if ( typeof Components !== 'undefined' ) {
  is_xulrunner = true;
  }

function write_buffer_xulrunner ( buf, filename ) {
  Components.utils.import ( 'resource://gre/modules/FileUtils.jsm' );

  var ofile = new FileUtils.File ( filename );
  var ostream = FileUtils.openFileOutputStream ( ofile, FileUtils.MODE_WRONLY | FileUtils.MODE_CREATE );
  var binary_ostream = Components.classes[ '@mozilla.org/binaryoutputstream;1' ].
                       createInstance ( Components.interfaces.nsIBinaryOutputStream );

  binary_ostream.setOutputStream ( ostream );

  // Copy to a Uint8Array first, so that the write can be performed
  // asynchronously.  Note that this is exactly what the buffer module will do
  // behind the scenes anyway, if it were asked to produce such an array (it
  // can't be).
  var outarray = new Uint8Array ( buf.length );
  for ( var i = 0; i < buf.length; i++ ) {
    outarray[i] = buf[i];
    }

  binary_ostream.writeByteArray ( outarray, outarray.length );
  ostream.flush ( );
  ostream.close ( );
  }

function write_buffer_node ( buf, filename, ecallback ) {
  var fs = require ( 'fs' );
  fs.open ( filename, 'w', function ( err, ofile ) {
    if ( err ) {
      if ( ecallback !== undefined ) {
        ecallback ( err );
        }
      }
    else {
      fs.write ( ofile, buf, 0, buf.length, 0, function ( err, w, b ) {
        if ( err && ecallback !== undefined ) {
          ecallback ( err );
          }
        } );
      }
    } );
  }

function write_buffer ( buf, filename, ecallback ) {
  if ( is_xulrunner ) {
    write_buffer_xulrunner ( buf, filename );
    }
  else {
    write_buffer_node ( buf, filename, ecallback );
    }
  }

exports.write_buffer = write_buffer;
