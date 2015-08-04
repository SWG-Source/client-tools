// ======================================================================
//
// TextureBakeView.cpp
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#include "FirstTextureBuilder.h"
#include "TextureBakeView.h"

#include "TextureBuilderDoc.h"
#include "resource.h"

#include "clientGraphics/DynamicVertexBuffer.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/Shader.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientGraphics/StaticShader.h"
#include "clientGraphics/StaticVertexBuffer.h"
#include "clientGraphics/Texture.h"
#include "clientGraphics/VertexBuffer.h"
#include "clientObject/ObjectListCamera.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedMath/PackedArgb.h"
#include "sharedMath/Transform.h"

#include <algorithm>
#include <limits>

// ======================================================================

namespace
{
	const char *const ms_bakedTextureShaderName = "shader\\simplet1z.sht";

	const float       ms_defaultNearPlane         = 0.01f;
	const float       ms_defaultFarPlane          = 10000.0f;
	const float       ms_defaultNoZoomDistance    = 100.0f; // ms_defaultNearPlane + 0.25f * (ms_defaultFarPlane - ms_defaultNearPlane);
	const float       ms_defaultHorizontalFov     = PI_OVER_2;

	const float       ms_cameraZoomPerScreenUnits = 1.0f;
	const float       ms_minDistanceFromObject    = ms_defaultNearPlane + std::numeric_limits<float>::epsilon();

	const int         ms_selectionThreshold       = 4;
}

// ======================================================================

TextureBakeView::TextureBakeView()
:	CView(),
	m_windowWidth(0),
	m_windowHeight(0),
	m_xCameraPerScreenUnits(1.0f),
	m_yCameraPerScreenUnits(1.0f),
	m_bakedTextureShader(ShaderTemplateList::fetchModifiableShader(ms_bakedTextureShaderName)),
	m_camera(new ObjectListCamera(2)),
	m_bakedTextureVertexBuffer(0),
	m_objectToWorld(new Transform(Transform::identity)),
	m_distanceFromObject(ms_defaultNoZoomDistance),
	m_noZoomDistance(ms_defaultNoZoomDistance),
	m_lastMousePosition(),
	m_toggleBackgroundColor(false),
	m_lastTextureWidth(0),
	m_lastTextureHeight(0),
	m_hasActiveSelection(false),
	m_activeSelection(),
	m_activeSelectionVertexBuffer(0),
	m_isDraggingSelectionBorder(0),
	m_xModify(NULL),
	m_xModifyAdjustment(0),
	m_yModify(NULL),
	m_yModifyAdjustment(0),
	m_isMovingSelection(false)
{
	//-- setup camera
	m_camera->setNearPlane(ms_defaultNearPlane);
	m_camera->setFarPlane(ms_defaultFarPlane);
	m_camera->setHorizontalFieldOfView(ms_defaultHorizontalFov);

	//-- setup baked texture object to world
	m_objectToWorld->setPosition_p(Vector(0.0f, 0.0f, m_noZoomDistance));

	//-- setup static portions of baked texture geometry
	VertexBufferFormat vbFlags;
	vbFlags.setPosition();
	vbFlags.setNumberOfTextureCoordinateSets(1);
	vbFlags.setTextureCoordinateSetDimension(0, 2);
	m_bakedTextureVertexBuffer = new StaticVertexBuffer(vbFlags, 4);

	m_bakedTextureVertexBuffer->lock();

		VertexBufferReadWriteIterator vit = m_bakedTextureVertexBuffer->begin();
	{
		//-- setup depth and UVs (ul, ur, lr, ll)
		vit.setPositionZ(0.0f);
		vit.setTextureCoordinates(0, 0.0f, 0.0f);
		++vit;

		vit.setPositionZ(0.0f);
		vit.setTextureCoordinates(0, 1.0f, 0.0f);
		++vit;

		vit.setPositionZ(0.0f);
		vit.setTextureCoordinates(0, 1.0f, 1.0f);
		++vit;

		vit.setPositionZ(0.0f);
		vit.setTextureCoordinates(0, 0.0f, 1.0f);
	}
	m_bakedTextureVertexBuffer->unlock();

	//-- create active selection vertex buffer
	VertexBufferFormat selectionVbFlags;
	selectionVbFlags.setPosition();
	selectionVbFlags.setColor0();
	m_activeSelectionVertexBuffer = new DynamicVertexBuffer(selectionVbFlags);

	m_activeSelection.m_x0 = 0.0f;
	m_activeSelection.m_y0 = 0.0f;
	m_activeSelection.m_x1 = 1.0f;
	m_activeSelection.m_y1 = 1.0f;
}

