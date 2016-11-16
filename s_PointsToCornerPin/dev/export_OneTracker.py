n = nuke.thisNode()

if nuke.thisNode()['exportBool'].getValue():
	point = n['onePtExport'].value()

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
	
		tra['add_track'].execute()
		
		k = tra['tracks']
		k.setAnimated()
		for f in range(nuke.root().firstFrame(),nuke.root().lastFrame()+1):
			k.setValueAt(n[point].valueAt(f)[0], f, X)
			k.setValueAt(n[point].valueAt(f)[1], f, Y)
			
		if choice == 3:
			tra['transform'].setValue('stabilize')
			tra['reference_frame'].setValue(_refFrame)
			
		elif choice == 4:
			tra['transform'].setValue('matchmove')
			tra['reference_frame'].setValue(_refFrame)
