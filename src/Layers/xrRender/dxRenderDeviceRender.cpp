#include "stdafx.h"
#include "dxRenderDeviceRender.h"

#ifndef USE_DX11
ECORE_API CHWCaps Caps;
#endif

#ifdef DEBUG_DRAW
#include "dxDebugRender.h"
#endif

#include "ResourceManager.h"
#include "imgui.h"

#ifdef USE_DX11
#include "imgui_impl_dx11.h"
#else
#include "../xrRenderDX9/imgui_impl_dx9.h"
#endif

dxRenderDeviceRender::dxRenderDeviceRender()
#ifndef XR_EDITOR_NEW
	:	Resources(0)
#endif
{
	;
}

void dxRenderDeviceRender::Copy(IRenderDeviceRender &_in)
{
	*this = *(dxRenderDeviceRender*)&_in;
}

void dxRenderDeviceRender::setGamma(float fGamma)
{
#ifndef XR_EDITOR_NEW
	m_Gamma.Gamma(fGamma);
#endif
}

void dxRenderDeviceRender::setBrightness(float fGamma)
{
#ifndef XR_EDITOR_NEW
	m_Gamma.Brightness(fGamma);
#endif
}

void dxRenderDeviceRender::setContrast(float fGamma)
{
#ifndef XR_EDITOR_NEW
	m_Gamma.Contrast(fGamma);
#endif
}

void dxRenderDeviceRender::updateGamma()
{
#ifndef XR_EDITOR_NEW
	m_Gamma.Update();
#endif
}

void dxRenderDeviceRender::OnDeviceDestroy( BOOL bKeepTextures)
{
#ifndef XR_EDITOR_NEW
	m_WireShader.destroy();
	m_SelectionShader.destroy();

	Resources->OnDeviceDestroy( bKeepTextures);
	RCache.OnDeviceDestroy();
#endif
}

void dxRenderDeviceRender::ValidateHW()
{
}

void dxRenderDeviceRender::DestroyHW()
{
#ifndef XR_EDITOR_NEW
	xr_delete					(Resources);
#ifdef USE_DX11
	ImGui_ImplDX11_Shutdown();
#else
	ImGui_ImplDX9_Shutdown();
#endif
#endif
}

void  dxRenderDeviceRender::Reset(SDL_Window* window, u32 &dwWidth, u32 &dwHeight, float &fWidth_2, float &fHeight_2)
{
#ifndef XR_EDITOR_NEW
#ifdef DEBUG
	_SHOW_REF("*ref -CRenderDevice::ResetTotal: DeviceREF:",RDevice);
#endif // DEBUG	
	
	Resources->reset_begin	();
	Memory.mem_compact		();
	ResourcesDeferredUnload();
#ifndef USE_DX11
	ImGui_ImplDX9_Shutdown();
#endif
	Device.ResizeBuffers(psCurrentVidMode[0], psCurrentVidMode[1]);
	ResourcesDeferredUpload();

	dwWidth = Device.GetSwapchainWidth();
	dwHeight = Device.GetSwapchainHeight();
	fWidth_2 = float(dwWidth / 2);
	fHeight_2 = float(dwHeight / 2);
	Resources->reset_end();

#ifndef USE_DX11
	ImGui_ImplDX9_Init(RDevice);
#endif

#ifdef DEBUG
	_SHOW_REF("*ref +CRenderDevice::ResetTotal: DeviceREF:",RDevice);
#endif // DEBUG
#endif // DEBUG
}

