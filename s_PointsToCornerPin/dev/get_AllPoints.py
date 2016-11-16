import nukescripts

nuke.thisNode()['code'].execute()

_input = checkInput()

if _input['cam'] and _input['geo']:
	
	### checks how many vertices are selected
	i = 0
	for vertex in nukescripts.snap3d.selectedPoints():
		i += 1
	
	if i:
		gen = nukescripts.snap3d.selectedPoints()
		points = [point for point in gen]
		for i in range(4):
			pName = "xPt%s" % str(i+1)
			nuke.toNode(pName)["translate"].setValue(points[i])

		nuke.thisNode()['generateBool'].setValue(1)

	else:
		nuke.message('Select some vertices first')
        
elif not _input['geo']:
	nuke.message('Geometry is not connected or recognized')
		
else:
	nuke.message('Camera is not connected or recognized')
