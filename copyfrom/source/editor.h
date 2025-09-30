//------------------------------------------------------------------------
// Copyright(c) 2025 Void Audio.
//------------------------------------------------------------------------

#pragma once



#include "vstgui\plugin-bindings\vst3editor.h"
#include "editordelegate.h"

#include <memory>



namespace VOID {

class RendEditor : public VSTGUI::VST3Editor
{
public:
    RendEditor(Steinberg::Vst::EditController* controller)
    : VST3Editor(controller, "view", "editor.uidesc"),
      delegate_(std::make_unique<RendEditorDelegate>())
    {
        auto* delegate = new RendEditorDelegate();
        setDelegate(delegate); 
        setZoomFactor(0.6);
    }

    VSTGUI::IVST3EditorDelegate* RendEditor::getDelegate() const
{
    return delegate_.get();
}



protected:
    VSTGUI::VST3Editor* editor = nullptr;


private:
    std::unique_ptr<RendEditorDelegate> delegate_;
};



}