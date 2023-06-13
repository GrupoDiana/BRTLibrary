#ifndef COMMAND_HPP
#define COMMAND_HPP

#include <iostream>
#include <third_party_libraries/nlohmann/json.hpp>

namespace BRTBase {
	class CCommand { //TODO repensar esto, se puede almacenar el json ya parseado y se ahorra tiempo de proceso
		using json = nlohmann::json;
	public:
		json j;
		
		CCommand() { };
		CCommand(std::string _commandJsonString) : j{ json::parse(_commandJsonString) } {}
		
		bool isNull() { 
			return (j["address"].is_null() && j["command"].is_null());
		}

		std::string GetCommand() {			
			return GetStringParameter("command");
		}

		std::string GetAddress() {
			std::string _command = "";
			if (!j["address"].is_null() && j["address"].is_string()) {
				_command = j["address"].get<std::string>();
			}
			return _command;			
		}


		std::string GetStringParameter(std::string fieldName) {
			std::string _parameter = "";
			if (!j[fieldName].is_null() && j[fieldName].is_string()) {
				_parameter = j[fieldName].get<std::string>();
			}
			return _parameter;
		}

		int GetIntParameter(std::string fieldName) {
			int _parameter;
			if (!j[fieldName].is_null() && j[fieldName].is_number_integer()) {
				_parameter = j[fieldName].get<int>();
			}
			return _parameter;
		}

		float GetFloatParameter(std::string fieldName) {
			return (float)GetDoubleParameter(fieldName);
		}

		double GetDoubleParameter(std::string fieldName) {
			double _parameter = 0.0;
			std::vector<double> tempV;
			if (!j[fieldName].is_null() && j[fieldName].is_number_float()) {
				_parameter = j[fieldName];
			}
			else if (!j[fieldName].is_null() && j[fieldName].is_structured()) {
				tempV = j[fieldName].get<std::vector<double>>();
			}
			//TODO: is this necessary?
			if (tempV.size() > 0) { _parameter = tempV[0]; }
			return _parameter;
		}

		bool GetBoolParameter(std::string fieldName) {
			bool _parameter;
			if (!j[fieldName].is_null() && j[fieldName].is_boolean()) {
				_parameter = j[fieldName];
			}
			return _parameter;
		}

		Common::CVector3 GetVector3Parameter(std::string fieldName) {

			std::vector<double> _parameter;
			if (!j[fieldName].is_null() && j[fieldName].is_structured()) {
				_parameter = j[fieldName].get<std::vector<double>>();
			}
			Common::CVector3 tempV3;
			if (_parameter.size() == 3) { tempV3 = Common::CVector3(_parameter[0], _parameter[1], _parameter[2]); }

			return tempV3;
		}

		Common::CQuaternion GetQuaternionParameter(std::string fieldName) {

			std::vector<double> _parameter;
			if (!j[fieldName].is_null() && j[fieldName].is_structured()) {
				_parameter = j[fieldName].get<std::vector<double>>();
			}
			Common::CQuaternion tempV4;
			if (_parameter.size() == 4) { tempV4 = Common::CQuaternion(_parameter[0], _parameter[1], _parameter[2], _parameter[3]); }

			return tempV4;
		}
	};
}
#endif