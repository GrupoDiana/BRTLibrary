/**
* \class CDirectivityTF
*
* \brief Declaration of CDirectivityTF class interface to store directivity data
* \version 
* \date	May 2023
*
* \authors 3DI-DIANA Research Group (University of Malaga), in alphabetical order: M. Cuevas-Rodriguez, D. Gonzalez-Toledo, L. Molina-Tanco, F. Morales-Benitez ||
* Coordinated by , A. Reyes-Lecuona (University of Malaga)||
* \b Contact: areyes@uma.es 
*
* \b Contributions: (additional authors/contributors can be added here)
*
* \b Project: SONICOM ||
* \b Website: https://www.sonicom.eu/
*
* \b Copyright: University of Malaga 
*
* \b Licence: 
*
* \b Acknowledgement: This project has received funding from the European Union�s Horizon 2020 research and innovation programme under grant agreement no.101017743
*/


#ifndef _CDIRECTIVITYTF_H_
#define _CDIRECTIVITYTF_H_

#include <unordered_map>
#include <Common/ErrorHandler.hpp>
#include <Common/GlobalParameters.hpp>
#include <Common/CommonDefinitions.hpp>
#include <ServiceModules/ServiceModuleInterfaces.hpp>
#include <ServiceModules/DirectivityTFDefinitions.hpp>
#include <ServiceModules/DirectivityTFAuxiliarMethods.hpp>
#include <ServiceModules/Extrapolation.hpp>
#include <ServiceModules/GridsManager.hpp>
#include <ServiceModules/OfflineInterpolation.hpp>
#include <ServiceModules/InterpolationAuxiliarMethods.hpp>
#include <ServiceModules/OnlineInterpolation.hpp>


#ifndef DEFAULT_DIRECTIVITYTF_RESAMPLING_STEP
#define DEFAULT_DIRECTIVITYTF_RESAMPLING_STEP 5
#endif


namespace BRTServices
{
	
	/** \details This class gets impulse response data to compose HRTFs and implements different algorithms to interpolate the HRIR functions.
	*/
	class CDirectivityTF : public CServicesBase
	{
	public:
		/** \brief Default Constructor
		*	\details By default, customized ITD is switched off, resampling step is set to 5 degrees and listener is a null pointer
		*   \eh Nothing is reported to the error handler.
		*/
		CDirectivityTF()
			:resamplingStep{ DEFAULT_DIRECTIVITYTF_RESAMPLING_STEP }, directivityTFloaded{ false }, setupDirectivityTFInProgress{ false }
		{}

		/** \brief Set the title of the SOFA file
		*    \param [in]	_title		string contains title
		*/
		void SetTitle(std::string _title) {
			title = _title;
		}

		/** \brief Set the title of the SOFA file
		*    \param [in]	_title		string contains title
		*/
		void SetDatabaseName(std::string _databaseName) {
			databaseName = _databaseName;
		}

		/** \brief Set the name of the SOFA file
		*    \param [in]	_fileName		string contains filename
		*/
		void SetFilename(std::string _fileName) {
			fileName = _fileName;
		}

		/** \brief Get the name of the SOFA file
		*   \return string contains filename
		*/
		std::string GetFilename() {
			return fileName;
		}
		
		/** \Start a new DirectivityTF configuration
		*	\param [in] directivityTFPartLength number of samples in the frequency domain (size of the real part or the imaginary part)
		*   \eh On success, RESULT_OK is reported to the error handler.
		*       On error, an error code is reported to the error handler.
		*/
		void BeginSetup(int32_t _directivityTFPartLength, std::string _extrapolationMethod){
			//Update parameters			
			elevationNorth = CInterpolationAuxiliarMethods::GetPoleElevation(TPole::north);
			elevationSouth = CInterpolationAuxiliarMethods::GetPoleElevation(TPole::south);

			if (_directivityTFPartLength != globalParameters.GetBufferSize()) //
			{
				SET_RESULT(RESULT_ERROR_BADSIZE, "Number of frequency samples (N) in SOFA file is different from Buffer Size");
			}

			bufferSize = globalParameters.GetBufferSize();
			directivityTFPart_length = _directivityTFPartLength;
			directivityTF_length = 4.0 * _directivityTFPartLength; //directivityTF will store the Real and Img parts interlaced
			directivityTF_numberOfSubfilters = 1;
			SetExtrapolationMethod(_extrapolationMethod);

			//Clear every table			
			t_DirectivityTF_DataBase.clear();
			t_DirectivityTF_Resampled.clear();
			 
			//Change class state
			setupDirectivityTFInProgress = true;
			directivityTFloaded = false;

			SET_RESULT(RESULT_OK, "HRTF Setup started");
		}

