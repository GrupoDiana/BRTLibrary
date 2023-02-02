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
			return (j["command"].is_null());
		}

		std::string GetCommand() {
			std::string _command = "";
			if (!j["command"].is_null() && j["command"].is_string()) {
				_command = j["command"].get<std::string>();
			}
			return _command;			
		}

		std::string GetID() {
			std::string _command = "";
			if (!j["id"].is_null() && j["id"].is_string()) {
				_command = j["id"].get<std::string>();
			}
			return _command;
		}

		std::string GetStringParameter() {
			std::string _temp = "";
			if (!j["parameter"].is_null() && j["parameter"].is_string()) {
				_temp = j["parameter"].get<std::string>();
			}
			return _temp;
		}

		bool GetBoolParameter() {
			bool temp;
			if (!j["parameter"].is_null() && j["parameter"].is_boolean()) {
				temp = j["parameter"];
			}
			return temp;
		}

		double GetDoubleParameter() {			
			double temp = 0.0;
			std::vector<double> tempV;
			if (!j["parameter"].is_null() && j["parameter"].is_number_float()) {
				temp = j["parameter"];
			}
			else if (!j["parameter"].is_null() && j["parameter"].is_structured()) {
				tempV = j["parameter"].get<std::vector<double>>();
			}
			if (tempV.size() > 0) { temp = tempV[0]; }
			return temp;
		}

		float GetFloatParameter() {
			return (float)GetDoubleParameter();
		}		

		std::vector<double> GetDoubleVector() {			
			std::vector<double> tempV;
			if (!j["parameter"].is_null() && j["parameter"].is_structured()) {
				tempV = j["parameter"].get<std::vector<double>>();
			}
			return tempV;
		}			

		Common::CVector3 GetVector3Parameter() {
			
			std::vector<double> tempV;
			if (!j["parameter"].is_null() && j["parameter"].is_structured()) {
				tempV = j["parameter"].get<std::vector<double>>();
			}
			Common::CVector3 tempV3;
			if (tempV.size() == 3) { tempV3 = Common::CVector3(tempV[0], tempV[1], tempV[2]);	}
			
			return tempV3;
		}

		std::vector<std::string> GetStringVector() {
			std::vector<std::string> tempV;
			if (!j["parameter"].is_null() && j["parameter"].is_structured()) {
				tempV = j["parameter"].get<std::vector<std::string>>();
			}
			return tempV;
		}

		std::string GetTuplaPath() {
			std::string _temp = "";
			if (!j["path"].is_null() && j["path"].is_string()) {
				_temp = j["path"].get<std::string>();
			}
			return _temp;
		}

		std::string GetTuplaData() {
			std::string _temp = "";
			if (!j["data"].is_null() && j["data"].is_string()) {
				_temp = j["data"].get<std::string>();
			}
			return _temp;
		}

		float GetTuplaFloatValue() {
			double temp = 0.0;
			std::vector<double> tempV;
			if (!j["value"].is_null() && j["value"].is_number_float()) {
				temp = j["value"];
			}						
			return temp;
		}
	};
}
#endif