void dxRenderDeviceRender::SetupStates()
{
#ifndef XR_EDITOR_NEW
#ifdef USE_DX11
	//	TODO: DX10: Implement Resetting of render states into default mode
	// SSManager.SetMaxAnisotropy(ps_r__tf_Anisotropic);
	// SSManager.SetMipLodBias(ps_r__tf_Mipbias);
#else //USE_DX11
	for (u32 i=0; i<Caps.raster.dwStages; i++)				{
		CHK_DX(RDevice->SetSamplerState(i, D3DSAMP_MAXANISOTROPY, ps_r__tf_Anisotropic));
		CHK_DX(RDevice->SetSamplerState(i, D3DSAMP_MIPMAPLODBIAS, *(LPDWORD)&ps_r__tf_Mipbias));
		CHK_DX(RDevice->SetSamplerState	( i, D3DSAMP_MINFILTER,	D3DTEXF_LINEAR 		));
		CHK_DX(RDevice->SetSamplerState	( i, D3DSAMP_MAGFILTER,	D3DTEXF_LINEAR 		));
		CHK_DX(RDevice->SetSamplerState	( i, D3DSAMP_MIPFILTER,	D3DTEXF_LINEAR		));
	}
	CHK_DX(RDevice->SetRenderState( D3DRS_DITHERENABLE,		TRUE				));
	CHK_DX(RDevice->SetRenderState( D3DRS_COLORVERTEX,		TRUE				));
	CHK_DX(RDevice->SetRenderState( D3DRS_ZENABLE,			TRUE				));
	CHK_DX(RDevice->SetRenderState( D3DRS_SHADEMODE,			D3DSHADE_GOURAUD	));
	CHK_DX(RDevice->SetRenderState( D3DRS_CULLMODE,			D3DCULL_CCW			));
	CHK_DX(RDevice->SetRenderState( D3DRS_ALPHAFUNC,			D3DCMP_GREATER		));
	CHK_DX(RDevice->SetRenderState( D3DRS_LOCALVIEWER,		TRUE				));

	CHK_DX(RDevice->SetRenderState( D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_MATERIAL	));
	CHK_DX(RDevice->SetRenderState( D3DRS_SPECULARMATERIALSOURCE,D3DMCS_MATERIAL	));
	CHK_DX(RDevice->SetRenderState( D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_MATERIAL	));
	CHK_DX(RDevice->SetRenderState( D3DRS_EMISSIVEMATERIALSOURCE,D3DMCS_COLOR1	));
	CHK_DX(RDevice->SetRenderState( D3DRS_MULTISAMPLEANTIALIAS,	FALSE			));
	CHK_DX(RDevice->SetRenderState( D3DRS_NORMALIZENORMALS,		TRUE			));

	if (psDeviceFlags.test(rsWireframe))	{ CHK_DX(RDevice->SetRenderState( D3DRS_FILLMODE,			D3DFILL_WIREFRAME	)); }
	else									{ CHK_DX(RDevice->SetRenderState( D3DRS_FILLMODE,			D3DFILL_SOLID		)); }

	// ******************** Fog parameters
	CHK_DX(RDevice->SetRenderState( D3DRS_FOGCOLOR,			0					));
	CHK_DX(RDevice->SetRenderState( D3DRS_RANGEFOGENABLE,	FALSE				));
	if (Caps.bTableFog)	{
		CHK_DX(RDevice->SetRenderState( D3DRS_FOGTABLEMODE,	D3DFOG_LINEAR		));
		CHK_DX(RDevice->SetRenderState( D3DRS_FOGVERTEXMODE,	D3DFOG_NONE			));
	} else {
		CHK_DX(RDevice->SetRenderState( D3DRS_FOGTABLEMODE,	D3DFOG_NONE			));
		CHK_DX(RDevice->SetRenderState( D3DRS_FOGVERTEXMODE,	D3DFOG_LINEAR		));
	}

#endif
#endif
}

void dxRenderDeviceRender::OnDeviceCreate(LPCSTR shName)
{
#ifndef XR_EDITOR_NEW
#ifndef USE_DX11
	Caps.Update();
#endif

	// Signal everyone - device created
	RCache.OnDeviceCreate		();
	m_Gamma.Update				();
	Resources->OnDeviceCreate	(shName);
	::Render->create			();

//#ifndef DEDICATED_SERVER
#if !defined(_EDITOR) && !defined(XR_EDITOR_NEW)
	if (!g_dedicated_server)
#endif
	{
		m_WireShader.create			("editor\\wire");
		m_SelectionShader.create	("editor\\selection");

		DUImpl.OnDeviceCreate			();
	}
#endif
}