// ----------------------------------------------------------------------

TextureBakeView::~TextureBakeView()
{
	m_xModify = 0;
	m_yModify = 0;

	delete m_activeSelectionVertexBuffer;
	delete m_objectToWorld;
	delete m_bakedTextureVertexBuffer;
	delete m_camera;

	m_bakedTextureShader->release();
	m_bakedTextureShader = 0;
}

// ----------------------------------------------------------------------

IMPLEMENT_DYNCREATE(TextureBakeView, CView)

// ----------------------------------------------------------------------

BEGIN_MESSAGE_MAP(TextureBakeView, CView)
	//{{AFX_MSG_MAP(TextureBakeView)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONUP()
	ON_WM_SIZE()
	ON_WM_MOUSEMOVE()
	ON_WM_ERASEBKGND()
	ON_COMMAND(ID_VIEW_TOGGLE_BKGND_COLOR, OnViewToggleBackgroundColor)
	ON_UPDATE_COMMAND_UI(ID_VIEW_TOGGLE_BKGND_COLOR, OnUpdateViewToggleBackgroundColor)
	ON_COMMAND(ID_VIEW_ZOOM_100_PERCENT, OnViewZoom100Percent)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ----------------------------------------------------------------------

#ifdef _DEBUG

TextureBuilderDoc *TextureBakeView::GetDocument()
{
	return NON_NULL(dynamic_cast<TextureBuilderDoc*>(CView::GetDocument()));
}

// ----------------------------------------------------------------------

const TextureBuilderDoc *TextureBakeView::GetDocument() const
{
	return NON_NULL(dynamic_cast<const TextureBuilderDoc*>(CView::GetDocument()));
}

#endif

// ----------------------------------------------------------------------

void TextureBakeView::calculateCameraPerScreenUnits(float depth, float &xCameraPerScreenUnits, float &yCameraPerScreenUnits) const
{
	//-- calculate camera units per screen units such that, at z = depth, 1 screen pixel in a given
	//   direction is a known # camera units.
	const Vector upperLeft        = m_camera->reverseProjectInViewportSpace(0.0f, 0.0f, depth);

	const float  halfScreenWidth  = static_cast<float>(m_windowWidth) * 0.5f;
	const float  halfScreenHeight = static_cast<float>(m_windowHeight) * 0.5f;

	xCameraPerScreenUnits       = -upperLeft.x / halfScreenWidth;
	yCameraPerScreenUnits       = upperLeft.y / halfScreenHeight;
}

// ----------------------------------------------------------------------

void TextureBakeView::calculateScreenPerTextureUnits(float depth, float &xScreenPerTextureUnits, float &yScreenPerTextureUnits) const
{
	//-- get upper left camera-space texture coordinate from vertex buffer
	m_bakedTextureVertexBuffer->lockReadOnly();
		const Vector textureUpperLeft_raw = m_bakedTextureVertexBuffer->begin().getPosition();
	m_bakedTextureVertexBuffer->unlock();

	//-- convert upper left texture coordinate into screen space
	float  textureLeft_s;
	float  textureTop_s;

	IGNORE_RETURN(m_camera->projectInCameraSpace(Vector(textureUpperLeft_raw.x, textureUpperLeft_raw.y, depth), &textureLeft_s, &textureTop_s, NULL, false));

	const float  halfScreenWidth  = static_cast<float>(m_windowWidth) * 0.5f;
	const float  halfScreenHeight = static_cast<float>(m_windowHeight) * 0.5f;

	const TextureBuilderDoc *document = GetDocument();

	xScreenPerTextureUnits = 2.0f * (halfScreenWidth - textureLeft_s) / document->getTextureWidth();
	yScreenPerTextureUnits = 2.0f * (halfScreenHeight - textureTop_s) / document->getTextureHeight();
}

