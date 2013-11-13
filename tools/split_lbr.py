#!/usr/bin/env python

import os
import sys
import shutil
import xml.dom.minidom
import re


def find_only_subnode(element, subnode_name):
    ret = element.getElementsByTagName(subnode_name)
    assert len(ret) == 1, "Found more or less than 1 %s node" % subnode_name
    return ret[0]


def filter_filenames(in_name, outdir, extension):
    new_name = re.sub("[^0-9A-Za-z_\-]", "-", in_name)
    filename = "%s/%s.%s" % (outdir, new_name, extension)
    if not os.path.exists(filename):
        return filename

    extra_num = 0
    filename = "%s/%s.%s" % (outdir, new_name + str(extra_num), extension)
    while os.path.exists(filename):
        extra_num = extra_num + 1
        filename = "%s/%s.%s" % (outdir, new_name + str(extra_num), extension)
    return filename


def write_out_subnode(node, outdir, extension):
    name = node.getAttribute("name")
    filename = filter_filenames(name, outdir, extension)
    assert not os.path.exists(filename), (
        "The output file %s already exists!" % filename)
    f = open(filename, "w")
    f.write(node.toxml("utf-8"))
    f.close()


def main():
    if len(sys.argv) != 3:
        print("Usage: %s file.lbr outdir" % (sys.argv[0]))
        sys.exit(1)

    lbrfile = sys.argv[1]
    outdir = sys.argv[2]

    #parse lbr
    lbr = xml.dom.minidom.parse(lbrfile)
    libraryNode = find_only_subnode(lbr, "library")

    #find the main stuff
    packagesNode = find_only_subnode(libraryNode, "packages")
    symbolsNode = find_only_subnode(libraryNode, "symbols")
    devicesetsNode = find_only_subnode(libraryNode, "devicesets")

    #FIXME: may be dangerous
    shutil.rmtree(outdir, ignore_errors=True)
    os.mkdir(outdir)

    #remove all the packages, symbols, devicesets
    packagesNode.parentNode.removeChild(packagesNode)
    symbolsNode.parentNode.removeChild(symbolsNode)
    devicesetsNode.parentNode.removeChild(devicesetsNode)

    #write out the remainder (the "metadata")
    f = open("%s/meta.xml" % outdir, "w")
    lbr.writexml(f)
    f.close()

    #write out all the "main stuff"
    for n in packagesNode.childNodes:
        if n.nodeType == xml.dom.Node.TEXT_NODE:
            assert n.data.strip() == "", (
                "Text node with some data not understood (%s)" % n.data)
        else:
            assert n.nodeType == xml.dom.Node.ELEMENT_NODE, (
                "Unknown node type (%s)" % n.nodeType)
            write_out_subnode(n, outdir, "pac")

    for n in symbolsNode.childNodes:
        if n.nodeType == xml.dom.Node.TEXT_NODE:
            assert n.data.strip() == "", (
                "Text node with some data not understood (%s)" % n.data)
        else:
            assert n.nodeType == xml.dom.Node.ELEMENT_NODE, (
                "Unknown node type (%s)" % n.nodeType)
            write_out_subnode(n, outdir, "sym")

    for n in devicesetsNode.childNodes:
        if n.nodeType == xml.dom.Node.TEXT_NODE:
            assert n.data.strip() == "", (
                "Text node with some data not understood (%s)" % n.data)
        else:
            assert n.nodeType == xml.dom.Node.ELEMENT_NODE, (
                "Unknown node type (%s)" % n.nodeType)
            write_out_subnode(n, outdir, "dev")

if __name__ == '__main__':
    main()
