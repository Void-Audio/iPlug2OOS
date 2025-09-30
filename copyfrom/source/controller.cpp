//------------------------------------------------------------------------
// Copyright(c) 2025 Void Audio.
//------------------------------------------------------------------------

#include "controller.h"
#include "cids.h"

#include "vstgui/plugin-bindings/vst3editor.h"
#include "base/source/fstreamer.h"
#include "public.sdk/source/vst/vstaudioeffect.h"

using namespace Steinberg;


namespace VOID {

//------------------------------------------------------------------------
// RendController Implementation
//------------------------------------------------------------------------
tresult PLUGIN_API RendController::initialize (FUnknown* context)
{
	// Here the Plug-in will be instantiated

	//---do not forget to call parent ------
	tresult result = EditControllerEx1::initialize (context);
	if (result != kResultOk)
	{
		return result;
	}

		// License Check ------------------------- !

	isLicenseValid = GlobalLicenseState.isLicenseUnlocked.load();

	if (isLicenseValid <= 0.5f) {

		std::string appName = "Rend";
		std::string appVersion = "1.0.0.0";

		ExtendedOptions options;
		options.collectNetworkInfo(true);

		auto config = Configuration::Create(
			EncryptStr("a2337e2f-a073-43c1-9605-7bd364a1277c"),
			EncryptStr("mOdY5mU0PKsRm-MZd1aNLHd5IrVKJhUz2inFN0y-6R4"),
			EncryptStr("42069"),
			appName, appVersion, options
		);

		licenseManager = LicenseManager::create(config);


		
		try {
			
			license = licenseManager->getCurrentLicense();

			if (license && license->isActive() && !license->isTrial()) {

				isLicenseValid = 1.0f; 
				onLicenseActivated();
			}																	// forced resets
			else if (license && !license->isActive() && !license->isTrial()) {

				isLicenseValid = 0.0f;
				onLicenseDeactivated(); 
			} else if (license && license->isTrial()){
			// 	/*
			// 	 *   TO DO:  impliment trial logic
			// 	 */
			}
		} catch (...) {
			// No valid license found — fall through to show UI
		}

	}
	// PARAMETERS ---------------------------- !

	    /*
	 *
	 * Register all the parameters we plan on using
	 *
	 */

	parameters.addParameter(
	STR16("distortionAmount"),         // parameter title/name
	nullptr,                 // units (nullptr if none)
	0,                       // no step count (continuous)
	0.0,                     // default normalized value (0.5)
	Vst::ParameterInfo::kCanAutomate,  // flags (can automate)
	10                // a unique parameter ID (uint32)
    );

	parameters.addParameter(
	STR16("driveAmount"),         // parameter title/name
	nullptr,                 // units (nullptr if none)
	0,                       // no step count (continuous)
	0.0,                     // default normalized value (0.5)
	Vst::ParameterInfo::kCanAutomate,  // flags (can automate)
	11                // a unique parameter ID (uint32)
    );

	parameters.addParameter(
	STR16("extraParam"),         // parameter title/name
	nullptr,                 // units (nullptr if none)
	0,                       // no step count (continuous)
	0.0,                     // default normalized value (0.5)
	Vst::ParameterInfo::kCanAutomate,  // flags (can automate)
	12                // a unique parameter ID (uint32)
    );

	parameters.addParameter(
	STR16("switch"),         // parameter title/name
	nullptr,                 // units (nullptr if none)
	2,                       // no step count (continuous)
	0.0,                     // default normalized value (0.5)
	Vst::ParameterInfo::kCanAutomate,  // flags (can automate)
	13                // a unique parameter ID (uint32)
    );






	// ----------- Bypass ---------------------------------
	parameters.addParameter(
    STR16("Bypass"),                 // parameter title/name
    nullptr,                         // units (nullptr if none)
    2,                               // step count: 2 (Off/On)
    0.0,                             // default normalized value (0 = Off)
    Vst::ParameterInfo::kCanAutomate,// flags (can automate)
    0                                // unique parameter ID (uint32)
	);

	// Mono Switch
	parameters.addParameter(
	STR16("mono"),         // parameter title/name
	nullptr,                 // units (nullptr if none)
	2,                       // no step count (continuous)
	1.0,                     // default normalized value (0.5)
	Vst::ParameterInfo::kCanAutomate,  // flags (can automate)
	1                // a unique parameter ID (uint32)
    );

	

	return result;
}

//------------------------------------------------------------------------
tresult PLUGIN_API RendController::terminate ()
{
	// Here the Plug-in will be de-instantiated, last possibility to remove some memory!

	//---do not forget to call parent ------
	return EditControllerEx1::terminate ();
}

//------------------------------------------------------------------------
tresult PLUGIN_API RendController::setComponentState (IBStream* state)
{
	// Here you get the state of the component (Processor part)
	if (!state)
		return kResultFalse;

	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API RendController::setState (IBStream* state)
{
	// Here you get the state of the controller
    Steinberg::IBStreamer s(state, kLittleEndian);
    double val = 0.0;
    if (!s.readDouble(val))
        return kResultFalse;
    distortionAmountMix = val;       // keep adding for params

	if (!s.readDouble(val))
        return kResultFalse;
    driveAmountMix = val; 

	if (!s.readDouble(val))
        return kResultFalse;
    extraParamAmountMix = val; 

	if (!s.readDouble(val))
    return kResultFalse;
    switchstate = val; 



	// bypass
    if (!s.readDouble(val))
        return kResultFalse;
    bypassValue = val;
	
	if (!s.readDouble(val))
        return kResultFalse;
    forceMono = val;

    return kResultOk;
        
}

//------------------------------------------------------------------------
tresult PLUGIN_API RendController::getState (IBStream* state)
{
	// Here you are asked to deliver the state of the controller (if needed)
	// Note: the real state of your plug-in is saved in the processor
	Steinberg::IBStreamer s(state, kLittleEndian);
    s.writeDouble(distortionAmountMix); 
	s.writeDouble(driveAmountMix);
	s.writeDouble(extraParamAmountMix);     // keep adding for params
	s.writeDouble(switchstate); 

	// bypass
	s.writeDouble(bypassValue); 
	s.writeDouble(forceMono);
	return kResultOk;
}

//------------------------------------------------------------------------
IPlugView* PLUGIN_API RendController::createView(FIDString name)
{
    if (FIDStringsEqual(name, Vst::ViewType::kEditor))
    {
       // "view" is the template name in your .uidesc
       // "editor.uidesc" should be your actual UI description file name

		
        auto* view = new RendEditor(this);
		

        return view;
    }
    return nullptr;
}

//------------------------------------------------------------------------

tresult PLUGIN_API RendController::setParamNormalized(Steinberg::Vst::ParamID pID, Steinberg::Vst::ParamValue value)
	{
	if (pID == 10) // Distortion Amount param
	{
		// clamp, check change, assign, notify
		if (value < 0.0) value = 0.0;
		else if (value > 1.0) value = 1.0;

		if (distortionAmountMix != value)
		{
			distortionAmountMix = value;
			performEdit(pID, distortionAmountMix);
		}
	}

	if (pID == 11) // Drive Amount param
	{
		if (value < 0.0) value = 0.0;
		else if (value > 1.0) value = 1.0;

		if (driveAmountMix != value)
		{
			driveAmountMix = value;
			performEdit(pID, driveAmountMix);
		}
	}

	if (pID == 12) // Extra Param Amount param
	{
		if (value < 0.0) value = 0.0;
		else if (value > 1.0) value = 1.0;

		if (extraParamAmountMix != value)
		{
			extraParamAmountMix = value;
			performEdit(pID, extraParamAmountMix);
		}
	}

	if (pID == 13) // Extra Param Amount param
	{
		if (value < 0.0) value = 0.0;
		else if (value > 1.0) value = 1.0;

		if (switchstate != value)
		{
			switchstate = value;
			performEdit(pID, switchstate);
		}
	}

	if (pID == 0) // Bypass param
	{
		if (value < 0.0) value = 0.0;
		else if (value > 1.0) value = 1.0;

		if (bypassValue != value)
		{
			bypassValue = value;
			performEdit(pID, bypassValue);     // For any other parameters, pass it up the chain ADD MORE IF NEEDED 
		}
	}

		if (pID == 1) // Mono param
	{
		if (value < 0.0) value = 0.0;
		else if (value > 1.0) value = 1.0;

		if (forceMono != value)
		{
			forceMono = value;
			performEdit(pID, bypassValue);     // For any other parameters, pass it up the chain ADD MORE IF NEEDED 
		}
	}
	else
	{
		
		return EditControllerEx1::setParamNormalized(pID, value);
	}

    return kResultOk;
}

Steinberg::Vst::ParamValue PLUGIN_API RendController::getParamNormalized(Steinberg::Vst::ParamID pID)
{
    switch (pID)
    {
        case 10:
            return distortionAmountMix;
		case 11:
            return driveAmountMix;
		case 12:
            return extraParamAmountMix;  // add more params here
		case 13:
            return switchstate;

		// bypass	
		case 0:
            return bypassValue;
		
		case 1:
            return forceMono;
        default:
            return EditControllerEx1::getParamNormalized(pID);
    }
}

void RendController::onLicenseActivated() {
	GlobalLicenseState.isLicenseUnlocked.store(1.0f);
}

void RendController::onLicenseDeactivated() {
	GlobalLicenseState.isLicenseUnlocked.store(0.0f);
}

} // namespace VOID
