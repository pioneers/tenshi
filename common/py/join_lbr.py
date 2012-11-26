#!/usr/bin/env python

import os
import sys
import xml.dom.minidom

def find_only_subnode(element, subnode_name):
    ret = element.getElementsByTagName(subnode_name)
    assert len(ret) == 1, "Found more or less than 1 %s node" % subnode_name
    return ret[0]

def main():
    if len(sys.argv) != 3:
        print("Usage: %s indir file.lbr" % (sys.argv[0]))
        sys.exit(1)

    indir = sys.argv[1]
    lbrfile = sys.argv[2]

    #parse meta.xml
    lbr = xml.dom.minidom.parse(indir + "/meta.xml")
    libraryNode = find_only_subnode(lbr, "library")

    #create the subnodes
    packagesNode = lbr.createElement("packages")
    libraryNode.appendChild(packagesNode)
    symbolsNode = lbr.createElement("symbols")
    libraryNode.appendChild(symbolsNode)
    devicesetsNode = lbr.createElement("devicesets")
    libraryNode.appendChild(devicesetsNode)

    for f in os.listdir(indir):
        #the "real" filename with the directory
        filename = indir + "/" + f
        if f.endswith(".pac"):
            n = xml.dom.minidom.parse(filename)
            packagesNode.appendChild(n.documentElement)
        elif f.endswith(".sym"):
            n = xml.dom.minidom.parse(filename)
            symbolsNode.appendChild(n.documentElement)
        elif f.endswith(".dev"):
            n = xml.dom.minidom.parse(filename)
            devicesetsNode.appendChild(n.documentElement)
        elif f == "meta.xml":
            pass
        else:
            print("WARN: ignoring file %s" % f)

    f = open(lbrfile, "w")
    f.write(lbr.toxml("utf-8"))
    f.close()

if __name__=='__main__':
    main()