// ----------------------------------------------------------------------

void TextureBakeView::setNoZoomEffectDistance(float depth)
{
	m_noZoomDistance = depth;
	calculateCameraPerScreenUnits(depth, m_xCameraPerScreenUnits, m_yCameraPerScreenUnits);

	TextureBuilderDoc *const doc = GetDocument();
	prepareBakedTextureGeometry(doc->getTextureWidth(), doc->getTextureHeight());
}

// ----------------------------------------------------------------------

void TextureBakeView::prepareBakedTextureGeometry(int textureWidth, int textureHeight)
{
	const float  halfTextureWidth      = static_cast<float>(textureWidth) * 0.5f;
	const float  halfTextureHeight     = static_cast<float>(textureHeight) * 0.5f;

	const float  halfGeometryWidth     = m_xCameraPerScreenUnits * halfTextureWidth;
	const float  halfGeometryHeight    = m_yCameraPerScreenUnits * halfTextureHeight;

	// build geometry (depth stays zero)
	m_bakedTextureVertexBuffer->lock();

		VertexBufferReadWriteIterator vit = m_bakedTextureVertexBuffer->begin();

		// ul
		vit.setPositionX(-halfGeometryWidth);
		vit.setPositionY( halfGeometryHeight);
		++vit;

		// ur
		vit.setPositionX( halfGeometryWidth);
		vit.setPositionY( halfGeometryHeight);
		++vit;

		// lr
		vit.setPositionX( halfGeometryWidth);
		vit.setPositionY(-halfGeometryHeight);
		++vit;

		// ll
		vit.setPositionX(-halfGeometryWidth);
		vit.setPositionY(-halfGeometryHeight);

	m_bakedTextureVertexBuffer->unlock();

	m_lastTextureWidth  = textureWidth;
	m_lastTextureHeight = textureHeight;
}

// ----------------------------------------------------------------------

void TextureBakeView::setupObjectPosition()
{
	m_objectToWorld->setPosition_p(Vector(0.0f, 0.0f, m_distanceFromObject));
}

// ----------------------------------------------------------------------

void TextureBakeView::convertTextureToCameraSpace(const Selection &textureSpaceSelection, Selection &cameraSpaceSelection) const
{
	const TextureBuilderDoc *document = GetDocument();
	const float        textureWidth   = static_cast<float>(document->getTextureWidth());
	const float        textureHeight  = static_cast<float>(document->getTextureHeight());

	cameraSpaceSelection.m_x0         = (textureWidth  * (textureSpaceSelection.m_x0 - 0.5f)) * m_xCameraPerScreenUnits;
	cameraSpaceSelection.m_y0         = (textureHeight * (0.5f - textureSpaceSelection.m_y0)) * m_yCameraPerScreenUnits;
	cameraSpaceSelection.m_x1         = (textureWidth  * (textureSpaceSelection.m_x1 - 0.5f)) * m_xCameraPerScreenUnits;
	cameraSpaceSelection.m_y1         = (textureHeight * (0.5f - textureSpaceSelection.m_y1)) * m_yCameraPerScreenUnits;
}

// ----------------------------------------------------------------------

