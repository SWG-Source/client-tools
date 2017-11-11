// ======================================================================
// RenderView.cpp : implementation file
// ======================================================================

#include "FirstShaderBuilder.h"
#include "ShaderBuilder.h"
#include "RenderView.h"
#include "Node.h"
#include "LeftView.h"
#include "RecentDirectory.h"
#include "sharedFile/TreeFile.h"
#include "sharedMath/Vector.h"
#include "sharedMath/Vector2d.h"
#include "sharedFoundation/FloatMath.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#pragma warning(disable:4244) // conversion from A to B, possible loss of data

#define FAILED_RETURN(hr, error) if (FAILED(hr)) { errorMessage = error; return; }

// ======================================================================

const int  FPS         = 20;
//const float PI_TIMES_2 = 3.14159f * 2.0f;

struct TransformedVertex
{
	float x;
	float y;
	float z;
	float rhw;
	float u;
	float v;
};

namespace Globals
{
	DWORD sheetVertFormat =	D3DFVF_XYZ | 
							D3DFVF_NORMAL | 
							D3DFVF_DIFFUSE | 
							D3DFVF_SPECULAR | 
							D3DFVF_TEX2 | 
							D3DFVF_TEXCOORDSIZE3(1);
};

// ----------------------------------------------------------------------

D3DMATERIAL9 defaultMaterial =
{
	{1,1,1,1},
	{1,1,1,1},
	{1,1,1,1},
	{0,0,0,0},
	40
};

// ======================================================================

IMPLEMENT_DYNCREATE(CRenderView, CView)

BEGIN_MESSAGE_MAP(CRenderView, CView)
	//{{AFX_MSG_MAP(CRenderView)
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_WM_ERASEBKGND()
	ON_WM_RBUTTONDOWN()
	ON_COMMAND(IDR_DISABLED, OnDisabled)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ======================================================================

CRenderView::CRenderView()
:
	leftView(0),
	errorMessage(0),
	oldErrorMessage(0),
	direct3d(0),
	device(0),
	disabled(false),
	width(0),
	height(0),
	vertexBuffer(NULL)
{	
	D3DXMatrixIdentity(&worldMatrix);
}

// ----------------------------------------------------------------------

CRenderView::~CRenderView()
{
	if (GetSafeHwnd())
		KillTimer(1);

	DestroyDevice();
}

// ----------------------------------------------------------------------

#ifdef _DEBUG
void CRenderView::AssertValid() const
{
	CView::AssertValid();
}

// ----------------------------------------------------------------------

void CRenderView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

// ----------------------------------------------------------------------

void CRenderView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);
	width = cx;
	height = cy;
	DestroyDevice();
}

// ----------------------------------------------------------------------

void CRenderView::OnInitialUpdate() 
{
	CView::OnInitialUpdate();
	int result = SetTimer(1, 1000 / FPS, NULL);
	ASSERT(result != 0);
}

// ----------------------------------------------------------------------

void CRenderView::OnTimer(UINT nIDEvent) 
{
	CView::OnTimer(nIDEvent);
	if (nIDEvent == 1)
		Invalidate(FALSE);
}

// ----------------------------------------------------------------------

BOOL CRenderView::OnEraseBkgnd(CDC* pDC) 
{
	UNREF(pDC);
	return FALSE;
}

// ----------------------------------------------------------------------

void CRenderView::OnDraw(CDC* pDC)
{
	errorMessage = NULL;

	if (disabled)
		errorMessage = "Render window disabled";
	else
		if (!direct3d)
			CreateDevice();

	if (device)
	{
		const CNode *node = leftView->GetRenderNode();
		if (node)
		{
			switch (node->GetType())
			{
				case CNode::Texture:
					RenderTexture(static_cast<const CTextureNode &>(*node));
					break;

				default:
					errorMessage = "Non-renderable node selected";
			}	
		}
		else
			errorMessage = "No node selected";
	}

	if (errorMessage)
	{
		if (!oldErrorMessage || strcmp(errorMessage, oldErrorMessage) != 0)
		{
			OutputDebugString(errorMessage);
			OutputDebugString("\n");
		}
		RECT r = { 0, 0, width, height};
		CBrush brush;
		brush.CreateSolidBrush(0x00ffffff);
		pDC->FillRect(&r, &brush);
		pDC->SetTextColor(0x00FF00FF);
		pDC->TextOut(0, 0, errorMessage);
	}
	
	oldErrorMessage = errorMessage;
}

// ----------------------------------------------------------------------

