/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "defines.h"
#include "KIJIMIInterface.h"
#include "TimbreSpaceEngine.h"


//==============================================================================
/**
*/
class BabuFrikAudioProcessor  : public AudioProcessor,
                                private AudioProcessorValueTreeState::Listener,
                                public ActionBroadcaster,
                                public ActionListener,
                                public MidiInputCallback,
                                public Timer
{
public:
    //==============================================================================
    BabuFrikAudioProcessor();
    ~BabuFrikAudioProcessor();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (AudioBuffer<float>&, MidiBuffer&) override;

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

    //==============================================================================
    void sendControlToSynth (const String& parameterID, int value);
    void parameterChanged (const String& parameterID, float newValue) override;
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    void setDefaultState ();
    void setStateFromXml (XmlElement* xmlState);
    bool isLoadingFromState = false;
    bool needsToLoadDefaultState = true;
    
    void sendLCDRefreshMessageToKijimi ();
    
    // Parameters tree
    AudioProcessorValueTreeState parameters;
    
    // MIDI input/output
    bool midiDevicesAutoScanEnabled = false;
    void setMidiDevicesAutoScan (bool enabled);
    void triggerMidiDevicesScan ();
    std::unique_ptr<MidiOutput> midiOutput;
    std::unique_ptr<MidiInput> midiInput;
    int midiOutputChannel;  // Range 1-16
    int midiInputChannel;  // Range 1-16
    void handleIncomingMidiMessage(MidiInput* source,const MidiMessage& m) override;
    void setMidiInputDevice (const String& deviceIdentifier);
    void setMidiInputDeviceByName (const String& deviceName);
    void setMidiOutputDevice (const String& deviceIdentifier);
    void setMidiOutputDeviceByName (const String& deviceName);
    void setMidiInputChannel (int channel);
    void setMidiOutputChannel (int channel);
    bool isReceivingFromMidiInput = false;  // To distinguish when a parameter is changed from the onscren Slider or from MIDI input
    
    // UI Scale factor and hide/show panels
    float uiScaleFactor = 1.0;
    void setUIScaleFactor(float newUIScaleFactor);
    bool showMainControlsPanel = true;
    bool showExtraControlsPanel = true;
    bool showLfosPanel = false;
    void showOrHideKIJIMIPanel(String panelName, bool doShow); // panel names: "main", "extra" or "lfos"; doShow=true to show panel, false to hide it
    
    // Presetting
    void requestGetPresetFromKIJIMI(int bankNumber, int presetNumber);
    KIJIMIPresetBytes lastReceivedKIJIMIPresetBytes = {0};
    bool waitingToReceiveKIJIMIPresetBytes = false;
    std::vector<KIJIMIPresetBytes> receivedPresetsBytes;
    void loadPresetBankFromPresetsBytes (std::vector<KIJIMIPresetBytes> presetsBytes, const String& bankNameToUse);
    void loadBankFile (File* bankFile);
    int currentPreset;
    bool currentPresetOutOfSyncWithSliders;
    void nextPreset();
    void previousPreset();
    void savePresetToBankLocation (int bankLocation);
    void saveBankFile ();
    void loadPresetAtIndex (int index);
    void setParametersFromSynthControlIdValuePairs (SynthControlIdValuePairs idValuePairs, bool skipGlobal);
    bool isChangingFromPresetLoader = false;
    
    // KIJIMI Interface
    KIJIMIInterface* kijimiInterface;
    
    // Actions from KIJIMI control panel menu
    void sendControlsToSynth (bool skipGlobal);
    void randomizeControlValues ();
    void importFromPatchFile ();
    void saveToPatchFile ();
    void loadControlsStateFromSynth ();
    bool isChangingFromLoadingAPatchFile = false;
    bool isChangingFromGettingKijimiState = false;
    bool isChangingFromRandomizer = false;
    
    // Timbre Space Engine
    void computeTimbreSpace ();
    TimbreSpaceEngine* timbreSpaceEngine;
    void updateSpacePointAudioParametersFromMouseEvent(float x, float y);
    bool isChangingFromTimbreSpace = false;  // To distinguish when a parameter is changed from the onscren Slider or from MIDI input
    
    // Logging code
    void logMessage (const String& message);
    
    // Action listener
    void actionListenerCallback (const String &message) override;
    
    // Other
    void requestFirmwareVersion();
    void toggleAutomaticSyncWithSynth();
    bool neverShowScrollbars = false;
    bool automaticSyncWithSynthEnabled = true;
    float getValueForAudioParameter(const String& parameterID);
    File getDirectoryForFileSaveLoad ();
    void setLastUserDirectoryForFileSaveLoad (File file);
    File lastUsedDirectoryForFileIO;
    TimestampsLastCCSent timestampsLastCCSent;
    RandomizationConfigStruct randomizationSettings;
    bool ccReceiveOffNotificationShown = false;
    void checkConfigurationIsOk();
    bool isLFO1SyncedToMidi();
    bool isLFO2SyncedToMidi();
    bool isLFOPolyMode();
    void timerCallback() override; // timer callback for requesting firmware if not already detected
    
    class DelayedRequestLoadControlsFromSynthThread : private Thread
    {
    public:
        DelayedRequestLoadControlsFromSynthThread(BabuFrikAudioProcessor& p) : Thread ("DelayedRequestLoadControlsFromSynthThread"), processor (p){}
        
        void run() override
        {
            sleep(50);
            processor.loadControlsStateFromSynth();
        }
        BabuFrikAudioProcessor& processor;
    };
    DelayedRequestLoadControlsFromSynthThread delayedRequestLoadControlsSysexThread;
    
    String currentFirmwareLabel = "0.0.0";
    String requiredFirmwareLabel = "0.0.0";

    // The flags below are used to provide compatibility of old and new sysex protocol for simple things like setting parameters, but some other things
    // like sending and receiving banks will not work with old FW versions and newer Babu Frik because it would complicate the code too much to support
    // both protocols at the same time.
    bool sysexProtocolResolved = false;
    bool usesNewSysexProtocol = true;
    
private:    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BabuFrikAudioProcessor)
};
