#include "stdafx.h"
#include "dxRenderDeviceRender.h"

#include "ResourceManager.h"

dxRenderDeviceRender::dxRenderDeviceRender()
	:	Resources(0)
{
	;
}

void dxRenderDeviceRender::Copy(IRenderDeviceRender &_in)
{
	*this = *(dxRenderDeviceRender*)&_in;
}

void dxRenderDeviceRender::setGamma(float fGamma)
{
	m_Gamma.Gamma(fGamma);
}

void dxRenderDeviceRender::setBrightness(float fGamma)
{
	m_Gamma.Brightness(fGamma);
}

void dxRenderDeviceRender::setContrast(float fGamma)
{
	m_Gamma.Contrast(fGamma);
}

void dxRenderDeviceRender::updateGamma()
{
	m_Gamma.Update();
}

void dxRenderDeviceRender::OnDeviceDestroy( BOOL bKeepTextures)
{
	m_WireShader.destroy();
	m_SelectionShader.destroy();

	Resources->OnDeviceDestroy( bKeepTextures);
	RCache.OnDeviceDestroy();
}

void dxRenderDeviceRender::ValidateHW()
{
	HW.Validate();
}

void dxRenderDeviceRender::DestroyHW()
{
	xr_delete					(Resources);
	HW.DestroyDevice			();
}

void  dxRenderDeviceRender::Reset( HWND hWnd, u32 &dwWidth, u32 &dwHeight, float &fWidth_2, float &fHeight_2)
{
#ifdef DEBUG
	_SHOW_REF("*ref -CRenderDevice::ResetTotal: DeviceREF:",HW.pDevice);
#endif // DEBUG	

	Resources->reset_begin	();
	Memory.mem_compact		();
	ResourcesDeferredUnload();
	HW.Reset				(hWnd);
	ResourcesDeferredUpload();

#ifdef USE_DX11
	dwWidth					= HW.m_ChainDesc.BufferDesc.Width;
	dwHeight				= HW.m_ChainDesc.BufferDesc.Height;
#else //USE_DX11
	dwWidth					= HW.DevPP.BackBufferWidth;
	dwHeight				= HW.DevPP.BackBufferHeight;
#endif

	fWidth_2				= float(dwWidth/2);
	fHeight_2				= float(dwHeight/2);
	Resources->reset_end	();

#ifdef DEBUG
	_SHOW_REF("*ref +CRenderDevice::ResetTotal: DeviceREF:",HW.pDevice);
#endif // DEBUG
}