void CRenderView::CreateDevice()
{
	// create Direct3d
	direct3d = Direct3DCreate9(D3D_SDK_VERSION);
	if (!direct3d)
	{
		errorMessage = "Direct3dCreate failed";
		return;
	}

	// get the current display mode
	D3DDISPLAYMODE displayMode;
	memset(&displayMode, 0, sizeof(displayMode));
	HRESULT hresult = direct3d->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &displayMode);
	if (FAILED(hresult))
	{
		errorMessage = "GetAdapterDisplayMode failed";
		DestroyDevice();
		return;
	}
	const D3DFORMAT adapterFormat = displayMode.Format;

	// get the device type
	const D3DDEVTYPE deviceType = D3DDEVTYPE_HAL;

	// try to create the most powerful rendering target available
#if 1
	// hack around destination alpha bug in new nvidia drivers
	const D3DFORMAT backBufferFormats[] = { D3DFMT_R8G8B8, D3DFMT_X8R8G8B8, D3DFMT_R5G6B5, D3DFMT_X1R5G5B5, D3DFMT_UNKNOWN };
#else	
	const D3DFORMAT backBufferFormats[] = { D3DFMT_A8R8G8B8, D3DFMT_R8G8B8, D3DFMT_X8R8G8B8, D3DFMT_A1R5G5B5, D3DFMT_R5G6B5, D3DFMT_X1R5G5B5, D3DFMT_UNKNOWN };
#endif
	const D3DFORMAT depthStencilFormats[] = { D3DFMT_D24S8, D3DFMT_D24X4S4, D3DFMT_D32, D3DFMT_D24X8, D3DFMT_D16, D3DFMT_D15S1, D3DFMT_UNKNOWN };
	bool created = false;
	for (int i = 0; !created && backBufferFormats[i] != D3DFMT_UNKNOWN; ++i)
	{
		const D3DFORMAT backBufferFormat = backBufferFormats[i];

		for (int j = 0; !created && depthStencilFormats[j] != D3DFMT_UNKNOWN; ++j)
		{
			const D3DFORMAT depthStencilFormat = depthStencilFormats[j];

			// check adapter format, back buffer format, depth and stencil buffer compatibilities
			if (SUCCEEDED(direct3d->CheckDeviceType(D3DADAPTER_DEFAULT, deviceType, adapterFormat, backBufferFormat, TRUE)) && 
					SUCCEEDED(direct3d->CheckDeviceFormat(D3DADAPTER_DEFAULT, deviceType, adapterFormat, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, depthStencilFormat)) &&
					SUCCEEDED(direct3d->CheckDepthStencilMatch(D3DADAPTER_DEFAULT, deviceType, adapterFormat, backBufferFormat, depthStencilFormat)))
			{
				// try to create the d3d device
				D3DPRESENT_PARAMETERS presentParameters; 
				memset(&presentParameters, 0, sizeof(presentParameters));
				presentParameters.BackBufferWidth        = width;
				presentParameters.BackBufferHeight       = height;
				presentParameters.BackBufferCount        = 1;
				presentParameters.BackBufferFormat       = backBufferFormat;
				presentParameters.MultiSampleType        = D3DMULTISAMPLE_NONE;
				presentParameters.SwapEffect             = D3DSWAPEFFECT_DISCARD;
				presentParameters.hDeviceWindow          = GetSafeHwnd();
				presentParameters.Windowed               = TRUE;
				presentParameters.EnableAutoDepthStencil = TRUE;
				presentParameters.AutoDepthStencilFormat = depthStencilFormat;
				presentParameters.Flags                  = 0; 

				const DWORD behavior = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
				hresult = direct3d->CreateDevice(D3DADAPTER_DEFAULT, deviceType, GetSafeHwnd(), behavior, &presentParameters, &device);
				if (SUCCEEDED(hresult))
					created = true;
			}
		}
	}

	if (!created)
	{
		errorMessage = "CreateDevice failed";
		DestroyDevice();
		return;
	}

	hresult = device->CreateVertexBuffer(sizeof(TransformedVertex) * 4, 0, D3DFVF_XYZRHW | D3DFVF_TEX1, D3DPOOL_MANAGED, &vertexBuffer, NULL);
	FAILED_RETURN(hresult, "Could not create vertex buffer");

	const real w = static_cast<float>(width);
	const real h = static_cast<float>(height);

	TransformedVertex vertices[4] =
	{
		{ 0, 0, 0.5, 0.5, -0.5, -0.5 },
		{ w, 0, 0.5, 0.5, 1.5, -0.5 },
		{ w, h, 0.5, 0.5, 1.5, 1.5 },
		{ 0, h, 0.5, 0.5, -0.5, 1.5 },
	};

	void *data = 0;
	hresult = vertexBuffer->Lock(0, 0, &data, 0);
	FAILED_RETURN(hresult, "Could not lock vertex buffer");
	memcpy(data, vertices, sizeof(vertices));
	hresult = vertexBuffer->Unlock();
	FAILED_RETURN(hresult, "Could not unlock vertex buffer");

	// setup the rendering matrices
	D3DXMATRIX matrix;
	hresult = device->SetTransform(D3DTS_PROJECTION, D3DXMatrixPerspectiveFovLH(&matrix, 3.14f / 3.0f, static_cast<float>(width) / static_cast<float>(height), 0.0001f, 100.0f));
	FAILED_RETURN(hresult, "SetTransform projection failed");
	
	// setup lighting
	hresult = device->SetRenderState(D3DRS_AMBIENT, 0x30303030);
	FAILED_RETURN(hresult, "SetRenderState Ambient failed");

	D3DLIGHT9 light;
	memset(&light, 0, sizeof(light));
	light.Type = D3DLIGHT_DIRECTIONAL;
	light.Diffuse.r = 1.0;
	light.Diffuse.g = 1.0;
	light.Diffuse.b = 1.0;
	light.Diffuse.a = 1.0;
	light.Specular.r = 1.0;
	light.Specular.g = 1.0;
	light.Specular.b = 1.0;
	light.Specular.a = 1.0;
	light.Ambient.r = 0.0;
	light.Ambient.g = 0.0;
	light.Ambient.b = 0.0;
	light.Ambient.a = 0.0;
	light.Position.x = 0;
	light.Position.y = 0;
	light.Position.z = 0;
	light.Direction.x = -1;
	light.Direction.y = -1;
	light.Direction.z = 1;
	light.Attenuation0 = 1;
	light.Attenuation1 = 0;
	light.Attenuation2 = 0;
	hresult = device->SetLight(0, &light);
	FAILED_RETURN(hresult, "SetLight failed");

	hresult = device->LightEnable(0, TRUE);
	FAILED_RETURN(hresult, "LightEnable failed");

	D3DMATERIAL9 material = defaultMaterial;
	hresult = device->SetMaterial(&material);
	FAILED_RETURN(hresult, "SetMaterial failed");
}