		/** \brief Stop the DirectivityTF configuration
		*   \eh On success, RESULT_OK is reported to the error handler.
		*       On error, an error code is reported to the error handler.
		*/
		bool EndSetup()
		{
			if (setupDirectivityTFInProgress) {
				if (!t_DirectivityTF_DataBase.empty())
				{
					// Preparation of table read from sofa file
					t_DirectivityTF_DataBase_ListOfOrientations = offlineInterpolation.CalculateListOfOrientations(t_DirectivityTF_DataBase);
					CalculateExtrapolation();							// Make the extrapolation if it's needed
					offlineInterpolation.CalculateTF_InPoles<T_DirectivityTFTable, BRTServices::TDirectivityTFStruct>(t_DirectivityTF_DataBase, directivityTFPart_length, resamplingStep, CDirectivityTFAuxiliarMethods::CalculateDirectivityTFFromHemisphereParts());
					offlineInterpolation.CalculateTF_SphericalCaps<T_DirectivityTFTable, BRTServices::TDirectivityTFStruct>(t_DirectivityTF_DataBase, directivityTFPart_length, DEFAULT_GAP_THRESHOLD, resamplingStep, CDirectivityTFAuxiliarMethods::CalculateDirectivityTF_FromBarycentrics_OfflineInterpolation());
					//Creation and filling of resampling HRTF table
					t_DirectivityTF_DataBase_ListOfOrientations = offlineInterpolation.CalculateListOfOrientations(t_DirectivityTF_DataBase);
					quasiUniformSphereDistribution.CreateGrid<T_DirectivityTFInterlacedDataTable, TDirectivityInterlacedTFStruct>(t_DirectivityTF_Resampled, gridResamplingStepsVector, resamplingStep);
					offlineInterpolation.FillResampledTable<T_DirectivityTFTable, T_DirectivityTFInterlacedDataTable, BRTServices::TDirectivityTFStruct, BRTServices::TDirectivityInterlacedTFStruct>(t_DirectivityTF_DataBase, t_DirectivityTF_Resampled, bufferSize, directivityTFPart_length, directivityTF_numberOfSubfilters, CalculateInterlacedTFTo2PI(), CDirectivityTFAuxiliarMethods::CalculateDirectivityTFFromBarycentrics_OfflineInterpolation());		
					//TESTING:
					for (auto it = t_DirectivityTF_Resampled.begin(); it != t_DirectivityTF_Resampled.end(); it++) {
						if (it->second.data.size() == 0) {
							SET_RESULT(RESULT_ERROR_NOTSET, "The t_DirectivityTF_Resampled table has an empty DirectivityTF in position [" + std::to_string(it->first.azimuth) + ", " + std::to_string(it->first.elevation) + "]");
						}

					}

					//Setup values
					setupDirectivityTFInProgress = false;
					directivityTFloaded = true;

					SET_RESULT(RESULT_OK, "DirectivityTF Table completed succesfully");
					return true;
				}
				else
				{
					// TO DO: Should be ASSERT?
					SET_RESULT(RESULT_ERROR_NOTSET, "The t_DirectivityTF_DataBase map has not been set");
				}
			}
			return false;
		}


		/** \brief Set the step for the DirectivityTF resampled table
		*	\param [in] Step to create the resampled table
		*/
		void SetResamplingStep(int _resamplingStep) {
			resamplingStep = _resamplingStep;
		}
		
		/** \brief Ask for the step used to create the resampled table
		*	\retval Step used to create the resampling step
		*/
		int GetResamplingStep() {
			return resamplingStep;
		}

		/** \brief Get the number of samples of the Directivity TF
		*	\retval Length of the TF with the Real and Img parts interlaced
		*/
		int GetDirectivityTFLength() { return directivityTF_length; }

		/** \brief Get the number of subfilters if the TF is partitioned
		*	\retval Number of partitions or 1 if there is no partition
		*/
		int GetDirectivityTFNumOfSubfilters() { return directivityTF_numberOfSubfilters; }

