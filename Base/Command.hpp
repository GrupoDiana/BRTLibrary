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

		std::string GetSourceID() {
			std::string _command = "";
			if (!j["sourceID"].is_null() && j["sourceID"].is_string()) {
				_command = j["sourceID"].get<std::string>();
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
			if (!j["parameter"].is_null() && j["parameter"].is_number_float()) {
				temp = j["parameter"];
			}
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

		std::vector<std::string> GetStringVector() {
			std::vector<std::string> tempV;
			if (!j["parameter"].is_null() && j["parameter"].is_structured()) {
				tempV = j["parameter"].get<std::vector<std::string>>();
			}
			return tempV;
		}
	};
}
#endif