import xml.dom.minidom
import os
import glob
import re
import codecs
import time

def remove_empty_groups(root, iteration_limit=10):
    """Remove empty groups from an SVG file"""
    converged = True
    for group in root.getElementsByTagName("g"):
        for child in group.childNodes:
            if child.nodeType != root.TEXT_NODE:
                break
        else:
            converged = False
            group.parentNode.removeChild(group)
    if not converged and iteration_limit > 0:
        remove_empty_groups(root, iteration_limit-1)

def descendants(node):
    """Get all descendants of a node"""
    for sub in node.childNodes:
        if sub.hasChildNodes():
            for n in descendants(sub):
                yield n
        else:
            yield sub

def rename_classes_dom(root, mapping):
    """
    Replace classes in the DOM using the mapping
    Mapping is a dict from old names to new names
    """
    for node in descendants(root):
        if "hasAttribute" not in dir(node) or not node.hasAttribute("class"):
            continue
        classes = node.getAttribute("class").split()
        changed = False
        for i, name in enumerate(classes):
            if name in mapping:
                classes[i] = mapping[name]
                changed = True
        if changed:
            node.setAttribute("class", " ".join(classes))

NMCHAR = r"[_a-zA-Z0-9-]|[\240-\377]|\\h{1,6}(\r\n|[ \t\r\n\f])?|\\[^\r\n\f0-9a-f]"
# NMCHAR: non-initial character of CSS selector, according to CSS 3 spec
def rename_classes_style(root, mapping):
    """
    Rename classes in a style tag, using the mapping
    Mapping is a dict from old names to new names
    """
    for style in root.getElementsByTagName("style"):
        # First remove CDATA attributes and merge all the text
        text = style.childNodes[0]
        for node in style.childNodes[1:]:
            if node.nodeType == root.CDATA_SECTION_NODE:
                text.nodeValue += node.wholeText
            elif node.nodeType == root.TEXT_NODE:
                text.nodeValue += node.nodeValue
            node.parentNode.removeChild(node)


        def replace_fn(match):
            """
            Replace classes according to mapping.
            Unused styles get replaced with a random name to avoid conflicts
            """
            old = match.group()[1:]
            if old in mapping:
                return "." + mapping[old]
            else:
                return ".unused" + hex(hash(time.time()))[2:]

        text.nodeValue = re.sub(r"\." + r"(" + NMCHAR + ")+",
            replace_fn,
            text.nodeValue)


def extract_class_name(id):
    """
    Extract a class name from an id.
    Return None if this can't be done
    """
    match = re.match("(.+)style_[0-9]+_", id)
    if match:
        return match.group(1)
    else:
        return None

def get_swatches(root):
    """Get swatch elements in the SVG"""
    swatches = {}
    for node in descendants(root):
        if "hasAttribute" not in dir(node) or not node.hasAttribute("id"):
            continue
        classname = extract_class_name(node.getAttribute("id"))
        if classname:
            swatches[classname] = node
    return swatches

def construct_mapping(root):
    """
    Construct a mapping for class names using swatches

    Change the names of non-swatch classes to avoid conflicts when the same
    original classname occurs in different files
    """
    global fileseed
    swatches = get_swatches(root)
    mapping = {}
    for classname, swatch in swatches.items():
        if not swatch.hasAttribute("class"):
            continue # TODO(nikita): This is actually an error
        classes = swatch.getAttribute("class").split()
        if len(classes) != 1:
            continue # TODO(nikita): This is actually an error
        old = classes[0]
        mapping[old] = classname
        swatch.parentNode.removeChild(swatch)


    for node in descendants(root):
        if "hasAttribute" not in dir(node) or not node.hasAttribute("class"):
            continue
        classes = node.getAttribute("class").split()
        for old in classes:
            mapping[old] = old + "_" + fileseed
    return mapping

def cleanup_svg(root):
    """
    Clean up an illustrator-generated SVG
    """
    mapping = construct_mapping(root)
    remove_empty_groups(root) # Do this after constructing the mapping so empty swatch groups go away
    rename_classes_dom(root, mapping)
    rename_classes_style(root, mapping)

def cleanup_whitespace(root):
    """
    Clean up whitespace in an SVG to make it print nicer
    """
    for node in list(descendants(root)):
        # Convert descendants to list to avoid simultaneously modifying and traversing the DOM
        if node.nodeType == root.TEXT_NODE and node.nodeValue.isspace():
            node.parentNode.removeChild(node)

def encode(txt, err):
    """Hack to avoid generating erroneous XML"""
    #TODO(nikita): backslash (\) is not encoded. This causes parsing problems
    res = codecs.ascii_encode(txt, err)
    res =  res[0].replace(r'unicode="\"', r'unicode="&#92;"')
    return res, len(res)
codecs.register(lambda x: None if x != 'xmlfix' else (encode, codecs.ascii_decode, None, None))

def main():
    global fileseed
    script_dir = os.path.dirname(os.path.realpath(__file__))
    for svg_file in glob.glob(os.path.join(script_dir, "*.svg")):
        fileseed = str(hash(svg_file))
        with open(svg_file, "r") as f:
            svg = xml.dom.minidom.parse(f)
        cleanup_svg(svg)
        cleanup_whitespace(svg)
        with open(svg_file, "w") as f:
            text = svg.toprettyxml(indent="", newl="\n")
            #Encode non-ascii characters as references, e.g. "&#198;"
            text = text.encode('xmlfix', 'xmlcharrefreplace')
            f.write(text)
            print "PROCESSED:", os.path.basename(svg_file)

if __name__ == "__main__":
    main()
