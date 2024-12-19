/**
* \class CLibMySOFALoader
*
* \brief Declaration of CLibMySOFALoader class
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

#ifndef _LIBMYSOFA_LOADER_
#define _LIBMYSOFA_LOADER_

#include <ostream>
#include <string>
#include <Common/ErrorHandler.hpp>
#include <third_party_libraries/libmysofa/include/mysofa.h>

namespace BRTReaders {

	class CLibMySOFALoader {

	public:

		enum class TSofaConvention { SimpleFreeFieldHRIR, SimpleFreeFieldHRSOS, FreeFieldDirectivityTF, SingleRoomMIMOSRIR};
		const char* SofaConventioToString(TSofaConvention e) noexcept
		{
			switch (e)
			{
			case TSofaConvention::SimpleFreeFieldHRIR: return "SimpleFreeFieldHRIR";
			case TSofaConvention::SimpleFreeFieldHRSOS: return "SimpleFreeFieldHRSOS";
			case TSofaConvention::FreeFieldDirectivityTF: return "FreeFieldDirectivityTF";
			case TSofaConvention::SingleRoomMIMOSRIR: return "SingleRoomMIMOSRIR";
			}
		}

		CLibMySOFALoader(const std::string& sofaFile) : error{ -1 } {
			int error = MySOFAInit(sofaFile);			
		}

		~CLibMySOFALoader() {
			if (hrtf && hrtf->hrtf) {
				mysofa_close(hrtf);				
			}
		}


		struct MYSOFA_EASY* get() const {
			return hrtf;
		}

		struct MYSOFA_HRTF* getHRTF() const {
			return hrtf->hrtf;
		}

		int getError() const {
			return error;
		}

		MYSOFA_ARRAY* GetDataSOS() {

			return mysofa_getVariable(hrtf->hrtf->variables, "Data.SOS");
		}

		MYSOFA_ARRAY* GetDataRealDirectivity() {

			return mysofa_getVariable(hrtf->hrtf->variables, "Data.Real");
		}

		MYSOFA_ARRAY* GetDataImagDirectivity() {

			return mysofa_getVariable(hrtf->hrtf->variables, "Data.Imag");
		}
	
		char* GetSourcePositionType() {
			return mysofa_getAttribute(hrtf->hrtf->SourcePosition.attributes, "Type");
		}
		
		char* GetSourceViewType() {

			if (mysofa_getSourceView()== NULL) { return "cartesian"; }
			return mysofa_getAttribute(mysofa_getSourceView()->attributes, "Type");
		}

		char* GetReceiverPositionType() {
			return mysofa_getAttribute(hrtf->hrtf->ReceiverPosition.attributes, "Type");
		}

		std::string GetDataType() {
			return mysofa_getAttribute(hrtf->hrtf->attributes, "DataType");
		}

		std::string GetSofaConvention(){
			return mysofa_getAttribute(hrtf->hrtf->attributes, "SOFAConventions");
		}

		bool CheckSofaConvention(TSofaConvention sofaConvention) {
			
			// To check SimpleFreeFieldHRIR we can use a method from the library 
			if (sofaConvention == TSofaConvention::SimpleFreeFieldHRIR) {
				return IsValidHRTFFile();
			}
			// To check other conventions we have to do it manually
			std::basic_string sofaConventions = mysofa_getAttribute(hrtf->hrtf->attributes, "SOFAConventions");
			if (sofaConventions != SofaConventioToString(sofaConvention))
			{
				SET_RESULT(RESULT_ERROR_INVALID_PARAM, "Not a valid SOFA convention file");
				return false;
			}
			return true;
		}

		int GetSamplingRate() {
			
			if (error == -1) return -1;
			
			try
			{				
				std::string samplingRatesUnits = mysofa_getAttribute(hrtf->hrtf->DataSamplingRate.attributes, "Units");
				std::transform(samplingRatesUnits.begin(), samplingRatesUnits.end(), samplingRatesUnits.begin(), ::tolower);
				if (samplingRatesUnits != "hertz")
				{
					SET_RESULT(RESULT_ERROR_INVALID_PARAM, "Sampling rate units are not Herzs");
					return -1;
				}

				int samplingRateArraySize = sizeof(hrtf->hrtf->DataSamplingRate.values) / sizeof(double);
				if (samplingRateArraySize > 1) {

					SET_RESULT(RESULT_ERROR_INVALID_PARAM, "Multiple sampling rates in one SOFA is not a supported charasteristic.");
					return -1;
				}

				return hrtf->hrtf->DataSamplingRate.values[0];
			}
			catch (...)
			{
				SET_RESULT(RESULT_ERROR_UNKNOWN, "Unknown error when reading samplerate from SOFA");
				return -1;
			}
		}

		int GetListenerViewSize(){
			if (error == -1) return -1;
			return hrtf->hrtf->ListenerView.elements;
		}		
		std::vector<double> GetListenerView() {
			if (error == -1) return std::vector< double >();
			std::vector< double > _listenerView(hrtf->hrtf->ListenerView.values, hrtf->hrtf->ListenerView.values + hrtf->hrtf->ListenerView.elements);
			return _listenerView;
		}

		std::vector<double> GetListenerUp() {
			if (error == -1) return std::vector< double >();
			std::vector< double > _listenerUp(hrtf->hrtf->ListenerUp.values, hrtf->hrtf->ListenerUp.values + hrtf->hrtf->ListenerUp.elements);
			return _listenerUp;
		}

		std::vector<double> GetReceiverPosition() {
			std::vector<double> _receiverPositions(hrtf->hrtf->ReceiverPosition.values, hrtf->hrtf->ReceiverPosition.values + hrtf->hrtf->ReceiverPosition.elements);
			return _receiverPositions;
		}

		// Source Positions
		std::vector<double> GetSourcePositionVector() {
			if (error == -1) return std::vector< double >();
			std::vector<double> _sourcePositions(hrtf->hrtf->SourcePosition.values, hrtf->hrtf->SourcePosition.values + hrtf->hrtf->SourcePosition.elements);
			return _sourcePositions;
		}
		
		//Source View
		int GetSourceViewSize() {
			if (error == -1) return -1;
			return mysofa_getSourceView()->elements;
		}

		std::vector<double> GetSourceViewVector() {
			if (error == -1) return std::vector< double >();	
			if (mysofa_getSourceView() == NULL) { return std::vector< double >();}
			std::vector< double > _sourceView(mysofa_getSourceView()->values, mysofa_getSourceView()->values + mysofa_getSourceView()->elements);			
			return _sourceView;
		}

		// Source Up
		int GetSourceUpSize() {
			if (error == -1) return -1;
			return mysofa_getSourceUp()->elements;
		}
		Common::CVector3 GetSourceUp() {
			if (error == -1) return Common::CVector3();
			Common::CVector3 _sourceUp(mysofa_getSourceUp()->values[0], mysofa_getSourceUp()->values[1], mysofa_getSourceUp()->values[2]);
			return _sourceUp;
		}
		std::vector<double> GetSourceUpVector() {
			if (error == -1) return std::vector< double >();
			if (mysofa_getSourceUp() == NULL) { return std::vector< double >(); }
			std::vector<double> _sourceUp(mysofa_getSourceUp()->values, mysofa_getSourceUp()->values + mysofa_getSourceUp()->elements);
			return _sourceUp;
		}
		
		

		//Emitter
		std::vector<double> GetEmitterPositionVector() {	
			if (error == -1) return std::vector< double >();
			std::vector<double> _emitterPositions(hrtf->hrtf->EmitterPosition.values, hrtf->hrtf->EmitterPosition.values + hrtf->hrtf->EmitterPosition.elements);			
			return _emitterPositions;
		}

		//Listener
		std::vector<double> GetListenerPositionVector() {
			if (error == -1) return std::vector< double >();
			std::vector<double> _listenerPositions(hrtf->hrtf->ListenerPosition.values, hrtf->hrtf->ListenerPosition.values + hrtf->hrtf->ListenerPosition.elements);
			return _listenerPositions;
		}

		std::vector<double> GetListenerViewVector() {
			if (error == -1) return std::vector< double >();
			std::vector<double> _listenerView(hrtf->hrtf->ListenerView.values, hrtf->hrtf->ListenerView.values + hrtf->hrtf->ListenerView.elements);
			return _listenerView;
		}

		std::vector<double> GetListenerUpVector() {
			if (error == -1) return std::vector< double >();
			std::vector<double> _listenerUp(hrtf->hrtf->ListenerUp.values, hrtf->hrtf->ListenerUp.values + hrtf->hrtf->ListenerUp.elements);
			return _listenerUp;
		}

		// Others methods
		void Cartesian2Spherical() {
			if (error == -1) return ;
			mysofa_tospherical(hrtf->hrtf);			
		}

		bool IsValidHRTFFile() {
			bool error = mysofa_check(hrtf->hrtf);
			if (error != MYSOFA_OK) {
				//mysofa_close(hrtf);
				SET_RESULT(RESULT_ERROR_INVALID_PARAM, "Not a valid HRTF SOFA file");
				return false;
			}
			return true;
		}
		
		std::string GetErrorName(int _error) const {
			switch (_error) {
			case TLibMySOFAErrorList::MYSOFA_OK:
				return "MYSOFA_OK";
			case TLibMySOFAErrorList::MYSOFA_INTERNAL_ERROR:
				return "MYSOFA_INTERNAL_ERROR";
			case TLibMySOFAErrorList::MYSOFA_INVALID_FORMAT:
				return "MYSOFA_INVALID_FORMAT";
			case TLibMySOFAErrorList::MYSOFA_UNSUPPORTED_FORMAT:
				return "MYSOFA_UNSUPPORTED_FORMAT";
			case TLibMySOFAErrorList::MYSOFA_NO_MEMORY:
				return "MYSOFA_NO_MEMORY";
			case TLibMySOFAErrorList::MYSOFA_READ_ERROR:
				return "MYSOFA_READ_ERROR";
			case TLibMySOFAErrorList::MYSOFA_INVALID_ATTRIBUTES:
				return "MYSOFA_INVALID_ATTRIBUTES";
			case TLibMySOFAErrorList::MYSOFA_INVALID_DIMENSIONS:
				return "MYSOFA_INVALID_DIMENSIONS";
			case TLibMySOFAErrorList::MYSOFA_INVALID_DIMENSION_LIST:
				return "MYSOFA_INVALID_DIMENSION_LIST";
			case TLibMySOFAErrorList::MYSOFA_INVALID_COORDINATE_TYPE:
				return "MYSOFA_INVALID_COORDINATE_TYPE";
			case TLibMySOFAErrorList::MYSOFA_ONLY_EMITTER_WITH_ECI_SUPPORTED:
				return "MYSOFA_ONLY_EMITTER_WITH_ECI_SUPPORTED";
			case TLibMySOFAErrorList::MYSOFA_ONLY_DELAYS_WITH_IR_OR_MR_SUPPORTED:
				return "MYSOFA_ONLY_DELAYS_WITH_IR_OR_MR_SUPPORTED";
			case TLibMySOFAErrorList::MYSOFA_ONLY_THE_SAME_SAMPLING_RATE_SUPPORTED:
				return "MYSOFA_ONLY_THE_SAME_SAMPLING_RATE_SUPPORTED";
			case TLibMySOFAErrorList::MYSOFA_RECEIVERS_WITH_RCI_SUPPORTED:
				return "MYSOFA_RECEIVERS_WITH_RCI_SUPPORTED";
			case TLibMySOFAErrorList::MYSOFA_RECEIVERS_WITH_CARTESIAN_SUPPORTED:
				return "MYSOFA_RECEIVERS_WITH_CARTESIAN_SUPPORTED";
			case TLibMySOFAErrorList::MYSOFA_INVALID_RECEIVER_POSITIONS:
				return "MYSOFA_INVALID_RECEIVER_POSITIONS";
			case TLibMySOFAErrorList::MYSOFA_ONLY_SOURCES_WITH_MC_SUPPORTED:
				return "MYSOFA_ONLY_SOURCES_WITH_MC_SUPPORTED";
			default:
				return "UNKNOWN_ERROR";
			}
		}

	private:				
		// Initialize MySOFA struct and open a sofafile
		bool MySOFAInit(const std::string& sofafile) {

			try
			{
				hrtf = (MYSOFA_EASY*)malloc(sizeof(struct MYSOFA_EASY));
				// Check file open and format				
				if (!hrtf)
				{
					SET_RESULT(RESULT_ERROR_BADALLOC, "Error trying to set the variable MYSOFA_EASY");
					return false;
				}

				hrtf->lookup = NULL;
				hrtf->neighborhood = NULL;
				hrtf->fir = NULL;
				
				hrtf->hrtf = mysofa_load(sofafile.data(), &error);
				if (!hrtf->hrtf) {
					//mysofa_close(hrtf);
					SET_RESULT(RESULT_ERROR_INVALID_PARAM, "Error opening SOFA file");
					return false;
				}

				/*error = mysofa_check(hrtf->hrtf);
				if (error != MYSOFA_OK) {
					mysofa_close(hrtf);
					SET_RESULT(RESULT_ERROR_INVALID_PARAM, "Not a valid SOFA file");
					return -1;
				}*/

				//mysofa_tocartesian(hrtf->hrtf);

				/*hrtf->lookup = mysofa_lookup_init(hrtf->hrtf);
				if (hrtf->lookup == NULL) {
					error = MYSOFA_INTERNAL_ERROR;
					mysofa_close(hrtf);
					SET_RESULT(RESULT_ERROR_INVALID_PARAM, "MYSOFA INTERNAL ERROR");
					return -1;
				}
				hrtf->neighborhood = mysofa_neighborhood_init(hrtf->hrtf, hrtf->lookup);*/
				
				SET_RESULT(RESULT_OK, "SOFA file loaded");
				return true;
			}
			catch (int error)
			{
				// the description of the exception will be printed when raised
				SET_RESULT(RESULT_ERROR_UNKNOWN, "Sofa exception, please consider previous messages from the sofa library");
				return false;
			}
			catch (...)
			{
				SET_RESULT(RESULT_ERROR_UNKNOWN, "Unknown error when reading samplerate from SOFA");
				return false;
			}
		}
		
		MYSOFA_ARRAY* mysofa_getSourceView() {
			return mysofa_getVariable(hrtf->hrtf->variables, "SourceView");
		}

		MYSOFA_ARRAY* mysofa_getSourceUp() {
			return mysofa_getVariable(hrtf->hrtf->variables, "SourceUp");
		}

		MYSOFA_ARRAY* mysofa_getVariable(struct MYSOFA_VARIABLE* var, char* name) {
			while (var) {
				if (var->name && !strcmp(name, var->name)) {
					return var->value;
				}
				var = var->next;
			}
			return NULL;
		}

		
		

		////////////////
		// Attributes
		///////////////
		int error;		
		struct MYSOFA_EASY * hrtf;

		enum TLibMySOFAErrorList{
			MYSOFA_OK = 0,
			MYSOFA_INTERNAL_ERROR = -1,
			MYSOFA_INVALID_FORMAT = 10000,
			MYSOFA_UNSUPPORTED_FORMAT,
			MYSOFA_NO_MEMORY,
			MYSOFA_READ_ERROR,
			MYSOFA_INVALID_ATTRIBUTES,
			MYSOFA_INVALID_DIMENSIONS,
			MYSOFA_INVALID_DIMENSION_LIST,
			MYSOFA_INVALID_COORDINATE_TYPE,
			MYSOFA_ONLY_EMITTER_WITH_ECI_SUPPORTED,
			MYSOFA_ONLY_DELAYS_WITH_IR_OR_MR_SUPPORTED,
			MYSOFA_ONLY_THE_SAME_SAMPLING_RATE_SUPPORTED,
			MYSOFA_RECEIVERS_WITH_RCI_SUPPORTED,
			MYSOFA_RECEIVERS_WITH_CARTESIAN_SUPPORTED,
			MYSOFA_INVALID_RECEIVER_POSITIONS,
			MYSOFA_ONLY_SOURCES_WITH_MC_SUPPORTED
		};
		
	};          
};
#endif 
