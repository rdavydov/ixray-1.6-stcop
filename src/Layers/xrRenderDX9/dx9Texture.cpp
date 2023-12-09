// Texture.cpp: implementation of the CTexture class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#pragma hdrstop

#ifndef _EDITOR
#include "../xrRender/dxRenderDeviceRender.h"
#endif

#include <DirectXTex.h>
#include "../xrRenderDX9/DDSTextureLoader9.h"
using namespace DirectX;


void fix_texture_name(LPSTR fn)
{
	LPSTR _ext = strext(fn);
	if(  _ext					&&
	  (0==_stricmp(_ext,".tga")	||
		0==_stricmp(_ext,".dds")	||
		0==_stricmp(_ext,".bmp")	||
		0==_stricmp(_ext,".ogm")	) )
		*_ext = 0;
}

int get_texture_load_lod(LPCSTR fn) {
    auto& sect = pSettings->r_section("reduce_lod_texture_list");

    for (const auto& data : sect.Data) {
        if (strstr(fn, data.first.c_str())) {
            if (psTextureLOD < 1) {
                return 0;
            } else {
                if (psTextureLOD < 3) {
                    return 1;
                } else {
                    return 2;
                }
            }
        }
    }

    if (psTextureLOD < 2) {
        return 0;
    } else {
        if (psTextureLOD < 4) {
            return 1;
        } else {
            return 2;
        }
    }
}

u32 calc_texture_size(int lod, u32 mip_cnt, u32 orig_size)
{
	if(1==mip_cnt)
		return orig_size;

	int _lod		= lod;
	float res		= float(orig_size);

	while(_lod>0){
		--_lod;
		res		-= res/1.333f;
	}
	return iFloor	(res);
}

const float		_BUMPHEIGH = 8.f;
//////////////////////////////////////////////////////////////////////
// Utility pack
//////////////////////////////////////////////////////////////////////
IC u32 GetPowerOf2Plus1	(u32 v)
{
        u32 cnt=0;
        while (v) {v>>=1; cnt++; };
        return cnt;
}
IC void	Reduce				(int& w, int& h, int& l, int& skip)
{
	while ((l>1) && skip)
	{
		w /= 2;
		h /= 2;
		l -= 1;

		skip--;
	}
	if (w<1)	w=1;
	if (h<1)	h=1;
}

template	<class _It>
IC	void	TW_Iterate_1OP
(
	ID3DTexture2D*		t_dst,
	ID3DTexture2D*		t_src,
	const _It				pred
)
{
	DWORD mips							= t_dst->GetLevelCount();
	R_ASSERT							(mips == t_src->GetLevelCount());
	for (DWORD i = 0; i < mips; i++)	{
		D3DLOCKED_RECT				Rsrc,Rdst;
		D3DSURFACE_DESC				desc,descS;

		t_dst->GetLevelDesc			(i, &desc);
		t_src->GetLevelDesc			(i, &descS);
		VERIFY						(desc.Format==descS.Format);
		VERIFY						(desc.Format==D3DFMT_A8R8G8B8);
		t_src->LockRect				(i,&Rsrc,0,0);
		t_dst->LockRect				(i,&Rdst,0,0);
		for (u32 y = 0; y < desc.Height; y++)	{
			for (u32 x = 0; x < desc.Width; x++)	{
				DWORD&	pSrc	= *(((DWORD*)((BYTE*)Rsrc.pBits + (y * Rsrc.Pitch)))+x);
				DWORD&	pDst	= *(((DWORD*)((BYTE*)Rdst.pBits + (y * Rdst.Pitch)))+x);
				pDst			= pred(pDst,pSrc);
			}
		}
		t_dst->UnlockRect			(i);
		t_src->UnlockRect			(i);
	}
}
template	<class _It>
IC	void	TW_Iterate_2OP
(
	ID3DTexture2D*		t_dst,
	ID3DTexture2D*		t_src0,
	ID3DTexture2D*		t_src1,
	const _It				pred
 )
{
	DWORD mips							= t_dst->GetLevelCount();
	R_ASSERT							(mips == t_src0->GetLevelCount());
	R_ASSERT							(mips == t_src1->GetLevelCount());
	for (DWORD i = 0; i < mips; i++)	{
		D3DLOCKED_RECT				Rsrc0,Rsrc1,Rdst;
		D3DSURFACE_DESC				desc,descS0,descS1;

		t_dst->GetLevelDesc			(i, &desc);
		t_src0->GetLevelDesc		(i, &descS0);
		t_src1->GetLevelDesc		(i, &descS1);
		VERIFY						(desc.Format==descS0.Format);
		VERIFY						(desc.Format==descS1.Format);
		VERIFY						(desc.Format==D3DFMT_A8R8G8B8);
		t_src0->LockRect			(i,&Rsrc0,	0,0);
		t_src1->LockRect			(i,&Rsrc1,	0,0);
		t_dst->LockRect				(i,&Rdst,	0,0);
		for (u32 y = 0; y < desc.Height; y++)	{
			for (u32 x = 0; x < desc.Width; x++)	{
				DWORD&	pSrc0	= *(((DWORD*)((BYTE*)Rsrc0.pBits + (y * Rsrc0.Pitch)))+x);
				DWORD&	pSrc1	= *(((DWORD*)((BYTE*)Rsrc1.pBits + (y * Rsrc1.Pitch)))+x);
				DWORD&	pDst	= *(((DWORD*)((BYTE*)Rdst.pBits  + (y * Rdst.Pitch)))+x);
				pDst			= pred(pDst,pSrc0,pSrc1);
			}
		}
		t_dst->UnlockRect			(i);
		t_src0->UnlockRect			(i);
		t_src1->UnlockRect			(i);
	}
}