void TextureBakeView::fillActiveSelectionVb(const Selection &cameraSpaceSelection, const PackedArgb &color)
{
	// setup vb for selection
	m_activeSelectionVertexBuffer->lock(5);

		VertexBufferWriteIterator vit = m_activeSelectionVertexBuffer->begin();

		// upper left
		vit.setPosition(cameraSpaceSelection.m_x0, cameraSpaceSelection.m_y0, 0.0f);
		vit.setColor0(color);
		++vit;

		// upper right
		vit.setPosition(cameraSpaceSelection.m_x1, cameraSpaceSelection.m_y0, 0.0f);
		vit.setColor0(color);
		++vit;

		// lower right
		vit.setPosition(cameraSpaceSelection.m_x1, cameraSpaceSelection.m_y1, 0.0f);
		vit.setColor0(color);
		++vit;

		// lower left
		vit.setPosition(cameraSpaceSelection.m_x0, cameraSpaceSelection.m_y1, 0.0f);
		vit.setColor0(color);
		++vit;

		// back to upper right (line strip
		vit.setPosition(cameraSpaceSelection.m_x0, cameraSpaceSelection.m_y0, 0.0f);
		vit.setColor0(color);

	m_activeSelectionVertexBuffer->unlock();
}

// ----------------------------------------------------------------------

void TextureBakeView::OnDraw(CDC* pDC)
{
	UNREF(pDC);

	//CDocument &doc = *NON_NULL(GetDocument());
	if (ExitChain::isFataling())
		return;

#if 0
	static int drawCount = 0;
	++drawCount;
	DEBUG_REPORT_LOG(true, ("OnDraw [%d]\n", drawCount));
#endif

	//-- setup the bake texture shader (this may cause a pending bake to be executed)
	StaticShader *const staticShader = dynamic_cast<StaticShader*>(m_bakedTextureShader);
	DEBUG_FATAL(!staticShader, ("baked texture shader is not a StaticShader"));

	const Texture &texture = GetDocument()->getTexture();
	staticShader->setTexture(TAG(M,A,I,N), texture); //lint !e613 // possible use of null pointer // huh?  see DEBUG_FATAL above.

	//-- begin frame render
	Graphics::beginScene();
	Graphics::setRenderTarget(NULL, CF_none, 0);

	//-- clear the frame
	{
		uint32 clearColor;

		if (!m_toggleBackgroundColor)
			clearColor = 0xff000000;
		else
		{
			static int colorSelector = 0;

			colorSelector = (colorSelector + 1) % 3;

			switch (colorSelector)
			{
				case 0:
					clearColor = 0xff0000ff;
					break;

				case 1:
					clearColor = 0xff00ff00;
					break;

				case 2:
				default:
					clearColor = 0xffff0000;
					break;
			}
		}
		Graphics::clearViewport(true, clearColor, true, 1.0f, true, 0);
	}

	//-- set fill mode
	Graphics::setFillMode(GFM_solid);
	Graphics::setCullMode(GCM_none);

	//-- set the camera
	// force camera to create its gl data
	m_camera->renderScene();

	// set the camera
	m_camera->applyState();
	Graphics::setObjectToWorldTransformAndScale(*m_objectToWorld, Vector::xyz111);

	//-- render baked texture
	Graphics::setStaticShader(m_bakedTextureShader->prepareToView(), 0);
	Graphics::setVertexBuffer(*m_bakedTextureVertexBuffer);
	Graphics::drawTriangleFan();

	//-- render the active selection
	if (m_hasActiveSelection)
	{
		Graphics::setStaticShader(ShaderTemplateList::get3dVertexColorStaticShader(), 0);

		//-- setup VB for active selection
		Selection  cameraSpaceSelection;

		convertTextureToCameraSpace(m_activeSelection, cameraSpaceSelection);
		fillActiveSelectionVb(cameraSpaceSelection, PackedArgb::solidWhite);

		Graphics::setVertexBuffer(*m_activeSelectionVertexBuffer);
		Graphics::drawLineStrip();
	}

	//-- end frame render
	Graphics::endScene();
	Graphics::present(m_hWnd, m_windowWidth, m_windowHeight);
}