		/** \brief Add a new TF to the DirectivityTF table
		*	\param [in] azimuth azimuth angle in degrees
		*	\param [in] elevation elevation angle in degrees
		*	\param [in] newDirectivityTF DirectivityTF data for both ears
		*   \eh Warnings may be reported to the error handler.
		*/
		void AddDirectivityTF(float _azimuth, float _elevation, TDirectivityTFStruct&& directivityTF)
		{
			if (setupDirectivityTFInProgress) {
				_azimuth = CInterpolationAuxiliarMethods::CalculateAzimuthIn0_360Range(_azimuth);
				_elevation = CInterpolationAuxiliarMethods::CalculateElevationIn0_90_270_360Range(_elevation);
				auto returnValue = t_DirectivityTF_DataBase.emplace(orientation(_azimuth, _elevation), std::forward<TDirectivityTFStruct>(directivityTF));
				//Error handler
				if (!returnValue.second) { SET_RESULT(RESULT_WARNING, "Error emplacing DirectivityTF in t_DirectivityTF_DataBase map in position [" + std::to_string(_azimuth) + ", " + std::to_string(_elevation) + "]"); }
			}
		}
				

		/** \brief  Calculate the azimuth step for each orientation
		*	\retval unordered map with all the orientations of the resampled table 
		*   \eh Warnings may be reported to the error handler.
		*/
		std::unordered_map<orientation, float> CalculateStep() const
		{
			std::unordered_map<orientation, float> stepVector;
			float elevation, aziStep, diff = 360, actual_ele = -1.0f;
			int secondTime = 0;
			std::vector<orientation> orientations;

			// Create a vector with all the orientations of the Database
			orientations.reserve(t_DirectivityTF_DataBase.size());
			for (auto& itr : t_DirectivityTF_DataBase)
			{
				orientations.push_back(itr.first);
			}
			//  Sort orientations of the DataBase with a lambda function in sort.
			std::sort(orientations.begin(), orientations.end(), [](orientation a, orientation b) {return (a.elevation < b.elevation); });


			for (int ori = 0; ori< orientations.size(); ori++)
			{
				// Maybe stop in each different elevation and make the difference between the start azimuth, 0, and the next azimuth in this elevation
				// with this form, we could save a vector like this [aziStep elevation]

				elevation = orientations[ori].elevation;

				if (actual_ele != elevation)
				{
					for (int ele = ori+1; ele < orientations.size(); ele++)
					{
						if (orientations[ele].elevation != elevation || orientations[ele] == orientations.back())
						{	
							if (diff == 360) { diff = 0; }
							stepVector.emplace(orientation(0, elevation), diff);
							diff = 360;
							break;
						}
						if (abs(orientations[ele].azimuth - orientations[ele -1].azimuth) < diff) { 
							diff = abs(orientations[ele].azimuth - orientations[ele - 1].azimuth); 
						}
					}
					actual_ele = elevation;

				}
			}
			return stepVector;
		}

	
		/** \brief  Get the Directivity TF of the nearest point to  the given one
		*	\param [in] _azimuth orientation of the sound source (relative to the listener)
		*	\param [in] _elevation orientation of the sound source (relative to the listener)
		* *	\param [in] _stepsMap map that contains orientations of the grid
		*	\retval  Directivity TF with the Real and Imag part interlaced
		*/
		const TDirectivityInterlacedTFStruct GetDirectivityTF(float _azimuth, float _elevation) const {
			
			TDirectivityInterlacedTFStruct newSRIR;
			auto it0 = t_DirectivityTF_Resampled.find(orientation(_azimuth, _elevation));
			if (it0 != t_DirectivityTF_Resampled.end()) {
				newSRIR.data = it0->second.data;
			}
			else {

				// HARCODE ELEVATION STEP TO 10
				int eleStep = 10;

				float aziCeilBack, aziCeilFront, aziFloorBack, aziFloorFront;
				int idxEle = ceil(_elevation / eleStep);
				float eleCeil = eleStep * idxEle;
				float eleFloor = eleStep * (idxEle - 1);

				eleCeil = CheckLimitsElevation_and_Transform(eleCeil);										//			   Back	  Front
				eleFloor = CheckLimitsElevation_and_Transform(eleFloor);									//	Ceil		A		B

				auto stepItr = gridResamplingStepsVector.find(orientation(0, eleCeil));						//	Floor		D		C
				float aziStepCeil = stepItr->second;
																										
				CalculateAzimuth_BackandFront(aziCeilBack, aziCeilFront, aziStepCeil, _azimuth);
				// azimuth values passed by reference

				auto stepIt = gridResamplingStepsVector.find(orientation(0, eleFloor));
				float aziStepFloor = stepIt->second;							

				CalculateAzimuth_BackandFront(aziFloorBack, aziFloorFront, aziStepFloor, _azimuth); 

				// Mid Point of a trapezoid can be compute by averaging all azimuths
				float pntMid_azimuth = (aziCeilBack + aziCeilFront + aziFloorBack + aziFloorFront) / 4;
				float pntMid_elevation = (eleCeil - eleStep * 0.5f);

				// compute eleCeil being 360 to find triangles at border
				//eleCeil = eleStep * idxEle;

				if (_azimuth >= pntMid_azimuth)
				{
					if (_elevation >= pntMid_elevation)
					{
						//Second quadrant
						auto it = t_DirectivityTF_Resampled.find(orientation(aziCeilFront, eleCeil));
						newSRIR.data = it->second.data;

					}
					else if (_elevation < pntMid_elevation)
					{
						//Forth quadrant
						auto it = t_DirectivityTF_Resampled.find(orientation(aziFloorFront, eleFloor));
						newSRIR.data = it->second.data;
					}
				}
				else if (_azimuth < pntMid_azimuth)
				{
					if (_elevation >= pntMid_elevation)
					{
						//First quadrant
						auto it = t_DirectivityTF_Resampled.find(orientation(aziCeilBack, eleCeil));
						newSRIR.data = it->second.data;
					}
					else if (_elevation < pntMid_elevation) {
						//Third quadrant
						auto it = t_DirectivityTF_Resampled.find(orientation(aziFloorFront, eleFloor));
						newSRIR.data = it->second.data;
					}
				}
				else { return newSRIR = {}; }
			}

			//SET_RESULT(RESULT_OK, "CalculateHRIR_InterpolationMethod completed succesfully");
			return newSRIR;
		}