// ----------------------------------------------------------------------

void CRenderView::DestroyDevice()
{
	// release all the textures
	{
		for (TextureDataMap::iterator i = textureDataMap.begin(); i != textureDataMap.end(); i = textureDataMap.begin())
		{
			i->second->Release();
			textureDataMap.erase(i);
		}
	}

	if (vertexBuffer)
	{
		vertexBuffer->Release();
		vertexBuffer = 0;
	}

	if (device)
	{
		// free the texture from every stage
		for (int i = 0; i < 8; ++i)
			device->SetTexture(i, NULL);

		device->Release();
		device = 0;
	}

	if (direct3d)
	{
		direct3d->Release();
		direct3d = 0;
	}
}

// ----------------------------------------------------------------------

IDirect3DBaseTexture9 *CRenderView::GetTexture(const CString &fileName)
{
	if (fileName == "")
		return NULL;

	TextureDataMap::iterator iterator = textureDataMap.find(fileName);

	// find the texture
	if (iterator == textureDataMap.end())
	{
		IDirect3DBaseTexture9 *baseTexture = NULL;

		//get full path name from the local filename (i.e. convert texture\default.dds to c:\swg\current\..\texture\default.dds)
		char fullFileName[2048];
		TreeFile::getPathName(static_cast<LPCSTR>(fileName), fullFileName, sizeof(fullFileName));

		IDirect3DCubeTexture9 *cubeTexture = NULL;
		const HRESULT hresult = D3DXCreateCubeTextureFromFileEx(device, fullFileName,	D3DX_DEFAULT, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, D3DX_FILTER_TRIANGLE, D3DX_FILTER_BOX, 0, NULL, NULL, &cubeTexture);
		if (SUCCEEDED(hresult) && cubeTexture)
		{
			baseTexture = cubeTexture;
		}
		else
		{
			// create it if it wasn't found
			IDirect3DTexture9 *texture = NULL;
			const HRESULT hresult = D3DXCreateTextureFromFileEx(device, fullFileName,	D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, D3DX_FILTER_TRIANGLE, D3DX_FILTER_BOX, 0, NULL, NULL, &texture);
			if (FAILED(hresult))
			{
				errorMessage = "CreateTexture failed";
				return NULL;
			}
			baseTexture = texture;
		}

		// add it to the map
		textureDataMap[fileName] = baseTexture;

		return baseTexture;
	}

	return iterator->second;
}

// ----------------------------------------------------------------------