// ----------------------------------------------------------------------

#ifdef _DEBUG
void TextureBakeView::AssertValid() const
{
	CView::AssertValid();
}
#endif

// ----------------------------------------------------------------------

#ifdef _DEBUG
void TextureBakeView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif

// ----------------------------------------------------------------------

void TextureBakeView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);
	
	//-- get new window width and height
	if (cx && cy)
	{
		m_windowWidth  = cx;
		m_windowHeight = cy;

		//-- set new camera viewport
		m_camera->setViewport(0, 0, m_windowWidth, m_windowHeight);

		//-- must reset baked texture geometry anytime camera changes
		TextureBuilderDoc *doc = GetDocument();

		calculateCameraPerScreenUnits(m_noZoomDistance, m_xCameraPerScreenUnits, m_yCameraPerScreenUnits);
		prepareBakedTextureGeometry(doc->getTextureWidth(), doc->getTextureHeight());
	}

	Invalidate();
}

// ----------------------------------------------------------------------

void TextureBakeView::moveActiveSelection(int deltaWindowX, int deltaWindowY)
{
	//-- get texture / screen dimension information
	float xScreenPerTextureUnits;
	float yScreenPerTextureUnits;

	calculateScreenPerTextureUnits(m_distanceFromObject, xScreenPerTextureUnits, yScreenPerTextureUnits);

	//-- move the active selection
	TextureBuilderDoc *const document = GetDocument();

	const float deltaTextureX = (deltaWindowX / xScreenPerTextureUnits) / document->getTextureWidth();
	const float deltaTextureY = (deltaWindowY / yScreenPerTextureUnits) / document->getTextureHeight();

	m_activeSelection.m_x0 += deltaTextureX;
	m_activeSelection.m_y0 += deltaTextureY;
	m_activeSelection.m_x1 += deltaTextureX;
	m_activeSelection.m_y1 += deltaTextureY;

	//-- update the doc's selection
	TextureBuilderDoc::Selection textureSelection;

	textureSelection.m_x0 = m_activeSelection.m_x0;
	textureSelection.m_y0 = m_activeSelection.m_y0;
	textureSelection.m_x1 = m_activeSelection.m_x1;
	textureSelection.m_y1 = m_activeSelection.m_y1;

	document->setActiveSelection(textureSelection);
}

// ----------------------------------------------------------------------

void TextureBakeView::dragActiveSelectionBorder(int windowX, int windowY)
{
	FATAL(!m_isDraggingSelectionBorder, ("we're not in a mode where we're modifying the active selection"));

	//-- get texture / screen dimension information
	float xScreenPerTextureUnits;
	float yScreenPerTextureUnits;

	calculateScreenPerTextureUnits(m_distanceFromObject, xScreenPerTextureUnits, yScreenPerTextureUnits);

	TextureBuilderDoc *const document = GetDocument();

	const float textureWidth  = static_cast<float>(document->getTextureWidth());
	const float textureHeight = static_cast<float>(document->getTextureHeight());

	//-- calculate texture x,y for given screen x,y
	if (m_xModify)
	{
		const float screenX = static_cast<float>(windowX - m_xModifyAdjustment);
		*m_xModify          = (0.5f * textureWidth + (screenX - 0.5f * static_cast<float>(m_windowWidth)) / xScreenPerTextureUnits) / textureWidth;

		if (m_activeSelection.m_x0 > m_activeSelection.m_x1)
		{
			//-- need to swap values
			std::swap(m_activeSelection.m_x0, m_activeSelection.m_x1);

			//-- also need to swap which attribute we're modifying
			if (m_xModify == &m_activeSelection.m_x0)
				m_xModify = &m_activeSelection.m_x1;
			else
				m_xModify = &m_activeSelection.m_x0;
		}
	}

	if (m_yModify)
	{
		const float screenY = static_cast<float>(windowY - m_yModifyAdjustment);
		*m_yModify          = (0.5f * textureHeight + (screenY - 0.5f * static_cast<float>(m_windowHeight)) / yScreenPerTextureUnits) / textureHeight;

		if (m_activeSelection.m_y0 > m_activeSelection.m_y1)
		{
			//-- need to swap values
			std::swap(m_activeSelection.m_y0, m_activeSelection.m_y1);

			//-- also need to swap which attribute we're modifying
			if (m_yModify == &m_activeSelection.m_y0)
				m_yModify = &m_activeSelection.m_y1;
			else
				m_yModify = &m_activeSelection.m_y0;
		}
	}

	//-- update the doc's selection
	TextureBuilderDoc::Selection textureSelection;

	textureSelection.m_x0 = m_activeSelection.m_x0;
	textureSelection.m_y0 = m_activeSelection.m_y0;
	textureSelection.m_x1 = m_activeSelection.m_x1;
	textureSelection.m_y1 = m_activeSelection.m_y1;

	document->setActiveSelection(textureSelection);
}