		/** \brief Get interpolated and partitioned HRIR buffer with Delay, for one ear
		*	\param [in] ear for which ear we want to get the HRIR
		*	\param [in] _azimuth azimuth angle in degrees
		*	\param [in] _elevation elevation angle in degrees
		*	\param [in] runTimeInterpolation switch run-time interpolation
		*	\param [in] runTimeInterpolation switch run-time interpolation
		*	\retval HRIR interpolated buffer with delay for specified ear
		*   \eh On error, an error code is reported to the error handler.
		*       Warnings may be reported to the error handler.
		*/
		const std::vector<CMonoBuffer<float>> GetDirectivityTF2(float _azimuth, float _elevation, bool runTimeInterpolation) const
		{
			std::vector<CMonoBuffer<float>> newDirectivityTF;
						
			if (setupDirectivityTFInProgress) {
				SET_RESULT(RESULT_ERROR_NOTSET, "GetDirectivityTF: Directivity setup in progress, return empty");
				return newDirectivityTF;

			}
			if (!runTimeInterpolation) {
				TDirectivityInterlacedTFStruct temp = quasiUniformSphereDistribution.FindNearest<T_DirectivityTFInterlacedDataTable, TDirectivityInterlacedTFStruct>(t_DirectivityTF_Resampled, gridResamplingStepsVector, _azimuth, _elevation);
				return temp.data;
				
			}

			//  We have to do the run time interpolation -- (runTimeInterpolation = true)

			// Check if we are close to 360 azimuth or elevation and change to 0
			if (Common::AreSame(_azimuth, SPHERE_BORDER, EPSILON_SEWING)) { _azimuth = DEFAULT_MIN_AZIMUTH; }
			if (Common::AreSame(_elevation, SPHERE_BORDER, EPSILON_SEWING)) { _elevation = DEFAULT_MIN_ELEVATION; }

			// Check if we are at a pole
			int ielevation = static_cast<int>(round(_elevation));
			if ((ielevation == elevationNorth) || (ielevation == elevationSouth)) {
				_elevation = ielevation;
				_azimuth = DEFAULT_MIN_AZIMUTH;
				//newDirectivityTF = GetPoleDirectivityTF(ielevation);
				//return newDirectivityTF;
				//GetDirectivityTF2(DEFAULT_MIN_AZIMUTH, ielevation, true);
			}

			// We search if the point already exists
			auto it = t_DirectivityTF_Resampled.find(orientation(_azimuth, _elevation));
			if (it != t_DirectivityTF_Resampled.end())
			{
				return it->second.data;
			}
			else
			{
				// ONLINE Interpolation 
				const  BRTServices::TDirectivityInterlacedTFStruct temp = slopesMethodOnlineInterpolator2.CalculateTF_OnlineMethod<T_DirectivityTFInterlacedDataTable, BRTServices::TDirectivityInterlacedTFStruct>
					(t_DirectivityTF_Resampled, directivityTF_numberOfSubfilters, directivityTF_length, _azimuth, _elevation, gridResamplingStepsVector, CDirectivityTFAuxiliarMethods::CalculateDirectivityTF_FromBarycentric_OnlineInterpolation());
				return temp.data;
			}
		}
	