void dxRenderDeviceRender::SetupStates()
{
	HW.Caps.Update			();

#ifdef USE_DX11
	//	TODO: DX10: Implement Resetting of render states into default mode
	SSManager.SetMaxAnisotropy(ps_r__tf_Anisotropic);
	SSManager.SetMipLodBias(ps_r__tf_Mipbias);
#else //USE_DX11
	for (u32 i=0; i<HW.Caps.raster.dwStages; i++)				{
		CHK_DX(HW.pDevice->SetSamplerState(i, D3DSAMP_MAXANISOTROPY, ps_r__tf_Anisotropic));
		CHK_DX(HW.pDevice->SetSamplerState(i, D3DSAMP_MIPMAPLODBIAS, ps_r__tf_Mipbias));
		CHK_DX(HW.pDevice->SetSamplerState	( i, D3DSAMP_MINFILTER,	D3DTEXF_LINEAR 		));
		CHK_DX(HW.pDevice->SetSamplerState	( i, D3DSAMP_MAGFILTER,	D3DTEXF_LINEAR 		));
		CHK_DX(HW.pDevice->SetSamplerState	( i, D3DSAMP_MIPFILTER,	D3DTEXF_LINEAR		));
	}
	CHK_DX(HW.pDevice->SetRenderState( D3DRS_DITHERENABLE,		TRUE				));
	CHK_DX(HW.pDevice->SetRenderState( D3DRS_COLORVERTEX,		TRUE				));
	CHK_DX(HW.pDevice->SetRenderState( D3DRS_ZENABLE,			TRUE				));
	CHK_DX(HW.pDevice->SetRenderState( D3DRS_SHADEMODE,			D3DSHADE_GOURAUD	));
	CHK_DX(HW.pDevice->SetRenderState( D3DRS_CULLMODE,			D3DCULL_CCW			));
	CHK_DX(HW.pDevice->SetRenderState( D3DRS_ALPHAFUNC,			D3DCMP_GREATER		));
	CHK_DX(HW.pDevice->SetRenderState( D3DRS_LOCALVIEWER,		TRUE				));

	CHK_DX(HW.pDevice->SetRenderState( D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_MATERIAL	));
	CHK_DX(HW.pDevice->SetRenderState( D3DRS_SPECULARMATERIALSOURCE,D3DMCS_MATERIAL	));
	CHK_DX(HW.pDevice->SetRenderState( D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_MATERIAL	));
	CHK_DX(HW.pDevice->SetRenderState( D3DRS_EMISSIVEMATERIALSOURCE,D3DMCS_COLOR1	));
	CHK_DX(HW.pDevice->SetRenderState( D3DRS_MULTISAMPLEANTIALIAS,	FALSE			));
	CHK_DX(HW.pDevice->SetRenderState( D3DRS_NORMALIZENORMALS,		TRUE			));

	if (psDeviceFlags.test(rsWireframe))	{ CHK_DX(HW.pDevice->SetRenderState( D3DRS_FILLMODE,			D3DFILL_WIREFRAME	)); }
	else									{ CHK_DX(HW.pDevice->SetRenderState( D3DRS_FILLMODE,			D3DFILL_SOLID		)); }

	// ******************** Fog parameters
	CHK_DX(HW.pDevice->SetRenderState( D3DRS_FOGCOLOR,			0					));
	CHK_DX(HW.pDevice->SetRenderState( D3DRS_RANGEFOGENABLE,	FALSE				));
	if (HW.Caps.bTableFog)	{
		CHK_DX(HW.pDevice->SetRenderState( D3DRS_FOGTABLEMODE,	D3DFOG_LINEAR		));
		CHK_DX(HW.pDevice->SetRenderState( D3DRS_FOGVERTEXMODE,	D3DFOG_NONE			));
	} else {
		CHK_DX(HW.pDevice->SetRenderState( D3DRS_FOGTABLEMODE,	D3DFOG_NONE			));
		CHK_DX(HW.pDevice->SetRenderState( D3DRS_FOGVERTEXMODE,	D3DFOG_LINEAR		));
	}

#endif
}

void dxRenderDeviceRender::OnDeviceCreate(LPCSTR shName)
{
	// Signal everyone - device created
	RCache.OnDeviceCreate		();
	m_Gamma.Update				();
	Resources->OnDeviceCreate	(shName);
	::Render->create			();
	Device.Statistic->OnDeviceCreate	();

//#ifndef DEDICATED_SERVER
	if (!g_dedicated_server)
	{
		m_WireShader.create			("editor\\wire");
		m_SelectionShader.create	("editor\\selection");

		DUImpl.OnDeviceCreate			();
	}
//#endif
}

void dxRenderDeviceRender::Create( HWND hWnd, u32 &dwWidth, u32 &dwHeight, float &fWidth_2, float &fHeight_2, bool move_window)
{
	HW.CreateDevice		(hWnd, move_window);
#ifdef USE_DX11
	dwWidth					= HW.m_ChainDesc.BufferDesc.Width;
	dwHeight				= HW.m_ChainDesc.BufferDesc.Height;
#else //USE_DX11
	dwWidth					= HW.DevPP.BackBufferWidth;
	dwHeight				= HW.DevPP.BackBufferHeight;
#endif
	fWidth_2			= float(dwWidth/2)			;
	fHeight_2			= float(dwHeight/2)			;
	Resources			= xr_new<CResourceManager>		();
}

void dxRenderDeviceRender::SetupGPU( BOOL bForceGPU_SW, BOOL bForceGPU_NonPure, BOOL bForceGPU_REF)
{
	HW.Caps.bForceGPU_SW		= bForceGPU_SW;
	HW.Caps.bForceGPU_NonPure	= bForceGPU_NonPure;
	HW.Caps.bForceGPU_REF		= bForceGPU_REF;
}