// ----------------------------------------------------------------------
/**
 * Find which, if any, of the active selection boundary parameters are near
 * the screen space coordinate specified.
 *
 * @arg screenX            screen space x position to test against active selection
 * @arg screenY            screen space y position to test against active selection
 * @arg xModify            the address of the x axis selection variable to modify
 * @arg xModifyAdjustment  adjustment, in screen space, made to x value before converting to camera space
 *                         to compensate for initial screen space distance from the active selection.
 * @arg yModify            the address of the y axis selection variable to modify
 * @arg yModifyAdjustment  adjustment, in screen space, made to y value before converting to camera space
 *                         to compensate for initial screen space distance from the active selection.
 *
 * @return  true if the screen space coordinate is near any of the active
 *          region boundaries; false otherwise
 */
bool TextureBakeView::findSelectionModifyParameters(int screenX, int screenY, float **xModify, int &xModifyAdjustment, float **yModify, int &yModifyAdjustment)
{
	FATAL(!m_hasActiveSelection, ("no active selection"));

	float xScreenPerTextureUnits;
	float yScreenPerTextureUnits;

	calculateScreenPerTextureUnits(m_distanceFromObject, xScreenPerTextureUnits, yScreenPerTextureUnits);

	//-- convert active selection region to screen space
	TextureBuilderDoc *const document = GetDocument();

	const float halfScreenX   = static_cast<float>(m_windowWidth)  * 0.5f;
	const float halfScreenY   = static_cast<float>(m_windowHeight) * 0.5f;

	const float textureWidth  = static_cast<float>(document->getTextureWidth());
	const float textureHeight = static_cast<float>(document->getTextureHeight());

	const int screenSelX0     = static_cast<int>(halfScreenX + ((m_activeSelection.m_x0 - 0.5f) * textureWidth)  * xScreenPerTextureUnits);
	const int screenSelY0     = static_cast<int>(halfScreenY + ((m_activeSelection.m_y0 - 0.5f) * textureHeight) * yScreenPerTextureUnits);

	const int screenSelX1     = static_cast<int>(halfScreenX + ((m_activeSelection.m_x1 - 0.5f) * textureWidth)  * xScreenPerTextureUnits);
	const int screenSelY1     = static_cast<int>(halfScreenY + ((m_activeSelection.m_y1 - 0.5f) * textureHeight) * yScreenPerTextureUnits);

	//-- see which x we're closer to
	if (WithinRangeInclusiveInclusive(screenSelY0 - ms_selectionThreshold, screenY, screenSelY1 + ms_selectionThreshold))
	{
		const int    diffX0    = screenX - screenSelX0;
		const double absDiffX0 = fabs(static_cast<double>(diffX0));

		const int    diffX1    = screenX - screenSelX1;
		const double absDiffX1 = fabs(static_cast<double>(diffX1));

		if ((absDiffX0 <= absDiffX1) && (absDiffX0 <= static_cast<double>(ms_selectionThreshold)))
		{
			*xModify          = &m_activeSelection.m_x0;
			xModifyAdjustment = diffX0;
		}
		else if ((absDiffX1 < absDiffX0) && (absDiffX1 <= static_cast<double>(ms_selectionThreshold)))
		{
			*xModify          = &m_activeSelection.m_x1;
			xModifyAdjustment = diffX1;
		}
	}
	else
	{
		*xModify = 0;
	}

	if (WithinRangeInclusiveInclusive(screenSelX0 - ms_selectionThreshold, screenX, screenSelX1 + ms_selectionThreshold))
	{
		const int    diffY0    = screenY - screenSelY0;
		const double absDiffY0 = fabs(static_cast<double>(diffY0));

		const int    diffY1    = screenY - screenSelY1;
		const double absDiffY1 = fabs(static_cast<double>(diffY1));

		if ((absDiffY0 <= absDiffY1) && (absDiffY0 <= static_cast<double>(ms_selectionThreshold)))
		{
			*yModify          = &m_activeSelection.m_y0;
			yModifyAdjustment = diffY0;
		}
		else if ((absDiffY1 < absDiffY0) && (absDiffY1 <= static_cast<double>(ms_selectionThreshold)))
		{
			*yModify          = &m_activeSelection.m_y1;
			yModifyAdjustment = diffY1;
		}
	}
	else
	{
		*yModify = 0;
	}


	return *xModify || *yModify;
}

