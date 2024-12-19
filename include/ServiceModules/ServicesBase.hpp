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
* \b Copyright: University of Malaga
* 
* \b Contributions: (additional authors/contributors can be added here)
*
* \b Project: 3D Tune-In (https://www.3dtunein.eu) and SONICOM (https://www.sonicom.eu/) ||
*
* \b Acknowledgement: This project has received funding from the European Union's Horizon 2020 research and innovation programme under grant agreements no. 644051 and no. 101017743
* 
* This class is part of the Binaural Rendering Toolbox (BRT), coordinated by A. Reyes-Lecuona (areyes@uma.es) and L. Picinali (l.picinali@imperial.ac.uk)
* Code based in the 3DTI Toolkit library (https://github.com/3DTune-In/3dti_AudioToolkit).
* 
* \b Licence: This program is free software, you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
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

#define POSITION_RESOLUTION				0.01
#define POSITION_RESOLUTION_INVERSE		1/POSITION_RESOLUTION		// For faster operation, in case the compiler does not optimise the division

#define EPSILON_SEWING 0.001

// Structs and types definitions 

/** \brief Defines and holds data to work with orientations
*/
struct orientation
{
	double azimuth;					///< Azimuth angle in degrees
	double elevation;				///< Elevation angle in degrees	
	double distance;				///< Distance in meters	
	
	orientation(double _azimuth, double _elevation) :azimuth{ _azimuth }, elevation{ _elevation }, distance{ 0.0 } {}
	orientation(double _azimuth, double _elevation, double _distance) :azimuth{ _azimuth }, elevation{ _elevation }, distance{_distance} {}
	
	orientation() :orientation{ 0.0, 0.0, 0.0} {}
	bool operator==(const orientation& other) const
	{		
		return ((Common::AreSameDouble(this->azimuth, other.azimuth, ORIENTATION_RESOLUTION)) && (Common::AreSameDouble(this->elevation, other.elevation, ORIENTATION_RESOLUTION)));
	}
};

struct TVector3 {
	double x;
	double y;
	double z;
	
	TVector3() : x{ 0.0 }, y{ 0.0 }, z{ 0.0 } {}
	TVector3(double _x, double _y, double _z) : x{ _x }, y{ _y }, z{ _z } {}
	TVector3(Common::CVector3 _vector) : x{ _vector.x }, y{ _vector.y }, z{ _vector.z } {}
	
	bool operator==(const TVector3& other) const
	{
		return ((Common::AreSameDouble(this->x, other.x, ORIENTATION_RESOLUTION)) && (Common::AreSameDouble(this->y, other.y, ORIENTATION_RESOLUTION)) && (Common::AreSameDouble(this->z, other.z, ORIENTATION_RESOLUTION)));
	}
};

//struct TDuplaVector3 {
//	TVector3 listener;
//	TVector3 emitter;
//
//	TDuplaVector3() : listener{ 0.0, 0.0, 0.0 }, emitter{ 0.0, 0.0, 0.0 } {}
//	TDuplaVector3(TVector3 _listener, TVector3 _emitter) : listener{ _listener }, emitter{ _emitter } {}
//	TDuplaVector3(Common::CVector3 _listener, Common::CVector3 _emitter) : listener{ _listener }, emitter{ _emitter } {}
//	
//	bool operator==(const TDuplaVector3& other) const
//	{
//		return ((this->listener == other.listener) && (this->emitter == other.emitter));
//	}
//};

namespace std
{
	template<>
	struct hash<orientation>
	{
		// adapted from http://en.cppreference.com/w/cpp/utility/hash
		std::size_t operator()(const orientation& key) const
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

	inline void hash_combine(std::size_t& seed, const float& v) 
	{
		std::hash<float> hasher;
		seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	}

	template<>	
	struct hash<TVector3>
	{		
		std::size_t operator()(const TVector3& key) const
		{				
			size_t h = std::hash<float>()(key.x * POSITION_RESOLUTION_INVERSE);
			hash_combine(h, key.y * POSITION_RESOLUTION_INVERSE);
			hash_combine(h, key.z * POSITION_RESOLUTION_INVERSE);

			return h;
		}
	};

	/*template<>
	struct hash<TDuplaVector3>
	{
		std::size_t operator()(const TDuplaVector3& key) const
		{
			size_t h1 = std::hash<TVector3>()(key.listener);			
			size_t h2 = std::hash<TVector3>()(key.emitter);
			h1 ^= h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2);
			return h1;
		}
	};*/
	
}

namespace BRTServices {

	enum class TEXTRAPOLATION_METHOD {
		none,
		nearest_point,
		zero_insertion
	};
	/*const char EXTRAPOLATION_METHOD_NEARESTPOINT_STRING[]	= "NearestPoint";
	const char EXTRAPOLATION_METHOD_ZEROINSERTION_STRING[] = "ZeroInsertion";*/

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

	struct TSOSFilterStruct {
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

		virtual std::string GetLastError() { return ""; }

		virtual bool BeginSetup() { return true; }		
		virtual bool BeginSetup(int32_t _IRLength, BRTServices::TEXTRAPOLATION_METHOD _extrapolationMethod) { return false; }		
		virtual bool EndSetup() { return false; }

		virtual void SetGridSamplingStep(int _samplingStep) {};
		virtual void SetTitle(std::string _title) {}
		virtual void SetDatabaseName(std::string _databaseName) {}
		virtual void SetListenerShortName(std::string _listenerShortName) {};
		virtual void SetFilename(std::string _fileName) {}
		
		virtual void SetSamplingRate(int samplingRate) {};
		virtual void SetNumberOfEars(int _numberOfEars) {}
		virtual void SetEarPosition(Common::T_ear _ear, Common::CVector3 _earPosition) {};

		virtual void SetWindowingParameters(float _fadeInWindowThreshold, float _fadeInWindowRiseTime, float _fadeOutWindowThreshold, float _fadeOutWindowRiseTime) {};
		virtual void GetWindowingParameters(float& _fadeInWindowThreshold, float& _fadeInWindowRiseTime, float& _fadeOutWindowThreshold, float& _fadeOutWindowRiseTime) {};

		virtual void AddHRIR(double _azimuth, double _elevation, double _distance, Common::CVector3 listenerPosition, THRIRStruct&& newHRIR) {};
		//virtual void AddHRBRIR(double _azimuth, double _elevation, double _distance, Common::CVector3 listenerPosition, THRIRStruct&& newHRBRIR) {}
		virtual void AddCoefficients(float azimuth, float distance, TSOSFilterStruct&& newCoefs) {}
		virtual void AddDirectivityTF(float _azimuth, float _elevation, TDirectivityTFStruct&& DirectivityTF) {}
		
		virtual void AddImpulseResponse(int channel, const THRIRStruct&& newIR) {}		
		virtual void AddImpulseResponse(int channel, const THRIRPartitionedStruct&& newPartitionedIR) {}

		virtual int32_t GetHRIRLength() const { return 0; }
		virtual const int32_t GetHRIRNumberOfSubfilters() const { return 0; }
		virtual const int32_t GetHRIRSubfilterLength() const { return 0; }
		virtual float GetHeadRadius() { return 0.0f; };
		virtual Common::CVector3 GetEarLocalPosition(Common::T_ear _ear) { return Common::CVector3(); }
		virtual float GetHRTFDistanceOfMeasurement() { return 0; }
				
		virtual const std::vector<CMonoBuffer<float>> GetHRIRPartitioned(Common::T_ear ear, float _azimuth, float _elevation, bool runTimeInterpolation, const Common::CTransform& _listenerLocation) const
		{
			return std::vector < CMonoBuffer<float>>();
		};
		
		virtual THRIRPartitionedStruct GetHRIRDelay(Common::T_ear ear, float _azimuthCenter, float _elevationCenter, bool runTimeInterpolation,	Common::CTransform& _listenerLocation) {
			return THRIRPartitionedStruct();
		};	

		virtual std::vector <Common::CVector3> GetListenerPositions() {	return std::vector <Common::CVector3>{Common::CVector3()};	}
	};}

#endif
