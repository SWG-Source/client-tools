#include "FirstUiBuilder.h"
#include "ObjectFactory.h"
#include "DefaultObjectPropertiesManager.h"

#include "UIButton.h"
#include "UIButtonStyle.h"
#include "UICheckbox.h"
#include "UICheckboxStyle.h"
#include "UIClock.h"
#include "UIColorEffector.h"
#include "UIComboBox.h"
#include "UIComposite.h"
#include "UICursor.h"
#include "UICursorSet.h"
#include "UIData.h"
#include "UIDataSource.h"
#include "UIDataSourceContainer.h"
#include "UIDeformerHUD.h"
#include "UIDeformerRotate.h"
#include "UIDeformerWave.h"
#include "UIDropdownbox.h"
#include "UIDropdownboxStyle.h"
#include "UIEllipse.h"
#include "UIImage.h"
#include "UIImageStyle.h"
#include "UIGrid.h"
#include "UIGridStyle.h"
#include "UIList.h"
#include "UIListStyle.h"
#include "UIListbox.h"
#include "UIListboxStyle.h"
#include "UILocationEffector.h"
#include "UIManager.h"
#include "UINamespace.h"
#include "UIOpacityEffector.h"
#include "UIPage.h"
#include "UIPalette.h"
#include "UIPie.h"
#include "UIPieStyle.h"
#include "UIPopupMenu.h"
#include "UIPopupMenuStyle.h"
#include "UIProgressbar.h"
#include "UIRadialMenu.h"
#include "UIRadialMenuStyle.h"
#include "UIRectangleStyle.h"
#include "UIRotationEffector.h"
#include "UIRunner.h"
#include "UIScrollbar.h"
#include "UISizeEffector.h"
#include "UISliderbar.h"
#include "UISliderbarStyle.h"
#include "UISliderplane.h"
#include "UITabbedPane.h"
#include "UITabbedPaneStyle.h"
#include "UITable.h"
#include "UITableHeader.h"
#include "UITableModelDefault.h"
#include "UITabSet.h"
#include "UITabSetStyle.h"
#include "UITemplate.h"
#include "UIText.h"
#include "UITextbox.h"
#include "UITextboxStyle.h"
#include "UITooltipStyle.h"
#include "UITreeView.h"
#include "UITreeViewStyle.h"
#include "UIUnknown.h"
#include "UIVolumePage.h"

#include "ImportImageDialog.h"
#include "PageFileDialog.h"

ObjectFactory::ObjectFactory(const DefaultObjectPropertiesManager &defaultProperties)
: m_defaults(defaultProperties)
{
}

