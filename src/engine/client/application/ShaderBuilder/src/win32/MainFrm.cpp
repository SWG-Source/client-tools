// ======================================================================
// MainFrm.cpp : implementation of the CMainFrame class
// ======================================================================

#include "FirstShaderBuilder.h"
#include "ShaderBuilder.h"

#include "MainFrm.h"

#include "BlankView.h"
#include "ImplementationView.h"
#include "LeftView.h"
#include "MaterialView.h"
#include "PassView.h"
#include "SplitterView.h"
#include "StageView.h"
#include "TextureView.h"
#include "AlphaReferenceView.h"
#include "StencilReferenceView.h"
#include "TextureCoordinateSetView.h"
#include "TextureFactorView.h"
#include "TextureScrollView.h"
#include "EffectView.h"
#include "PixelShaderView.h"
#include "FixedFunctionPipelineView.h"
#include "VertexShaderView.h"
#include "VertexShaderProgram.h"
#include "PixelShaderProgramView.h"
#include "IncludeView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// ======================================================================
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

// ======================================================================

CMainFrame::CMainFrame()
{
	CNode::install();
}

// ----------------------------------------------------------------------

CMainFrame::~CMainFrame()
{
	CNode::remove();
}

// ----------------------------------------------------------------------

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!m_wndStatusBar.Create(this) || !m_wndStatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	return 0;
}

// ----------------------------------------------------------------------

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return TRUE;
}

// ----------------------------------------------------------------------

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

// ----------------------------------------------------------------------

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

// ----------------------------------------------------------------------

BOOL CMainFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext)
{
	UNREF(lpcs);
	// TODO: Add your specialized code here and/or call the base class
	if (!m_wndSplitter.CreateStatic(this, 1, 2))
	{
		TRACE0("Failed to create splitter window\n");
		return FALSE;
	}

	// Get the client rect first for calc left pane size
	CRect rect;
	GetClientRect(&rect);

	// create the left tree view first.
	if (!m_wndSplitter.CreateView(0, 0, RUNTIME_CLASS(CSplitterView), CSize(rect.Width()/3, 0), pContext))
	{
		TRACE0("Failed to create left pane view\n");
		return FALSE;
	}

	// the left window needs to know about this class to call ShowNode()
	CSplitterView *splitterView = static_cast<CSplitterView *>(m_wndSplitter.GetPane(0,0));
	leftView = static_cast<CLeftView *>(splitterView->m_wndSplitter.GetPane(0,0));
	leftView->mainFrame = this;

	// create the views in the splitter
	blankViewId                    = m_wndSplitter.AddView(0, 1, RUNTIME_CLASS(CBlankView), pContext);
	effectViewId                   = m_wndSplitter.AddView(0, 1, RUNTIME_CLASS(CEffectView), pContext);
	implementationViewId           = m_wndSplitter.AddView(0, 1, RUNTIME_CLASS(CImplementationView), pContext);
	passViewId                     = m_wndSplitter.AddView(0, 1, RUNTIME_CLASS(CPassView), pContext);
	stageViewId                    = m_wndSplitter.AddView(0, 1, RUNTIME_CLASS(CStageView), pContext);
	materialViewId                 = m_wndSplitter.AddView(0, 1, RUNTIME_CLASS(CMaterialView), pContext);
	textureViewId                  = m_wndSplitter.AddView(0, 1, RUNTIME_CLASS(CTextureView), pContext);
	alphaReferenceViewId           = m_wndSplitter.AddView(0, 1, RUNTIME_CLASS(CAlphaReferenceView), pContext);
	stencilReferenceViewId         = m_wndSplitter.AddView(0, 1, RUNTIME_CLASS(CStencilReferenceView), pContext);
	textureCoordinateSetViewId     = m_wndSplitter.AddView(0, 1, RUNTIME_CLASS(CTextureCoordinateSetView), pContext);
	textureFactorViewId            = m_wndSplitter.AddView(0, 1, RUNTIME_CLASS(CTextureFactorView), pContext);
	textureScrollViewId            = m_wndSplitter.AddView(0, 1, RUNTIME_CLASS(CTextureScrollView), pContext);
	pixelShaderViewId              = m_wndSplitter.AddView(0, 1, RUNTIME_CLASS(CPixelShaderView), pContext);
	fixedFunctionPipelineViewId    = m_wndSplitter.AddView(0, 1, RUNTIME_CLASS(CFixedFunctionPipelineView), pContext);
	vertexShaderViewId             = m_wndSplitter.AddView(0, 1, RUNTIME_CLASS(CVertexShaderView), pContext);
	vertexShaderProgramViewId      = m_wndSplitter.AddView(0, 1, RUNTIME_CLASS(CVertexShaderProgram), pContext);
	pixelShaderProgramViewId       = m_wndSplitter.AddView(0, 1, RUNTIME_CLASS(CPixelShaderProgramView), pContext);
	includeViewId                  = m_wndSplitter.AddView(0, 1, RUNTIME_CLASS(CIncludeView), pContext);

	// get the views back from the splitter
	effectView                 = static_cast<CEffectView *>                (m_wndSplitter.GetView(effectViewId));
	implementationView         = static_cast<CImplementationView *>        (m_wndSplitter.GetView(implementationViewId));
	passView                   = static_cast<CPassView *>                  (m_wndSplitter.GetView(passViewId));
	stageView                  = static_cast<CStageView *>                 (m_wndSplitter.GetView(stageViewId));
	materialView               = static_cast<CMaterialView *>              (m_wndSplitter.GetView(materialViewId));
	textureView                = static_cast<CTextureView *>               (m_wndSplitter.GetView(textureViewId));
	alphaReferenceView         = static_cast<CAlphaReferenceView *>        (m_wndSplitter.GetView(alphaReferenceViewId));
	stencilReferenceView       = static_cast<CStencilReferenceView *>      (m_wndSplitter.GetView(stencilReferenceViewId));
	textureCoordinateSetView   = static_cast<CTextureCoordinateSetView *>  (m_wndSplitter.GetView(textureCoordinateSetViewId));
	textureFactorView          = static_cast<CTextureFactorView *>         (m_wndSplitter.GetView(textureFactorViewId));
	textureScrollView          = static_cast<CTextureScrollView *>         (m_wndSplitter.GetView(textureScrollViewId));
	pixelShaderView            = static_cast<CPixelShaderView *>           (m_wndSplitter.GetView(pixelShaderViewId));
	fixedFunctionPipelineView  = static_cast<CFixedFunctionPipelineView *> (m_wndSplitter.GetView(fixedFunctionPipelineViewId));
	vertexShaderView           = static_cast<CVertexShaderView *>          (m_wndSplitter.GetView(vertexShaderViewId));
	vertexShaderProgramView    = static_cast<CVertexShaderProgram *>       (m_wndSplitter.GetView(vertexShaderProgramViewId));
	pixelShaderProgramView     = static_cast<CPixelShaderProgramView *>    (m_wndSplitter.GetView(pixelShaderProgramViewId));
	includeView                = static_cast<CIncludeView *>               (m_wndSplitter.GetView(includeViewId));

	return TRUE;
}

