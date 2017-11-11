//======================================================================
//
// UILoaderSetup.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "_precompile.h"
#include "UILoaderSetup.h"

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
#include "UIDropdownbox.h"
#include "UIDropdownboxStyle.h"
#include "UIDeformerHUD.h"
#include "UIDeformerRotate.h"
#include "UIDeformerWave.h"
#include "UIEllipse.h"
#include "UIFontCharacter.h"
#include "UIGrid.h"
#include "UIGridStyle.h"
#include "UIImage.h"
#include "UIImageFragment.h"
#include "UIImageFrame.h"
#include "UIImageStyle.h"
#include "UIList.h"
#include "UIListStyle.h"
#include "UIListbox.h"
#include "UIListboxStyle.h"
#include "UILoader.h"
#include "UILocationEffector.h"
#include "UINamespace.h"
#include "UIOpacityEffector.h"
#include "UIOutputStream.h"
#include "UIPage.h"
#include "UIPalette.h"
#include "UIPie.h"
#include "UIPieStyle.h"
#include "UIPopupMenu.h"
#include "UIPopupMenuStyle.h"
#include "UIProgressbar.h"
#include "UIProgressbarStyle.h"
#include "UIRadialMenu.h"
#include "UIRadialMenuStyle.h"
#include "UIRectangleStyle.h"
#include "UIRotationEffector.h"
#include "UIRunner.h"
#include "UIScrollLocationEffector.h"
#include "UIScrollbar.h"
#include "UISizeEffector.h"
#include "UISliderbar.h"
#include "UISliderbarStyle.h"
#include "UISliderplane.h"
#include "UIStandardLoader.h"
#include "UITabSet.h"
#include "UITabSetStyle.h"
#include "UITabbedPane.h"
#include "UITabbedPaneStyle.h"
#include "UITable.h"
#include "UITableHeader.h"
#include "UITableModelDefault.h"
#include "UITemplate.h"
#include "UIText.h"
#include "UITextStyle.h"
#include "UITextStyleManager.h"
#include "UITextbox.h"
#include "UITextboxStyle.h"
#include "UITooltipStyle.h"
#include "UITreeView.h"
#include "UITreeViewStyle.h"
#include "UIUnknown.h"
#include "UIVolumePage.h"

//======================================================================

namespace
{
	const UIStandardLoader<UINamespace>             gNamespaceLoader;
	const UIStandardLoader<UICursor>                gCursorLoader;
	const UIStandardLoader<UIClock>                 gClockLoader;
	
	const UIStandardLoader<UIImageFragment>         gImageFragmentLoader;
	const UIStandardLoader<UIImageFrame>            gImageFrameLoader;
	const UIStandardLoader<UIDataSource>            gStaticDataSourceLoader;
	const UIStandardLoader<UIData>                  gDataItemLoader;
	
	const UIStandardLoader<UITemplate>               gTemplateLoader;
	
	const UIStandardLoader<UIImageStyle>             gImageStyleLoader;
	const UIStandardLoader<UIFontCharacter>          gFontCharacterLoader;
	const UIStandardLoader<UITextStyle>              gTextStyleLoader;
	const UIStandardLoader<UIButtonStyle>            gButtonStyleLoader;
	const UIStandardLoader<UITextboxStyle>           gTextboxStyleLoader;
	const UIStandardLoader<UICheckboxStyle>          gCheckboxStyleLoader;
	const UIStandardLoader<UIListboxStyle>           gListboxStyleLoader;
	const UIStandardLoader<UIDropdownboxStyle>       gDropdownboxStyleLoader;
	const UIStandardLoader<UIProgressbarStyle>       gProgressbarStyleLoader;
	const UIStandardLoader<UISliderbarStyle>         gSliderbarStyleLoader;
	const UIStandardLoader<UIGridStyle>              gGridStyleLoader;
	const UIStandardLoader<UITabSetStyle>            gTabSetStyleLoader;
	const UIStandardLoader<UITooltipStyle>           gTooltipStyleLoader;
	const UIStandardLoader<UIRectangleStyle>         gRectangleStyleLoader;
	const UIStandardLoader<UIPopupMenuStyle>         gPopupMenuStyleLoader;
	const UIStandardLoader<UIRadialMenuStyle>        gRadialMenuStyleLoader;
	
