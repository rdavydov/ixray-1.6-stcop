#include "stdafx.h"
#pragma hdrstop

#include "r_backend_xform.h"

void	R_xforms::set_W			(const Fmatrix& m)
{
	m_w.set			(m);
	m_wv.mul_43		(m_v,m_w);
	m_wvp.mul		(m_p,m_wv);
	if (c_w)		RCache.set_c(c_w,	m_w);
	if (c_wv)		RCache.set_c(c_wv,	m_wv);
	if (c_wvp)		RCache.set_c(c_wvp,	m_wvp);
	m_bInvWValid	= false;
	if (c_invw)		apply_invw();
	RCache.set_xform(D3DTS_WORLD,m);
}
void	R_xforms::set_V			(const Fmatrix& m)
{
	m_v.set			(m);
	m_wv.mul_43		(m_v,m_w);
	m_vp.mul		(m_p,m_v);
	m_wvp.mul		(m_p,m_wv);
	if (c_v)		RCache.set_c(c_v,	m_v);
	if (c_vp)		RCache.set_c(c_vp,	m_vp);
	if (c_wv)		RCache.set_c(c_wv,	m_wv);
	if (c_wvp)		RCache.set_c(c_wvp,	m_wvp);
	RCache.set_xform(D3DTS_VIEW,m);
}
void	R_xforms::set_P			(const Fmatrix& m)
{
	m_p.set			(m);
	m_vp.mul		(m_p,m_v);
	m_wvp.mul		(m_p,m_wv);
	if (c_p)		RCache.set_c(c_p,	m_p);
	if (c_vp)		RCache.set_c(c_vp,	m_vp);
	if (c_wvp)		RCache.set_c(c_wvp,	m_wvp);
	// always setup projection - D3D relies on it to work correctly :(
	RCache.set_xform(D3DTS_PROJECTION,m);		
}

void	R_xforms::set_W_old			(const Fmatrix& m)
{
	m_w_old.set			(m);
	m_wv_old.mul_43		(m_v_old,m_w_old);
	m_wvp_old.mul		(m_p_old,m_wv_old);

	if (c_w_old)		RCache.set_c(c_w_old,	m_w_old);
	if (c_wv_old)		RCache.set_c(c_wv_old,	m_wv_old);
	if (c_wvp_old)		RCache.set_c(c_wvp_old,	m_wvp_old);
}
void	R_xforms::set_V_old			(const Fmatrix& m)
{
	m_v_old.set			(m);
	m_wv_old.mul_43		(m_v_old,m_w_old);
	m_vp_old.mul		(m_p_old,m_v_old);
	m_wvp_old.mul		(m_p_old,m_wv_old);

	if (c_v_old)		RCache.set_c(c_v_old,	m_v_old);
	if (c_vp_old)		RCache.set_c(c_vp_old,	m_vp_old);
	if (c_wv_old)		RCache.set_c(c_wv_old,	m_wv_old);
	if (c_wvp_old)		RCache.set_c(c_wvp_old,	m_wvp_old);
}
void	R_xforms::set_P_old			(const Fmatrix& m)
{
	m_p_old.set			(m);
	m_vp_old.mul		(m_p_old,m_v_old);
	m_wvp_old.mul		(m_p_old,m_wv_old);
	if (c_p_old)		RCache.set_c(c_p_old,	m_p_old);
	if (c_vp_old)		RCache.set_c(c_vp_old,	m_vp_old);
	if (c_wvp_old)		RCache.set_c(c_wvp_old,	m_wvp_old);
}

void	R_xforms::apply_invw()
{
	VERIFY(c_invw);

	if (!m_bInvWValid)
	{
		m_invw.invert_b(m_w);
		m_bInvWValid = true;
	}

	RCache.set_c( c_invw, m_invw);
}

void	R_xforms::unmap			()
{
	c_invw		= NULL;

	c_w			= NULL;
	c_v			= NULL;
	c_p			= NULL;
	c_wv		= NULL;
	c_vp		= NULL;
	c_wvp		= NULL;

	c_w_old		= NULL;
	c_v_old		= NULL;
	c_p_old		= NULL;
	c_wv_old	= NULL;
	c_vp_old	= NULL;
	c_wvp_old	= NULL;
}
R_xforms::R_xforms ()
{
	unmap			();
	m_invw.identity	();

	m_w.identity	();
	m_v.identity	();
	m_p.identity	();
	m_wv.identity	();
	m_vp.identity	();
	m_wvp.identity	();

	m_w_old.identity	();
	m_v_old.identity	();
	m_p_old.identity	();
	m_wv_old.identity	();
	m_vp_old.identity	();
	m_wvp_old.identity	();

	m_bInvWValid = true;
}
