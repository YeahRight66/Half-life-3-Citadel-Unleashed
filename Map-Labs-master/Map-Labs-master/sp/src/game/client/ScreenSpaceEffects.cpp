//========= Copyright Valve Corporation, All rights reserved. ============//
#include "cbase.h"

#include "KeyValues.h"
#include "cdll_client_int.h"
#include "view_scene.h"
#include "viewrender.h"
#include "tier0/icommandline.h"
#include "materialsystem/imesh.h"
#include "materialsystem/imaterial.h"
#include "materialsystem/imaterialsystemhardwareconfig.h"
#include "materialsystem/imaterialvar.h"

#include "c_basehlplayer.h"

#include "ScreenSpaceEffects.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//------------------------------------------------------------------------------
// CScreenSpaceEffectRegistration code
// Used to register and effect with the IScreenSpaceEffectManager
//------------------------------------------------------------------------------
CScreenSpaceEffectRegistration *CScreenSpaceEffectRegistration::s_pHead = NULL;

CScreenSpaceEffectRegistration::CScreenSpaceEffectRegistration( const char *pName, IScreenSpaceEffect *pEffect )
{
	m_pEffectName = pName;
	m_pEffect = pEffect;
	m_pNext = s_pHead;
	s_pHead = this;
}


//------------------------------------------------------------------------------
// CScreenSpaceEffectManager - Implementation of IScreenSpaceEffectManager
//------------------------------------------------------------------------------
class CScreenSpaceEffectManager : public IScreenSpaceEffectManager
{
public:

	virtual void InitScreenSpaceEffects( );
	virtual void ShutdownScreenSpaceEffects( );

	virtual IScreenSpaceEffect *GetScreenSpaceEffect( const char *pEffectName );

	virtual void SetScreenSpaceEffectParams( const char *pEffectName, KeyValues *params );
	virtual void SetScreenSpaceEffectParams( IScreenSpaceEffect *pEffect, KeyValues *params );
    
	virtual void EnableScreenSpaceEffect( const char *pEffectName );
	virtual void EnableScreenSpaceEffect( IScreenSpaceEffect *pEffect );

	virtual void DisableScreenSpaceEffect( const char *pEffectName );
	virtual void DisableScreenSpaceEffect( IScreenSpaceEffect *pEffect );

	virtual void DisableAllScreenSpaceEffects( );

	virtual void RenderEffects( int x, int y, int w, int h );
};

CScreenSpaceEffectManager g_ScreenSpaceEffectManager;
IScreenSpaceEffectManager *g_pScreenSpaceEffects = &g_ScreenSpaceEffectManager;

//---------------------------------------------------------------------------------------
// CScreenSpaceEffectManager::InitScreenSpaceEffects - Initialise all registered effects
//---------------------------------------------------------------------------------------
void CScreenSpaceEffectManager::InitScreenSpaceEffects( )
{
	if ( CommandLine()->FindParm( "-filmgrain" ) )
	{
		GetScreenSpaceEffect( "filmgrain" )->Enable( true );
	}

	for( CScreenSpaceEffectRegistration *pReg=CScreenSpaceEffectRegistration::s_pHead; pReg!=NULL; pReg=pReg->m_pNext )
	{
		IScreenSpaceEffect *pEffect = pReg->m_pEffect;
		if( pEffect )
		{
			bool bIsEnabled = pEffect->IsEnabled( );
			pEffect->Init( );
			pEffect->Enable( bIsEnabled );
		}
	}
}


//----------------------------------------------------------------------------------------
// CScreenSpaceEffectManager::ShutdownScreenSpaceEffects - Shutdown all registered effects
//----------------------------------------------------------------------------------------
void CScreenSpaceEffectManager::ShutdownScreenSpaceEffects( )
{
	for( CScreenSpaceEffectRegistration *pReg=CScreenSpaceEffectRegistration::s_pHead; pReg!=NULL; pReg=pReg->m_pNext )
	{
		IScreenSpaceEffect *pEffect = pReg->m_pEffect;
		if( pEffect )
		{
			pEffect->Shutdown( );
		}
	}
}