void dxRenderDeviceRender::Create(SDL_Window* window, u32 &dwWidth, u32 &dwHeight, float &fWidth_2, float &fHeight_2, bool move_window)
{
#ifndef XR_EDITOR_NEW
#ifdef USE_DX11
	ImGui_ImplDX11_Init(RDevice, RContext);
#else
	ImGui_ImplDX9_Init(RDevice);
#endif

	dwWidth = Device.GetSwapchainWidth();
	dwHeight = Device.GetSwapchainHeight();
	fWidth_2 = float(dwWidth / 2);
	fHeight_2 = float(dwHeight / 2);
	Resources = xr_new<CResourceManager>();

#ifdef DEBUG_DRAW
	Device.AddUICommand("dxDebugRenderer", 1, []()
	{
		if (!Engine.External.EditorStates[static_cast<std::uint8_t>(EditorUI::DebugDraw)] || DebugRenderImpl.m_lines.empty())
			return;

		constexpr auto DebugFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs;
		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2((float)Device.TargetWidth, (float)Device.TargetHeight));
		ImGui::SetNextWindowBgAlpha(0.0f);

		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		if (ImGui::Begin("DebugRender", nullptr, DebugFlags)) {
			ImDrawList& CmdList = *ImGui::GetWindowDrawList();
			for (const auto& Line : DebugRenderImpl.m_lines) {
				CmdList.AddLine(
					ImVec2(Line.first.p.x, Line.first.p.y),
					ImVec2(Line.second.p.x, Line.second.p.y),
					Line.first.color
				);
			}
		}

		ImGui::End();
		ImGui::PopStyleVar();
		ImGui::PopStyleVar();
	});
#endif
#endif
}

void dxRenderDeviceRender::SetupGPU( BOOL bForceGPU_SW, BOOL bForceGPU_NonPure, BOOL bForceGPU_REF)
{
}

void dxRenderDeviceRender::overdrawBegin()
{
#ifndef XR_EDITOR_NEW
#ifdef USE_DX11
	//	TODO: DX10: Implement overdrawBegin
	VERIFY(!"dxRenderDeviceRender::overdrawBegin not implemented.");
#else //USE_DX11
	// Turn stenciling
	CHK_DX(RDevice->SetRenderState( D3DRS_STENCILENABLE,		TRUE			));
	CHK_DX(RDevice->SetRenderState( D3DRS_STENCILFUNC,		D3DCMP_ALWAYS	));
	CHK_DX(RDevice->SetRenderState( D3DRS_STENCILREF,		0				));
	CHK_DX(RDevice->SetRenderState( D3DRS_STENCILMASK,		0x00000000		));
	CHK_DX(RDevice->SetRenderState( D3DRS_STENCILWRITEMASK,	0xffffffff		));

	// Increment the stencil buffer for each pixel drawn
	CHK_DX(RDevice->SetRenderState( D3DRS_STENCILFAIL,		D3DSTENCILOP_KEEP		));
	CHK_DX(RDevice->SetRenderState( D3DRS_STENCILPASS,		D3DSTENCILOP_INCRSAT	));

	if (1==Caps.SceneMode)		
	{ CHK_DX(RDevice->SetRenderState( D3DRS_STENCILZFAIL,	D3DSTENCILOP_KEEP		)); }	// Overdraw
	else 
	{ CHK_DX(RDevice->SetRenderState( D3DRS_STENCILZFAIL,	D3DSTENCILOP_INCRSAT	)); }	// ZB access
#endif
#endif
}

