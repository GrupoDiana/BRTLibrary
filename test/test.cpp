#include "Common/AudioState.h"
// Just a dummy test to test linking of libraries. 

#include "Common/CommonDefinitions.h"
#include "Common/GlobalParameters.hpp"
#include "Common/ErrorHandler.hpp"
#include "Base/BRTManager.hpp"
#include "ProcessingModules/SingleProcessor.hpp"
#include "ProcessingModules/SingleBinauralProcessor.hpp"
#include "ProcessingModules/DistanceAttenuationProcessor.hpp"
#include "ProcessingModules//HRTFConvolverProcessor.hpp"
#include "Readers/HRTFSOFAReader.hpp"

int main()
{
    Common::CGlobalParameters globalParameters;
	BRTBase::CBRTManager brtManager;
    BRTReaders::HRTFSOFAReader sofaReader;
    return 0;
}