//---------------------------------------------------------------------------------------
// CScreenSpaceEffectManager::GetScreenSpaceEffect - Returns a point to the named effect
//---------------------------------------------------------------------------------------
IScreenSpaceEffect *CScreenSpaceEffectManager::GetScreenSpaceEffect( const char *pEffectName )
{
	for( CScreenSpaceEffectRegistration *pReg=CScreenSpaceEffectRegistration::s_pHead; pReg!=NULL; pReg=pReg->m_pNext )
	{
		if( !Q_stricmp( pReg->m_pEffectName, pEffectName ) )
		{
			IScreenSpaceEffect *pEffect = pReg->m_pEffect;
			return pEffect;
		}
	}

	Warning( "Could not find screen space effect %s\n", pEffectName );

	return NULL;
}


//---------------------------------------------------------------------------------------
// CScreenSpaceEffectManager::SetScreenSpaceEffectParams 
//	- Assign parameters to the specified effect
//---------------------------------------------------------------------------------------
void CScreenSpaceEffectManager::SetScreenSpaceEffectParams( const char *pEffectName, KeyValues *params )
{
	IScreenSpaceEffect *pEffect = GetScreenSpaceEffect( pEffectName );
	if( pEffect )
		SetScreenSpaceEffectParams( pEffect, params );
}

void CScreenSpaceEffectManager::SetScreenSpaceEffectParams( IScreenSpaceEffect *pEffect, KeyValues *params )
{
	if( pEffect )
		pEffect->SetParameters( params );
}


//---------------------------------------------------------------------------------------
// CScreenSpaceEffectManager::EnableScreenSpaceEffect
//	- Enables the specified effect
//---------------------------------------------------------------------------------------
void CScreenSpaceEffectManager::EnableScreenSpaceEffect( const char *pEffectName )
{
	IScreenSpaceEffect *pEffect = GetScreenSpaceEffect( pEffectName );
	if( pEffect )
		EnableScreenSpaceEffect( pEffect );
}

void CScreenSpaceEffectManager::EnableScreenSpaceEffect( IScreenSpaceEffect *pEffect )
{
	if( pEffect )
		pEffect->Enable( true );
}


//---------------------------------------------------------------------------------------
// CScreenSpaceEffectManager::DisableScreenSpaceEffect
//	- Disables the specified effect
//---------------------------------------------------------------------------------------
void CScreenSpaceEffectManager::DisableScreenSpaceEffect( const char *pEffectName )
{
	IScreenSpaceEffect *pEffect = GetScreenSpaceEffect( pEffectName );
	if( pEffect )
		DisableScreenSpaceEffect( pEffect );
}

void CScreenSpaceEffectManager::DisableScreenSpaceEffect( IScreenSpaceEffect *pEffect )
{
	if( pEffect )
		pEffect->Enable( false );
}


//---------------------------------------------------------------------------------------
// CScreenSpaceEffectManager::DisableAllScreenSpaceEffects
//	- Disables all registered screen space effects
//---------------------------------------------------------------------------------------
void CScreenSpaceEffectManager::DisableAllScreenSpaceEffects( )
{
	for( CScreenSpaceEffectRegistration *pReg=CScreenSpaceEffectRegistration::s_pHead; pReg!=NULL; pReg=pReg->m_pNext )
	{
		IScreenSpaceEffect *pEffect = pReg->m_pEffect;
		if( pEffect )
		{
			pEffect->Enable( false );
		}
	}
}


//---------------------------------------------------------------------------------------
// CScreenSpaceEffectManager::RenderEffects
//	- Renders all registered screen space effects
//---------------------------------------------------------------------------------------
void CScreenSpaceEffectManager::RenderEffects( int x, int y, int w, int h )
{
	for( CScreenSpaceEffectRegistration *pReg=CScreenSpaceEffectRegistration::s_pHead; pReg!=NULL; pReg=pReg->m_pNext )
	{
		IScreenSpaceEffect *pEffect = pReg->m_pEffect;
		if( pEffect )
		{
			pEffect->Render( x, y, w, h );
		}
	}
}

