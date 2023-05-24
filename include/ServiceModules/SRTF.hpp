/**
* \class CSRTF
*
* \brief Declaration of CSRTF class interface
* \version 
* \date	May 2023
*
* \authors 3DI-DIANA Research Group (University of Malaga), in alphabetical order: M. Cuevas-Rodriguez, D. Gonzalez-Toledo, L. Molina-Tanco ||
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
* \b Acknowledgement: This project has received funding from the European Union’s Horizon 2020 research and innovation programme under grant agreement no.101017743
*/


#ifndef _CSRTF_H_
#define _CSRTF_H_

//#include <unordered_map>
//#include <vector>
//#include <utility>
//#include <list>
//#include <cstdint>
//#include <Base/Listener.hpp>
//#include <Common/Buffer.h>
#include <Common/ErrorHandler.hpp>
//#include <Common/Fprocessor.h>
#include <Common/GlobalParameters.hpp>
#include <Common/CommonDefinitions.h>
#include <ServiceModules/ServiceModuleInterfaces.hpp>


//#ifndef PI 
//#define PI 3.14159265
//#endif

#ifndef DEFAULT_SRTFRESAMPLING_STEP
#define DEFAULT_SRTF_RESAMPLING_STEP 5
#endif



/*! \file */

// Structs and types definitions 

/** \brief Defines and holds data to work with orientations
*/
//struct orientation
//{
//	float azimuth;		///< Azimuth angle in degrees
//	float elevation;	///< Elevation angle in degrees	
//	orientation(float _azimuth, float _elevation) :azimuth{ _azimuth }, elevation{ _elevation } {}
//	orientation() :orientation{ 0,0 } {}
//	bool operator==(const orientation& other) const
//	{
//		return ((Common::AreSame(this->azimuth, other.azimuth, ORIENTATION_RESOLUTION)) && (Common::AreSame(this->elevation, other.elevation, ORIENTATION_RESOLUTION)));
//	}
//};

///** \brief Type definition for a left-right pair of impulse response with the ITD removed and stored in a specific struct field
//*/
//struct THRIRStruct {
//	uint64_t leftDelay;				///< Left delay, in number of samples
//	uint64_t rightDelay;			///< Right delay, in number of samples
//	CMonoBuffer<float> leftHRIR;	///< Left impulse response data
//	CMonoBuffer<float> rightHRIR;	///< Right impulse response data
//};

/** \brief Type definition for a left-right pair of impulse response subfilter set with the ITD removed and stored in a specific struct field
*/
//struct THRIRPartitionedStruct {
//	uint64_t leftDelay;				///< Left delay, in number of samples
//	uint64_t rightDelay;			///< Right delay, in number of samples
//	std::vector<CMonoBuffer<float>> leftHRIR_Partitioned;	///< Left partitioned impulse response data
//	std::vector<CMonoBuffer<float>> rightHRIR_Partitioned;	///< Right partitioned impulse response data
//};



//namespace std
//{
//	template<>
//	struct hash<orientation>
//	{
//		// adapted from http://en.cppreference.com/w/cpp/utility/hash
//		size_t operator()(const orientation& key) const
//		{
//			int keyAzimuth_hundredth = static_cast<int> (round(key.azimuth / ORIENTATION_RESOLUTION));
//			int keyElevation_hundredth = static_cast<int> (round(key.elevation / ORIENTATION_RESOLUTION));
//
//			size_t h1 = std::hash<int32_t>()(keyAzimuth_hundredth);
//			size_t h2 = std::hash<int32_t>()(keyElevation_hundredth);
//			return h1 ^ (h2 << 1);  // exclusive or of hash functions for each int.
//		}
//	};
//}



/** \brief Type definition for the HRTF table
*/
//typedef std::unordered_map<orientation, BRTServices::THRIRStruct> T_HRTFTable;

namespace BRTBase { /*class CListener;*/ }

namespace BRTServices
{
	/** \details This class gets impulse response data to compose HRTFs and implements different algorithms to interpolate the HRIR functions.
	*/
	class CSRTF : public CServicesBase
	{
	public:
		/** \brief Default Constructor
		*	\details By default, customized ITD is switched off, resampling step is set to 5 degrees and listener is a null pointer
		*   \eh Nothing is reported to the error handler.
		*/
		CSRTF():resamplingStep{ DEFAULT_SRTF_RESAMPLING_STEP }
			
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

		/// <summary>
		/// 
		/// </summary>
		void BeginSetup()
		{}

		/// <summary>
		/// 
		/// </summary>
		/// <returns></returns>
		bool EndSetup()
		{
			return true;
		}

		void SetResamplingStep(int _resamplingStep) {
			resamplingStep = _resamplingStep;
		}

		int GetResamplingStep() {
			return resamplingStep;
		}

	private:
		std::string title;
		std::string databaseName;
		std::string fileName;
		int resamplingStep; 		

	};
}
#endif
