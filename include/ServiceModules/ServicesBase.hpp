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
#include <unordered_map>
#include <vector>
#include <Common/ErrorHandler.hpp>
#include <Common/Buffer.hpp>

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

#define EPSILON_SEWING 0.001

///////////////////////////////////
// Structs and types definitions 
//////////////////////////////////
// Resolutions
constexpr double AZIMUTH_ELEVATION_RESOLUTION = 0.01;		// 0.01 deg
constexpr double AZIMUTH_ELEVATION_RESOLUTION_INV = 100.0;	// 1 / 0.01 deg
constexpr double DISTANCE_RESOLUTION = 0.001;				// 1 mm
constexpr double DISTANCE_RESOLUTION_INV = 1000.0;			// 1 / 0.001 m (mm)

/**
 * @brief Combine a hash value with the hash of a new value
 * @tparam T type of the value to hash
 * @param seed current hash value
 * @param v new value to combine into the hash
 */
template <class T>
inline void hash_combine(std::size_t & seed, const T & v) {
	seed ^= std::hash<T> {}(v) + 0x9e3779b9u + (seed << 6) + (seed >> 2);
}

/**
 * @brief Quantise an azimuth or elevation angle in degrees to an integer representation with 0.01 degree resolution. 
		Assumes input is already normalised to [0,360).
 * @param deg_0_360 Angle in degrees in range [0,360)
 * @return Quantised angle as integer in range [0,36000)
 */
inline int32_t quantise_azel_0p01(double deg_0_360) {	
	int32_t q = static_cast<int32_t>(std::llround(deg_0_360 * AZIMUTH_ELEVATION_RESOLUTION_INV));
	return (q == 36000) ? 0 : q;
}

/**
 * @brief Quantise a distance in meters to an integer representation in millimetres.
 * @param meters Distance in meters
 * @return Quantised distance as integer in millimetres
 */
inline int32_t quantise_dist_mm(double meters) {
	return static_cast<int32_t>(std::llround(meters * DISTANCE_RESOLUTION_INV));
}

/** \brief Defines and holds data to work with orientations
*/
struct TOrientation {
	double azimuth; ///< Azimuth angle in degrees
	double elevation; ///< Elevation angle in degrees
	double distance; ///< Distance in meters

	// TODO : delete this - No distance version if not needed
	TOrientation(double _azimuth, double _elevation)
		: azimuth { _azimuth }
		, elevation { _elevation }
		, distance { 0.0 } { }
	TOrientation(double _azimuth, double _elevation, double _distance)
		: azimuth { _azimuth }
		, elevation { _elevation }
		, distance { _distance } { }

	TOrientation()
		: TOrientation { 0.0, 0.0, 0.0 } { }
	
	bool operator==(const TOrientation & other) const {
		return	(Common::AreSameDouble(this->azimuth, other.azimuth, AZIMUTH_ELEVATION_RESOLUTION)) && 
				(Common::AreSameDouble(this->elevation, other.elevation, AZIMUTH_ELEVATION_RESOLUTION)) /*&& 
				(Common::AreSameDouble(this->distance, other.distance, DISTANCE_RESOLUTION))*/;
	}
};


struct TOrientation_key {
	int32_t azimuth_q;
	int32_t elevation_q;
	int32_t distance_q;

	TOrientation_key()
		: azimuth_q { 0 }
		, elevation_q { 0 }
		, distance_q { 0 } { }
	TOrientation_key(TOrientation o)
		: azimuth_q { quantise_azel_0p01(o.azimuth) }
		, elevation_q { quantise_azel_0p01(o.elevation) }
		, distance_q { quantise_dist_mm(o.distance) } { }
	TOrientation_key(double _azimuth, double _elevation)
		: azimuth_q { quantise_azel_0p01(_azimuth) }
		, elevation_q { quantise_azel_0p01(_elevation) }
		, distance_q { 0 } { }
	TOrientation_key(double _azimuth, double _elevation, double _distance)
		: azimuth_q { quantise_azel_0p01(_azimuth) }
		, elevation_q { quantise_azel_0p01(_elevation) }
		, distance_q { quantise_dist_mm(_distance) } { }
		
