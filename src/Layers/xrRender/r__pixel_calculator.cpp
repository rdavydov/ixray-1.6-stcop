#include	"stdafx.h"
#include	"r__pixel_calculator.h"
#define		rt_dimensions 1024
#include	"../xrRender/FBasicVisual.h"

using namespace DirectX;

#ifndef USE_DX11
void	r_pixel_calculator::begin	()
{
	rt.create		("$user$test",		rt_dimensions,rt_dimensions,HW.Caps.fTarget);
	R_CHK			(HW.pDevice->CreateDepthStencilSurface	(rt_dimensions,rt_dimensions,HW.Caps.fDepth,D3DMULTISAMPLE_NONE,0,TRUE,&zb,NULL));

	RCache.set_RT	(rt->pRT);
	RCache.set_ZB	(zb);

	R_ASSERT		(Device.Begin());
}

void	r_pixel_calculator::end		()	
{
	Device.End		();

	RCache.set_RT	(HW.pBaseRT);
	RCache.set_ZB	(HW.pBaseZB);

	_RELEASE		(zb);
	rt			=	0	;
}

//								+X,				-X,				+Y,				-Y,			+Z,				-Z
static Fvector cmNorm[6]	= {{0.f,1.f,0.f}, {0.f,1.f,0.f}, {0.f,0.f,-1.f},{0.f,0.f,1.f}, {0.f,1.f,0.f}, {0.f,1.f,0.f}};
static Fvector cmDir[6]		= {{1.f,0.f,0.f}, {-1.f,0.f,0.f},{0.f,1.f,0.f}, {0.f,-1.f,0.f},{0.f,0.f,1.f}, {0.f,0.f,-1.f}};

r_aabb_ssa		r_pixel_calculator::calculate	(dxRender_Visual* V)	{
	r_aabb_ssa	result			= {0};
	float		area			= float(_sqr(rt_dimensions));

	// 
	u32	id				[6]		;
	for (u32 face=0; face<6; face++)	{
		// setup matrices
		Fmatrix						mProject,mView	;
		Fvector						vFrom			;
		Fbox						aabb			;

		// camera - left-to-right
		mView.build_camera_dir		(vFrom.invert(cmDir[face]).mul(100.f),	cmDir[face],	cmNorm[face])	;
		aabb.xform					(V->vis.box,mView);
		XMStoreFloat4x4(reinterpret_cast<XMFLOAT4X4*>(&mProject),
			XMMatrixOrthographicOffCenterLH(aabb.min.x, aabb.max.x, aabb.min.y, aabb.max.y, aabb.min.z, aabb.max.z));
		RCache.set_xform_world		(Fidentity);
		RCache.set_xform_view		(mView);
		RCache.set_xform_project	(mProject);

		// render-0
		Device.Clear				();	// clear-ZB
		RCache.set_Shader			(V->shader);
		V->Render					(1.f);

		// render-1
		RImplementation.HWOCC.occq_begin	(id[face]);
		V->Render							(1.f);
		RImplementation.HWOCC.occq_end		(id[face]);
	}

	// 
	for (u32 it=0; it<6; it++)	{
		float	pixels	= (float)RImplementation.HWOCC.occq_get	(id[it]);
		float	coeff	= clampr(pixels/area,float(0),float(1));
		Msg		("[%d]ssa_c: %1.3f,%f/%f",it,coeff,pixels,area);
		result.ssa	[it]= (u8)clampr(iFloor(coeff*255.f+0.5f),int(0),int(255));
	}

	return result	;
}

void	r_pixel_calculator	::run	()
{
	begin	();
	for (u32 it=0; it<RImplementation.Visuals.size(); it++)
	{
		if (0==dynamic_cast<IRender_Mesh*>(RImplementation.Visuals[it]))		continue;
		Msg	("*%d*",it);
		calculate	((dxRender_Visual*)RImplementation.Visuals[it]);
	}
	end		();
}
#endif