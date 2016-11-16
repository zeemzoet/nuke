n = nuke.thisNode()

if nuke.thisNode()['exportBool'].getValue():
	point1 = n['twoPtExport_1'].value()
	point2 = n['twoPtExport_2'].value()


	secondTrack = 31
	X = 2
	Y = 3
	T = 6
	R = 7
	S = 8
	
	_refFrame = int(n['refFrame'].getValue())

	if not _refFrame:
		_refFrame = nuke.frame()
		n['refFrame'].setValue(_refFrame)
	
	nuke.frame(_refFrame)
	
	choice = int(n['exportChoice'].getValue())
	if 1 <= choice <= 2:
		nuke.message('A relative link to trackers is not (yet) supported')
		
	else:
		nuke.root().begin()
		tra = nuke.createNode("Tracker4")
		tra.setInput(0, None)

		for i in range(2):
			tra['add_track'].execute()
			
		k = tra['tracks']
		k.setAnimated()

		k.setValue(True, secondTrack + T)
		k.setValue(True, secondTrack + R)
		k.setValue(True, secondTrack + S)
		
		for f in range(nuke.root().firstFrame(),nuke.root().lastFrame()+1):
			k.setValueAt(n[point1].valueAt(f)[0], f, X)
			k.setValueAt(n[point1].valueAt(f)[1], f, Y) 
			k.setValueAt(n[point2].valueAt(f)[0], f, secondTrack + X)
			k.setValueAt(n[point2].valueAt(f)[1], f, secondTrack + Y)
					
		if choice == 3:
			tra['transform'].setValue('stabilize')
			tra['reference_frame'].setValue(_refFrame)
			
		elif choice == 4:
			tra['transform'].setValue('matchmove')
			tra['reference_frame'].setValue(_refFrame)
