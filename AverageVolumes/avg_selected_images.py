import slicer
import vtk
import ctk
import qt

# average selected scalar volumes
class avg_selected_images:
  def __init__(self, parent):
    parent.title = "Average Selected Volumes"
    parent.categories = ["Examples"]
    parent.dependencies = []
    parent.contributors = ["Isaiah Norton (BWH)"] # replace with "Firstname Lastname (Org)"
    parent.helpText = """
    Average selected scalar volumes for atlas creation
    """
    parent.acknowledgementText = """
    Acknonledgement text ...""" # replace with organization, grant and thanks.
    self.parent = parent


#
# AvgSelectedWidget
#

class avg_selected_imagesWidget:
  def __init__(self, parent = None):
    if not parent:
      self.parent = slicer.qMRMLWidget()
      self.parent.setLayout(qt.QVBoxLayout())
      self.parent.setMRMLScene(slicer.mrmlScene)
    else:
      self.parent = parent
      self.layout = self.parent.layout()
    if not parent:
      self.setup()
      self.parent.show()

  def setup(self):
    # Collapsible button
    self.avgvolumesCollapsibleButton = ctk.ctkCollapsibleButton()
    self.avgvolumesCollapsibleButton.text = "Average Scalar Volumes"
    self.layout.addWidget(self.avgvolumesCollapsibleButton)

    # Layout within the averaging collapsible button
    self.avgvolumesFormLayout = qt.QFormLayout(self.avgvolumesCollapsibleButton)

    #
    # the volume selector
    #
    self.inputFrame = qt.QFrame(self.avgvolumesCollapsibleButton)
    self.inputFrame.setLayout(qt.QHBoxLayout())
    self.avgvolumesFormLayout.addWidget(self.inputFrame)
    self.inputSelector = qt.QLabel("Select input nodes: ", self.inputFrame)


    self.inputFrame.layout().addWidget(self.inputSelector)
    self.inputSelector = slicer.qMRMLCheckableNodeComboBox(self.inputFrame)
    self.inputSelector.nodeTypes = ( ("vtkMRMLScalarVolumeNode"), "" )
    self.inputSelector.addEnabled = True
    self.inputSelector.removeEnabled = True
    self.inputSelector.setMRMLScene( slicer.mrmlScene )
    self.inputFrame.layout().addWidget(self.inputSelector)

    self.outputFrame = qt.QFrame(self.avgvolumesCollapsibleButton)
    self.outputFrame.setLayout(qt.QHBoxLayout())
    self.avgvolumesFormLayout.addWidget(self.outputFrame)
    self.outputSelector = qt.QLabel("Select output reference volume ", self.outputFrame)
    self.outputFrame.layout().addWidget(self.outputSelector)
    self.outputSelector = slicer.qMRMLNodeComboBox(self.outputFrame)
    self.outputSelector.nodeTypes = ( ("vtkMRMLScalarVolumeNode"), "" )
    self.outputSelector.setMRMLScene( slicer.mrmlScene )
    self.outputFrame.layout().addWidget(self.outputSelector)


    # Ok button
    okButton = qt.QPushButton("Ok")
    okButton.toolTip = "Average selected volumes"
    self.avgvolumesFormLayout.addWidget(okButton)
    okButton.connect('clicked(bool)', self.onApply)

    # Add vertical spacer
    self.layout.addStretch(1)

    # Set local var as instance attribute
    self.okButton = okButton

  def onApply(self):

    inputVolume = self.inputSelector.currentNode()
    outputVolume = self.outputSelector.currentNode()

    if not (inputVolume):
      qt.QMessageBox.critical(
          slicer.util.mainWindow(),
          'Averaging', 'Input volumes are required')
      return

    outVolumeNode = self.outputSelector.currentNode()
    volumesLogic = slicer.modules.volumes.logic()
    # newNode = volumesLogic.CloneVolume(slicer.mrmlScene, sourceVolumeNode, 'average volume')

    # create output volume by cloning the selected reference volume
    # volumesLogic = slicer.modules.volumes.logic()
    newNode = volumesLogic.CloneVolume(self.inputSelector.currentNode(), "average volume")

    # vtk function to average the images
    imgsum = vtk.vtkImageWeightedSum()
    imgsum.NormalizeByWeightOn()

    # add all of the input image data
    for (i,n) in enumerate(self.inputSelector.checkedNodes()):
       print("adding input image: {0}".format(n.GetName()))
       cast = vtk.vtkImageCast()
       cast.SetOutputScalarTypeToFloat()
       cast.AddInputConnection(n.GetImageDataConnection())
       imgsum.AddInputConnection(cast.GetOutputPort())
       imgsum.SetWeight(i, 1.)

    # run the filter and set the result to the output image
    imgsum.Update() 
    newNode.SetAndObserveImageData(imgsum.GetOutput())