//------------------------------------------------------------------------------
// Example post-processing effect
//------------------------------------------------------------------------------
class CExampleEffect : public IScreenSpaceEffect
{
public:
	CExampleEffect( );
   ~CExampleEffect( );

	void Init( );
	void Shutdown( );

	void SetParameters( KeyValues *params );

	void Render( int x, int y, int w, int h );

	void Enable( bool bEnable );
	bool IsEnabled( );

private:

	bool				m_bEnable;

	CMaterialReference	m_Material;
};

ADD_SCREENSPACE_EFFECT( CExampleEffect, exampleeffect );

//------------------------------------------------------------------------------
// CExampleEffect constructor
//------------------------------------------------------------------------------
CExampleEffect::CExampleEffect( )
{
	m_bEnable = false;
}


//------------------------------------------------------------------------------
// CExampleEffect destructor
//------------------------------------------------------------------------------
CExampleEffect::~CExampleEffect( )
{
}


//------------------------------------------------------------------------------
// CExampleEffect init
//------------------------------------------------------------------------------
void CExampleEffect::Init( )
{
	// This is just example code, init your effect material here
	//m_Material.Init( "engine/exampleeffect", TEXTURE_GROUP_OTHER );

	m_bEnable = false;
}


//------------------------------------------------------------------------------
// CExampleEffect shutdown
//------------------------------------------------------------------------------
void CExampleEffect::Shutdown( )
{
	m_Material.Shutdown();
}

//------------------------------------------------------------------------------
// CExampleEffect enable
//------------------------------------------------------------------------------
void CExampleEffect::Enable( bool bEnable )
{
	// This is just example code, don't enable it
	// m_bEnable = bEnable;
}

bool CExampleEffect::IsEnabled( )
{
	return m_bEnable;
}

//------------------------------------------------------------------------------
// CExampleEffect SetParameters
//------------------------------------------------------------------------------
void CExampleEffect::SetParameters( KeyValues *params )
{
	if( params->GetDataType( "example_param" ) == KeyValues::TYPE_STRING )
	{
		// ...
	}
}

//------------------------------------------------------------------------------
// CExampleEffect render
//------------------------------------------------------------------------------
void CExampleEffect::Render( int x, int y, int w, int h )
{
	if ( !IsEnabled() )
		return;

	// Render Effect
	Rect_t actualRect;
	UpdateScreenEffectTexture( 0, x, y, w, h, false, &actualRect );
	ITexture *pTexture = GetFullFrameFrameBufferTexture( 0 );

	CMatRenderContextPtr pRenderContext( materials );

	pRenderContext->DrawScreenSpaceRectangle( m_Material, x, y, w, h,
											actualRect.x, actualRect.y, actualRect.x+actualRect.width-1, actualRect.y+actualRect.height-1, 
											pTexture->GetActualWidth(), pTexture->GetActualHeight() );
}

//------------------------------------------------------------------------------
// Health post-processing effects
//------------------------------------------------------------------------------
class CHealthEffects : public IScreenSpaceEffect
{
public:
	CHealthEffects() { };
	~CHealthEffects() { };

	void Init();
	void Shutdown();

	void SetParameters(KeyValues *params) { };

	void Render(int x, int y, int w, int h);

	void Enable(bool bEnable) { m_bEnable = bEnable; }
	bool IsEnabled() { return m_bEnable; }

private:

	bool				m_bEnable;

	float				fDispersionAmount;
	float				fDispersionLerpTo;

	int					iLastHealth;
	bool				bDisableDispersionOneFrame;

	CMaterialReference	m_ChromaticDisp;
};

ADD_SCREENSPACE_EFFECT(CHealthEffects, c17_healthfx);

