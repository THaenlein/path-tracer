/*
 * ApplicationArgParser.hpp
 */

#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace utility
{
	struct ApplicationOptions
	{
		bool showHelp{ false };
		bool useAA{ false };
		bool useDOF{ false };
		bool verbose{ false };

		uint16_t width{ 512U };
		uint16_t height{ 512U };
		uint8_t maxSamples{ 4U };
		uint8_t maxDepth{ 4U };
		uint8_t threadCount{ 1U };

		float bias{ 0.001f };
		float aperture{ 0.f };
		float focalDistance{ 0.f };

		std::filesystem::path inputScene;
		std::filesystem::path outputDir;
	};

	class ApplicationArgParser
	{
	public:
		ApplicationArgParser(int argc, char** argv);

		ApplicationOptions parse() const;

		static std::string usage();

	private:
		const std::string& getOptionValue(const std::string& option) const;

		bool hasFlag(const std::string& option) const;

		uint16_t parseU16(const std::string& option, uint16_t minValue, uint16_t maxValue) const;

		uint8_t parseU8(const std::string& option, uint8_t minValue, uint8_t maxValue) const;

		float parseFloat(const std::string& option) const;

		std::vector<std::string> tokens;
	};
}
