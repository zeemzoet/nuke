nuke.thisNode()['code'].execute()

_input = checkInput()

if _input['cam'] and _input['geo']:
	
	if nuke.thisNode()['generateBool'].getValue():

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

			camera = getInput(nuke.thisNode(),1)

			frames = [frame for frame in range(first,last+1)]

			pB = nuke.ProgressTask('PointsToCornerPin')
			pB.setMessage("Generating projection:")

			ct = nuke.nodes.CurveTool()
			for frame in frames:

				if pB.isCancelled():
					break
				
				nuke.execute(ct,frame,frame)
				gen = projectPoints(frame, nuke.thisNode()['format'].value(), camera, ([nuke.toNode("xPt%s" % str(points+1))['translate'].value() for points in range(4)]))
				
				if gen:
					
					for p in range(4):
						point = gen.next()
						pName = 'to%s' % str(p+1)
						
						nuke.thisNode()[pName].setAnimated()
						nuke.thisNode()[pName].setValueAt(point[0],frame,0)
						nuke.thisNode()[pName].setValueAt(point[1],frame,1)
						
						pB.setProgress(int((frame-first)*100/len(frames)))
						
			del pB
			nuke.delete(ct)
                
			nuke.thisNode()['exportBool'].setValue(1)            
	else:
		nuke.message('Get points first')

elif not _input['geo']:
	nuke.message('Geometry is not connected or recognized')
		
else:
	nuke.message('Camera is not connected or recognized')

