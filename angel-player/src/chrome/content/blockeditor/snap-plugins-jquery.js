/* Custom plugins for Snap SVG
Nikita Kitaev

Require jQuery to be loaded.

New member methods:
Element.createEmbeddedHtml(x, y, width, height)
* Creates an HTML fragment embedded into the SVG
* If the element is a group or svg, the HTML fragment is embedded inside it.
Otherwise, it appears immediately afterwards in the enclosing group.
* Returns: {
svg: Snap object representing the foreignObject
html: jQuery object representing the HTML root div
}
*/
Snap.plugin(function (Snap, Element, Paper, glob) {
  var elproto = Element.prototype,
      is = Snap.is;

  elproto.createEmbeddedHTML = function (x, y, width, height) {
    // Creates an HTML fragment embedded into the SVG
    var params = Snap.format("x='{x}' y='{y}' width='{width}' height='{height}'", {
         x:x,
         y:y,
         width:width,
         height:height
      });

    // The namespace specifications are required for this to render correctly
    // TODO(nikita): embedded HTML should be XHTML5, not older
    var obj = $("<svg>\
<foreignObject " + params + "xmlns='http://www.w3.org/2000/svg'>\
<div xmlns='http://www.w3.org/1999/xhtml'></div>\
</foreignObject></svg>");

    // Return the foreign object as a Snap element, and the div a jQuery element
    var res = {
      'svg': Snap(obj.children()[0]),
      'html': $(obj.children()[0].children[0])
    };
    if (this.type === "g" || this.type === "svg") {
      this.node.appendChild(res.svg.node);
    } else {
      this.after(res.svg.node);
    }
    return res;
  };
});