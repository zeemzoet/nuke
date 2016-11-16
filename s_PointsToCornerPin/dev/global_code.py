import math

### FUNCTIONS
def getInput(node, i, ignoreMe = 'Dot'):
	
	nextNode = node.input(i)
	check = False
	
	# Check if the node is not None
	if nextNode is None:
		return
	
	while not check:
		if nextNode.Class() == ignoreMe:
			return getInput(nextNode,0)
		else:
			check = True
			return nextNode
			
def getFrameRange(first, last):	
	
	if not all((first,last)):
		askFrameRange = nuke.getFramesAndViews('framerange', '%s-%s' %(nuke.thisNode().firstFrame(),nuke.thisNode().lastFrame()))
	elif first == last:
		askFrameRange = nuke.getFramesAndViews('framerange', '%s' %(first))
	else:
		askFrameRange = nuke.getFramesAndViews('framerange', '%s-%s' %(first,last))
	
	if askFrameRange:
		FrameRange = askFrameRange[0].split('-')
	else:
		return
	
	try:
		map(int, FrameRange)
		return map(int, FrameRange)			
	except ValueError:
		nuke.message('Enter a valid framerange')
		return

def checkInput():
	
	### list possilbe geometry and camera classes known in nuke and dneg-nuke
	geoClass = ['Card2', 'Card', 'Cube', 'Cylinder', 'PoissonMesh', 'Sphere', 'ReadGeo2', 'ReadGeo', 'ZenGeo4', 'Scene']
	camClass = ['Camera2', 'Camera', 'ZenCamera']
	
	returnDir = {'geo': False, 'cam': False}
	
	input0 = getInput(nuke.thisNode(), 0)
	input1 = getInput(nuke.thisNode(), 1)
	
	if input0 is not None:
		if input0.Class() in geoClass:
			returnDir['geo'] = True
			
	if input1 is not None:
		if input1.Class() in camClass:
			returnDir['cam'] = True
			
	return returnDir
	
def cameraProjectionMatrix(cameraNode, frame, outputFormat):
    wm = nuke.math.Matrix4()
    for i in xrange(16):
        wm[i] = cameraNode['matrix'].getValueAt(frame,i)

    wm.transpose()
    camTransform = wm.inverse()

    roll = float(cameraNode['winroll'].getValueAt(frame,0))
    scale_x = float(cameraNode['win_scale'].getValueAt(frame,0))
    scale_y = float(cameraNode['win_scale'].getValueAt(frame,1))
    translate_x = float(cameraNode['win_translate'].getValueAt(frame,0))
    translate_y = float(cameraNode['win_translate'].getValueAt(frame,1))
    m = nuke.math.Matrix4()
    m.makeIdentity()
    m.rotateZ(math.radians(roll))
    m.scale(1.0 / scale_x, 1.0 / scale_y, 1.0)
    m.translate(-translate_x, -translate_y, 0.0)
    
    focal_length = float(cameraNode['focal'].getValueAt(frame))
    h_aperture = float(cameraNode['haperture'].getValueAt(frame))
    near = float(cameraNode['near'].getValueAt(frame))
    far = float(cameraNode['far'].getValueAt(frame))
    projection_mode = int(cameraNode['projection_mode'].getValueAt(frame))
    p = nuke.math.Matrix4()
    p.projection(focal_length / h_aperture, near, far, projection_mode == 0)
    
    imageAspect = float(outputFormat.height()) / float(outputFormat.width())
    t = nuke.math.Matrix4()
    t.makeIdentity()
    t.translate( 1.0, 1.0 - (1.0 - imageAspect / float(outputFormat.pixelAspect())), 0.0 )
    
    x_scale = float(outputFormat.width()) / 2.0
    y_scale = x_scale * outputFormat.pixelAspect()
    s = nuke.math.Matrix4()
    s.makeIdentity()
    s.scale(x_scale, y_scale, 1.0)


    return s * t * p * m * camTransform

def projectPoints(frame, outputFormat, camera = None, points = None):
    camNode = camera

    camMatrix = cameraProjectionMatrix(camNode, frame, outputFormat)

    for point in points:
        pt = nuke.math.Vector3(point[0],point[1],point[2])
        tPos = camMatrix * nuke.math.Vector4(pt.x, pt.y, pt.z, 1.0)
        yield nuke.math.Vector2(tPos.x / tPos.w, tPos.y / tPos.w)