void dxRenderDeviceRender::overdrawBegin()
{
#ifdef USE_DX11
	//	TODO: DX10: Implement overdrawBegin
	VERIFY(!"dxRenderDeviceRender::overdrawBegin not implemented.");
#else //USE_DX11
	// Turn stenciling
	CHK_DX(HW.pDevice->SetRenderState( D3DRS_STENCILENABLE,		TRUE			));
	CHK_DX(HW.pDevice->SetRenderState( D3DRS_STENCILFUNC,		D3DCMP_ALWAYS	));
	CHK_DX(HW.pDevice->SetRenderState( D3DRS_STENCILREF,		0				));
	CHK_DX(HW.pDevice->SetRenderState( D3DRS_STENCILMASK,		0x00000000		));
	CHK_DX(HW.pDevice->SetRenderState( D3DRS_STENCILWRITEMASK,	0xffffffff		));

	// Increment the stencil buffer for each pixel drawn
	CHK_DX(HW.pDevice->SetRenderState( D3DRS_STENCILFAIL,		D3DSTENCILOP_KEEP		));
	CHK_DX(HW.pDevice->SetRenderState( D3DRS_STENCILPASS,		D3DSTENCILOP_INCRSAT	));

	if (1==HW.Caps.SceneMode)		
	{ CHK_DX(HW.pDevice->SetRenderState( D3DRS_STENCILZFAIL,	D3DSTENCILOP_KEEP		)); }	// Overdraw
	else 
	{ CHK_DX(HW.pDevice->SetRenderState( D3DRS_STENCILZFAIL,	D3DSTENCILOP_INCRSAT	)); }	// ZB access
#endif
}

void dxRenderDeviceRender::overdrawEnd()
{
#ifdef USE_DX11
	//	TODO: DX10: Implement overdrawEnd
	VERIFY(!"dxRenderDeviceRender::overdrawBegin not implemented.");
#else //USE_DX11
	// Set up the stencil states
	CHK_DX	(HW.pDevice->SetRenderState( D3DRS_STENCILZFAIL,		D3DSTENCILOP_KEEP	));
	CHK_DX	(HW.pDevice->SetRenderState( D3DRS_STENCILFAIL,		D3DSTENCILOP_KEEP	));
	CHK_DX	(HW.pDevice->SetRenderState( D3DRS_STENCILPASS,		D3DSTENCILOP_KEEP	));
	CHK_DX	(HW.pDevice->SetRenderState( D3DRS_STENCILFUNC,		D3DCMP_EQUAL		));
	CHK_DX	(HW.pDevice->SetRenderState( D3DRS_STENCILMASK,		0xff				));

	// Set the background to black
	CHK_DX(HW.pDevice->Clear(0, 0, D3DCLEAR_TARGET, color_xrgb(255, 0, 0), 0, 0));

	// Draw a rectangle wherever the count equal I
	RCache.OnFrameEnd	();
	CHK_DX	(HW.pDevice->SetFVF( FVF::F_TL ));

	// Render gradients
	for (int I=0; I<12; I++ ) 
	{
		u32	_c	= I*256/13;
		u32	c = color_xrgb(_c, _c, _c);

		FVF::TL	pv[4];
		pv[0].set(float(0),			float(RCache.get_height()),	c,0,0);			
		pv[1].set(float(0),			float(0),			c,0,0);					
		pv[2].set(float( RCache.get_width()),	float(RCache.get_height()),	c,0,0);	
		pv[3].set(float( RCache.get_width()),	float(0),			c,0,0);

		CHK_DX(HW.pDevice->SetRenderState	( D3DRS_STENCILREF,		I	));
		CHK_DX(HW.pDevice->DrawPrimitiveUP	( D3DPT_TRIANGLESTRIP,	2,	pv, sizeof(FVF::TL) ));
	}
	CHK_DX(HW.pDevice->SetRenderState( D3DRS_STENCILENABLE,		FALSE ));
#endif
}

void dxRenderDeviceRender::DeferredLoad(BOOL E)
{
	Resources->DeferredLoad(E);
}

void dxRenderDeviceRender::ResourcesDeferredUpload()
{
	Resources->DeferredUpload();
}

void dxRenderDeviceRender::ResourcesDeferredUnload()
{
	Resources->DeferredUnload();
}

void dxRenderDeviceRender::ResourcesGetMemoryUsage(u32& m_base, u32& c_base, u32& m_lmaps, u32& c_lmaps)
{
	if (Resources)
		Resources->_GetMemoryUsage(m_base, c_base, m_lmaps, c_lmaps);
}

void dxRenderDeviceRender::ResourcesStoreNecessaryTextures()
{
	dxRenderDeviceRender::Instance().Resources->StoreNecessaryTextures();
}

void dxRenderDeviceRender::ResourcesDumpMemoryUsage()
{
	dxRenderDeviceRender::Instance().Resources->_DumpMemoryUsage();
}

