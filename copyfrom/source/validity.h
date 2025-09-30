#pragma once

#include <atomic>

struct isPluginUnlocked {
     std::atomic<float> isLicenseUnlocked{0.0f};
};

extern isPluginUnlocked GlobalLicenseState;