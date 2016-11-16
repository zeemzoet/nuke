import nukescripts
axis = 1

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
		
		avgLen = int(len(points)/2)
		
		x = 0; y = 0; z = 0
		
		for i in range(avgLen):
			x += points[i][0]
			y += points[i][1]
			z += points[i][2]
			
		x /= avgLen; y /= avgLen; z /= avgLen

		nuke.toNode('xPt%d' %axis)['translate'].setValue([x,y,z])
		
	else:
		nuke.message('Select some vertices first')

elif not _input['geo']:
	nuke.message('Geometry is not connected or recognized')
		
else:
	nuke.message('Camera is not connected or recognized')
