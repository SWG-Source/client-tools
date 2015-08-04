//
// RadialView.cpp
// asommers 10-13-2000
//
// copyright 2000, verant interactive
//

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "RadialView.h"

#include "sharedDebug/Profiler.h"
#include "sharedFile/TreeFile.h"
#include "sharedMath/PackedRgb.h"
#include "sharedUtility/FileName.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/Shader.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientGraphics/StaticShader.h"
#include "clientGraphics/StaticVertexBuffer.h"
#include "clientGraphics/Texture.h"
#include "clientGraphics/VertexBuffer.h"

//-------------------------------------------------------------------

IMPLEMENT_DYNCREATE(RadialView, CView)

//-------------------------------------------------------------------

RadialView::RadialView (void) :
	shader (0),
	vertexBuffer (0),
	hasName (false)
{
}

RadialView::~RadialView()
{
	destroy ();
}

//-------------------------------------------------------------------

BEGIN_MESSAGE_MAP(RadialView, CView)
	//{{AFX_MSG_MAP(RadialView)
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//-------------------------------------------------------------------

void RadialView::OnInitialUpdate()
{
	CView::OnInitialUpdate();
}

//-------------------------------------------------------------------

void RadialView::OnDraw(CDC* pDC)
{
	if (shader)
	{

		CRect rect;
		GetClientRect (&rect);

		//-- point the Gl at this window
		Graphics::setViewport (0, 0, rect.Width (), rect.Height ());

		//-- render a frame
		Graphics::beginScene ();

			Graphics::clearViewport(true, 0xffa1a1a1, true, 1.f, true, 0);
			Graphics::setStaticShader (shader->prepareToView());
			Graphics::setVertexBuffer (*vertexBuffer);
			Graphics::drawTriangleFan ();

		//-- done rendering the frame
		Graphics::endScene ();
		Graphics::present (m_hWnd, rect.Width (), rect.Height ());
	}
	else
	{
		pDC->TextOut (0, 0, hasName ? "Could not find shader" : "No shader selected");
	}
}

//-------------------------------------------------------------------

#ifdef _DEBUG
void RadialView::AssertValid() const
{
	CView::AssertValid();
}

void RadialView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

//-------------------------------------------------------------------

void RadialView::destroy (void)
{
	if (shader)
	{
		shader->release ();
		shader = 0;
	}

	if (vertexBuffer)
	{
		delete vertexBuffer;
		vertexBuffer = 0;
	}
}

//-------------------------------------------------------------------

void RadialView::loadShader (const char* newName)
{
	destroy ();

	hasName = newName != 0;

	if (newName)
	{
		if (_strnicmp (newName, "child", 5) == 0)
			return;

		FileName fileName (FileName::P_shader, newName);

		if (!TreeFile::exists (fileName))
		{
			CString msg(fileName.getString());
			msg += " not found";
			MessageBox ( msg );
			return;
		}
	
		const StaticShader* staticShader = dynamic_cast<const StaticShader*> (ShaderTemplateList::fetchModifiableShader (fileName));

		if (!staticShader)
		{
			CString msg(fileName.getString());
			msg += " is not a static shader template";
			MessageBox ( msg );
			return;
		}

		const Texture* texture = 0;
		bool result = staticShader->getTexture (TAG (M,A,I,N), texture);

		if (!result)
		{
			CString msg(fileName.getString());
			msg += " does not have a texture with tag MAIN";
			MessageBox ( msg );
			return;
		}

		shader = ShaderTemplateList::fetchModifiableShader ("shader/terraineditora.sht");
		safe_cast<StaticShader*> (shader)->setTexture (TAG (M,A,I,N), *texture);
		staticShader->release ();

		VertexBufferFormat format;
		format.setPosition();
		format.setTransformed();
		format.setColor0();
		format.setNumberOfTextureCoordinateSets(1);
		format.setTextureCoordinateSetDimension(0, 2);

		vertexBuffer = new StaticVertexBuffer (format, 4);

		vertexBuffer->lock();

			VertexBufferWriteIterator v = vertexBuffer->begin();

			const float width = 256.0f; 

			v.setPosition  (0.f, 0.f,   0.f);
			v.setOoz  (CONST_REAL (1));
			v.setColor0 (VectorArgb::solidWhite);
			v.setTextureCoordinates (0, 0.f, 0.f);
			++v;

			v.setPosition  (width, 0.f,   0.f);
			v.setOoz  (CONST_REAL (1));
			v.setColor0 (VectorArgb::solidWhite);
			v.setTextureCoordinates (0, 1.f, 0.f);
			++v;

			v.setPosition  (width, width, 0.f);
			v.setOoz  (CONST_REAL (1));
			v.setColor0 (VectorArgb::solidWhite);
			v.setTextureCoordinates (0, 1.f, 1.f);
			++v;

			v.setPosition  (0.f, width, 0.f);
			v.setOoz  (CONST_REAL (1));
			v.setColor0 (VectorArgb::solidWhite);
			v.setTextureCoordinates (0, 0.f, 1.f);

		vertexBuffer->unlock();
	}

	Invalidate ();
}

//-------------------------------------------------------------------

BOOL RadialView::OnEraseBkgnd(CDC* pDC) 
{
	// TODO: Add your message handler code here and/or call default
	if (shader)
		return TRUE;

	return CView::OnEraseBkgnd(pDC);
}

//-------------------------------------------------------------------

void RadialView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);
	
	Invalidate ();
}

//-------------------------------------------------------------------

