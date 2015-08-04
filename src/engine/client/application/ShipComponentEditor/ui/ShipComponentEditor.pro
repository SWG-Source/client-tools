unix {
  UI_DIR = .ui
  MOC_DIR = .moc
  OBJECTS_DIR = .obj
}
FORMS	= BaseChassisEditor.ui \
	BaseChassisEditorDetails.ui \
	BaseChassisEditorList.ui \
	BaseChassisNewDialog.ui \
	BaseTemplateNewDialog.ui \
	BaseComponentDescriptorsList.ui \
	BaseComponentDescriptorsDetails.ui \
	BaseComponentDescriptors.ui \
	BaseCompatibilityEditor.ui \
	BaseCompatibilityConfirmDialog.ui \
	BaseAttachmentsChassisEditorDetails.ui \
	BaseAttachmentsEditorList.ui \
	BaseAttachmentsChooser.ui \
	BaseShipComponentEditor.ui
TEMPLATE	=app
CONFIG	+= qt warn_on release
LANGUAGE	= C++