void dxRenderDeviceRender::overdrawEnd()
{
#ifndef XR_EDITOR_NEW
#ifdef USE_DX11
	//	TODO: DX10: Implement overdrawEnd
	VERIFY(!"dxRenderDeviceRender::overdrawBegin not implemented.");
#else //USE_DX11
	// Set up the stencil states
	CHK_DX	(RDevice->SetRenderState( D3DRS_STENCILZFAIL,		D3DSTENCILOP_KEEP	));
	CHK_DX	(RDevice->SetRenderState( D3DRS_STENCILFAIL,		D3DSTENCILOP_KEEP	));
	CHK_DX	(RDevice->SetRenderState( D3DRS_STENCILPASS,		D3DSTENCILOP_KEEP	));
	CHK_DX	(RDevice->SetRenderState( D3DRS_STENCILFUNC,		D3DCMP_EQUAL		));
	CHK_DX	(RDevice->SetRenderState( D3DRS_STENCILMASK,		0xff				));

	// Set the background to black
	CHK_DX(RDevice->Clear(0, 0, D3DCLEAR_TARGET, color_xrgb(255, 0, 0), 0, 0));

	// Draw a rectangle wherever the count equal I
	RCache.OnFrameEnd	();
	CHK_DX	(RDevice->SetFVF( FVF::F_TL ));

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

		CHK_DX(RDevice->SetRenderState	( D3DRS_STENCILREF,		I	));
		CHK_DX(RDevice->DrawPrimitiveUP	( D3DPT_TRIANGLESTRIP,	2,	pv, sizeof(FVF::TL) ));
	}
	CHK_DX(RDevice->SetRenderState( D3DRS_STENCILENABLE,		FALSE ));
#endif
#endif
}

void dxRenderDeviceRender::DeferredLoad(BOOL E)
{
#ifndef XR_EDITOR_NEW
	Resources->DeferredLoad(E);
#endif
}

void dxRenderDeviceRender::ResourcesDeferredUpload()
{
#ifndef XR_EDITOR_NEW
	Resources->DeferredUpload();
#endif
}

void dxRenderDeviceRender::ResourcesDeferredUnload()
{
#ifndef XR_EDITOR_NEW
	Resources->DeferredUnload();
#endif
}

void dxRenderDeviceRender::ResourcesGetMemoryUsage(u32& m_base, u32& c_base, u32& m_lmaps, u32& c_lmaps)
{
#ifndef XR_EDITOR_NEW
	if (Resources)
		Resources->_GetMemoryUsage(m_base, c_base, m_lmaps, c_lmaps);
#endif
}

void dxRenderDeviceRender::ResourcesStoreNecessaryTextures()
{
#ifndef XR_EDITOR_NEW
	dxRenderDeviceRender::Instance().Resources->StoreNecessaryTextures();
#endif
}

void dxRenderDeviceRender::ResourcesDumpMemoryUsage()
{
#ifndef XR_EDITOR_NEW
	dxRenderDeviceRender::Instance().Resources->_DumpMemoryUsage();
#endif
}

dxRenderDeviceRender::DeviceState dxRenderDeviceRender::GetDeviceState()
{
#ifndef XR_EDITOR_NEW
#ifdef USE_DX11
	//	TODO: DX10: Implement GetDeviceState
	//	TODO: DX10: Implement DXGI_PRESENT_TEST testing
	//VERIFY(!"dxRenderDeviceRender::overdrawBegin not implemented.");
#else //USE_DX11
	HRESULT	_hr		= RDevice->TestCooperativeLevel();
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
#endif

	return dsOK;
}

BOOL dxRenderDeviceRender::GetForceGPU_REF()
{
	return false;
}

u32 dxRenderDeviceRender::GetCacheStatPolys()
{
	return RCache.stat.polys;
}

void dxRenderDeviceRender::Begin()
{
#ifndef XR_EDITOR_NEW
#ifndef USE_DX11
	CHK_DX					(RDevice->BeginScene());
#endif //USE_DX11
	RCache.OnFrameBegin		();
	RCache.set_CullMode		(CULL_CW);
	RCache.set_CullMode		(CULL_CCW);
#endif
}

