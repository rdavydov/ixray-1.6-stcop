#pragma once

#include "weaponcustompistol.h"
#include "../xrScripts/script_export_space.h"

class CWeaponShotgun :	public CWeaponCustomPistol
{
	typedef CWeaponCustomPistol inherited;
public:
					CWeaponShotgun		();
	virtual			~CWeaponShotgun		();

	virtual void	Load				(LPCSTR section);
	
	virtual void	net_Destroy			();
	virtual void	net_Export			(NET_Packet& P);
	virtual void	net_Import			(NET_Packet& P);

	virtual bool	TryReload			();
	virtual void	switch2_Fire		();
	void			switch2_StartReload ();
	void			switch2_AddCartgidge();
	void			switch2_EndReload	();

	virtual void	PlayAnimOpenWeapon	();
	virtual void	PlayAnimAddOneCartridgeWeapon();
	void			PlayAnimCloseWeapon	();

	virtual bool	Action(u16 cmd, u32 flags);

	virtual bool	SwitchAmmoType(u32 flags);
	bool			bStopReloadSignal;
	bool			m_bAddCartridgeOpen;
	bool			m_bEmptyPreloadMode;
	bool			bPreloadAnimAdapter;

protected:
	virtual void	OnAnimationEnd		(u32 state);
	bool			TriStateReload		();
	virtual void	OnStateSwitch		(u32 S);

	bool			HaveCartridgeInInventory(u8 cnt);
	virtual u8		AddCartridge		(u8 cnt);

	ESoundTypes		m_eSoundOpen;
	ESoundTypes		m_eSoundAddCartridge;
	ESoundTypes		m_eSoundClose;

	DECLARE_SCRIPT_REGISTER_FUNCTION
};
add_to_type_list(CWeaponShotgun)
#undef script_type_list
#define script_type_list save_type_list(CWeaponShotgun)
