/**
* \class CServicesBase
*
* \brief Declaration of CServicesBase class
* \date	June 2023
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
* \b Licence: This program is free software, you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
*
* \b Acknowledgement: This project has received funding from the European Union�s Horizon 2020 research and innovation programme under grant agreement no.101017743
*/

#ifndef _SERVICE_INTERFACES_H_
#define _SERVICE_INTERFACES_H_

#include <cstdint>

#define MAX_DISTANCE_BETWEEN_ELEVATIONS 5
#define NUMBER_OF_PARTS 4 
#define MARGIN 10
#define ELEVATION_NORTH_POLE 90
#define ELEVATION_SOUTH_POLE 270

#define DEFAULT_GAP_THRESHOLD 10

#define SPHERE_BORDER 360.0f

#define DEFAULT_MIN_AZIMUTH 0.0f
#define DEFAULT_MAX_AZIMUTH 360.0f
#define DEFAULT_MIN_ELEVATION 0.0f
#define DEFAULT_MAX_ELEVATION 360.0f

#define ORIENTATION_RESOLUTION			0.01
#define ORIENTATION_RESOLUTION_INVERSE	1/ORIENTATION_RESOLUTION		// For faster operation, in case the compiler does not optimise the division

#define EPSILON_SEWING 0.001

// Structs and types definitions 

/** \brief Defines and holds data to work with orientations
*/
struct orientation
{
	double azimuth;					///< Azimuth angle in degrees
	double elevation;				///< Elevation angle in degrees	
	//Common::CVector3 cartessianPos; ///< Position in X, Y and Z
	orientation(double _azimuth, double _elevation) :azimuth{ _azimuth }, elevation{ _elevation } {}
	//orientation(double _azimuth, double _elevation, Common::CVector3 _cartessianPos) :azimuth{ _azimuth }, elevation{ _elevation }, cartessianPos{ _cartessianPos } {}
	//orientation(float _azimuth, float _elevation) :azimuth{ static_cast<double>(_azimuth) }, elevation{ static_cast<double>(_elevation) } {}
	//orientation(float _azimuth, float _elevation, Common::CVector3 _cartessianPos) :azimuth{ static_cast<double>(_azimuth) }, elevation{ static_cast<double>(_elevation) }, cartessianPos{ _cartessianPos } {}
	orientation() :orientation{ 0.0, 0.0 } {}
	bool operator==(const orientation& other) const
	{		
		return ((Common::AreSameDouble(this->azimuth, other.azimuth, ORIENTATION_RESOLUTION)) && (Common::AreSameDouble(this->elevation, other.elevation, ORIENTATION_RESOLUTION)));
	}
};

//struct orientation
//{
//	float azimuth;					///< Azimuth angle in degrees
//	float elevation;				///< Elevation angle in degrees	
//	Common::CVector3 cartessianPos; ///< Position in X, Y and Z
//	orientation(float _azimuth, float _elevation) :azimuth{ _azimuth }, elevation{ _elevation } {}
//	orientation(float _azimuth, float _elevation, Common::CVector3 _cartessianPos) :azimuth{ _azimuth }, elevation{ _elevation }, cartessianPos{ _cartessianPos } {}
//	orientation() :orientation{ 0,0 } {}
//	bool operator==(const orientation& other) const
//	{
//		return ((Common::AreSame(this->azimuth, other.azimuth, ORIENTATION_RESOLUTION)) && (Common::AreSame(this->elevation, other.elevation, ORIENTATION_RESOLUTION)));
//	}
//};

