//------------------------------------------------------------------------
// Copyright(c) 2025 Void Audio.
//------------------------------------------------------------------------
#pragma once

#include "validity.h"

#include "LicenseSpring/Configuration.h"
#include "LicenseSpring/EncryptString.h"
#include "LicenseSpring/LicenseManager.h"
#include "LicenseSpring/Exceptions.h"
#include "LicenseSpring/LicenseID.h"
#include "LicenseSpring/ExtendedOptions.h"

#include <iostream>
#include <thread>
#include <functional>
#include <memory>
#include <string>



using namespace VSTGUI;
using namespace LicenseSpring;

class LicenseOverlayView : public CViewContainer, public IControlListener 
{
public:
    using ActivationCallback = std::function<void()>;
    float isLicenseValid = 0.0f;

    LicenseOverlayView(const CRect& size)
        : CViewContainer(size)
    {
        setTransparency(false);
        setupLicenseSpring();
    }

    void setOnActivatedCallback(ActivationCallback callback)
    {
        onActivated = std::move(callback);
    }

    void valueChanged(CControl* pControl) override
    {
        if (pControl == submitButton)
        {
            std::string key = licenseInput->getText().data();
            activateWithKey(key);
        }
    }

    void setupUI()
    {
        const float padding = 20.f;
        const float width = getViewSize().getWidth();
        const float height = getViewSize().getHeight();

        const float inputHeight = 30.f;   
        const float buttonHeight = 35.f;
        const float labelHeight = 30.f;
        const float spacing = 15.f;

        const float totalHeight = labelHeight + spacing + inputHeight + spacing + buttonHeight;
        const float startY = (height - totalHeight) / 2;
        const float elementWidth = width - 2 * padding;


        CRect labelRect(padding, startY, padding + elementWidth, startY + labelHeight);
        CRect inputRect(padding, labelRect.bottom + spacing, padding + elementWidth, labelRect.bottom + spacing + inputHeight);
        CRect buttonRect(padding, inputRect.bottom + spacing, padding + elementWidth, inputRect.bottom + spacing + buttonHeight);

        messageLabel = new CTextLabel(labelRect, "Unshackle:");
        licenseInput = new CTextEdit(inputRect, this, 123456, "Enter License Key");
        submitButton = new CTextButton(buttonRect, this, 234567, "Activate");

        licenseInput->setBackColor(kWhiteCColor);
        licenseInput->setFontColor(kBlackCColor);

        submitButton->setTextAlignment(kCenterText);

        submitButton->setTextColor(kRedCColor);
        submitButton->setFrameColor(kRedCColor);

        addView(messageLabel);
        addView(licenseInput);
        addView(submitButton);
    }


    void setupLicenseSpring()
    {
        if ( GlobalLicenseState.isLicenseUnlocked.load() <= 0.5)
        {
            setupUI();
            updateMessage("Enter Psychodelix tester key:");
            onLicenseActivated();
        }
    }
    void notifyLicenseStateChanged(float newValue) {
        isLicenseValid = newValue;
        GlobalLicenseState.isLicenseUnlocked.store(newValue);

    }

    void activateWithKey(const std::string& key)
    {
        try {

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

            licenseId = LicenseID::fromKey(key);

            license = licenseManager->activateLicense(licenseId);

            if (license && license->isActive()) {
            updateMessage("Unshackling...");
                notifyLicenseStateChanged(1.0f);
                proceedAfterActivation();
            } else {
            updateMessage("Enter Psychodelix tester key:");
                notifyLicenseStateChanged(0.0f);
            }
        }
        catch (LicenseNotFoundException&) {
            updateMessage("LicenseNotFoundException");
            notifyLicenseStateChanged(0.0f);
        }
        catch (LicenseNoAvailableActivationsException&) {
            updateMessage("LicenseNoAvailableActivationsException");
            notifyLicenseStateChanged(0.0f);
        }
        catch (LicenseStateException&) {
            updateMessage("LicenseStateException");
            notifyLicenseStateChanged(0.0f);
        }
        catch (...) {
            updateMessage("FUCK ME, NO BUG INFO");
            notifyLicenseStateChanged(0.0f);
        }
    }

    void updateMessage(const std::string& msg)
    {
        if (messageLabel)
            messageLabel->setText(msg.c_str());
    }
    
    void proceedAfterActivation()
    {
        if (auto parent = dynamic_cast<CViewContainer*>(getParentView()))
            parent->removeView(this); 
    }

private:

    void onLicenseActivated() {
        GlobalLicenseState.isLicenseUnlocked.store(1.0f);
    }

    void onLicenseDeactivated() {
        GlobalLicenseState.isLicenseUnlocked.store(0.0f);
    }


    CTextEdit* licenseInput = nullptr;
    CTextButton* submitButton = nullptr;
    CTextLabel* messageLabel = nullptr;

    
    ActivationCallback onActivated;

    LicenseManager::ptr_t licenseManager = nullptr;
    License::ptr_t license = nullptr;

    ExtendedOptions options;
    LicenseID licenseId;
};