void dxRenderDeviceRender::Clear()
{
#ifndef XR_EDITOR_NEW
#ifdef USE_DX11
	RContext->ClearDepthStencilView(RCache.get_ZB(), 
		D3D_CLEAR_DEPTH|D3D_CLEAR_STENCIL, 1.0f, 0);

	if (psDeviceFlags.test(rsClearBB))
	{
		FLOAT ColorRGBA[4] = {0.0f,0.0f,0.0f,0.0f};
		RContext->ClearRenderTargetView(RCache.get_RT(), ColorRGBA);
	}
#else //USE_DX11
	CHK_DX(RDevice->Clear(0,0,
		D3DCLEAR_ZBUFFER|
		(psDeviceFlags.test(rsClearBB)?D3DCLEAR_TARGET:0)|
		(Caps.bStencil?D3DCLEAR_STENCIL:0),
		color_xrgb(0,0,0),1,0
		));
#endif
#endif
}

void DoAsyncScreenshot();

void dxRenderDeviceRender::End()
{
#ifndef XR_EDITOR_NEW
	VERIFY	(RDevice);

	//if (dxRenderDeviceRender::Instance().Caps.SceneMode)	overdrawEnd();

	RCache.OnFrameEnd	();

	{
		SCOPE_EVENT_NAME_GROUP("Async screenshot", "Render");
		DoAsyncScreenshot();
	}

#ifdef USE_DX11
	ImGui_ImplDX11_NewFrame();
	ID3D11RenderTargetView* RTV = RSwapchainTarget;
	RContext->OMSetRenderTargets(1, &RTV, nullptr);
#else
	ImGui_ImplDX9_NewFrame();
	RDevice->SetRenderTarget(0, RSwapchainTarget);
#endif

	{
		SCOPE_EVENT_NAME_GROUP("ImGui draw", "Render");
		ImGui::NewFrame();
		Device.DrawUI();
		ImGui::Render();

#ifdef USE_DX11
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
#else
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
#endif
	}

#ifdef DEBUG_DRAW
	DebugRenderImpl.m_lines.resize(0);
#endif


	{
		SCOPE_EVENT_NAME_GROUP("GPU Wait", "Render");
#ifdef USE_DX11
		RSwapchain->Present(psDeviceFlags.test(rsVSync) ? 1 : 0, 0);
#else
		CHK_DX(RDevice->EndScene());
		RDevice->Present(NULL, NULL, NULL, NULL);
#endif	
	}
#endif	
}

void dxRenderDeviceRender::ResourcesDestroyNecessaryTextures()
{
#ifndef XR_EDITOR_NEW
	Resources->DestroyNecessaryTextures();
#endif
}

void dxRenderDeviceRender::ClearTarget()
{
#ifndef XR_EDITOR_NEW
#ifdef USE_DX11
	FLOAT ColorRGBA[4] = {0.0f,0.0f,0.0f,0.0f};
	RContext->ClearRenderTargetView(RCache.get_RT(), ColorRGBA);
#else //USE_DX11
	CHK_DX(RDevice->Clear(0, 0, D3DCLEAR_TARGET, color_xrgb(0,0,0), 1, 0));
#endif
#endif
}

void dxRenderDeviceRender::SetCacheXform(Fmatrix &mView, Fmatrix &mProject)
{
#ifndef XR_EDITOR_NEW
	RCache.set_xform_view(mView);
	RCache.set_xform_project(mProject);
#endif
}

bool dxRenderDeviceRender::HWSupportsShaderYUV2RGB()
{
	return true;
}

void  dxRenderDeviceRender::OnAssetsChanged()
{
#ifndef XR_EDITOR_NEW
	Resources->m_textures_description.UnLoad();
	Resources->m_textures_description.Load();
#endif
}