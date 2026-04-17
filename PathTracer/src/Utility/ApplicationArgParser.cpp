/*
 * ApplicationArgParser.cpp
 */

#include <algorithm>
#include <limits>
#include <sstream>
#include <stdexcept>

#include "ApplicationArgParser.hpp"

namespace utility
{
	ApplicationArgParser::ApplicationArgParser(int argc, char** argv)
	{
		for (int i = 0; i < argc; ++i)
		{
			this->tokens.push_back(std::string(argv[i]));
		}
	}

	ApplicationOptions ApplicationArgParser::parse() const
	{
		ApplicationOptions options;
		options.showHelp = hasFlag("-h") || hasFlag("--help");
		if (options.showHelp)
		{
			return options;
		}

		options.useAA = hasFlag("--use-anti-aliasing");
		options.verbose = hasFlag("--verbose");

		const std::string& width = getOptionValue("--width");
		if (!width.empty())
		{
			options.width = parseU16("--width", 1, (std::numeric_limits<uint16_t>::max)());
		}

		const std::string& height = getOptionValue("--height");
		if (!height.empty())
		{
			options.height = parseU16("--height", 1, (std::numeric_limits<uint16_t>::max)());
		}

		const std::string& samples = getOptionValue("--max-samples");
		if (!samples.empty())
		{
			options.maxSamples = parseU8("--max-samples", 1, (std::numeric_limits<uint8_t>::max)());
		}

		const std::string& depth = getOptionValue("--max-depth");
		if (!depth.empty())
		{
			options.maxDepth = parseU8("--max-depth", 1, (std::numeric_limits<uint8_t>::max)());
		}

		const std::string& bias = getOptionValue("--bias");
		if (!bias.empty())
		{
			options.bias = parseFloat("--bias");
		}

		const std::string& aperture = getOptionValue("--aperture");
		if (!aperture.empty())
		{
			options.aperture = parseFloat("--aperture");
		}

		const std::string& focalDistance = getOptionValue("--focal");
		if (!focalDistance.empty())
		{
			options.focalDistance = parseFloat("--focal");
		}
		options.useDOF = !aperture.empty() && !focalDistance.empty();

		const std::string& threadCount = getOptionValue("--threading");
		if (!threadCount.empty())
		{
			options.threadCount = parseU8("--threading", 1, (std::numeric_limits<uint8_t>::max)());
		}

		const std::string& inputScene = getOptionValue("--input");
		if (inputScene.empty())
		{
			throw std::runtime_error("Missing required option: --input <path to collada scene file>");
		}
		options.inputScene = std::filesystem::path(inputScene);
		if (options.inputScene.extension().string() != ".dae")
		{
			throw std::runtime_error("Invalid input scene. Expected a .dae COLLADA file.");
		}

		const std::string& outputDir = getOptionValue("--output");
		if (!outputDir.empty())
		{
			options.outputDir = std::filesystem::path(outputDir);
		}
		else
		{
			std::filesystem::path executablePath(this->tokens.front());
			options.outputDir = executablePath.remove_filename() / "out";
		}

		return options;
	}

	std::string ApplicationArgParser::usage()
	{
		return
			"Usage: PathTracer --input <path to collada scene file> [options]\n"
			"Options:\n"
			"  --width <value>               Output width in pixels (default 512)\n"
			"  --height <value>              Output height in pixels (default 512)\n"
			"  --max-samples <value>         Samples per axis (default 4)\n"
			"  --max-depth <value>           Max ray depth (default 4)\n"
			"  --bias <value>                Shadow bias (default 0.001)\n"
			"  --aperture <value>            Aperture radius for depth of field\n"
			"  --focal <value>               Focal distance for depth of field\n"
			"  --output <directory>          Output directory (default: executable_dir/out)\n"
			"  --use-anti-aliasing           Enable anti aliasing\n"
			"  --threading <value>           Number of render threads (default 1)\n"
			"  --verbose                     Print scene debug information\n"
			"  -h, --help                    Show this help\n";
	}

	const std::string& ApplicationArgParser::getOptionValue(const std::string& option) const
	{
		const std::vector<std::string>::const_iterator optionIt =
			std::find(this->tokens.begin(), this->tokens.end(), option);
		if (optionIt == this->tokens.end())
		{
			static const std::string empty;
			return empty;
		}

		const std::vector<std::string>::const_iterator valueIt = std::next(optionIt);
		if (valueIt == this->tokens.end() || (!valueIt->empty() && (*valueIt)[0] == '-'))
		{
			throw std::runtime_error("Option requires a value: " + option);
		}

		return *valueIt;
	}

	bool ApplicationArgParser::hasFlag(const std::string& option) const
	{
		return std::find(this->tokens.begin(), this->tokens.end(), option) != this->tokens.end();
	}

	uint16_t ApplicationArgParser::parseU16(const std::string& option, uint16_t minValue, uint16_t maxValue) const
	{
		const std::string& value = getOptionValue(option);
		const unsigned long parsed = std::stoul(value);
		if (parsed < minValue || parsed > maxValue)
		{
			std::ostringstream error;
			error << "Option out of range: " << option;
			throw std::runtime_error(error.str());
		}
		return static_cast<uint16_t>(parsed);
	}

	uint8_t ApplicationArgParser::parseU8(const std::string& option, uint8_t minValue, uint8_t maxValue) const
	{
		const std::string& value = getOptionValue(option);
		const unsigned long parsed = std::stoul(value);
		if (parsed < minValue || parsed > maxValue)
		{
			std::ostringstream error;
			error << "Option out of range: " << option;
			throw std::runtime_error(error.str());
		}
		return static_cast<uint8_t>(parsed);
	}

	float ApplicationArgParser::parseFloat(const std::string& option) const
	{
		const std::string& value = getOptionValue(option);
		return std::stof(value);
	}
}
