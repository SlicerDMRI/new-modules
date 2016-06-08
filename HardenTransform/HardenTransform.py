
import slicer
import vtk
import ctk
import qt
import os
import ntpath

#
# HardenTransform
#

class HardenTransform:
  def __init__(self, parent):
    parent.title = "HardenTransform"
    parent.categories = ["Examples"]
    parent.dependencies = []
    parent.contributors = [""] # replace with "Firstname Lastname (Org)"
    parent.helpText = """"""
    parent.acknowledgementText = """
    """ # replace with organization, grant and thanks.
    self.parent = parent

#
# qHardenTransformWidget
#

class HardenTransformWidget:
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
    ioCollapsible = ctk.ctkCollapsibleButton()
    ioCollapsible.text = "Input and Output Selection"
    self.layout.addWidget(ioCollapsible)

    # Layout within the sample collapsible button
    ioFormLayout = qt.QFormLayout(ioCollapsible)
       
    # input directory selection
    self.inputDirSelector = ctk.ctkPathLineEdit()
    self.inputDirSelector.filters = ctk.ctkPathLineEdit.Dirs
    self.inputDirSelector.options = ctk.ctkPathLineEdit.ShowDirsOnly
    self.inputDirSelector.settingKey = 'inputDir'
    ioFormLayout.addRow("Input Directory:", self.inputDirSelector)
       
    # output directory selection
    self.outputDirSelector = ctk.ctkPathLineEdit()
    self.outputDirSelector.filters = ctk.ctkPathLineEdit.Dirs
    self.outputDirSelector.options = ctk.ctkPathLineEdit.ShowDirsOnly
    self.outputDirSelector.settingKey = 'outputDir'
    ioFormLayout.addRow("Empty Output Directory:", self.outputDirSelector)
     
    # transform file selection
    self.transformationSelector = ctk.ctkPathLineEdit()
    self.transformationSelector.filters = ctk.ctkPathLineEdit.Files
    self.transformationSelector.settingKey = 'transformationFile'
    ioFormLayout.addRow("Transform file:", self.transformationSelector)
       
    # button to run
    runTransformButton = qt.QPushButton("Harden Transform") 
    runTransformButton.toolTip = "Harden models in selected directory and save to output directory." 
    ioFormLayout.addWidget(runTransformButton)
    runTransformButton.connect('clicked(bool)', self.onrunTransformButtonClicked) 

    # Set local var as instance attribute
    self.runTransformButton = runTransformButton
    
  def onrunTransformButtonClicked(self):
	  
    inputDirPath = self.inputDirSelector.currentPath
    outputDirPath = self.outputDirSelector.currentPath
    transformFilePath = self.transformationSelector.currentPath
	  
    if not (inputDirPath) or not (outputDirPath) or not (transformFilePath):
      qt.QMessageBox.critical(slicer.util.mainWindow(),
          'Harden Transform', 'Tranformation file, input and output directories are required')
      return
	# model files with these extensions are loaded and transformed
    extensions = ('.vtk', '.vtp', '.stl', '.ply')

    # function to check if a dir2 is a subdirectory of dir1
    # returns true if dir2 is a subdirectory of dir1 or dir1 == dir2
    def is_subdir(dir1, dir2):
      path = os.path.realpath(dir1)
      directory = os.path.realpath(dir2)
      relative = os.path.relpath(dir1, dir2)
      return not (relative == os.pardir or relative.startswith(os.pardir + os.sep))  

    # check if input and output folders are identical or output is subdirectory of input, if so, do nothing to avoid overwriting
    if is_subdir(inputDirPath,outputDirPath):
	  qt.QMessageBox.critical(slicer.util.mainWindow(),'Harden Transform', 'Please select an output directory that is different from the input directory and not a subdirectory of the input to avoid overwriting')
    else:
      # load transform file
      checkSuccessT, transformNode = slicer.util.loadTransform(str(transformFilePath),returnNode = True)
      if not checkSuccessT:
		  print 'Could not load transform file'
		  qt.QMessageBox.information(slicer.util.mainWindow(),'Harden Transform', 'Could not load transform file')
		  exit()

      logic = slicer.vtkSlicerTransformLogic()
      tNodeID = transformNode.GetID()
      
      transformCounter = 0;
      for root, subfolders, files in os.walk(inputDirPath):
        for file in files:
		  # only consider model filetypes slicer an read and write (vtk,vtp,stl,ply)
          ext = os.path.splitext(file)[-1].lower()
          if ext in extensions:
            filePath = os.path.join(root,file)
            checkSuccessM, modelNode = slicer.util.loadModel(filePath,returnNode = True)
            if not checkSuccessM:
              print 'Could not load model file ' + str(filepath)
              qt.QMessageBox.information(slicer.util.mainWindow(),'Harden Transform', 'Could not load model file' + str(filepath))

            modelFileName, modelFileExt = os.path.splitext(ntpath.basename(filePath))

            modelNode.SetAndObserveTransformNodeID(tNodeID)
                      
            # harden transform
            logic.hardenTransform(modelNode)    

            ## save transformed node
            # if file with this name already exists in the output folder, append '_new'
            app = ''
            while os.path.isfile(os.path.join(outputDirPath,os.path.splitext(ntpath.basename(file))[0]+app+modelFileExt)):
              app += '_new'

            newFileName = modelFileName+app+modelFileExt
            slicer.util.saveNode(modelNode, os.path.join(outputDirPath,newFileName)) 

            slicer.mrmlScene.RemoveNode(modelNode)

            transformCounter += 1

    slicer.mrmlScene.RemoveNode(transformNode)
    statusString = 'Transformed ' + str(transformCounter) + ' models'
    print statusString
    qt.QMessageBox.information(slicer.util.mainWindow(),'Harden Transform', statusString)
