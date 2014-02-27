$(function() {
  var x_loc = 600,
      y_loc = 20,
      x_inc = 0,
      y_inc = 45;

  loadAssets("base_inputbody.svg");

  function loadAssets() {
    function loadAsset(i, asset) {
      Snap.load("assets/images/" + asset, function(f) {
        var s = Snap("#workarea");
        s.append(f.select("style"));
        s.append(f.selectAll("pattern"));
        s.append(f.select("font"));
        s.append(f.select(".block"));
        var block_temp = s.select(".block");
        block_temp.transform((new Snap.Matrix()).translate(x_loc, y_loc));
        x_loc += x_inc;
        y_loc += y_inc;
        // makeClonable(block_temp);
      });
    }
    $.each(arguments, loadAsset);
  }
});