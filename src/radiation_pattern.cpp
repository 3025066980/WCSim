#include "radiation_pattern.hpp"
#include<iostream>
#include<fstream>

#include <glm/glm.hpp>

#include "pattern.hpp"

RadiationPattern::RadiationPattern(std::string pattern_file_path) {
	std::ifstream input_file_stream(pattern_file_path);
	if (input_file_stream.is_open()) {
		std::cout << "Reading Radiation Pattern file" << std::endl;
		std::string line;
		min_gain_ = 0.0f;
		max_gain_ = 0.0f;
		while(getline(input_file_stream, line)) {
			if (line[0] == '#' || line[0] == '*')continue;
			float theta, phi, total_gain;
			float reeth, imeth, rephi, imphi, gth, gphi;
			input_file_stream >> phi >> theta >> reeth >> imeth >> rephi >> imphi >> gth >> gphi >> total_gain;
			pattern_[theta].insert({ phi, total_gain });


			if (total_gain < min_gain_) min_gain_ = total_gain;
			if (total_gain > max_gain_) max_gain_ = total_gain;
		}
		std::cout << "Radiation Pattern Reading Completed." << std::endl;
		input_file_stream.close();
	}
	else {
		std::cout << "Couldn't open radiation pattern file." << std::endl;
		return;
	}

	for (int phi = 0; phi < 180; phi += 10) {
		for (int theta = 0; theta < 360; theta += 10) {
			float value = pattern_[theta][phi];
		}
	}
}

void RadiationPattern::GetPattern()
{
	//pattern_object_ = new Pattern(pattern_);
}

float RadiationPattern::GetGain(float theta, float phi)
{
	//std::cout << "getting gain. " << std::endl;
	float theta_deg = glm::degrees(theta);
	float phi_deg = glm::degrees(phi);
	if (theta_deg >= 359.0f) theta_deg = 0.0f;


	auto& phi_value = (*pattern_.lower_bound(theta_deg)).second;
	auto gain = (*phi_value.lower_bound(phi_deg)).second;
	//std::cout << "theta:" << theta_deg << " phi:" << phi_deg << " total gain: " << gain << std::endl;

	// Debug the problem
	return gain;
}
