#!/usr/bin/env python

import sys
import xml.dom.minidom

def node_iterator(node):
	child = node.firstChild
	while child != None:
		yield child
		child = child.nextSibling

def transform_affine(mat, coord):
	return (
		mat[0] * coord[0] + mat[1] * coord[1] + mat[2],
		mat[3] * coord[0] + mat[4] * coord[1] + mat[5],
	)

def main():
    if len(sys.argv) != 9:
        print("Usage: %s in.sym out.sym a11 a12 a13 a21 a22 a23" % (sys.argv[0]))
        sys.exit(1)

    infile = sys.argv[1]
    outfile = sys.argv[2]

    a11 = float(sys.argv[3])
    a12 = float(sys.argv[4])
    a13 = float(sys.argv[5])
    a21 = float(sys.argv[6])
    a22 = float(sys.argv[7])
    a23 = float(sys.argv[8])
    mat = (a11, a12, a13, a21, a22, a23)

    # print mat

    part = xml.dom.minidom.parse(infile)
    rootElement = part.documentElement
    for elem in node_iterator(rootElement):
    	# print elem
    	if elem.nodeType == xml.dom.Node.TEXT_NODE:
    		# We don't care about these
    		pass
    	elif elem.nodeType == xml.dom.Node.ELEMENT_NODE:
    		# print elem.tagName
    		if elem.tagName == 'wire':
    			# print "A WIRE"
    			coord1 = (float(elem.getAttribute('x1')), float(elem.getAttribute('y1')))
    			coord2 = (float(elem.getAttribute('x2')), float(elem.getAttribute('y2')))
    			print "Wire from (%f,%f) to (%f,%f)" % (coord1[0], coord1[1], coord2[0], coord2[1])
    			newcoord1 = transform_affine(mat, coord1)
    			newcoord2 = transform_affine(mat, coord2)
    			print "New wire  (%f,%f) to (%f,%f)" % (newcoord1[0], newcoord1[1], newcoord2[0], newcoord2[1])
    			elem.setAttribute('x1', str(newcoord1[0]))
    			elem.setAttribute('y1', str(newcoord1[1]))
    			elem.setAttribute('x2', str(newcoord2[0]))
    			elem.setAttribute('y2', str(newcoord2[1]))
    		elif elem.tagName == 'text':
    			coord = (float(elem.getAttribute('x')), float(elem.getAttribute('y')))
    			print "Text at (%f,%f)" % (coord[0], coord[1])
    			newcoord = transform_affine(mat, coord)
    			print "New at  (%f,%f)" % (newcoord[0], newcoord[1])
    			elem.setAttribute('x', str(newcoord[0]))
    			elem.setAttribute('y', str(newcoord[1]))
    		elif elem.tagName == 'pin':
    			coord = (float(elem.getAttribute('x')), float(elem.getAttribute('y')))
    			print "Pin at (%f,%f)" % (coord[0], coord[1])
    			newcoord = transform_affine(mat, coord)
    			print "New at (%f,%f)" % (newcoord[0], newcoord[1])
    			elem.setAttribute('x', str(newcoord[0]))
    			elem.setAttribute('y', str(newcoord[1]))
    		else:
    			print "Warning: skipping unknown %s" % elem.tagName
    	else:
    		print "Skipping unknown node: %s" % elem

    newpartdata = part.toxml('utf-8')
    f = open(outfile, 'w')
    f.write(newpartdata)
    f.close()

if __name__=='__main__':
    main()
