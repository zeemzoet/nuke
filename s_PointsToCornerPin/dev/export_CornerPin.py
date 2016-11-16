n = nuke.thisNode()

if nuke.thisNode()['exportBool'].getValue():

    _refFrame = int(n['refFrame'].getValue())

    if not _refFrame:
        _refFrame = nuke.frame()
        n['refFrame'].setValue(_refFrame)

    nuke.root().begin()
    cp = nuke.createNode("CornerPin2D")
    cp.setInput(0, None)

    nuke.frame(_refFrame)

    tmpKnob = nuke.XY_Knob('')

    # compare to1 to to2,to3,t04
    smallest = n['to1'].getValue()[1]
    knob = 1
    for k in range(2,5):
        if n['to'+str(k)].getValue()[1] < smallest:
            smallest = n['to'+str(k)].getValue()[1]
            knob = k
                
    if knob != 1:
        tmpKnob.copyAnimations(n['to1'].animations())   
        n['to1'].copyAnimations(n['to'+str(knob)].animations())
        n['to'+str(knob)].copyAnimations(tmpKnob.animations())
        
    # compare to2 to to3,to4
    smallest = n['to2'].getValue()[1]
    knob = 2
    for k in range(3,5):
        if n['to'+str(k)].getValue()[1] < smallest:
            smallest = n['to'+str(k)].getValue()[1]
            knob = k

    if knob != 2:        
        tmpKnob.copyAnimations(n['to2'].animations())
        n['to2'].copyAnimations(n['to'+str(knob)].animations())
        n['to'+str(knob)].copyAnimations(tmpKnob.animations())

    # compare to1 to to2
    if n['to1'].getValue()[0] > n['to2'].getValue()[0]:
        tmpKnob.copyAnimations(n['to1'].animations())
        n['to1'].copyAnimations(n['to2'].animations())
        n['to2'].copyAnimations(tmpKnob.animations())

    # compare to3 to to4
    if n['to3'].getValue()[0] < n['to4'].getValue()[0]:
        tmpKnob.copyAnimations(n['to3'].animations())
        n['to3'].copyAnimations(n['to4'].animations())
        n['to4'].copyAnimations(tmpKnob.animations())


    choice = int(n['exportChoice'].getValue())
    _range = [1, 2, 3, 4]
    if choice == 0:
        for i in _range:
            cp['to%d' %i].copyAnimations(n['to%d' %i].animations())
    elif choice == 1:
        for i in _range:
            if n['refAlive'].getValue():
                cp['to%d' %i].setExpression('%s.to%d(%s.refFrame)' %(n.name(),i,n.name()))
            else:   
                cp['to%d' %i].setExpression('%s.to%d(%d)' %(n.name(),i,_refFrame))
            cp['from%d' %i].setExpression('%s.to%d' %(n.name(),i))
    elif choice == 2:
        for i in _range:
            cp['to%d' %i].setExpression('%s.to%d' %(n.name(),i))
            if n['refAlive'].getValue():
                cp['from%d' %i].setExpression('%s.to%d(%s.refFrame)' %(n.name(),i,n.name()))
            else:
                cp['from%d' %i].setExpression('%s.to%d(%d)' %(n.name(),i,_refFrame))
    elif choice == 3:
        for i in _range:
            cp['to%d' %i].setValue(n['to%d' %i].getValueAt(n['refFrame'].getValue()))
            cp['from%d' %i].copyAnimations(n['to%d' %i].animations())
    elif choice == 4:
        for i in _range:
            cp['to%d' %i].copyAnimations(n['to%d' %i].animations())
            cp['from%d' %i].setValue(n['to%d' %i].getValueAt(n['refFrame'].getValue()))
    
else:
    nuke.message('Generate your cornerpin first')
