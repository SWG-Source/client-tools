//======================================================================
//
// UITypeID.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_UITypeID_H
#define INCLUDED_UITypeID_H

//======================================================================

enum UITypeID
{
	TUIObject,                  // All objects are of this type

		TUINamespace,

		TUIImageFragment,
			TUIImageFrame,      // Frames are polymorphic with fragments, but image styles can only contain frames		

		TUIFontCharacter,
		TUICursor,

		TUIClock,

		TUIDataSource,
		TUIData,

		TUITemplate,
		TUITooltip,

		TUIWidget,
			TUI3DObjectListViewer,
			TUIAnimation,
			TUIButton,
			TUICheckbox,
			TUIComboBox,
			TUIComposite,
			TUIDropdownbox,
			TUIEllipse,
			TUIGBuffer,
			TUIGrid,
			TUIImage,
			TUIList,
			TUIListbox,
			TUIPage,
			TUIPopupMenu,
			TUIProgressbar,
			TUIRadialMenu,
			TUIScrollbar,
			TUISliderbar,
			TUISliderbase,
			TUISliderplane,
			TUITabSet,
			TUITabbedPane,
			TUIText,
			TUITextbox,
			TUITreeView,
			TUIUnknown,
			TUIVolumePage,
			TUIPie,
			TUIRunner,
			TUIWebBrowser,

		TUIStyle,
			TUIButtonStyle,
			TUICheckboxStyle,
			TUIDropdownboxStyle,
			TUIGridStyle,
			TUIImageStyle,
			TUIListStyle,
			TUIListboxStyle,
			TUIPopupMenuStyle,
			TUIProgressbarStyle,
			TUIRadialMenuStyle,
			TUIRectangleStyle,
			TUIScrollbarStyle,
			TUISliderbarStyle,
			TUISliderplaneStyle,
			TUITabSetStyle,
			TUITabbedPaneStyle,
			TUITextStyle,
			TUITextboxStyle,
			TUITooltipStyle,
			TUIWidgetStyle,
			TUITreeViewStyle,
			TUIPieStyle,

		TUIPalette,

		TUIEffector,
			TUIColorEffector,
			TUIDeformer,
				TUIDeformerHUD,
				TUIDeformerRotate,
				TUIDeformerWave,
			TUILocationEffector,
			TUIOpacityEffector,
			TUIScrollLocationEffector,
			TUISizeEffector,
			TUIRotationEffector,

		TUICanvas,
			TUIDirect3DPrimaryCanvas,
			TUIDirect3DTextureCanvas,
			TUIDirectDrawCanvas,
			TUIDirectDrawOffscreenCanvas,
			TUIDirectDrawPrimaryCanvas,		

		TUICanvasGenerator,

		TUICursorSet,

		TUITable,
		TUITableModel,
		TUITableModelDefault,
		TUITableColumnModel,
		TUITableColumn,
		TUITableHeader,

		TUIDataSourceBase,
		TUIDataSourceContainer,

		TUITextStyleManager,

		TUINumTypes
};


//======================================================================

#endif
