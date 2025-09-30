//------------------------------------------------------------------------
// Copyright(c) 2025 Void Audio.
//------------------------------------------------------------------------

#pragma once

#include "pluginterfaces/base/funknown.h"
#include "pluginterfaces/vst/vsttypes.h"

namespace VOID {
//------------------------------------------------------------------------
static const Steinberg::FUID kRendProcessorUID (0x8E9537ED, 0xF9915C74, 0xB33300A7, 0x5FBCE049);
static const Steinberg::FUID kRendControllerUID (0x25DF0B40, 0xBC1B506A, 0xBC713186, 0x83DB34B1);

#define RendVST3Category "Fx"

//------------------------------------------------------------------------
} // namespace VOID
