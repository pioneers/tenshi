Components.utils.import('resource://gre/modules/devtools/dbg-server.jsm');
Components.utils.import("resource://gre/modules/Services.jsm");
const console =
  Components.utils.import("resource://gre/modules/devtools/Console.jsm").console;

const MAIN_WINDOW_TYPE = "angel-player:main";

function xulrunnerRemoteDebugInit() {
  // Start remote debugger (won't actually start unless
  // devtools.debugger.remote-enabled is set to true)
  if (!DebuggerServer.initialized) {
    DebuggerServer.init(function() {
      // Don't warn if debugging connections restricted to the local machine.
      return Services.prefs.getBoolPref("devtools.debugger.force-local") ||
        DebuggerServer._defaultAllowConnection();
    });
    DebuggerServer.addBrowserActors(MAIN_WINDOW_TYPE);

    DebuggerServer.addActors("chrome://angel-player/content/XULRootActor.js");
  }
  try {
    var port =
      Services.prefs.getIntPref('devtools.debugger.remote-port') || 6000;
    DebuggerServer.openListener(port);
  } catch (_) {
    console.log("Failed to launch DebuggerServer!");
  }
}