		/** \brief  Check limit values for elevation and transform to the desired intervals
		*	\retval elevation value within the desired intervals
		*/
		float CheckLimitsElevation_and_Transform(float elevation)const
		{
			if (elevation < 0) { elevation = elevation + 360; }
			if (elevation >= 360) { elevation = elevation - 360; }
			return elevation;
		}

		/** \brief  Check limit values for elevation and transform to the desired intervals
		*	\retval azimuth value within the desired intervals
		*/
		float CheckLimitsAzimuth_and_Transform(float azimuth)const
		{
			if (azimuth < 0) { azimuth = azimuth + 360; }
			else if (azimuth > 360) { azimuth = azimuth - 360; }
			return azimuth;
		}

		void CalculateAzimuth_BackandFront(float& aziBack, float& aziFront, float aziStep, float _azimuth)const
		{

			int idxAzi = ceil(_azimuth / aziStep);

			aziFront = idxAzi * aziStep;
			aziBack = (idxAzi - 1) * aziStep;

			aziFront = CheckLimitsAzimuth_and_Transform(aziFront);
			aziBack = CheckLimitsAzimuth_and_Transform(aziBack);
		}

		/** \brief Calculate the regular resampled-table using an interpolation method
		*	\param [in] _resamplingStep step for both azimuth and elevation
		*   \eh Warnings may be reported to the error handler.
		*/
		struct CalculateInterlacedTFTo2PI {
			TDirectivityInterlacedTFStruct operator()(const TDirectivityTFStruct& newData, int _bufferSize, int _TF_NumberOfSubfilters)
			{
				TDirectivityInterlacedTFStruct interlacedData;

				if (newData.realPart.size() == 0 || newData.imagPart.size() == 0) {
					SET_RESULT(RESULT_ERROR_NOTSET, "CalculateInterlacedTFTo2PI() get an empty data");
				}
				// Extend to 2PI real part
				CMonoBuffer<float> dataRealPart2PI;
				CalculateTFRealPartTo2PI(newData.realPart, dataRealPart2PI);
				// Extend to 2PI imag part
				CMonoBuffer<float> dataImagPart2PI;
				CalculateTFImagPartTo2PI(newData.imagPart, dataImagPart2PI);
				// Invert sign of imag part
				CalculateTFImagPartToBeCompatibleWithOouraFFTLibrary(dataImagPart2PI);
				// Interlaced real and imag part of the first subfilter 
				interlacedData.data.resize(_TF_NumberOfSubfilters);
				interlacedData.data.front().Interlace(dataRealPart2PI, dataImagPart2PI); //IMPORTANT: We only have one partition in the Directivity

				return interlacedData;
			}
		};

	private:

		enum TExtrapolationMethod { zeroInsertion, nearestPoint };

		std::string title;
		std::string databaseName;
		std::string fileName;
		int bufferSize;
		int resamplingStep;
		bool directivityTFloaded;
		bool setupDirectivityTFInProgress;
		int32_t directivityTF_length;	
		int32_t directivityTFPart_length;
		int32_t directivityTF_numberOfSubfilters;	
		TExtrapolationMethod extrapolationMethod;						// Methods that is going to be used to extrapolate
		
		T_DirectivityTFTable					t_DirectivityTF_DataBase;
		std::vector<orientation>				t_DirectivityTF_DataBase_ListOfOrientations;
		T_DirectivityTFInterlacedDataTable		t_DirectivityTF_Resampled;
		
		std::unordered_map<orientation, float>  gridResamplingStepsVector;		// Store hrtf interpolated grids steps

		Common::CGlobalParameters globalParameters;
		
		float  elevationNorth, elevationSouth;	

		CQuasiUniformSphereDistribution quasiUniformSphereDistribution;
		CSlopesMethodOnlineInterpolator slopesMethodOnlineInterpolator2;
		COfflineInterpolation offlineInterpolation;
		CExtrapolation extrapolation;
		
		///////////////////
		///// METHODS
		///////////////////