	bool operator==(const TOrientation_key & o) const {
		return azimuth_q == o.azimuth_q && elevation_q == o.elevation_q/* && distance_q == o.distance_q*/;
	}
};


namespace std {
	template <>
	struct std::hash<TOrientation_key> {
		std::size_t operator()(const TOrientation_key & k) const {
			std::size_t seed = 0;
			hash_combine(seed, k.azimuth_q);
			hash_combine(seed, k.elevation_q);			
			return seed;
		}
	};

	// TODO Remove this
	template<>
	struct std::hash<TOrientation>
	{
		// adapted from http://en.cppreference.com/w/cpp/utility/hash
		std::size_t operator()(const TOrientation& key) const
		{
			int keyAzimuth_hundredth = static_cast<int>(std::round(key.azimuth * AZIMUTH_ELEVATION_RESOLUTION_INV));
			int keyElevation_hundredth = static_cast<int>(std::round(key.elevation * AZIMUTH_ELEVATION_RESOLUTION_INV));			

			size_t h1 = std::hash<int32_t>()(keyAzimuth_hundredth);
			size_t h2 = std::hash<int32_t>()(keyElevation_hundredth);
			return h1 ^ (h2 << 1);  // exclusive or of hash functions for each int.			
		}
	};
	}


/////
	// TODO Remove this struct if not needed
	struct TVector3 {
	double x;
	double y;
	double z;
	
	TVector3() : x{ 0.0 }, y{ 0.0 }, z{ 0.0 } {}
	TVector3(double _x, double _y, double _z) : x{ _x }, y{ _y }, z{ _z } {}
	TVector3(Common::CVector3 _vector) : x{ _vector.x }, y{ _vector.y }, z{ _vector.z } {}
	
	bool operator==(const TVector3& other) const
	{
		return (Common::AreSameDouble(this->x, other.x, AZIMUTH_ELEVATION_RESOLUTION)) && 
			(Common::AreSameDouble(this->y, other.y, AZIMUTH_ELEVATION_RESOLUTION)) && 
			(Common::AreSameDouble(this->z, other.z, DISTANCE_RESOLUTION));
	}
};

struct TVector3_key {
	int32_t x_q;
	int32_t y_q;
	int32_t z_q;	
	
	TVector3_key()
		: x_q { 0 }
		, y_q { 0 }
		, z_q { 0 } { }
	TVector3_key(double _x, double _y, double _z)
		: x_q { quantise_dist_mm(_x) }
		, y_q { quantise_dist_mm(_y) }
		, z_q { quantise_dist_mm(_z) } { }
	TVector3_key(Common::CVector3 _vector)
		: x_q { quantise_dist_mm(_vector.x) }
		, y_q { quantise_dist_mm(_vector.y) }
		, z_q { quantise_dist_mm(_vector.z) } { }
	
	bool operator==(const TVector3_key & v) const {
		return x_q == v.x_q && y_q == v.y_q && z_q == v.z_q;
	}
};


namespace std
{
	template<>	
	struct hash<TVector3>
	{		
		std::size_t operator()(const TVector3& key) const
		{				
			size_t h = std::hash<float>()(key.x * DISTANCE_RESOLUTION_INV);
			hash_combine(h, key.y * DISTANCE_RESOLUTION_INV);
			hash_combine(h, key.z * DISTANCE_RESOLUTION_INV);

			return h;
		}
	};	
	template <>
	struct std::hash<TVector3_key> {
		std::size_t operator()(const TVector3_key & k) const {
			std::size_t seed = 0;
			hash_combine(seed, k.x_q);
			hash_combine(seed, k.y_q);
			hash_combine(seed, k.z_q);
			return seed;
		}
	};

}

namespace BRTServices {

	enum class TEXTRAPOLATION_METHOD {
		none,
		nearest_point,
		zero_insertion
	};
	

	
	//TODO delete this struct 
	struct THRIRStruct {
		uint64_t leftDelay;				///< Left delay, in number of samples
		uint64_t rightDelay;			///< Right delay, in number of samples
		CMonoBuffer<float> leftHRIR;	///< Left impulse response data
		CMonoBuffer<float> rightHRIR;	///< Right impulse response data

