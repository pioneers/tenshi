/* Custom plugins for Snap SVG
Nikita Kitaev

New static methods:
Matrix.fromTransformString(tString, [format])
* Creates a matrix based on an SVG transform string
* This method performs formatting if a format specifier is given, e.g.
Matrix.fromTransformString("scale({x}, {y})", el.getBBox())

New class: Snap.BBox
new Snap.BBox(obj)
* Converts the output of getBBox() into an object with methods
Snap.BBox.keypoint(name)
* Converts a keypoint name ('topleft', 'topright', 'bottomleft', 'bottomright',
'center') into an object with x/y fields, relative to the BBox

New member methods:
Element.addTransform(t, [center])
* Applies a transformation to an element, *on top of the existing one*
* t can by an SVG transform string, or a Matrix
* center may be 'topleft', 'topright', 'bottomleft', 'bottomright', 'center',
or an object with fields x/y representing coordinates in the local geometry
* If center is specified, the transform is performed relative to that point
Element.dynamicSize(subelement, width, height)
* Applied to a group element
* Resize the subelement to have a given width/height, and reflow the rest of
the group intelligently
* subelement may be an Element, or a selector
* If subelement is a selector, it should return a single object only
Element.getSmartBBox(isWithoutTransform)
* Gets the bounding box, as a Snap.BBox object
* Does not cache the bbox size
Element.resizeAtPoint(x, y, dx, dy)
* Applied to an element or group
* Any polygons or rectangles in the group/element are affected by the
resizing
* Points to the right of (x, y) are shifted by dx
* Points to the bottom of (x, y) are shifted by dy
*/


