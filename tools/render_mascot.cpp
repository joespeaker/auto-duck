// Dev-only utility: renders the DuckMascot component (and the full plugin
// editor) to PNG files so the UI can be reviewed headlessly — run under xvfb
// on Linux. Not part of the plugin.

#include <juce_gui_basics/juce_gui_basics.h>

#include "../Source/PluginEditor.h"
#include "../Source/PluginProcessor.h"
#include "../Source/ui/DuckMascot.h"

static void savePng (const juce::Image& image, const char* name)
{
    juce::File out (juce::File::getCurrentWorkingDirectory().getChildFile (name));
    juce::FileOutputStream stream (out);
    if (stream.openedOk())
    {
        stream.setPosition (0);
        stream.truncate();
        juce::PNGImageFormat().writeImageToStream (image, stream);
    }
}

static void renderMascotPoses()
{
    struct Shot { const char* name; float pose; bool triggered; bool invert; };
    const Shot shots[] = {
        { "duck_idle.png",   0.0f, false, false },
        { "duck_half.png",   0.5f, true,  false },
        { "duck_full.png",   1.0f, true,  false },
        { "duck_invert.png", 0.0f, true,  true  },
    };

    for (const auto& shot : shots)
    {
        autoduck::DuckMascot mascot;
        mascot.setSize (300, 260);
        // Feed the smoothed state repeatedly so it converges on the target.
        for (int i = 0; i < 60; ++i)
            mascot.setState (shot.pose, shot.triggered, shot.invert);

        juce::Image image (juce::Image::ARGB, 300, 260, true);
        juce::Graphics g (image);
        g.fillAll (juce::Colour (0xff1f2226));
        mascot.paintEntireComponent (g, true);
        savePng (image, shot.name);
    }
}

static void renderEditor()
{
    AutoDuckAudioProcessor processor;
    processor.prepareToPlay (48000.0, 512);

    // Drive some loud sidechain signal through so the gate triggers and the
    // duck poses for the screenshot.
    juce::AudioBuffer<float> buffer (4, 512);
    for (int block = 0; block < 40; ++block)
    {
        buffer.clear();
        for (int ch = 0; ch < 4; ++ch)
            for (int i = 0; i < 512; ++i)
                buffer.setSample (ch, i, 0.5f * std::sin (0.05f * static_cast<float> (i)));
        juce::MidiBuffer midi;
        processor.processBlock (buffer, midi);
    }

    std::unique_ptr<juce::AudioProcessorEditor> editor (processor.createEditor());
    juce::MessageManager::getInstance()->runDispatchLoopUntil (700);

    juce::Image image (juce::Image::ARGB, editor->getWidth(), editor->getHeight(), true);
    juce::Graphics g (image);
    editor->paintEntireComponent (g, false);
    savePng (image, "editor.png");
}

int main()
{
    juce::ScopedJuceInitialiser_GUI juceInit;
    renderMascotPoses();
    renderEditor();
    return 0;
}
