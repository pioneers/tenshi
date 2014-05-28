// If we're actually inside XULRunner.
if (typeof process === 'undefined') {
  var fake_process = {
    'platform': require('jetpack/sdk/system').platform,
    'argv': ['fake_node', 'fake_script.js'],
    };
  var fake_require = (function (real_require) {
    return function (path) {
        // Put the original arguments into an Array we can slice.
        var args = Array.prototype.slice.call(arguments);

        // These are the node module replacements
        var replacements = {
          'fs': 'jetpack/sdk/io/fs',
          'path': 'jetpack/sdk/fs/path',
        };

        // If there's a replacement, use it.
        var replacement = replacements[path];
        if (replacement !== undefined) {
          return real_require.apply(null, [replacement].concat(args.slice(1)));
        }
        else {
          // Otherwise, pass all our arguments to the real require.
          return real_require.apply(null, args);
        }
      };
    })(require);
  // The host (Node.js or XULRunner) will not even create the needed real
  // variables if there is a normal declaration of the variable in this if
  // statement.
  // So hide the declaration inside an eval.
  eval('var process = fake_process;');
  eval('var require = fake_require;');
}
