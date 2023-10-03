// Just a dummy test to test linking of libraries. 
#include <BRTLibrary.h>

#define SOFA1_FILEPATH "hrtf.sofa"
#define SOURCE1_FILEPATH "speech.wav"
#define HRTFRESAMPLINGSTEP 15

Common::CGlobalParameters globalParameters;                                                     // Class where the global BRT parameters are defined.
BRTBase::CBRTManager brtManager;                                                                // BRT global manager interface
std::shared_ptr<BRTListenerModel::CListenerHRTFbasedModel> listener;                            // Pointer to listener model
BRTReaders::CSOFAReader sofaReader;                                                             // SOFA reader provide by BRT Library
std::vector<std::shared_ptr<BRTServices::CHRTF>> HRTF_list;                                     // List of HRTFs sofa loaded

bool LoadSofaFile(std::string _filePath) {
    std::shared_ptr<BRTServices::CHRTF> hrtf = std::make_shared<BRTServices::CHRTF>();

    int sampleRateInSOFAFile = sofaReader.GetSampleRateFromSofa(_filePath);
    if (sampleRateInSOFAFile == -1) {
        std::cout << ("Error loading HRTF Sofa file") << std::endl;
        return false;
    }
    if (globalParameters.GetSampleRate() != sampleRateInSOFAFile)
    {
        std::cout<<"The sample rate in HRTF SOFA file doesn't match the configuration." << std::endl;
        return false;
    }
    bool result = sofaReader.ReadHRTFFromSofa(_filePath, hrtf, HRTFRESAMPLINGSTEP);
    if (result) {
        std::cout << ("HRTF Sofa file loaded successfully.") << std::endl;
        HRTF_list.push_back(hrtf);
        return true;
    }
    else {
        std::cout << ("Error loading HRTF") << std::endl;
        return false;
    }
}

int main()
{
    
    // Configure BRT Error handler
    BRT_ERRORHANDLER.SetVerbosityMode(VERBOSITYMODE_ERRORSANDWARNINGS);
    BRT_ERRORHANDLER.SetErrorLogStream(&std::cout, true);

    // Global Parametert setup    
    globalParameters.SetSampleRate(44100);     // Setting sample rate
    globalParameters.SetBufferSize(512);    // Setting buffer size

    /////////////////////
    // Listener setup
    /////////////////////
    brtManager.BeginSetup();
    listener = brtManager.CreateListener<BRTListenerModel::CListenerHRTFbasedModel>("listener1");
    brtManager.EndSetup();    
    Common::CTransform listenerPosition = Common::CTransform();		 // Setting listener in (0,0,0)
    listenerPosition.SetPosition(Common::CVector3(0, 0, 0));
    listener->SetListenerTransform(listenerPosition);

    // Load HRTFs from SOFA file            
    ASSERT(LoadSofaFile(SOFA1_FILEPATH), RESULT_ERROR_FILE, std::string("Could not load sofa file ") + SOFA1_FILEPATH, ""); 
    listener->SetHRTF(HRTF_list[0]);
    return 0;
 
}