		THRIRStruct() : leftDelay{0}, rightDelay{0} {}
	};

		
	struct TIRStruct {
		TOrientation orientation; ///< Original orientation of the IR		
		
		Common::CEarPair <uint64_t> delay; ///< Delay, in number of samples
		Common::CEarPair <CMonoBuffer<float>> IR; ///< Impulse response data

		TIRStruct() 
			: delay { 0, 0 }
		{}
		TIRStruct(const TOrientation & o)
			: orientation { o }
			, delay { 0, 0} { }

		TIRStruct(const TOrientation & o, const Common::CEarPair<uint64_t> & _delay, const Common::CEarPair<CMonoBuffer<float>> & _ir)
			: orientation { o }
		{
			delay.left = _delay.left;
			delay.right = _delay.right;
			IR.left = _ir.left;
			IR.right = _ir.right;
		}
		TIRStruct(const TOrientation & o, Common::CEarPair<uint64_t> && _delay, Common::CEarPair<CMonoBuffer<float>> && _ir)
			: orientation { o }
		{
			delay.left = _delay.left;
			delay.right = _delay.right;
			IR.left = std::move(_ir.left);
			IR.right = std::move(_ir.right);
		}
		TIRStruct(const TOrientation & o, const uint64_t & leftDelay, CMonoBuffer<float> && leftIR, const uint64_t & rightDelay, CMonoBuffer<float> && rightIR)
			: orientation { o }
		{
			delay.left = leftDelay;
			delay.right = rightDelay;
			IR.left = std::move(leftIR);
			IR.right = std::move(rightIR);
		}
		TIRStruct(const TOrientation & o, THRIRStruct && s)
			: orientation { o }
		{
			delay.left = s.leftDelay;
			delay.right = s.rightDelay;
			IR.left = std::move(s.leftHRIR);
			IR.right = std::move(s.rightHRIR);
		}		
	};
	
	using TFRPartitions = std::vector<CMonoBuffer<float>>; 

	struct TFRPartitionedStruct { 
		TOrientation orientation;			///< Orientation of the FR
		Common::CEarPair<uint64_t> delay;	///< Delay, in number of samples
		Common::CEarPair<TFRPartitions> IR; ///< Impulse response dataa		
		TFRPartitionedStruct() 
			: delay { 0, 0 } 
		{ }
	};
		
	/** \brief Type definition for a left-right pair of impulse response subfilter set with the ITD removed and stored in a specific struct field
	*/
	struct THRIRPartitionedStruct {
		TOrientation orientation; ///< Original orientation of the IR
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
	
	enum class TServiceType {
		none,
		hrir_database,
		hrir_database_interpolated,
		brir_database,
		ir_database,
		sos_filter_database,
		directivity_tf_database
	};

	/**
	 * @brief 
	 */
	class CServicesBase {
		
	public:
		CServicesBase()
			: serviceType { TServiceType::none }
			, dataReady { false }
			, spatiallyOriented { false }						
			, impulseResponseLength { 0 }
			, title { "" }
			, fileName { "" }
			, databaseName { "" }
			, listenerShortName { "" }
			, samplingRate { -1 }
		{}
		virtual ~CServicesBase() {}		

		virtual std::string GetLastError() { return ""; }
		
		virtual bool BeginSetup() { return false; }		
		virtual bool BeginSetup(const int32_t& _IRLength,const BRTServices::TEXTRAPOLATION_METHOD & _extrapolationMethod) { return false; }
		virtual bool EndSetup() { return false; }

		virtual void SetGridSamplingStep(int _samplingStep) {}
		virtual int GetGridSamplingStep() const { return 0; }
		
		virtual void SetNumberOfEars(int& _numberOfEars) {}
		virtual int GetNumberOfEars() { return 0; }
		
