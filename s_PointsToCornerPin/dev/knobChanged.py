n = nuke.selectedNode()

code = """
n=nuke.thisNode() 
k=nuke.thisKnob() 
if k.name() == "exportChoice": 
    if k.getValue() == 1.0 or k.getValue() == 2.0:
        n["refAlive"].setEnabled(True)
    else: 
        n["refAlive"].setEnabled(False)
"""

n['knobChanged'].setValue(code)



print n['knobChanged'].value()