IC u32 it_gloss_rev		(u32 d, u32 s)	{	return	color_rgba	(
	color_get_A(s),		// gloss
	color_get_B(d),
	color_get_G(d),
	color_get_R(d)		);
}
IC u32 it_gloss_rev_base(u32 d, u32 s)	{	
	u32		occ		= color_get_A(d)/3;
	u32		def		= 8;
	u32		gloss	= (occ*1+def*3)/4;
	return	color_rgba	(
		gloss,			// gloss
		color_get_B(d),
		color_get_G(d),
		color_get_R(d)
	);
}
IC u32 it_difference	(u32 d, u32 orig, u32 ucomp)	{	return	color_rgba(
	128+(int(color_get_R(orig))-int(color_get_R(ucomp)))*2,		// R-error
	128+(int(color_get_G(orig))-int(color_get_G(ucomp)))*2,		// G-error
	128+(int(color_get_B(orig))-int(color_get_B(ucomp)))*2,		// B-error
	128+(int(color_get_A(orig))-int(color_get_A(ucomp)))*2	);	// A-error	
}
IC u32 it_height_rev	(u32 d, u32 s)	{	return	color_rgba	(
	color_get_A(d),					// diff x
	color_get_B(d),					// diff y
	color_get_G(d),					// diff z
	color_get_R(s)	);				// height
}
IC u32 it_height_rev_base(u32 d, u32 s)	{	return	color_rgba	(
	color_get_A(d),					// diff x
	color_get_B(d),					// diff y
	color_get_G(d),					// diff z
	(color_get_R(s)+color_get_G(s)+color_get_B(s))/3	);	// height
}

