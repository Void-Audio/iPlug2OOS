//------------------------------------------------------------------------
// Copyright(c) 2025 Void Audio.
//------------------------------------------------------------------------

#pragma once

#include "public.sdk/source/vst/vsteditcontroller.h"
#include "vstgui\lib\controls\icontrollistener.h"

#include "LicenseSpring/Configuration.h"
#include "LicenseSpring/EncryptString.h"
#include "LicenseSpring/LicenseManager.h"
#include "LicenseSpring/Exceptions.h"
#include "LicenseSpring/LicenseID.h"

#include "editor.h"
#include "validity.h"
namespace VOID {

//------------------------------------------------------------------------
//  RendController
//------------------------------------------------------------------------
class RendController : public Steinberg::Vst::EditControllerEx1
{
public:
//------------------------------------------------------------------------
	RendController () = default;
	~RendController () SMTG_OVERRIDE = default;

    // Create function
	static Steinberg::FUnknown* createInstance (void* /*context*/)
	{
		return (Steinberg::Vst::IEditController*)new RendController;
	}

	//--- from IPluginBase -----------------------------------------------
	Steinberg::tresult PLUGIN_API initialize (Steinberg::FUnknown* context) SMTG_OVERRIDE;
	Steinberg::tresult PLUGIN_API terminate () SMTG_OVERRIDE;

	//--- from EditController --------------------------------------------
	Steinberg::tresult PLUGIN_API setComponentState (Steinberg::IBStream* state) SMTG_OVERRIDE;
	Steinberg::IPlugView* PLUGIN_API createView (Steinberg::FIDString name) SMTG_OVERRIDE;
	Steinberg::tresult PLUGIN_API setState (Steinberg::IBStream* state) SMTG_OVERRIDE;
	Steinberg::tresult PLUGIN_API getState (Steinberg::IBStream* state) SMTG_OVERRIDE;

	// for parameter access
	Steinberg::tresult PLUGIN_API setParamNormalized(Steinberg::Vst::ParamID pID, Steinberg::Vst::ParamValue value) SMTG_OVERRIDE;
	Steinberg::Vst::ParamValue  PLUGIN_API getParamNormalized(Steinberg::Vst::ParamID pID) SMTG_OVERRIDE;

	
	void onLicenseActivated();
	void onLicenseDeactivated();

 	//---Interface---------
	DEFINE_INTERFACES
		// Here you can add more supported VST3 interfaces
		// DEF_INTERFACE (Vst::IXXX)
	END_DEFINE_INTERFACES (EditController)
    DELEGATE_REFCOUNT (EditController)

//------------------------------------------------------------------------
protected:
private:


LicenseManager::ptr_t licenseManager = nullptr;
License::ptr_t license = nullptr;
LicenseID licenseId;

float isLicenseValid      = 0.0f;
float distortionAmountMix = 0.0; 
float bypassValue         = 0.0; 
float driveAmountMix      = 0.0;
float extraParamAmountMix = 0.0;
float switchstate         = 0.0;
float forceMono           = 0.0; // handle mono switch ,theres probably an easier way to do this



};

//------------------------------------------------------------------------
} // namespace VOID
