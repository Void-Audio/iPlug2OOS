//------------------------------------------------------------------------
// Copyright(c) 2025 Void Audio.
//------------------------------------------------------------------------

#include "processor.h"
#include "cids.h"



#include "base/source/fstreamer.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"




using namespace Steinberg;

namespace VOID {
using namespace Dsp;
//------------------------------------------------------------------------
// RendProcessor
//------------------------------------------------------------------------
RendProcessor::RendProcessor ()
{
	//--- set the wanted controller for our processor
	setControllerClass (kRendControllerUID);
}

//------------------------------------------------------------------------
RendProcessor::~RendProcessor ()
{}

//------------------------------------------------------------------------
tresult PLUGIN_API RendProcessor::initialize (FUnknown* context)
{
/*
 * The host initiates this then resets with setBusArrangements() if necessary
 */
    tresult result = AudioEffect::initialize (context);
    if (result != kResultOk)
        return result;

    // main stereo signal
    addAudioInput  (STR16("Stereo In"),  Steinberg::Vst::SpeakerArr::kStereo);
    addAudioOutput (STR16("Stereo Out"), Steinberg::Vst::SpeakerArr::kStereo);

    return kResultOk;
}

tresult PLUGIN_API RendProcessor::setBusArrangements (Steinberg::Vst::SpeakerArrangement* inputs, Steinberg::int32 numIns,
                                                            Steinberg::Vst::SpeakerArrangement* outputs, Steinberg::int32 numOuts)
/*
 * The host automatically calls setBusArrangement() from the base class, so we can override it
 * to handle the exact cases we need
 */
{

	// initialize filters Forst

	// parameters for filters
	int32 sampleRate = processSetup.sampleRate;
	
	int32 numChannels = 0;
    if (numIns > 0)
        numChannels = Vst::SpeakerArr::getChannelCount(inputs[0]);

	


	lowShelfFilter.setup (sampleRate,// sample rate
						250, // cutoff frequency
						5,  // gain
						1);   // shelf slope


	highShelfFilter.setup (sampleRate,// sample rate
						3500, // cutoff frequency
						10,  // gain
						0.5);  // shelf slope

	bandStopFilter.setup (sampleRate,// sample rate
						120, // center frequency
						30);   // bandwitch frequency


	compressor.emplace(sampleRate, 18.0f);
	makeupGain = std::pow(10.0f, compressor.value().ratio / 20.0f);



    // Confirm proper format
    if (numIns == 2 && numOuts == 1) {
        int32 chin1  = Steinberg::Vst::SpeakerArr::getChannelCount (inputs[0]);
        int32 chout1 = Steinberg::Vst::SpeakerArr::getChannelCount (outputs[0]);

        // handle case mono in/out/sidechain
        if (chin1 == 1 && chout1 == 1 ) {
            // initiate the correct buffers
            removeAudioBusses();
            addAudioInput(STR16("Mono In"),  inputs[0]);
            addAudioOutput(STR16("Mono Out"), outputs[0]);
            addAudioInput(STR16("Mono Sidechain"), inputs[1], Steinberg::Vst::kAux, 0);
            return kResultOk;
        }
        // handle case stereo in/out mono sidechain
        if (chin1 == 2 && chout1 == 2) {
            // initiate the correct buffers
            removeAudioBusses();
            addAudioInput(STR16("Mono In"),  inputs[0]);
            addAudioOutput(STR16("Mono Out"), outputs[0]);
            return kResultOk;
        }
        // handle case stereo in/out/sidechain
        if(chin1 == 2 && chout1 == 2 ) {
            // initiate the correct buffers
            removeAudioBusses();
            addAudioInput(STR16("Mono In"),  inputs[0]);
            addAudioOutput(STR16("Mono Out"), outputs[0]);
            return kResultOk;
        }
        // handle case mono in/out stereo sidechain (who tf would ever do this? idk but we gotta make it stupid proof)
        if(chin1 == 1 && chout1 == 1) {
            // initiate the correct buffers
            removeAudioBusses();
            addAudioInput(STR16("Mono In"),  inputs[0]);
            addAudioOutput(STR16("Mono Out"), outputs[0]);
            return kResultOk;
        }
    }
    return kResultFalse;
}


//------------------------------------------------------------------------
tresult PLUGIN_API RendProcessor::terminate ()
{
	// Here the Plug-in will be de-instantiated, last possibility to remove some memory!
	
	//---do not forget to call parent ------
	return AudioEffect::terminate ();
}

//------------------------------------------------------------------------
tresult PLUGIN_API RendProcessor::setActive (TBool state)
{
	//--- called when the Plug-in is enable/disable (On/Off) -----
	return AudioEffect::setActive (state);
}

//------------------------------------------------------------------------
tresult PLUGIN_API RendProcessor::process(Vst::ProcessData& data) 
{
	
	isLicenseValid = GlobalLicenseState.isLicenseUnlocked.load();    // secure to check at runtime

	// Handle parameter changes
	if (data.inputParameterChanges)
	{
		int32 numParamsChanged = data.inputParameterChanges->getParameterCount();
		for (int32 index = 0; index < numParamsChanged; ++index)
		{
			if (auto* paramQueue = data.inputParameterChanges->getParameterData(index))
			{
				Vst::ParamID id = paramQueue->getParameterId();
				int32 numPoints = paramQueue->getPointCount();

				if (numPoints > 0)
				{
					Vst::ParamValue value;
					int32 sampleOffset;
					if (paramQueue->getPoint(numPoints - 1, sampleOffset, value) == kResultTrue)
					{
						if        (id == 10) {
							distortionAmountMix = value;
						} else if (id == 11) {
							driveAmountMix      = value;
						} else if (id == 12) {
							extraParamAmountMix = value;
						} else if (id == 13) {
							switchstate         = value;
						} else if (id == 0) {              // PARAMSSSSSSSSSSSSS for each
							bypassValue         = value;
						} else if (id == 1) {              // PARAMSSSSSSSSSSSSS for each
							forceMono           = value;
						}
					}
				}
			}
		}
	}

mix      = static_cast<float>(distortionAmountMix);
drive    = static_cast<float>(driveAmountMix);
extra    = static_cast<float>(extraParamAmountMix);
bypass   = static_cast<float>(bypassValue);
if (data.numSamples > 0)
    {			
        int32 minBus = std::min (data.numInputs, data.numOutputs);
        for (int32 i = 0; i < minBus; i++)
        {
			int32 numSamples = data.numSamples;
			int32 minChan = std::min (data.inputs[i].numChannels, data.outputs[i].numChannels);

			filteredPtrs.resize(minChan);
			filteredSignal.resize(minChan);
			for (int ch = 0; ch < minChan; ++ch) {
				filteredSignal[ch].resize(numSamples);  // allocate space for samples here
			}
			//std::vector<std::vector<float>> filteredSignal(minChan, std::vector<float>(numSamples));

			// multichannel filter processing
			//std::vector<float*> filteredPtrs(minChan);
			// applyFilterFilter(filteredPtrs, filteredSignal, data.inputs[i].channelBuffers32, numSamples, minChan, lowShelfFilter);



			// for (int ch = 0; ch < minChan; ++ch) {
			// 	memcpy(filteredSignal[ch].data(), data.inputs[i].channelBuffers32[ch], numSamples * sizeof(float));
			// }
			
			// for (int ch = 0; ch < minChan; ++ch) {
			// 	filteredPtrs[ch] = filteredSignal[ch].data();
			// }
			// float** filteredSignalPtr = filteredPtrs.data();

			// lowShelfFilter.process(numSamples, filteredSignalPtr);

			RendProcessor::applyFilter(filteredPtrs, filteredSignal, data.inputs[i].channelBuffers32, numSamples, minChan, highShelfFilter);
			bandStopFilter.process(numSamples, filteredPtrs.data());
			lowShelfFilter.process(numSamples, filteredPtrs.data());



            
            for (int32 c = 0; c < minChan; c++)
            {
                float* in  = data.inputs[i].channelBuffers32[c];
				float* filtered = filteredPtrs[c];
                float* out = data.outputs[i].channelBuffers32[c];

				
                for (int32 s = 0; s < data.numSamples; s++)
                { 

					if (isLicenseValid <= 0.5) {out[s] = in[s];} else 
					{
						if (bypass >= 0.5f) { out[s] = in[s]; } 
						else {
						/*
						*	-------- Main Process Logic ----------------
						*/

						float signal = (1 - extra) * in[s] + extra * filtered[s]; // add dry and filtered signal together
						
						if (switchstate <= 0.5) {
						/* normal mode */

							float normSignal = (1.0f) * (signal * (1.0f - mix) + mix *  ( std::sqrt(mix)* std::tanh( signal * (5.0f + drive * 8.0f )) +
													(1.0f-std::sqrt(mix)) * (2.0f * RendProcessor::inv_pi)  * std::atan((5.0f + drive * 8.0f ))) ); //  ( sqrt.mix * tanh + (1-sqrt.mix) * atan)
							
							float uncompressed = normSignal - extra * RendProcessor::clip(0.5f * normSignal * normSignal * normSignal,std::abs(1-normSignal) );
							if (compressor.has_value()) {
								out[s] = (1-extra * 0.5f) * uncompressed + 0.5f * extra * makeupGain *  compressor->process(uncompressed);
							} else {
								out[s] = uncompressed; 
							}
							
							//out[s]=uncompressed;
						
						} else if (switchstate > 0.5) {
						/* BROKEN mode */

							float tastefulInclusion =  (1.0f) * (signal * (1.0f - mix) + mix *  ( std::sqrt(mix)* std::tanh( signal * (5.0f + drive * 8.0f )) +
													(1.0f-std::sqrt(mix)) * (2.0f * RendProcessor::inv_pi)  * std::atan((5.0f + drive * 8.0f ))) ); //  ( sqrt.mix * tanh + (1-sqrt.mix) * atan)

							float normSignal =    RendProcessor::clip( ((1 + 2.0f * mix  + 10 * drive) * signal * 0.85f + tastefulInclusion * 0.15f)+ extra*noiseAmount * ((rand() / (float)RAND_MAX) * 2.0f - 1.0f), 1 - 0.99 * drive);
							float uncompressed = RendProcessor::clip(normSignal - extra * (0.5f * normSignal * normSignal * normSignal  + 0.1f * filtered[s]),  1.0f - 0.99f * drive );

							if (compressor.has_value()) {
								out[s] = (1.0f - 0.3f * extra) * uncompressed + 0.7f * extra * makeupGain * compressor->process(uncompressed);
							} else {
								out[s] = uncompressed; 
							}
							//out[s]=uncompressed;
							
						}
						}
						
					}	

                }
            } // end of channel loop

			
			    // Apply mono logic here if forceMono is true
			if (forceMono && data.numOutputs > 0) {
				Vst::AudioBusBuffers& output = data.outputs[0];
				int32 numChannels = output.numChannels;
				int32 numSamples = data.numSamples;

				if (numChannels >= 2) {
					float* left = output.channelBuffers32[0];
					float* right = output.channelBuffers32[1];

					for (int32 i = 0; i < numSamples; ++i) {
						float mono = 0.5f * (left[i] + right[i]);
						left[i] = right[i] = mono;
					}
				}
			}

            data.outputs[i].silenceFlags = data.inputs[i].silenceFlags;

            // clear the remaining output channels
            for (int32 c = minChan; c < data.outputs[i].numChannels; c++)
            {
                memset(data.outputs[i].channelBuffers32[c], 0, data.numSamples * sizeof(Vst::Sample32));
                data.outputs[i].silenceFlags |= ((uint64)1 << c);
            }
        }
        // clear the remaining output buses
        for (int32 i = minBus; i < data.numOutputs; i++)
        {
            for (int32 c = 0; c < data.outputs[i].numChannels; c++)
            {
                memset(data.outputs[i].channelBuffers32[c], 0, data.numSamples * sizeof(Vst::Sample32));
            }
            data.outputs[i].silenceFlags = ((uint64)1 << data.outputs[i].numChannels) - 1;
        }
    }

    return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API RendProcessor::setupProcessing (Vst::ProcessSetup& newSetup)
{
	//--- called before any processing ----
	return AudioEffect::setupProcessing (newSetup);
}

//------------------------------------------------------------------------
tresult PLUGIN_API RendProcessor::canProcessSampleSize (int32 symbolicSampleSize)
{
	// by default kSample32 is supported
	if (symbolicSampleSize == Vst::kSample32)
		return kResultTrue;

	// disable the following comment if your processing support kSample64
	/* if (symbolicSampleSize == Vst::kSample64)
		return kResultTrue; */

	return kResultFalse;
}

//------------------------------------------------------------------------
tresult PLUGIN_API RendProcessor::setState(IBStream* state)
{
    IBStreamer streamer(state, kLittleEndian);
    double val = 0.0;

    if (!streamer.readDouble(val))
        return kResultFalse;
    distortionAmountMix = val;

	if (!streamer.readDouble(val))
	return kResultFalse;
    driveAmountMix = val;

    if (!streamer.readDouble(val))
        return kResultFalse;
    extraParamAmountMix = val;

	if (!streamer.readDouble(val))
    return kResultFalse;
    switchstate = val;


	// Bypass

    if (!streamer.readDouble(val))    //  ^^^
        return kResultFalse;          //  add these blocks for each param
    bypassValue = val;                //

	// mono override
	if (!streamer.readDouble(val))
    return kResultFalse;
    forceMono = val;


    return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API RendProcessor::getState(IBStream* state)
{
    IBStreamer streamer(state, kLittleEndian);    // kLittleEndian is for OS agnostic functionality
    streamer.writeDouble(distortionAmountMix);
	streamer.writeDouble(driveAmountMix);
	streamer.writeDouble(extraParamAmountMix);
	streamer.writeDouble(switchstate);
    streamer.writeDouble(bypassValue);    // just add lines for param
	streamer.writeDouble(forceMono); 
	
    return kResultOk;
}

//------------------------------------------------------------------------
/*
 * Functions to be used in processing for effects
 */

// Hard Clip
float RendProcessor::clip(float signal, float threshhold) {
	return std::max(-threshhold, std::min(threshhold,signal)); 
}
// bitcrush 
// float RendProcessor::bitcrush(float signal, float mix) {
// 	// mixes in bitcrush 0-1[00%]
// 	int resolution = std::round(2048 * (1-mix * 0.998f)) ;
// 	// Quantize the signal
// 	return std::round(signal / resolution) * resolution;
// }
// --------------- FILTERS -----------------


} // namespace VOID


