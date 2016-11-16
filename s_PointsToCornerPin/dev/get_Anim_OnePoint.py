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
		first = int(nuke.thisNode()['firstFrame'].getValue())
		last = int(nuke.thisNode()['lastFrame'].getValue())
		
		FrameRange = getFrameRange(first, last)
		if FrameRange:
		
			first = int(FrameRange[0])
			if len(FrameRange) == 1:
				last = int(FrameRange[0])
			else:
				last = int(FrameRange[1])
				
			nuke.thisNode()['firstFrame'].setValue(first)    
			nuke.thisNode()['lastFrame'].setValue(last)
			
			frames = [frame for frame in range(first, last+1)]
			
			pB = nuke.ProgressTask('PointsToCornerPin')
			pB.setMessage("Tracing points:")
			
			ct = nuke.nodes.CurveTool()
			for frame in frames:
				
				if pB.isCancelled():
					break
					
				nuke.execute(ct, frame, frame)
				gen = nukescripts.snap3d.selectedPoints()
				points = [point for point in gen]
				
				avgLen = int(len(points)/2)
				
				x = 0; y = 0; z = 0
				
				for i in range(avgLen):
					x += points[i][0]
					y += points[i][1]
					z += points[i][2]
					
				x /= avgLen; y /= avgLen; z /= avgLen
				
				nuke.toNode('xPt%d' %axis)['translate'].setAnimated()
				nuke.toNode('xPt%d' %axis)['translate'].setValueAt(x, frame,0)
				nuke.toNode('xPt%d' %axis)['translate'].setValueAt(y, frame,1)
				nuke.toNode('xPt%d' %axis)['translate'].setValueAt(z, frame,2)
				
				pB.setProgress(int((frame-first)*100/len(frames)))
				
			del pB
			nuke.delete(ct)
			
			nuke.thisNode()['generateBool'].setValue(1)
		
	else:
		nuke.message('Select some vertices first')

elif not _input['geo']:
	nuke.message('Geometry is not connected or recognized')
		
else:
	nuke.message('Camera is not connected or recognized')