ConVar r_post_chromatic_dispersion_offset("r_post_chromatic_dispersion_offset", "1.3", FCVAR_CHEAT, "Controls constant chromatic dispersion strength, 0 for off.");
ConVar r_post_chromatic_dispersion_offset_heavydamage("r_post_chromatic_dispersion_offset_heavydamage", "1.5", FCVAR_CHEAT, "Controls constant chromatic dispersion strength when the player takes heavy damage.");
ConVar r_post_chromatic_dispersion_offset_damage("r_post_chromatic_dispersion_offset_damage", "8.0", FCVAR_CHEAT, "Controls constant chromatic dispersion strength when the player takes damage.");

ConVar r_post_healtheffects_debug("r_post_healtheffects_debug", "0", FCVAR_CHEAT);

ConVar r_post_healtheffects("r_post_healtheffects", "0", FCVAR_ARCHIVE);

//------------------------------------------------------------------------------
// CHealthEffects init
//------------------------------------------------------------------------------
void CHealthEffects::Init()
{
	m_ChromaticDisp.Init(materials->FindMaterial("effects/shaders/chromaticDisp", TEXTURE_GROUP_PIXEL_SHADERS, true));

	fDispersionAmount = r_post_chromatic_dispersion_offset.GetFloat();
	fDispersionLerpTo = r_post_chromatic_dispersion_offset.GetFloat();

	iLastHealth = -1;
	bDisableDispersionOneFrame = false;
}


//------------------------------------------------------------------------------
// CHealthEffects shutdown
//------------------------------------------------------------------------------
void CHealthEffects::Shutdown()
{
	m_ChromaticDisp.Shutdown();
}

//------------------------------------------------------------------------------
// CHealthEffects render
//------------------------------------------------------------------------------
void CHealthEffects::Render(int x, int y, int w, int h)
{
	if (!r_post_healtheffects.GetBool() || (IsEnabled() == false))
		return;

	C_BaseHLPlayer *pPlayer = (C_BaseHLPlayer *)C_BasePlayer::GetLocalPlayer();
	if (!pPlayer)
		return;

	fDispersionLerpTo = r_post_chromatic_dispersion_offset.GetFloat();

	if (iLastHealth > pPlayer->GetHealth() /*&& pPlayer->GetHealth() > 15*/)
	{
		//Player took damage.
		if (iLastHealth - pPlayer->GetHealth() >= 20)
		{
			fDispersionAmount = r_post_chromatic_dispersion_offset_heavydamage.GetFloat();
		}
		else
		{
			fDispersionAmount = r_post_chromatic_dispersion_offset_damage.GetFloat();
		}
		bDisableDispersionOneFrame = true;
	}
	iLastHealth = pPlayer->GetHealth();

	if (fDispersionAmount != fDispersionLerpTo && !bDisableDispersionOneFrame)
		fDispersionAmount = FLerp(fDispersionAmount, fDispersionLerpTo, 0.1f);
	else if (bDisableDispersionOneFrame)
		bDisableDispersionOneFrame = false;

	IMaterialVar *var;

	if (fDispersionAmount >= 0.01f)
	{
		var = m_ChromaticDisp->FindVar("$FOCUSOFFSET", NULL);
		var->SetFloatValue(fDispersionAmount);
		var = m_ChromaticDisp->FindVar("$radial", NULL);
		var->SetIntValue(0);
		DrawScreenEffectMaterial(m_ChromaticDisp, x, y, w, h);
		if (r_post_healtheffects_debug.GetBool())
			DevMsg("Dispersion Amount: %.2f\n", fDispersionAmount);
	}
}

//------------------------------------------------------------------------------
// Unsharp post-processing effect
//------------------------------------------------------------------------------
class CUnsharpEffect : public IScreenSpaceEffect
{
public:
	CUnsharpEffect() { };
	~CUnsharpEffect() { };

	void Init();
	void Shutdown();

	void SetParameters(KeyValues *params) { };

	void Render(int x, int y, int w, int h);

	void Enable(bool bEnable) { m_bEnable = bEnable; }
	bool IsEnabled() { return m_bEnable; }

private:

	bool				m_bEnable;