		virtual void SetHeadRadius(float _headRadius)  {};
		virtual float GetHeadRadius() { return 0.0f; }
		virtual void RestoreHeadRadius() { }
		
		virtual void SetEarPosition(Common::T_ear _ear, Common::CVector3 _earPosition) { };
		virtual Common::CVector3 GetEarLocalPosition(Common::T_ear _ear) { return Common::CVector3(); }
		
		virtual void SetCranialGeometryAsDefault() {};

		virtual void EnableWoodworthITD() { };
		virtual void DisableWoodworthITD() { };
		virtual bool IsWoodworthITDEnabled() { return false; }

		virtual void SetWindowingParameters(float _fadeInBegin, float _riseTime, float _fadeOutCutoff, float _fallTime) {};
		virtual void GetWindowingParameters(float & _fadeInBegin, float & _riseTime, float & _fadeOutCutoff, float & _fallTime) {};

		virtual void AddHRIR(double _azimuth, double _elevation, double _distance, Common::CVector3 listenerPosition, THRIRStruct&& newHRIR) {};		
		
		virtual void AddDirectivityTF(float _azimuth, float _elevation, TDirectivityTFStruct&& DirectivityTF) {}

		//virtual void AddIR(const double & _azimuth, const double & _elevation, const double& _distance, const Common::CVector3& referencePosition, THRIRStruct && newHRIR) { };		

		//virtual void AddImpulseResponse(int channel, const THRIRStruct&& newIR) {}		
		//virtual void AddImpulseResponse(int channel, const THRIRPartitionedStruct&& newPartitionedIR) {}
		
		virtual int32_t GetIRLength() const { return impulseResponseLength; }
		//virtual const int32_t GetHRIRNumberOfSubfilters() const { return 0; } // To be removed
		//virtual const int32_t GetHRIRSubfilterLength() const { return 0; } // To be removed
		
		virtual const int32_t GetTFNumberOfSubfilters() const { return 0; }
		virtual const int32_t GetTFSubfilterLength() const { return 0; }

		
		
		//virtual float GetHRTFDistanceOfMeasurement() { return 0; }
		//virtual double GetDistanceOfMeasurement() const { return 0; }
		virtual double GetDistanceOfMeasurement(const Common::CTransform & _referenceLocation, const double & _azimuth, const double & _elevation, const double & _distance) const { return 0; }
		
		virtual std::vector<Common::CVector3> GetListenerPositions() const {
			return std::vector<Common::CVector3> { Common::CVector3() };
		}

		virtual std::vector<float> GetSOSFilterCoefficients(Common::T_ear ear, float distance_m, float azimuth) { 
			return std::vector<float>();
		}

		virtual const CMonoBuffer<float> GetIRTimeDomain(const float & _azimuth, const float & _elevation, const float & _distance, const Common::T_ear & ear) const {
			return CMonoBuffer<float>();
		}

		virtual const std::vector<CMonoBuffer<float>> GetIRTFPartitioned(const Common::T_ear & ear) const {	return std::vector<CMonoBuffer<float>>();}
		virtual const std::vector<CMonoBuffer<float>> GetIRTFPartitionedSpatiallyOriented(const float & _azimuth, const float & _elevation, const Common::T_ear & ear, bool _findNearest) const	{ return std::vector<CMonoBuffer<float>>();	}
						
		virtual void GetIRTFPartitioned2Ears(std::vector<CMonoBuffer<float>> & leftEarIRTF, std::vector<CMonoBuffer<float>> & rightEarIRTF) const { }
		virtual void GetIRTFPartitionedSpatiallyOriented2Ears(std::vector<CMonoBuffer<float>> & leftEarIRTF, std::vector<CMonoBuffer<float>> & rightEarIRTF, const float & _azimuth, const float & _elevation, bool _findNearest) const { }

		virtual const std::vector<CMonoBuffer<float>> GetHRIRPartitioned(Common::T_ear ear, float _azimuth, float _elevation, bool runTimeInterpolation, const Common::CTransform& _listenerLocation) const
		{
			return std::vector < CMonoBuffer<float>>();
		};
		
