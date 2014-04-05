$(function() {
  var s = Snap("#workarea");

  function updateText() {
    var obj = s.select(".block"),
        val = $("input").val(),
        old_bbox = obj.select("text").getSmartBBox(),
        test_bbox = obj.getSmartBBox(),
        ref_x = test_bbox.cx,
        ref_y = test_bbox.cy,
        new_bbox;

    if (val.length < 1) {
      return;
    }

    obj.select("text").attr("text", val);
    new_bbox = obj.select("text").getSmartBBox();
    obj.resizeAtPoint(ref_x, ref_y,
                      new_bbox.width - old_bbox.width,
                      new_bbox.height - old_bbox.height,
                      "text");
  }

  Snap.load("assets/images/base_inputbody.svg", function(f) {
    s.append(f.select("style"));
    s.append(f.select("pattern"));
    s.append(f.select("font"));
    s.append(f.select(".block"));
    var obj = s.select(".block");
    obj.drag();

    // Add an input area to modify the text
    var emb = s.createEmbeddedHTML("50%", "10px", "50%", "30px");
    emb.html.append("<form accept-charset='utf-8'> Block Text: <input id='resizeupdatetext' value='text'></input></form>");

    // Event handler to change the text inside the block
    $("#resizeupdatetext").keyup(updateText);
    $("#resizeupdatetext").change(updateText);
    updateText();
  });
});