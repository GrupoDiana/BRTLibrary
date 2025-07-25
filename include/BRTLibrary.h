/**
*
* \brief Includes of BRT LIBRARY
* \date	June 2023
*
* \authors 3DI-DIANA Research Group (University of Malaga), in alphabetical order: M. Cuevas-Rodriguez, D. Gonzalez-Toledo, L. Molina-Tanco, F. Morales-Benitez ||
* Coordinated by , A. Reyes-Lecuona (University of Malaga)||
* \b Contact: areyes@uma.es
*
* \b Copyright: University of Malaga
* 
* \b Contributions: (additional authors/contributors can be added here)
*
* \b Project: SONICOM ||
* \b Website: https://www.sonicom.eu/
*
* \b Acknowledgement: This project has received funding from the European Union�s Horizon 2020 research and innovation programme under grant agreement no.101017743
* 
* \b Licence: This program is free software, you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
*/

#ifndef _BRT_LIBRARY_
#define _BRT_LIBRARY_

#include "Common/CommonDefinitions.hpp"
#include "Common/GlobalParameters.hpp"
#include "Common/ErrorHandler.hpp"
#include "Base/BRTManager.hpp"
#include "Base/Listener.hpp"
#include "Base/ListenerBase.hpp"
#include "SourceModels/SourceOmnidirectionalModel.hpp"
#include "SourceModels/SourceDirectivityModel.hpp"
#include <ListenerModels/ListenerDirectHRTFConvolution.hpp>
#include <ListenerModels/ListenerDirectBRIRConvolution.hpp>
#include <ListenerModels/ListenerAmbisonicVirtualLoudspeakersModel.hpp>
#include <ListenerModels/ListenerAmbisonicReverberantVirtualLoudspeakersModel.hpp>
#include "ProcessingModules//HRTFConvolverProcessor.hpp"
#include "ProcessingModules/DirectivityTFConvolver.hpp"
#include "ServiceModules/HRTF.hpp"
#include "ServiceModules/HRBRIR.hpp"
#include "ServiceModules/SOSFilters.hpp"
#include "ServiceModules/DirectivityTF.hpp"
#include "Readers/SofaReader.hpp"
#include "third_party_libraries/nlohmann/json.hpp"
#include "Common/EnvelopeDetector.hpp"
#include "EnvironmentModels/SDNEnvironmentModel.hpp"
#include "EnvironmentModels/FreeFieldEnvironmentModel.hpp"
#include "BinauralFilter/SOSBinauralFilter.hpp"

#endif