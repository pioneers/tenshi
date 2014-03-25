var misc = require ( './misc.js' );

function write_buffer_xulrunner ( buf, filename ) {
  'use strict';
  var chrome = require('chrome');
  var _FileUtils = {};
  chrome.Cu.import ( 'resource://gre/modules/FileUtils.jsm', _FileUtils );
  var FileUtils = _FileUtils.FileUtils;

  var ofile = new FileUtils.File ( filename );
  var ostream = FileUtils.openFileOutputStream ( ofile, FileUtils.MODE_WRONLY | FileUtils.MODE_CREATE );
  var binary_ostream = chrome.Cc[ '@mozilla.org/binaryoutputstream;1' ].
                       createInstance ( chrome.Ci.nsIBinaryOutputStream );

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
  if ( misc.environment === 'xulrunner' ) {
    write_buffer_xulrunner ( buf, filename );
    }
  else {
    write_buffer_node ( buf, filename, ecallback );
    }
  }

exports.write_buffer = write_buffer;