		/** \brief Transform Real part of the Directivity TF to 2PI
		*	\param [in] inBuffer Samples with the original Real part
		* *	\param [in] outBuffer Samples transformed to 2PI
		*/
		static void CalculateTFRealPartTo2PI(const CMonoBuffer<float>& inBuffer, CMonoBuffer<float>& outBuffer) {
			outBuffer.reserve(inBuffer.size() * 2);
			outBuffer.insert(outBuffer.begin(), inBuffer.begin(), inBuffer.end());
			outBuffer.insert(outBuffer.end(), 0);
			outBuffer.insert(outBuffer.end(), inBuffer.rbegin(), inBuffer.rend() - 1);
		}

		/** \brief Transform Imag part of the Directivity TF to 2PI
		*	\param [in] inBuffer Samples with the original IMAG part
		* *	\param [in] outBuffer Samples transformed to 2PI
		*/		
		static void CalculateTFImagPartTo2PI(const CMonoBuffer<float>& inBuffer, CMonoBuffer<float>& outBuffer) {
			outBuffer.reserve(inBuffer.size() * 2);
			outBuffer.insert(outBuffer.begin(), inBuffer.begin(), inBuffer.end());
			outBuffer.insert(outBuffer.end(), 0);

			CMonoBuffer<float> temp;
			temp.reserve(inBuffer.size() - 1);
			temp.insert(temp.begin(), inBuffer.begin() + 1, inBuffer.end());
			std::transform(temp.begin(), temp.end(), temp.begin(), [](float v) -> float { return -v; });

			outBuffer.insert(outBuffer.end(), temp.rbegin(), temp.rend());
		}

		/** \brief Invert the Imag part to prepare the data in the way that the Ooura Library is expecting to do the complex multiplication
		*	\param [in] buffer with the samples to be inverted (multiplied by -1)
		*/
		static void CalculateTFImagPartToBeCompatibleWithOouraFFTLibrary(CMonoBuffer<float>& buffer) {
			for (int i = 0; i < buffer.size(); i++) {
				buffer[i] = buffer[i] * -1;
			}
		}


		/**
		 * @brief Set the extrapolation method that is going to be used
		 * @param _extrapolationMethod
		*/
		void SetExtrapolationMethod(std::string _extrapolationMethod) {

			if (_extrapolationMethod == EXTRAPOLATION_METHOD_ZEROINSERTION_STRING) {
				extrapolationMethod = TExtrapolationMethod::zeroInsertion;
			}
			else if (_extrapolationMethod == EXTRAPOLATION_METHOD_NEARESTPOINT_STRING) {
				extrapolationMethod = TExtrapolationMethod::nearestPoint;
			}
			else {
				SET_RESULT(RESULT_ERROR_INVALID_PARAM, "Extrapolation Method not identified.");
				extrapolationMethod = TExtrapolationMethod::nearestPoint;
			}

		}
		/**
		 * @brief Call the extrapolation method
		*/
		void CalculateExtrapolation() {
			// Select the one that extrapolates with zeros or the one that extrapolates based on the nearest point according to some parameter.

			if (extrapolationMethod == TExtrapolationMethod::zeroInsertion) {
				SET_RESULT(RESULT_WARNING, "At least one large gap has been found in the loaded DirectivityTF sofa file, an extrapolation with zeros will be performed to fill it.");
				extrapolation.Process<T_DirectivityTFTable, BRTServices::TDirectivityTFStruct>(t_DirectivityTF_DataBase, t_DirectivityTF_DataBase_ListOfOrientations, directivityTFPart_length, DEFAULT_EXTRAPOLATION_STEP, CDirectivityTFAuxiliarMethods::GetZerosDirectivityTF());			
			}
			else if (extrapolationMethod == TExtrapolationMethod::nearestPoint) {
				SET_RESULT(RESULT_WARNING, "At least one large gap has been found in the loaded DirectivityTF sofa file, an extrapolation will be made to the nearest point to fill it.");
				extrapolation.Process<T_DirectivityTFTable, BRTServices::TDirectivityTFStruct>(t_DirectivityTF_DataBase, t_DirectivityTF_DataBase_ListOfOrientations, directivityTFPart_length, DEFAULT_EXTRAPOLATION_STEP, CDirectivityTFAuxiliarMethods::GetNearestPointDirectivityTF());
			}
			else {
				SET_RESULT(RESULT_ERROR_NOTSET, "Extrapolation Method not set up.");
				// Do nothing
			}
		}
	};
}
#endif