Snap.plugin(function (Snap, Element, Paper, glob) {
  var elproto = Element.prototype,
      is = Snap.is,
      $ = Snap._.$;

  function svgTransform2string(tstr) {
    tstr = String(tstr);
    // copied from snap.svg.js
    var res = [];
    tstr = tstr.replace(/(?:^|\s)(\w+)\(([^)]+)\)/g, function (all, name, params) {
      params = params.split(/\s*,\s*/);
      if (name == "rotate" && params.length == 1) {
        params.push(0, 0);
      }
      if (name == "scale") {
        if (params.length == 2) {
          params.push(0, 0);
        }
        if (params.length == 1) {
          params.push(params[0], 0, 0);
        }
      }
      if (name == "skewX") {
        res.push(["m", 1, 0, math.tan(rad(params[0])), 1, 0, 0]);
      } else if (name == "skewY") {
        res.push(["m", 1, math.tan(rad(params[0])), 0, 1, 0, 0]);
      } else {
        res.push([name.charAt(0)].concat(params));
      }
      return all;
    });
    return res;
    //end copied
  }

  Snap.BBox = function (obj) {
    for(var key in obj) {
        this[key] = obj[key];
    }
  }

  Snap.BBox.prototype.keypoint = function(name) {
    if (!is(name, 'string')) {
      return name;
    }

    var x, y;
    switch(name) {
      case 'topleft':
        x = this.x;
        y = this.y;
        break;
      case 'topright':
        x = this.x2;
        y = this.y;
        break;
      case 'bottomleft':
        x = this.x;
        y = this.y2;
        break;
      case 'bottomright':
        x = this.x2;
        y = this.y2;
        break;
      case 'center':
        x = (this.x + this.x2) / 2;
        y = (this.y + this.y2) / 2;
        break;
    }
    return {x:x, y:y};
  }


  elproto.getSmartBBox = function(isWithoutTransform) {
    this._.dirty = 1; // HACK. Snap never (!) sets the dirty flag except when
                      // the wrapper element is created
    // TODO(nikita): revisit this
    return new Snap.BBox(this.getBBox(isWithoutTransform));
  }

  Snap.Matrix.fromTransformString = function(tString, format) {
    // Create a matrix from an SVG transform string (optionally formatting it)
    if (arguments.length > 1) {
      tString = Snap.format(tString, format);
    }
    return Snap._.transform2matrix(svgTransform2string(tString), {
        x : -5,
        y : -5,
        width : 10,
        height : 10
      });
  }

  elproto.addTransform = function (t, center) {
    // Apply a transform to an element, on top of the transform it already has
    // Optionally center the transform around a particular feature of the bounding box
    if (is(t, "string")) {
      t = Snap.Matrix.fromTransformString(t);
    }

    if (arguments.length < 2) {
      return this.transform(this.transform().localMatrix.add(t))
    }

    var bbox = this.getSmartBBox();
    center = bbox.keypoint(center);

    return this.addTransform((new Snap.Matrix())
                      .translate(center.x, center.y)
                      .add(t)
                      .translate(-center.x, -center.y));
  }

  var mapPoints = (function(){
    // Point-mapping code
    function parsePolygonString(polygonString) {
      var spaces = "\x09\x0a\x0b\x0c\x0d\x20\xa0\u1680\u180e\u2000\u2001\u2002\u2003\u2004\u2005\u2006\u2007\u2008\u2009\u200a\u202f\u205f\u3000\u2028\u2029",
          separator = "[" + spaces + ",]*",
          number = "-?\\d*\\.?\\d*(?:e[\\-+]?\\d+)?",
          tCoordinates = new RegExp(separator + "(" + number + ")"
                                    + separator + "(" + number + ")" + separator,
                                    "ig"),
          points = [];
      polygonString.replace(tCoordinates,
                            function(match, p1, p2) {
                                if (p1 === "") {
                                  return "";
                                }
                                points.push([+p1, +p2]);
                                return "";
                            });
      return points;
    }

    function polygonMap(polygonString, fn) {
      var points = parsePolygonString(polygonString),
          transformed = points.map(fn),
          output = "";
      transformed.map(function(el) {
        output = output + " " + el[0] + "," + el[1];
      })
      return output;
    }

    function mapPoints(el, mapping) {
      if (el.type === "polygon") {
        var points = el.attr("points"),
            mtx = el.transform().localMatrix,
            mtx_inv = mtx.invert(),
            tpoints;
        tpoints = polygonMap(points, function(pt) {
              var x = mtx.x(pt[0], pt[1]),
                  y = mtx.y(pt[0], pt[1]),
                  post = mapping(x, y);
              x = mtx_inv.x(post[0], post[1]);
              y = mtx_inv.y(post[0], post[1]);
              return [x, y];
            });
        el.attr("points", tpoints)
      } else if (el.type == "rect") {
        var x = +el.attr("x"), // Note the cast to numeric type
            y = +el.attr("y"),
            width = +el.attr("width"),
            height = +el.attr("height"),
            mtx = el.transform().localMatrix,
            mtx_inv = mtx.invert(),
            x1 = mtx.x(x, y),
            y1 = mtx.y(x, y),
            x2 = mtx.x(x + width, y + height),
            y2 = mtx.y(x + width, y + height),
            post1 = mapping(x1, y1),
            post2 = mapping(x2, y2);

        x1 = mtx_inv.x(post1[0], post1[1]),
        y1 = mtx_inv.y(post1[0], post1[1]),
        x2 = mtx_inv.x(post2[0], post2[1]),
        y2 = mtx_inv.y(post2[0], post2[1]);
        el.attr("x", x1);
        el.attr("y", y1);
        el.attr("width", x2 - x1);
        el.attr("height", y2 - y1);
      } else {
        // Silently ignore applying this to unsupported element type
      }
    }

    return mapPoints;
  })();

  elproto.resizeAtPoint = function(thresh_x, thresh_y, dx, dy, exception) {
    if (arguments.length > 4) {
      exception = this.selectAll(exception);
    } else {
      exception = [];
    }

    var mapping = function (x, y) {
          if (x > thresh_x) {
            x += dx;
          }

          if (y > thresh_y) {
            y += dy;
          }
          return [x, y];
        };

    this.selectAll("*").forEach(function(element) {
      if (element.type === "tspan"
         || element.type === "pattern") {
        // Skip elements that cause problems, e.g. because they don't have a transformation
        return;
      }
      if (exception.length > 0) {
        var skip = false;
        exception.forEach(function(ex) {
          skip |= (ex === element);
        });
        if (skip) {
          return;
        }
      }

      mapPoints(element, mapping);
    });
    return this;
  }

  elproto.dynamicSize = function(subelement, width, height, exception) {
    // Dynamically scale subelement to have width/height
    if (is(subelement, "string")) {
      subelement = this.select(subelement);
    }
    if (arguments.length > 3) {
      exception = this.selectAll(exception);
    } else {
      exception = [];
    }

    var thresh_x = subelement.getSmartBBox().x + 1,
        thresh_y = subelement.getSmartBBox().y + 1,
        old_width = subelement.getSmartBBox().width,
        old_height = subelement.getSmartBBox().height,
        translation_x = (new Snap.Matrix()).translate(width - old_width, 0),
        translation_y = (new Snap.Matrix()).translate(0, height - old_height);

    subelement.addTransform((new Snap.Matrix()).scale(width / old_width, height / old_height), 'topleft');

    this.selectAll("*").forEach(function(element) {
      if (element.type === "tspan") return; // TODO(nikita): why do tspans cause bugs in Snap.svg?
      if (exception.length > 0) {
        var skip = false;
        exception.forEach(function(ex) {
          skip |= (ex === element);
        });
        if (skip) {
          return;
        }
      }

      if(element.getSmartBBox().x > thresh_x) {
        element.addTransform(translation_x, 'topleft');
      }
      if(element.getSmartBBox().y > thresh_y) {
        element.addTransform(translation_y, 'topleft');
      }
    });
    return this;
  }
});