	const UIStandardLoader<UIPage>                   gPageLoader;
	const UIStandardLoader<UIText>                   gTextLoader;
	const UIStandardLoader<UIImage>                  gImageLoader;
	const UIStandardLoader<UITextbox>                gTextboxLoader;
	const UIStandardLoader<UIButton>                 gButtonLoader;
	const UIStandardLoader<UICheckbox>               gCheckboxLoader;
	const UIStandardLoader<UIScrollbar>              gScrollbarLoader;
	const UIStandardLoader<UIListbox>                gListboxLoader;
	const UIStandardLoader<UIDropdownbox>            gDropdownboxLoader;
	const UIStandardLoader<UIProgressbar>            gProgressbarLoader;
	const UIStandardLoader<UISliderbar>              gSliderbarLoader;
	const UIStandardLoader<UISliderplane>            gSliderplaneLoader;
	const UIStandardLoader<UIGrid>                   gGridLoader;
	const UIStandardLoader<UITabSet>                 gTabSetLoader;
	const UIStandardLoader<UIEllipse>                gEllipseLoader;
	const UIStandardLoader<UIPopupMenu>              gPopupMenuLoader;
	const UIStandardLoader<UIRadialMenu>             gRadialMenuLoader;
	const UIStandardLoader<UIVolumePage>             gVolumePageLoader;
	const UIStandardLoader<UIPie>                    gPieLoader;
	const UIStandardLoader<UIRunner>                 gRunnerLoader;
	
	const UIStandardLoader<UIPalette>                gPaletteLoader;

	const UIStandardLoader<UILocationEffector>       gLocationEffectorLoader;
	const UIStandardLoader<UIScrollLocationEffector> gScrollLocationEffectorLoader;
	const UIStandardLoader<UISizeEffector>           gSizeEffectorLoader;
	const UIStandardLoader<UIOpacityEffector>        gOpacityEffectorLoader;
	const UIStandardLoader<UIColorEffector>          gColorEffectorLoader;
	const UIStandardLoader<UIRotationEffector>       gRotationEffectorLoader;

	const UIStandardLoader<UIDeformerHUD>            gDeformerHud;
	const UIStandardLoader<UIDeformerRotate>         gDeformerRotate;
	const UIStandardLoader<UIDeformerWave>           gDeformerWave;
	
	const UIStandardLoader<UITabbedPane>             gTabbedPaneLoader;
	const UIStandardLoader<UITabbedPaneStyle>        gTabbedPaneStyleLoader;

	const UIStandardLoader<UICursorSet>              gCursorSetLoader;

	const UIStandardLoader<UITable>                  gTableLoader;
	const UIStandardLoader<UITableModelDefault>      gTableModelDefaultLoader;
	const UIStandardLoader<UIDataSourceContainer>    gDataSourceContainerLoader;
	const UIStandardLoader<UITableHeader>            gTableHeaderLoader;

	const UIStandardLoader<UIComposite>              gCompositeLoader;
	const UIStandardLoader<UIComboBox>               gComboBoxLoader;
	const UIStandardLoader<UIList>                   gListLoader;
	const UIStandardLoader<UIListStyle>              gListStyleLoader;

	const UIStandardLoader<UITreeView>               gTreeViewLoader;
	const UIStandardLoader<UITreeViewStyle>          gTreeViewStyleLoader;
	const UIStandardLoader<UIPieStyle>               gPieStyleLoader;

	const UITextStyleManager::TextStyleManagerLoader gTextStyleManager;

}

//----------------------------------------------------------------------