// ----------------------------------------------------------------------

void TextureBakeView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	if ((nFlags & MK_MBUTTON) == 0)
		IGNORE_RETURN(SetCapture());

	const bool altKeyPressed = (GetKeyState(VK_MENU) & 0x8000) != 0;
	if (!altKeyPressed && m_hasActiveSelection)
	{
		//-- find the closest x and y selection dimensions to point.  If they're
		//   within range, we're in modify mode.
		m_isDraggingSelectionBorder = findSelectionModifyParameters(point.x, point.y, &m_xModify, m_xModifyAdjustment, &m_yModify, m_yModifyAdjustment);
		if (m_isDraggingSelectionBorder)
			dragActiveSelectionBorder(point.x, point.y);
		else
			m_isMovingSelection = true;
	}

	m_lastMousePosition = point;
	
	Invalidate();
	CView::OnLButtonDown(nFlags, point);
} //lint !e1746 // point could be const // sorry, MFC will have its way

// ----------------------------------------------------------------------

void TextureBakeView::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if ((nFlags & MK_MBUTTON) == 0)
		IGNORE_RETURN(ReleaseCapture());

	if (m_isDraggingSelectionBorder || m_isMovingSelection)
	{
		//-- tell doc to bake itself
		GetDocument()->requestTextureBake();

		//-- reset modify parameters
		m_isDraggingSelectionBorder = false;
		m_isMovingSelection         = false;
		m_xModify                   = NULL;
		m_yModify                   = NULL;
	}

	CView::OnLButtonUp(nFlags, point);
} //lint !e1746 // point could be const // sorry, MFC will have its way

// ----------------------------------------------------------------------

void TextureBakeView::OnMButtonDown(UINT nFlags, CPoint point) 
{
	if ((nFlags & MK_LBUTTON) == 0)
		IGNORE_RETURN(SetCapture());

	CView::OnMButtonDown(nFlags, point);
} //lint !e1746 // point could be const // sorry, MFC will have its way

// ----------------------------------------------------------------------

void TextureBakeView::OnMButtonUp(UINT nFlags, CPoint point) 
{
	if ((nFlags & MK_LBUTTON) == 0)
		IGNORE_RETURN(ReleaseCapture());

	CView::OnMButtonUp(nFlags, point);
} //lint !e1746 // point could be const // sorry, MFC will have its way