dxRenderDeviceRender::DeviceState dxRenderDeviceRender::GetDeviceState()
{
	HW.Validate		();
#ifdef USE_DX11
	//	TODO: DX10: Implement GetDeviceState
	//	TODO: DX10: Implement DXGI_PRESENT_TEST testing
	//VERIFY(!"dxRenderDeviceRender::overdrawBegin not implemented.");
#else //USE_DX11
	HRESULT	_hr		= HW.pDevice->TestCooperativeLevel();
	if (FAILED(_hr))
	{
		// If the device was lost, do not render until we get it back
		if		(D3DERR_DEVICELOST==_hr)
			return dsLost;

		// Check if the device is ready to be reset
		if		(D3DERR_DEVICENOTRESET==_hr)
			return dsNeedReset;
	}
#endif

	return dsOK;
}

BOOL dxRenderDeviceRender::GetForceGPU_REF()
{
	return HW.Caps.bForceGPU_REF;
}

u32 dxRenderDeviceRender::GetCacheStatPolys()
{
	return RCache.stat.polys;
}

void dxRenderDeviceRender::Begin()
{
#ifndef USE_DX11
	CHK_DX					(HW.pDevice->BeginScene());
#endif //USE_DX11
	RCache.OnFrameBegin		();
	RCache.set_CullMode		(CULL_CW);
	RCache.set_CullMode		(CULL_CCW);
	if (HW.Caps.SceneMode)	overdrawBegin	();
}

void dxRenderDeviceRender::Clear()
{
#ifdef USE_DX11
	HW.pContext->ClearDepthStencilView(RCache.get_ZB(), 
		D3D_CLEAR_DEPTH|D3D_CLEAR_STENCIL, 1.0f, 0);

	if (psDeviceFlags.test(rsClearBB))
	{
		FLOAT ColorRGBA[4] = {0.0f,0.0f,0.0f,0.0f};
		HW.pContext->ClearRenderTargetView(RCache.get_RT(), ColorRGBA);
	}
#else //USE_DX11
	CHK_DX(HW.pDevice->Clear(0,0,
		D3DCLEAR_ZBUFFER|
		(psDeviceFlags.test(rsClearBB)?D3DCLEAR_TARGET:0)|
		(HW.Caps.bStencil?D3DCLEAR_STENCIL:0),
		color_xrgb(0,0,0),1,0
		));
#endif
}

void DoAsyncScreenshot();

void dxRenderDeviceRender::End()
{
	VERIFY	(HW.pDevice);

	if (HW.Caps.SceneMode)	overdrawEnd();

	RCache.OnFrameEnd	();

	DoAsyncScreenshot();

#ifdef USE_DX11
	if (psDeviceFlags.test(rsVSync)) {
		HW.m_pSwapChain->Present(1, 0);
	} else {
		HW.m_pSwapChain->Present(0, 0);
	}
	
#else //USE_DX11
	CHK_DX				(HW.pDevice->EndScene());

	HW.pDevice->Present( NULL, NULL, NULL, NULL );
#endif
	//HRESULT _hr		= HW.pDevice->Present( NULL, NULL, NULL, NULL );
	//if				(D3DERR_DEVICELOST==_hr)	return;			// we will handle this later
}

void dxRenderDeviceRender::ResourcesDestroyNecessaryTextures()
{
	Resources->DestroyNecessaryTextures();
}

void dxRenderDeviceRender::ClearTarget()
{
#ifdef USE_DX11
	FLOAT ColorRGBA[4] = {0.0f,0.0f,0.0f,0.0f};
	HW.pContext->ClearRenderTargetView(RCache.get_RT(), ColorRGBA);
#else //USE_DX11
	CHK_DX(HW.pDevice->Clear(0, 0, D3DCLEAR_TARGET, color_xrgb(0,0,0), 1, 0));
#endif
}

void dxRenderDeviceRender::SetCacheXform(Fmatrix &mView, Fmatrix &mProject)
{
	RCache.set_xform_view(mView);
	RCache.set_xform_project(mProject);
}

bool dxRenderDeviceRender::HWSupportsShaderYUV2RGB()
{
	u32		v_dev	= CAP_VERSION(HW.Caps.raster_major, HW.Caps.raster_minor);
	u32		v_need	= CAP_VERSION(2,0);
	return (v_dev>=v_need);
}

void  dxRenderDeviceRender::OnAssetsChanged()
{
	Resources->m_textures_description.UnLoad();
	Resources->m_textures_description.Load();
}