void UILoaderSetup::performSetup (UILoader & loader)
{
	// Base Objects
	loader.AddToken( UINamespace::TypeName, &gNamespaceLoader );
	loader.AddToken( UICursor::TypeName, &gCursorLoader );

	loader.AddToken( UIImageFragment::TypeName, &gImageFragmentLoader );
	loader.AddToken( UIImageFrame::TypeName, &gImageFrameLoader );
	loader.AddToken( UIDataSource::TypeName, &gStaticDataSourceLoader );
	loader.AddToken( UIData::TypeName,       &gDataItemLoader );
	loader.AddToken( UIDataSourceContainer::TypeName, &gDataSourceContainerLoader);

	loader.AddToken( UITemplate::TypeName, &gTemplateLoader );
	loader.AddToken(UICursorSet::TypeName, &gCursorSetLoader);

	loader.AddToken(UITable::TypeName,  &gTableLoader);
	loader.AddToken(UITableModelDefault::TypeName, &gTableModelDefaultLoader);
	loader.AddToken(UITableHeader::TypeName,  &gTableHeaderLoader);

	// Styles
	loader.AddToken( UIImageStyle::TypeName,       &gImageStyleLoader );
	loader.AddToken( UIFontCharacter::TypeName,    &gFontCharacterLoader );
	loader.AddToken( UITextStyle::TypeName,        &gTextStyleLoader );
	loader.AddToken( UIButtonStyle::TypeName,      &gButtonStyleLoader );
	loader.AddToken( UITextboxStyle::TypeName,     &gTextboxStyleLoader );
	loader.AddToken( UICheckboxStyle::TypeName,    &gCheckboxStyleLoader );
	loader.AddToken( UIListboxStyle::TypeName,     &gListboxStyleLoader );
	loader.AddToken( UIDropdownboxStyle::TypeName, &gDropdownboxStyleLoader );
	loader.AddToken( UIProgressbarStyle::TypeName, &gProgressbarStyleLoader );
	loader.AddToken( UISliderbarStyle::TypeName,   &gSliderbarStyleLoader );
	loader.AddToken( UIGridStyle::TypeName,        &gGridStyleLoader );
	loader.AddToken( UITabSetStyle::TypeName,      &gTabSetStyleLoader );
	loader.AddToken( UITooltipStyle::TypeName,     &gTooltipStyleLoader );
	loader.AddToken( UIRectangleStyle::TypeName,   &gRectangleStyleLoader );
	loader.AddToken( UITabbedPaneStyle::TypeName,  &gTabbedPaneStyleLoader);
	loader.AddToken( UIPopupMenuStyle::TypeName,   &gPopupMenuStyleLoader);
	loader.AddToken( UIRadialMenuStyle::TypeName,  &gRadialMenuStyleLoader);
	loader.AddToken( UIListStyle::TypeName,        &gListStyleLoader);
	loader.AddToken( UITreeViewStyle::TypeName,    &gTreeViewStyleLoader);
	loader.AddToken( UIPieStyle::TypeName,         &gPieStyleLoader);

	loader.AddToken (UIPalette::TypeName,          &gPaletteLoader);

	// Widgets
	loader.AddToken( UIPage::TypeName, &gPageLoader );
	loader.AddToken( UIComposite::TypeName, &gCompositeLoader );
	loader.AddToken( UIText::TypeName, &gTextLoader );
	loader.AddToken( UIImage::TypeName, &gImageLoader );
	loader.AddToken( UITextbox::TypeName, &gTextboxLoader );
	loader.AddToken( UIButton::TypeName, &gButtonLoader );
	loader.AddToken( UICheckbox::TypeName, &gCheckboxLoader );
	loader.AddToken( UIScrollbar::TypeName, &gScrollbarLoader );
	loader.AddToken( UIListbox::TypeName, &gListboxLoader );
	loader.AddToken( UIDropdownbox::TypeName, &gDropdownboxLoader );
	loader.AddToken( UIProgressbar::TypeName, &gProgressbarLoader );
	loader.AddToken( UISliderbar::TypeName, &gSliderbarLoader );
	loader.AddToken( UISliderplane::TypeName, &gSliderplaneLoader );
	loader.AddToken( UIGrid::TypeName, &gGridLoader );
	loader.AddToken( UITabSet::TypeName, &gTabSetLoader );
	loader.AddToken( UIEllipse::TypeName, &gEllipseLoader );	
	loader.AddToken( UITabbedPane::TypeName, &gTabbedPaneLoader);
	loader.AddToken( UIPopupMenu::TypeName, &gPopupMenuLoader);
	loader.AddToken( UIRadialMenu::TypeName, &gRadialMenuLoader);
	loader.AddToken( UIVolumePage::TypeName, &gVolumePageLoader);
	loader.AddToken( UIComboBox::TypeName, &gComboBoxLoader);
	loader.AddToken( UIList::TypeName, &gListLoader);
	loader.AddToken( UITreeView::TypeName, &gTreeViewLoader);
	loader.AddToken( UIPie::TypeName, &gPieLoader);
	loader.AddToken( UIRunner::TypeName, &gRunnerLoader);

	// Effectors
	loader.AddToken( UILocationEffector::TypeName, &gLocationEffectorLoader );	
	loader.AddToken( UIScrollLocationEffector::TypeName, &gScrollLocationEffectorLoader );	
	loader.AddToken( UISizeEffector::TypeName, &gSizeEffectorLoader );	
	loader.AddToken( UIOpacityEffector::TypeName, &gOpacityEffectorLoader );	
	loader.AddToken( UIColorEffector::TypeName, &gColorEffectorLoader );	
	loader.AddToken( UIRotationEffector::TypeName, &gRotationEffectorLoader );	
	loader.AddToken( UIDeformerHUD::TypeName, &gDeformerHud);
	loader.AddToken( UIDeformerRotate::TypeName, &gDeformerRotate);
	loader.AddToken( UIDeformerWave::TypeName, &gDeformerWave);
	loader.AddToken( UITextStyleManager::TypeName, &gTextStyleManager);
}

//======================================================================