ID3DBaseTexture*	CRender::texture_load(LPCSTR fRName, u32& ret_msize)
{
	TexMetadata				imageInfo		= {};
	ID3DTexture2D*			pTexture2D		= NULL;
	IDirect3DCubeTexture9*	pTextureCUBE	= NULL;
	string_path				fn;
	u32						dwWidth,dwHeight;
	u32						img_size		= 0;
	int						img_loaded_lod	= 0;
	D3DFORMAT				fmt;
	u32						mip_cnt=u32(-1);
	// validation
	R_ASSERT				(fRName);
	R_ASSERT				(fRName[0]);

	HRESULT result_ = {};
	// make file name
	string_path				fname;
	xr_strcpy(fname,fRName); //. andy if (strext(fname)) *strext(fname)=0;
	fix_texture_name		(fname);
	IReader* S				= NULL;
	if (!FS.exist(fn, "$game_textures$", fname, ".dds") && strstr(fname, "_bump"))	goto _BUMP_from_base;
	if (FS.exist(fn,"$level$",			fname,	".dds"))							goto _DDS;
	if (FS.exist(fn,"$game_saves$",		fname,	".dds"))							goto _DDS;
	if (FS.exist(fn,"$game_textures$",	fname,	".dds"))							goto _DDS;


#ifdef _EDITOR
	ELog.Msg(mtError,"Can't find texture '%s'",fname);
	return 0;
#else

	Msg("! Can't find texture '%s'",fname);
	R_ASSERT(FS.exist(fn,"$game_textures$",	"ed\\ed_not_existing_texture",".dds"));
	goto _DDS;

//	Debug.fatal(DEBUG_INFO,"Can't find texture '%s'",fname);

#endif

_DDS:
	{
		// Load and get header
		S = FS.r_open(fn);
#ifdef DEBUG
		Msg("* Loaded: %s[%d]b", fn, S->length());
#endif // DEBUG
		img_size = S->length();
		R_ASSERT(S);
		R_CHK2(GetMetadataFromDDSMemory(S->pointer(), S->length(), DDS_FLAGS::DDS_FLAGS_NONE, imageInfo), fn);
		if (imageInfo.IsCubemap() || imageInfo.IsVolumemap()) {
			goto _DDS_CUBE;
		}
		else {
			goto _DDS_2D;
		}
_DDS_CUBE:
		{
			HRESULT const result_ = CreateDDSTextureFromMemory(
				HW.pDevice,
				static_cast<const uint8_t*>(S->pointer()),
				S->length(),
				&pTextureCUBE);

			FS.r_close				(S);

			if ( FAILED(result_) ) {
				Msg					("! Can't load texture '%s'",fn);
				string_path			temp;
				R_ASSERT			( FS.exist( temp, "$game_textures$", "ed\\ed_not_existing_texture", ".dds" ) );
				R_ASSERT			( xr_strcmp(temp,fn) );
				xr_strcpy			( fn, temp );
				goto _DDS;
			}

			// OK
			dwWidth					= imageInfo.width;
			dwHeight				= imageInfo.height;
			//fmt						= imageInfo.format;
			ret_msize				= calc_texture_size(img_loaded_lod, mip_cnt, img_size);
			mip_cnt					= pTextureCUBE->GetLevelCount();
			return					pTextureCUBE;
		}
_DDS_2D:
		{
			_strlwr					(fn);
			// Load   SYS-MEM-surface, bound to device restrictions
			ID3DTexture2D*		T_sysmem;
			
			HRESULT const result_ = CreateDDSTextureFromMemory(
				HW.pDevice, 
				static_cast<const uint8_t*>(S->pointer()), 
				S->length(), 
				&T_sysmem);

			FS.r_close				(S);

			if ( FAILED(result_) ) {
				Msg					("! Can't load texture '%s'",fn);
				string_path			temp;
				R_ASSERT			( FS.exist( temp, "$game_textures$", "ed\\ed_not_existing_texture", ".dds" ) );
				_strlwr				(temp);
				R_ASSERT			( xr_strcmp(temp,fn) );
				xr_strcpy			( fn, temp );
				goto _DDS;
			}

			img_loaded_lod = get_texture_load_lod(fn);
			mip_cnt = T_sysmem->GetLevelCount();
			ret_msize = calc_texture_size(img_loaded_lod, mip_cnt, img_size);
			return					T_sysmem;
		}
	}
_BUMP_from_base: 
	{
		Msg("! Fallback to default bump map: %s", fname);
		if (strstr(fname, "_bump#")) {
			R_ASSERT2(FS.exist(fn, "$game_textures$", "ed\\ed_dummy_bump#", ".dds"), "ed_dummy_bump#");
			S = FS.r_open(fn);
			R_ASSERT2(S, fn);
			img_size = S->length();
			goto _DDS_2D;
		}
		if (strstr(fname, "_bump")) {
			R_ASSERT2(FS.exist(fn, "$game_textures$", "ed\\ed_dummy_bump", ".dds"), "ed_dummy_bump");
			S = FS.r_open(fn);
			R_ASSERT2(S, fn);
			img_size = S->length();
			goto _DDS_2D;
		}
	}
}