// ----------------------------------------------------------------------

void CMainFrame::ShowNode(CNode &node)
{
	int viewId;

	vertexShaderProgramView->SwitchAway();
	pixelShaderProgramView->SwitchAway();
	includeView->SwitchAway();

	switch (node.GetType())
	{
		case CNode::Effect:
			viewId = effectViewId;
			effectView->SetNode(static_cast<CEffectNode &>(node));
			break;

		case CNode::Implementation:
			viewId = implementationViewId;
			implementationView->SetNode(static_cast<CImplementationNode &>(node));
			break;

		case CNode::Pass:
			viewId = passViewId;
			passView->SetNode(static_cast<CPassNode &>(node));
			break;

		case CNode::Stage:
			viewId = stageViewId;
			stageView->SetNode(static_cast<CStageNode &>(node));
			break;

		case CNode::Material:
			viewId = materialViewId;
			materialView->SetNode(static_cast<CMaterialNode &>(node));
			break;

		case CNode::Texture:
			viewId = textureViewId;
			textureView->SetNode(static_cast<CTextureNode &>(node));
			break;

		case CNode::TextureCoordinateSet:
			viewId = textureCoordinateSetViewId;
			textureCoordinateSetView->SetNode(static_cast<CTextureCoordinateSetNode &>(node));
			break;

		case CNode::TextureFactor:
			viewId = textureFactorViewId;
			textureFactorView->SetNode(static_cast<CTextureFactorNode &>(node));
			break;

		case CNode::TextureScroll:
			viewId = textureScrollViewId;
			textureScrollView->SetNode(static_cast<CTextureScrollNode &>(node));
			break;

		case CNode::AlphaReferenceValue:
			viewId = alphaReferenceViewId;
			alphaReferenceView->SetNode(static_cast<CAlphaReferenceValueNode &>(node));
			break;

		case CNode::StencilReferenceValue:
			viewId = stencilReferenceViewId;
			stencilReferenceView->SetNode(static_cast<CStencilReferenceValueNode &>(node));
			break;

		case CNode::PixelShader:
			viewId = pixelShaderViewId;
			pixelShaderView->SetNode(static_cast<CPixelShaderNode &>(node));
			break;

		case CNode::FixedFunctionPipeline:
			viewId = fixedFunctionPipelineViewId;
			fixedFunctionPipelineView->SetNode(static_cast<CFixedFunctionPipelineNode &>(node));
			break;

		case CNode::VertexShader:
			viewId = vertexShaderViewId;
			vertexShaderView->SetNode(static_cast<CVertexShaderNode &>(node));
			break;

		case CNode::VertexShaderProgram:
			viewId = vertexShaderProgramViewId;
			vertexShaderProgramView->SetNode(static_cast<CVertexShaderProgramNode &>(node));
			break;

		case CNode::PixelShaderProgram:
			viewId = pixelShaderProgramViewId;
			pixelShaderProgramView->SetNode(static_cast<CPixelShaderProgramNode &>(node));
			break;

		case CNode::Include:
			viewId = includeViewId;
			includeView->SetNode(static_cast<CIncludeNode &>(node));
			break;

		case CNode::Template:
		case CNode::MaterialTree:
		case CNode::TextureTree:
		case CNode::TextureCoordinateSetTree:
		case CNode::TextureFactorTree:
		case CNode::TextureScrollTree:
		case CNode::AlphaReferenceValueTree:
		case CNode::StencilReferenceValueTree:
		case CNode::IncludeTree:
		case CNode::VertexShaderProgramTree:
		case CNode::PixelShaderProgramTree:
			viewId = blankViewId;
			break;

		default:
			ASSERT(false);
			// can't actually get here, but MSVC complains
			viewId = 0;
	}

	m_wndSplitter.ShowView(viewId);
}

// ======================================================================
