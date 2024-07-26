#ifndef _CIR_WINDOWING_HPP_
#define _CIR_WINDOWING_HPP_

#include <Common/Buffer.hpp>
#include <Common/ErrorHandler.hpp>

namespace Common {
	
	class CIRWindowing
	{
	public:
		
		enum TWindowType {fadein, fadeout};
		
		CIRWindowing()
			/*: windowType {TWindowType::fadein}, windowThreshold { -1 }, windowRiseTime { -1 } */
		{
			
		};
		
		/**
		 * @brief 
		 * @param _windowType 
		 * @param _windowThreshold 
		 * @param _windowRiseTime 
		 */
		/*void Setup(TWindowType _windowType, float _windowThreshold, float _windowRiseTime) { 
			windowType = _windowType;
			windowThreshold = _windowThreshold;
			windowRiseTime = _windowRiseTime;
		}*/
		
		/**
		 * @brief 
		 * @param _inputIR 
		 * @return 
		 */
		/*CMonoBuffer<float> Proccess(const CMonoBuffer<float> & _inputIR, TWindowType _windowType, float _windowThreshold, float _windowRiseTime, float SampleRate) { 
			if (windowType == TWindowType::fadein) {
				return CalculateFadeInWindowingIR(_inputIR);
			}
			else {
				return CalculateFadeOutWindowingIR(_inputIR);
			}		
		}*/
		
		static CMonoBuffer<float> Proccess(const CMonoBuffer<float> & _inputIR, TWindowType _windowType, float _windowThreshold, float _windowRiseTime, float _sampleRate) {
			if (_windowType == TWindowType::fadein) {
				return CalculateFadeInWindowingIR(_inputIR, _windowThreshold, _windowRiseTime, _sampleRate);
			} else {
				return CalculateFadeOutWindowingIR(_inputIR, _windowThreshold, _windowRiseTime, _sampleRate);
			}
		}
	

	private:
		
		// Methods
		
		/**
		 * @brief Calculate the windowing of the IR
		 * @param _inputIR Input IR
		 * @return 
		 */
		static CMonoBuffer<float> CalculateFadeInWindowingIR(const CMonoBuffer<float> & _inputIR, float _windowThreshold, float _windowRiseTime, float _sampleRate) {
			// Vars to calculate the window
			int numberOfZeros = floor((_windowThreshold - _windowRiseTime / 2) * _sampleRate);
			int numberOfSamplesFadeIn = ceil(_windowRiseTime * _sampleRate);
			//int numberOfOnes = _inputIR.size() - numberOfZeros - numberOfSamplesFadeIn;

			// Check if the window is bigger than the IR
			if (numberOfZeros >= _inputIR.size()) {
				// If the window is bigger than the IR, we return the IR without windowing
				SET_RESULT(RESULT_WARNING, "The window is bigger than the IR, the IR will be returned without windowing.");
				return CMonoBuffer<float>(_inputIR);
			}

			// Create and fill first part of the window
			CMonoBuffer<float> windowedIR = CMonoBuffer<float>(numberOfZeros, 0);
			windowedIR.reserve(_inputIR.size());

			// Making the intermediate part with a raised cosine
			for (int i = numberOfZeros; i < numberOfZeros + numberOfSamplesFadeIn; i++) {
				windowedIR.push_back(_inputIR.at(i) * 0.5 * (1 - cos(M_PI * (i - numberOfZeros) / numberOfSamplesFadeIn)));
			}

			// Copy last samples
			windowedIR.insert(windowedIR.end(), _inputIR.begin() + numberOfZeros + numberOfSamplesFadeIn, _inputIR.end());

			return windowedIR;
		}


		/**
		 * @brief Calculate the windowing of the IR
		 * @param _inputIR Input IR
		 * @return 
		 */
		static CMonoBuffer<float> CalculateFadeOutWindowingIR(const CMonoBuffer<float> & _inputIR, float _windowThreshold, float _windowRiseTime, float _sampleRate) {
			// Vars to calculate the window
			int numberOfOnes = floor((_windowThreshold - _windowRiseTime / 2) * _sampleRate);
			int numberOfSamplesFadeIn = ceil(_windowRiseTime * _sampleRate);
			int numberOfZeros = _inputIR.size() - numberOfOnes - numberOfSamplesFadeIn;

			// Check if the window is bigger than the IR
			if (numberOfOnes >= _inputIR.size()) {
				// If the window is bigger than the IR, we return the IR without windowing
				SET_RESULT(RESULT_WARNING, "The window is bigger than the IR, the IR will be returned without windowing.");
				return CMonoBuffer<float>(_inputIR);
			}

			// Create and fill first part of the window
			CMonoBuffer<float> windowedIR;
			windowedIR.reserve(_inputIR.size());
			windowedIR.insert(windowedIR.end(), _inputIR.begin(), _inputIR.begin() + numberOfOnes);

			// Making the intermediate part with a raised cosine
			for (int i = numberOfOnes; i < numberOfOnes + numberOfSamplesFadeIn; i++) {
				windowedIR.push_back(_inputIR.at(i) * 0.5 * (1 + cos(M_PI * (i - numberOfOnes) / numberOfSamplesFadeIn)));
			}

			// Copy last samples
			//CMonoBuffer<float> zeros = CMonoBuffer<float>(numberOfZeros, 0);
			//windowedIR.insert(windowedIR.end(), zeros.begin(), zeros.end());

			return windowedIR;
		}
			
		// Attributes
		//TWindowType windowType;						// window type
		//float windowThreshold;						// window position
		//float windowRiseTime;						// window slope
		//Common::CGlobalParameters globalParameters;
	
	};


}

#endif