namespace std
{
	template<>
	struct hash<orientation>
	{
		// adapted from http://en.cppreference.com/w/cpp/utility/hash
		size_t operator()(const orientation& key) const
		{			
			//int keyAzimuth_hundredth	= static_cast<int> (round(key.azimuth / ORIENTATION_RESOLUTION));
			//int keyElevation_hundredth	= static_cast<int> (round(key.elevation / ORIENTATION_RESOLUTION));
			
			int keyAzimuth_hundredth	= static_cast<int> (std::round(key.azimuth * ORIENTATION_RESOLUTION_INVERSE));
			int keyElevation_hundredth	= static_cast<int> (std::round(key.elevation * ORIENTATION_RESOLUTION_INVERSE));
			

			size_t h1 = std::hash<int32_t>()(keyAzimuth_hundredth);
			size_t h2 = std::hash<int32_t>()(keyElevation_hundredth);
			return h1 ^ (h2 << 1);  // exclusive or of hash functions for each int.
		}
	};
}

namespace BRTServices {

	const char EXTRAPOLATION_METHOD_NEARESTPOINT_STRING[]	= "NearestPoint";
	const char EXTRAPOLATION_METHOD_ZEROINSERTION_STRING[] = "ZeroInsertion";

	/** \brief Type definition for a left-right pair of impulse response with the ITD removed and stored in a specific struct field
	*/
	struct THRIRStruct {
		uint64_t leftDelay;				///< Left delay, in number of samples
		uint64_t rightDelay;			///< Right delay, in number of samples
		CMonoBuffer<float> leftHRIR;	///< Left impulse response data
		CMonoBuffer<float> rightHRIR;	///< Right impulse response data

		THRIRStruct() : leftDelay{0}, rightDelay{0} {}
	};

	/** \brief Type definition for a left-right pair of impulse response subfilter set with the ITD removed and stored in a specific struct field
	*/
	struct THRIRPartitionedStruct {
		uint64_t leftDelay;				///< Left delay, in number of samples
		uint64_t rightDelay;			///< Right delay, in number of samples
		std::vector<CMonoBuffer<float>> leftHRIR_Partitioned;	///< Left partitioned impulse response data
		std::vector<CMonoBuffer<float>> rightHRIR_Partitioned;	///< Right partitioned impulse response data

		THRIRPartitionedStruct() : leftDelay{ 0 }, rightDelay{ 0 } {}
	};

	struct TNFCFilterStruct {
		CMonoBuffer<float> leftCoefs;	///< Left filters coefs
		CMonoBuffer<float> rightCoefs;	///< Right filters coefs
	};

	struct TDirectivityTFStruct {
		CMonoBuffer<float> realPart;
		CMonoBuffer<float> imagPart;
	};

	class CServicesBase {
		
	public:
		CServicesBase() {}
		virtual ~CServicesBase() {}		
		
		virtual void BeginSetup() {}
		virtual void BeginSetup(int32_t _DirectivityTFLength, std::string extrapolationMethod) {}
		virtual void BeginSetup(int32_t _HRIRLength, float _distance, std::string extrapolationMethod) {}
		virtual bool EndSetup() { return false; }

		virtual void SetResamplingStep(int _resamplingStep) {};
		virtual void SetTitle(std::string _title) {}
		virtual void SetDatabaseName(std::string _databaseName) {}
		virtual void SetListenerShortName(std::string _listenerShortName) {};
		virtual void SetFilename(std::string _fileName) {}
		
		virtual void SetSamplingRate(int samplingRate) {};
		virtual void SetNumberOfEars(int _numberOfEars) {}
		virtual void SetEarPosition(Common::T_ear _ear, Common::CVector3 _earPosition) {};

		virtual void AddHRIR(float _azimuth, float _elevation, THRIRStruct&& newHRIR) {}
		virtual void AddHRIR(double _azimuth, double _elevation, THRIRStruct&& newHRIR) {}
		virtual void AddCoefficients(float azimuth, float distance, TNFCFilterStruct&& newCoefs) {}
		virtual void AddDirectivityTF(float _azimuth, float _elevation, TDirectivityTFStruct&& DirectivityTF) {}
		
		virtual void AddImpulseResponse(int channel, const THRIRStruct&& newIR) {}		
		virtual void AddImpulseResponse(int channel, const THRIRPartitionedStruct&& newPartitionedIR) {}
	};}

#endif