UIBaseObject *ObjectFactory::newObject(unsigned menuID, UIBaseObject *const i_selectedParent) const
{
	UIBaseObject	*NewObject = 0;

	switch( menuID )
	{
		case ID_INSERT_BUTTON:
			NewObject  = new UIButton;
			break;
		case ID_INSERT_CHECKBOX:
			NewObject = new UICheckbox;
			break;
		case ID_INSERT_DATASOURCE:
			NewObject = new UIDataSource;
			break;
		case ID_INSERT_DATASOURCECONTAINER:
			NewObject = new UIDataSourceContainer;
			break;
		case ID_INSERT_DATAITEM:
			NewObject = new UIData;
			break;
		case ID_INSERT_DROPDOWNBOX:
			NewObject = new UIDropdownbox;
			break;
		case ID_INSERT_ELLIPSE:
			NewObject = new UIEllipse;
			break;		
		case ID_INSERT_IMAGE:
			NewObject = new UIImage;
			break;
		case ID_INSERT_GRID:
			NewObject = new UIGrid;
			break;
		case ID_INSERT_IMAGEFRAME:
		{
			ImportImageDialog iid;
			int result = iid.DoModal();
			if (result==-1)
			{
				result=0;
			}
			NewObject = reinterpret_cast<UIBaseObject *>(result);
		} break;
		case ID_INSERT_LISTBOX:
			NewObject = new UIListbox;
			break;
		case ID_INSERT_LIST:
			NewObject = new UIList;
			break;
		case ID_INSERT_TREEVIEW:
			NewObject = new UITreeView;
			break;
		case ID_INSERT_PIE:
			NewObject = new UIPie;
			break;
		case ID_INSERT_RUNNER:
			NewObject = new UIRunner;
			break;
		case ID_INSERT_COMBOBOX:
			NewObject = new UIComboBox;
			break;
		case ID_INSERT_NAMESPACE:
			NewObject = new UINamespace;
			break;
		case ID_INSERT_TEXTBOX:
			NewObject = new UITextbox;
			break;
		case ID_INSERT_SCROLLBAR:
			NewObject = new UIScrollbar;
			break;
		case ID_INSERT_SLIDERBAR:
			NewObject = new UISliderbar;
			break;
		case ID_INSERT_SLIDERPLANE:
			NewObject = new UISliderplane;
			break;
		case ID_INSERT_TABSET:
			NewObject = new UITabSet;
			break;
		case ID_INSERT_TABBEDPANE:
			NewObject = new UITabbedPane;
			break;
		case ID_INSERT_POPUPMENU:
			NewObject = new UIPopupMenu;
			break;
		case ID_INSERT_RADIALMENU:
			NewObject = new UIRadialMenu;
			break;
		case ID_INSERT_CURSORSET:
			NewObject = new UICursorSet;
			break;
		case ID_INSERT_VOLUMEPAGE:
			NewObject = new UIVolumePage;
			break;
		case ID_INSERT_STATICTEXT:
			NewObject = new UIText;
			break;
		case ID_INSERT_TEMPLATE:
			NewObject = new UITemplate;
			break;
		case ID_INSERT_PAGE:
		{
			PageFileDialog pfd;
			int result = pfd.DoModal();
			if (result==-1)
			{
				result=0;
			}
			NewObject = reinterpret_cast<UIBaseObject *>(result);
			break;
		}
		case ID_INSERT_COMPOSITE:
			NewObject = new UIComposite;
			break;
		case ID_INSERT_TABLE:
			NewObject = new UITable;
			break;
		case ID_INSERT_TABLEMODELDEFAULT:
			NewObject = new UITableModelDefault;
			break;
		case ID_INSERT_TABLEHEADER:
			NewObject = new UITableHeader;
			break;

		case ID_INSERT_PROGRESSBAR:
			NewObject = new UIProgressbar;
			break;
		case ID_INSERT_CURSOR:
			NewObject = new UICursor;
			break;
		case ID_INSERT_CLOCK:
			NewObject = new UIClock;
			break;
		case ID_INSERT_APPLICATIONOBJECT:
			NewObject = new UIUnknown;
			break;			
		case ID_INSERT_STYLE_BUTTON:
			NewObject = new UIButtonStyle;
			break;
		case ID_INSERT_STYLE_CHECKBOX:
			NewObject = new UICheckboxStyle;
			break;
		case ID_INSERT_STYLE_GRID:
			NewObject = new UIGridStyle;
			break;
		case ID_INSERT_STYLE_IMAGE:
			NewObject = new UIImageStyle;
			break;		
		case ID_INSERT_STYLE_LISTBOX:
			NewObject = new UIListboxStyle;
			break;
		case ID_INSERT_STYLE_LIST:
			NewObject = new UIListStyle;
			break;
		case ID_INSERT_STYLE_TREEVIEW:
			NewObject = new UITreeViewStyle;
			break;
		case ID_INSERT_STYLE_PIE:
			NewObject = new UIPieStyle;
			break;
		case ID_INSERT_STYLE_PALETTE:
			NewObject = new UIPalette;
			break;
		case ID_INSERT_STYLE_TEXTBOX:
			NewObject = new UITextboxStyle;
			break;
		case ID_INSERT_STYLE_TEXT:
			NewObject = new UITextStyle;
			break;
		case ID_INSERT_STYLE_PROGRESSBAR:
			NewObject = new UIProgressbarStyle;
			break;
		case ID_INSERT_STYLE_DROPDOWNBOX:
			NewObject = new UIDropdownboxStyle;
			break;
		case ID_INSERT_STYLE_SLIDERBAR:
			NewObject = new UISliderbarStyle;
			break;		
		case ID_INSERT_STYLE_TOOLTIP:
			NewObject = new UITooltipStyle;
			break;
		case ID_INSERT_STYLE_RECTANGLE:
			NewObject = new UIRectangleStyle;
			break;
		case ID_INSERT_STYLE_TABSET:
			NewObject = new UITabSetStyle;
			break;
		case ID_INSERT_STYLE_TABBEDPANE:
			NewObject = new UITabbedPaneStyle;
			break;
		case ID_INSERT_STYLE_POPUPMENU:
			NewObject = new UIPopupMenuStyle;
			break;
		case ID_INSERT_STYLE_RADIALMENU:
			NewObject = new UIRadialMenuStyle;
			break;
		
		case ID_INSERT_EFFECTOR_COLOR:
			NewObject = new UIColorEffector;
			break;

		case ID_INSERT_EFFECTOR_ROTATION:
			NewObject = new UIRotationEffector;
			break;

		case ID_INSERT_EFFECTOR_LOCATION:
			NewObject = new UILocationEffector;
			break;

		case ID_INSERT_EFFECTOR_OPACITY:
			NewObject = new UIOpacityEffector;
			break;

		case ID_INSERT_EFFECTOR_SIZE:
			NewObject = new UISizeEffector;
			break;

		case ID_INSERT_DEFORMER_HUD:
			NewObject = new UIDeformerHUD;
			break;

		case ID_INSERT_DEFORMER_WAVE:
			NewObject = new UIDeformerWave;
			break;

		case ID_INSERT_DEFORMER_ROTATE:
			NewObject = new UIDeformerRotate;
			break;
	}

	return NewObject;
}