	CTextureReference	m_UnsharpBlurFB;
	CMaterialReference	m_UnsharpBlur;
	CMaterialReference	m_Unsharp;
};

ADD_SCREENSPACE_EFFECT(CUnsharpEffect, c17_unsharp);

ConVar r_post_unsharp("r_post_unsharp", "0", FCVAR_ARCHIVE);
ConVar r_post_unsharp_debug("r_post_unsharp_debug", "0", FCVAR_CHEAT);
ConVar r_post_unsharp_strength("r_post_unsharp_strength", "0.5", FCVAR_CHEAT);
ConVar r_post_unsharp_blursize("r_post_unsharp_blursize", "5.0", FCVAR_CHEAT);

//------------------------------------------------------------------------------
// CUnsharpEffect init
//------------------------------------------------------------------------------
void CUnsharpEffect::Init()
{
	m_UnsharpBlurFB.InitRenderTarget(ScreenWidth() / 2, ScreenHeight() / 2, RT_SIZE_DEFAULT, IMAGE_FORMAT_RGBA8888, MATERIAL_RT_DEPTH_NONE, false, "_rt_UnsharpBlur");

	m_UnsharpBlur.Init(materials->FindMaterial("effects/shaders/unsharp_blur", TEXTURE_GROUP_PIXEL_SHADERS, true));
	m_Unsharp.Init(materials->FindMaterial("effects/shaders/unsharp", TEXTURE_GROUP_PIXEL_SHADERS, true));
}


//------------------------------------------------------------------------------
// CUnsharpEffect shutdown
//------------------------------------------------------------------------------
void CUnsharpEffect::Shutdown()
{
	m_UnsharpBlurFB.Shutdown();
	m_UnsharpBlur.Shutdown();
	m_Unsharp.Shutdown();
}

//------------------------------------------------------------------------------
// CUnsharpEffect render
//------------------------------------------------------------------------------
void CUnsharpEffect::Render(int x, int y, int w, int h)
{
	if (!r_post_unsharp.GetBool() || (IsEnabled() == false))
		return;

	// Grab the render context
	CMatRenderContextPtr pRenderContext(materials);

	// Set to the proper rendering mode.
	pRenderContext->MatrixMode(MATERIAL_VIEW);
	pRenderContext->PushMatrix();
	pRenderContext->LoadIdentity();
	pRenderContext->MatrixMode(MATERIAL_PROJECTION);
	pRenderContext->PushMatrix();
	pRenderContext->LoadIdentity();

	IMaterialVar *var;
	var = m_UnsharpBlur->FindVar("$blursize", NULL);
	var->SetFloatValue(r_post_unsharp_blursize.GetFloat());

	if (r_post_unsharp_debug.GetBool())
	{
		DrawScreenEffectMaterial(m_UnsharpBlur, x, y, w, h);
		return;
	}

	Rect_t actualRect;
	UpdateScreenEffectTexture(0, x, y, w, h, false, &actualRect);
	pRenderContext->PushRenderTargetAndViewport(m_UnsharpBlurFB);
	DrawScreenEffectQuad(m_UnsharpBlur, m_UnsharpBlurFB->GetActualWidth(), m_UnsharpBlurFB->GetActualHeight());
	pRenderContext->PopRenderTargetAndViewport();

	//Restore our state
	pRenderContext->MatrixMode(MATERIAL_VIEW);
	pRenderContext->PopMatrix();
	pRenderContext->MatrixMode(MATERIAL_PROJECTION);
	pRenderContext->PopMatrix();

	var = m_Unsharp->FindVar("$fbblurtexture", NULL);
	var->SetTextureValue(m_UnsharpBlurFB);
	var = m_Unsharp->FindVar("$unsharpstrength", NULL);
	var->SetFloatValue(r_post_unsharp_strength.GetFloat());
	var = m_Unsharp->FindVar("$blursize", NULL);
	var->SetFloatValue(r_post_unsharp_blursize.GetFloat());

	DrawScreenEffectMaterial(m_Unsharp, x, y, w, h);
}