		virtual THRIRPartitionedStruct GetHRIRDelay(Common::T_ear ear, float _azimuthCenter, float _elevationCenter, bool runTimeInterpolation,	Common::CTransform& _listenerLocation) {
			return THRIRPartitionedStruct();
		};	

		// NEW
		virtual void AddIR(const Common::CVector3 & referencePosition, const double & _azimuth, const double & _elevation, const double & _distance, THRIRStruct && newHRBRIR) { }
		virtual void AddCoefficients(float azimuth, float distance, TSOSFilterStruct && newCoefs) { }

		virtual const TFRPartitions GetFR_SpatiallyOriented(const float & _azimuth, const float & _elevation, const float & _distance, const Common::CTransform & _referenceLocation, const Common::T_ear & ear, bool _findNearest) const { return TFRPartitions(); }
		virtual const Common::CEarPair<TFRPartitions> GetFR_SpatiallyOriented_2Ears(const float & _azimuth, const float & _elevation, const float & _distance, const Common::CTransform & _referenceLocation, bool _findNearest) const { return Common::CEarPair<TFRPartitions>(); }
		virtual const Common::CEarPair<TFRPartitions> GetFR_2Ears() const { return Common::CEarPair<TFRPartitions>(); }

		virtual const Common::CEarPair<uint64_t> GetFR_Delay(const float & _azimuthCenter, const float & _elevationCenter, const float & _distance, const Common::CTransform & _referenceLocation, bool _findNearest) const { return Common::CEarPair<uint64_t> { 0, 0 }; }
		
		virtual std::vector<Common::CVector3> GetReferencePositions() const {	return std::vector<Common::CVector3> { Common::CVector3() };}

		//////////////////////
		// Public Methods
		//////////////////////
		void SetID(const std::string & _ID) { ID = _ID; }
		std::string GetID() { return ID; }
		
		bool IsDataReady() { return dataReady; }

		bool IsSpatiallyOriented() const { return spatiallyOriented; }
		
		void SetServiceType(const TServiceType & _serviceType) { serviceType = _serviceType; }
		TServiceType GetServiceType() const { return serviceType; }

		void SetTitle(const std::string & _title) {	title = _title;	}		
		std::string GetTitle() {return title;	}

		/** \brief Set the file name of the SOFA file
		*    \param [in]	_fileName		string contains filename
		*/
		void SetFilename(const std::string & _fileName) { fileName = _fileName;	}

		/** \brief Get the file name of the SOFA file
		* \return string contains filename
		*/
		std::string GetFilename() { return fileName; };

		/** \brief Set the name of the database of the SOFA file
		*    \param [in]	_title		string contains title
		*/
		void SetDatabaseName(const std::string & _databaseName) { 
			databaseName = _databaseName;
		}

		/** \brief Get the name of the database of the SOFA file
		* \return string contains title 
		*/
		std::string GetDatabaseName() { 
			return databaseName;
		}

		/**
		 * @brief set the listener short name from the SOFA file
		 * @param _listenerShortName 
		 */
		void SetListenerShortName(const std::string & _listenerShortName) { 
			listenerShortName = _listenerShortName;
		}
		std::string GetListenerShortName() { 
			return listenerShortName;
		}

		/** \brief Set the sampling rate for the HRTF
		*	\param [in] sampling rate
		*/
		void SetSamplingRate(int _samplingRate) {
			samplingRate = _samplingRate;
		}

		/** \brief Ask for the sampling rate
		*	\retval sampling step
		*/
		int GetSamplingRate() {
			return samplingRate;
		}

	protected:		
		TServiceType serviceType;
		int samplingRate;				// Sampling rate of the IR
		bool dataReady;					// Variable that indicates if the HRTF has been loaded correctly
		int32_t impulseResponseLength;	// IR vector length
		bool spatiallyOriented;			// Variable that indicates if the IRs are spatially oriented, if the are IR for different azimuths and elevations or not. 

	private:
		std::string ID;
		std::string title;
		std::string fileName;
		std::string databaseName;
		std::string listenerShortName;
	};
}

#endif