// ----------------------------------------------------------------------

void TextureBakeView::OnMouseMove(UINT nFlags, CPoint point) 
{
	// NOTES: if we start modifying the selection box, we do not allow zooming
	//        until the user releases the left mouse button and stops modifying
	//        the selection.

	if (m_isDraggingSelectionBorder)
	{
		dragActiveSelectionBorder(point.x, point.y);
		Invalidate();
	}
	else if (m_isMovingSelection)
	{
		moveActiveSelection(point.x - m_lastMousePosition.x, point.y - m_lastMousePosition.y);
		Invalidate();
	}
	else
	{
		const bool altKeyPressed = (GetKeyState(VK_MENU) & 0x8000) != 0;
		if (altKeyPressed)
		{
			const bool leftButtonDown   = (nFlags & MK_LBUTTON) != 0;
			const bool middleButtonDown = (nFlags & MK_MBUTTON) != 0;

			if (leftButtonDown && middleButtonDown)
			{
				// calculate zoom (move left = move farther away)
				const float deltaZoom = static_cast<float>(m_lastMousePosition.x - point.x) * ms_cameraZoomPerScreenUnits;

				// apply zoom
				m_distanceFromObject += deltaZoom;
				m_distanceFromObject = std::max(m_distanceFromObject, ms_minDistanceFromObject);
				
				setupObjectPosition();
				Invalidate();
			}
		}
	}

	m_lastMousePosition = point;

	CView::OnMouseMove(nFlags, point);
} //lint !e1746 // point could be const // sorry, MFC will have its way

// ----------------------------------------------------------------------

BOOL TextureBakeView::OnEraseBkgnd(CDC* pDC)
{
	//-- do nothing, we don't erase the background
	UNREF(pDC);
	return TRUE;
}

// ----------------------------------------------------------------------

void TextureBakeView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	UNREF(pSender);
	UNREF(lHint);
	UNREF(pHint);

	//-- if bake texture dimensions changed, prepare baked texture geometry
	TextureBuilderDoc *doc = GetDocument();

	if ((doc->getTextureWidth() != m_lastTextureWidth) || (doc->getTextureHeight() != m_lastTextureHeight))
		prepareBakedTextureGeometry(doc->getTextureWidth(), doc->getTextureHeight());

	//-- force a screen redisplay
	Invalidate();
}

// ----------------------------------------------------------------------

void TextureBakeView::OnUpdateViewToggleBackgroundColor(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_toggleBackgroundColor ? TRUE : FALSE);
}

// ----------------------------------------------------------------------

void TextureBakeView::OnViewToggleBackgroundColor() 
{
	m_toggleBackgroundColor = !m_toggleBackgroundColor;
	Invalidate();
}

// ----------------------------------------------------------------------

void TextureBakeView::OnViewZoom100Percent() 
{
	m_distanceFromObject = ms_defaultNoZoomDistance;
	setupObjectPosition();
	Invalidate();
}

// ----------------------------------------------------------------------

void TextureBakeView::notifyNewActiveSelection()
{
	//-- retrieve the document's selection in (0,0) - (1,1) texture space
	TextureBuilderDoc *document = GetDocument();

	TextureBuilderDoc::Selection  textureSelection;
	document->getActiveSelection(textureSelection);

	//-- convert texture space to our camera space
	m_activeSelection.m_x0 = textureSelection.m_x0;
	m_activeSelection.m_y0 = textureSelection.m_y0;
	m_activeSelection.m_x1 = textureSelection.m_x1;
	m_activeSelection.m_y1 = textureSelection.m_y1;

	m_hasActiveSelection = true;

	//-- force redraw
	Invalidate();
}

// ----------------------------------------------------------------------

void TextureBakeView::notifyNoActiveSelection()
{
	m_hasActiveSelection = false;

	//-- force redraw
	Invalidate();
}

// ======================================================================