void CRenderView::DrawBackground(const char *fileName)
{
#define RS(rs, value)       do { const HRESULT hresult = device->SetRenderState(rs, value);  if (FAILED(hresult)) { errorMessage = "SetRenderState " #rs " failed"; return; }} while (0)
#define TSS0(tss, value)    do { const HRESULT hresult = device->SetTextureStageState(0, tss, value);  if (FAILED(hresult)) { errorMessage = "SetTSS " #tss " failed"; return; }} while (0)
#define TSS1(tss, value)    do { const HRESULT hresult = device->SetTextureStageState(1, tss, value);  if (FAILED(hresult)) { errorMessage = "SetTSS " #tss " failed"; return; }} while (0)

	RS(D3DRS_ZENABLE,          FALSE);
	RS(D3DRS_ALPHABLENDENABLE, FALSE);
	RS(D3DRS_ALPHATESTENABLE,  FALSE);
	RS(D3DRS_LIGHTING,         FALSE);
	RS(D3DRS_STENCILENABLE,    FALSE);
	RS(D3DRS_DITHERENABLE,     FALSE);

	TSS0(D3DTSS_COLOROP,       D3DTOP_SELECTARG1);
	TSS0(D3DTSS_COLORARG1,     D3DTA_TEXTURE);
	TSS0(D3DTSS_ALPHAOP,       D3DTOP_SELECTARG1);
	TSS0(D3DTSS_ALPHAARG1,     D3DTA_TEXTURE);
	TSS0(D3DTSS_TEXCOORDINDEX, 0);

	TSS1(D3DTSS_COLOROP,       D3DTOP_DISABLE);
	TSS1(D3DTSS_ALPHAOP,       D3DTOP_DISABLE);

#undef RS
#undef TSS0
#undef TSS1

	IDirect3DBaseTexture9 *baseTexture = GetTexture(fileName);
	HRESULT hresult = device->SetTexture(0, baseTexture);
	FAILED_RETURN(hresult, "SetTexture failed");
	hresult = device->SetStreamSource(0, vertexBuffer, 0, sizeof(TransformedVertex));
	FAILED_RETURN(hresult, "SetStreamSource failed");
	hresult = device->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1);
	FAILED_RETURN(hresult, "SetVertexShader failed");
	hresult = device->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);
	FAILED_RETURN(hresult, "DrawPrimitive failed");

	device->SetTexture(0, NULL);
}

// ----------------------------------------------------------------------

void CRenderView::BeginScene()
{
	// set the view transform
	D3DXMATRIX matrix;
	const D3DXVECTOR3 eye(0, 0, -1);
	const D3DXVECTOR3 at(0, 0, 0);
	const D3DXVECTOR3 up(0, 1, 0);
	HRESULT hresult = device->SetTransform(D3DTS_VIEW, D3DXMatrixLookAtLH(&matrix, &eye, &at, &up));
	FAILED_RETURN(hresult, "SetTransform view failed");

	hresult = device->SetTransform(D3DTS_WORLDMATRIX(0), &worldMatrix);
	FAILED_RETURN(hresult, "SetTransform world failed");

	hresult = device->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, 0x00000000, 1.0f, 0);
	FAILED_RETURN(hresult, "Clear failed");

	hresult = device->BeginScene();
	FAILED_RETURN(hresult, "BeginScene failed");
}

// ----------------------------------------------------------------------

void CRenderView::EndScene()
{
	HRESULT hresult = device->EndScene();
	FAILED_RETURN(hresult, "EndScene failed");

	if (!errorMessage)
	{
		hresult = device->Present(NULL, NULL, NULL, NULL);
		FAILED_RETURN(hresult, "Present failed");
	}
}

// ----------------------------------------------------------------------

void CRenderView::RenderTexture(const CTextureNode &textureNode)
{
	if (errorMessage)
		return;

	if (textureNode.m_textureName.IsEmpty())
	{	
		errorMessage = "No texture name";
		return;
	}

	BeginScene();

		DrawBackground(textureNode.m_textureName);

	EndScene();
}

// ----------------------------------------------------------------------

void CRenderView::OnRButtonDown(UINT nFlags, CPoint point) 
{
	CView::OnRButtonDown(nFlags, point);

	// get the appropriate context menu
	CMenu *subMenu = leftView->rootMenu.GetSubMenu(0);
	ASSERT(subMenu);
	CMenu *popUpMenu = subMenu->GetSubMenu(5);
	ASSERT(popUpMenu);

	popUpMenu->CheckMenuItem(IDR_DISABLED,            MF_BYCOMMAND | (disabled                 ? MF_CHECKED : MF_UNCHECKED));

	// display it where the mouse was clicked
	ClientToScreen(&point);
	popUpMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, point.x, point.y, this);
}

// ----------------------------------------------------------------------

void CRenderView::OnDisabled() 
{
	disabled = !disabled;
	DestroyDevice();	
}

// ----------------------------------------------------------------------

void CRenderView::OnReset() 
{
	D3DXMatrixIdentity(&worldMatrix);
}

// ======================================================================
