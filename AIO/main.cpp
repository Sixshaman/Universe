#include <iostream>
#include "LOMatrix.hpp"
#include <bit>
#include <regex>
#include <optional>
#include <format>

template<> struct std::formatter<boost::multiprecision::cpp_int>: std::formatter<std::string> 
{
	auto format(const boost::multiprecision::cpp_int& val, std::format_context& context) const
	{
		return std::formatter<std::string>::format(val.str(), context);
	}
};

enum class LaunchMode
{
	BuildDirectMatrix,
	BuildInvertedMatrix,
	CheckSolvability,
	CalcDefaultClickRuleSolutionPeriod,
	CalcClickRuleSolutionPeriodAndVerify
};

enum class BoardTopology
{
	Square,
	Torus,
	Matrix
};

struct LaunchOptions
{
	LaunchMode LaunchMode = LaunchMode::BuildInvertedMatrix;

	SaveMode SaveMode = SaveMode::SaveNoBorders;

	bool Verbose = false;

	boost::multiprecision::cpp_int MatrixPower = 1;

	std::string MatrixFileName;
	std::string ClickRuleFilename;
	std::string OutMatrixFilename;

	uint32_t BoardWidth  = 5;
	uint32_t BoardHeight = 5;
	uint32_t BoardDepth  = 1;

	BoardTopology Topology = BoardTopology::Square;
};

std::optional<LaunchOptions> ParseCommandLineArgs(int argc, char* argv[])
{
	LaunchOptions result;

	bool buildInverseMatrix        = false;
	bool buildDirectMatrix         = false;
	bool checkSolvability          = false;
	bool calcDefaultSolutionPeriod = false;
	bool calcVerifySolutionPeriod  = false;

	bool saveWithoutBorders   = false;
	bool saveWithBorders      = false;
	bool saveWithSmallBorders = false;

	bool overrideTopology = false;

	int currArg = 0;
	while(currArg < argc)
	{
		if(strcmp(argv[currArg], "--power") == 0)
		{
			currArg++;
			
			if(currArg >= argc)
			{
				std::cout << "Please enter matrix power!" << std::endl;
				return std::nullopt;
			}
			else
			{
				try
				{
					result.MatrixPower = boost::multiprecision::cpp_int(argv[currArg]);
				}
				catch(std::runtime_error e)
				{
					std::cout << "Please enter valid matrix power!" << std::endl;
					return std::nullopt;
				}
			}
		}

		else if(strcmp(argv[currArg], "--size") == 0)
		{
			currArg++;
			
			if(currArg >= argc)
			{
				std::cout << "Please enter board size!" << std::endl;
				return std::nullopt;
			}
			else
			{
				std::string currStr(argv[currArg]);

				std::regex singleDigitSizeRegex("(\\d+)");
				std::regex doubleDigitSizeRegex("(\\d+)x(\\d+)");
				std::regex tripleDigitSizeRegex("(\\d+)x(\\d+)x(\\d+)");
				
				std::smatch singleDigitMatch;
				std::smatch doubleDigitMatch;
				std::smatch tripleDigitMatch;

				try
				{
					if (std::regex_match(currStr, singleDigitMatch, singleDigitSizeRegex))
					{
						//The entered size is a single number. Parse as a square board
						uint32_t size = std::stoul(singleDigitMatch[1].str());

						result.BoardWidth  = size;
						result.BoardHeight = size;
					}
					else if(std::regex_match(currStr, doubleDigitMatch, doubleDigitSizeRegex))
					{
						//The entered size is two numbers. Parse as a n x m board
						result.BoardWidth  = std::stoul(doubleDigitMatch[1].str());
						result.BoardHeight = std::stoul(doubleDigitMatch[2].str());
					}
					else if (std::regex_match(currStr, tripleDigitMatch, tripleDigitSizeRegex))
					{
						//The entered size is three numbers. Parse as a n x m x k board
						result.BoardWidth  = std::stoul(tripleDigitMatch[1].str());
						result.BoardHeight = std::stoul(tripleDigitMatch[2].str());
						result.BoardDepth  = std::stoul(tripleDigitMatch[3].str());
					}
				}
				catch(...)
				{
					std::cout << "Please enter valid board size!" << std::endl;
					return std::nullopt;
				}
			}
		}

		else if(strcmp(argv[currArg], "--click_rule") == 0)
		{
			currArg++;

			if(currArg >= argc)
			{
				std::cout << "Please enter click rule filename!" << std::endl;
				return std::nullopt;
			}
			else
			{
				result.ClickRuleFilename = argv[currArg];
			}
		}

		else if(strcmp(argv[currArg], "--matrix") == 0)
		{
			currArg++;

			if(currArg >= argc)
			{
				std::cout << "Please enter matrix filename!" << std::endl;
				return std::nullopt;
			}
			else
			{
				result.MatrixFileName = argv[currArg];
			}
		}

		else if(strcmp(argv[currArg], "--out") == 0)
		{
			currArg++;

			if(currArg >= argc)
			{
				std::cout << "Please enter output filename!" << std::endl;
				return std::nullopt;
			}
			else
			{
				result.OutMatrixFilename = argv[currArg];
			}
		}

		else if(strcmp(argv[currArg], "--topology") == 0)
		{
			currArg++;

			if(currArg >= argc)
			{
				std::cout << "Please enter topology!" << std::endl;
				return std::nullopt;
			}
			else if(strcmp(argv[currArg], "square") == 0)
			{
				result.Topology = BoardTopology::Square;
			}
			else if(strcmp(argv[currArg], "torus") == 0)
			{
				result.Topology = BoardTopology::Torus;
			}
			else
			{
				std::cout << "Unknown topology: " << argv[currArg] << std::endl;
				return std::nullopt;
			}

			overrideTopology = true;
		}

		else if(strcmp(argv[currArg], "--inverse") == 0)
		{
			buildInverseMatrix = true;
		}

		else if(strcmp(argv[currArg], "--direct") == 0)
		{
			buildDirectMatrix = true;
		}

		else if(strcmp(argv[currArg], "--check_solvability") == 0)
		{
			checkSolvability = true;
		}

		else if(strcmp(argv[currArg], "--calc_default_solution_period") == 0)
		{
			calcDefaultSolutionPeriod = true;
		}

		else if(strcmp(argv[currArg], "--calc_solution_period_verify") == 0)
		{
			calcVerifySolutionPeriod = true;
		}

		else if(strcmp(argv[currArg], "--borders") == 0 || strcmp(argv[currArg], "--default_borders") == 0 || strcmp(argv[currArg], "--with_borders") == 0)
		{
			saveWithBorders = true;
		}

		if(strcmp(argv[currArg], "--no_borders") == 0)
		{
			saveWithoutBorders = true;
		}

		if(strcmp(argv[currArg], "--small_borders") == 0)
		{
			saveWithSmallBorders = true;
		}

		if(strcmp(argv[currArg], "--verbose") == 0)
		{
			result.Verbose = true;
		}

		currArg++;
	}

	if(!result.MatrixFileName.empty() && !result.ClickRuleFilename.empty())
	{
		std::cout << "Error: click rule and matrix options are mutually exclusive." << std::endl;
		return std::nullopt;
	}

	if(checkSolvability && (calcDefaultSolutionPeriod || calcVerifySolutionPeriod || buildInverseMatrix || buildDirectMatrix))
	{
		std::cout << "Error: checking solvability is mutually exclusive with other options." << std::endl;
		return std::nullopt;
	}

	if(calcDefaultSolutionPeriod && (calcVerifySolutionPeriod || checkSolvability || buildInverseMatrix || buildDirectMatrix))
	{
		std::cout << "Error: calculating solution period is mutually exclusive with other options." << std::endl;
		return std::nullopt;
	}

	if(calcVerifySolutionPeriod && (calcDefaultSolutionPeriod || checkSolvability || buildInverseMatrix || buildDirectMatrix))
	{
		std::cout << "Error: calculating solution period with verification is mutually exclusive with other options." << std::endl;
		return std::nullopt;
	}

	if(buildInverseMatrix && buildDirectMatrix)
	{
		std::cout << "Error: direct and inverse matrix options are mutually exclusive." << std::endl;
		return std::nullopt;
	}

	if((int)saveWithBorders + (int)saveWithoutBorders + (int)saveWithSmallBorders > 1)
	{
		std::cout << "Error: different border settings are mutually exclusive." << std::endl;
		return std::nullopt;
	}

	if(overrideTopology && !result.MatrixFileName.empty())
	{
		std::cout << "Error: matrix option is incompatible with square and torus topologies." << std::endl;
		return std::nullopt;
	}

	if(checkSolvability)
	{
		result.LaunchMode = LaunchMode::CheckSolvability;
	}
	else if(calcDefaultSolutionPeriod)
	{
		result.LaunchMode = LaunchMode::CalcDefaultClickRuleSolutionPeriod;
	}
	else if(calcVerifySolutionPeriod)
	{
		result.LaunchMode = LaunchMode::CalcClickRuleSolutionPeriodAndVerify;
	}
	else if(buildInverseMatrix)
	{
		result.LaunchMode = LaunchMode::BuildInvertedMatrix;
	}
	else if(buildDirectMatrix)
	{
		result.LaunchMode = LaunchMode::BuildDirectMatrix;
	}

	if(saveWithBorders)
	{
		result.SaveMode = SaveMode::SaveWithBorders;
	}
	else if(saveWithoutBorders)
	{
		result.SaveMode = SaveMode::SaveNoBorders;
	}
	else if(saveWithSmallBorders)
	{
		result.SaveMode = SaveMode::SaveWithSmallBorders;
	}

	if(!result.MatrixFileName.empty())
	{
		result.Topology = BoardTopology::Matrix;
	}

	if(result.OutMatrixFilename.empty())
	{
		result.OutMatrixFilename = std::format("LightsOut{}x{}-{}-Power-{}-{}.bmp", result.BoardWidth, result.BoardHeight,
				                                                                    result.LaunchMode == LaunchMode::BuildDirectMatrix ? "Direct" : "Inverse", 
				                                                                    result.MatrixPower, result.Topology == BoardTopology::Torus ? "Torus" : "Square");
	}

	return result;
}

bool VerifySolutionPeriod(uint32_t gameSize, const std::string& clickRuleFilename, const boost::multiprecision::cpp_int& solutionPeriod)
{
	//Verify solution period
	if(solutionPeriod == 1)
	{
		return gameSize == 1; //Only possible for 1x1 board
	}
	else
	{
		LOMatrix mat;
		assert(solutionPeriod % 2 == 0);
		mat.LoadSquareClickRule(clickRuleFilename, gameSize); //TODO: only default click rule is supported!!!

		boost::dynamic_bitset<uint64_t> vectorTest(gameSize * gameSize, 0);
		vectorTest.set(0, true);

		//Test that the (A^p)*((A^p)*b) == (A^p)*b, i.e. this is indeed the solution period
		{
			LOMatrix solutionPeriodMatrix = mat.CalcMatrixPower(solutionPeriod);
			boost::dynamic_bitset<uint64_t> vectorRes = solutionPeriodMatrix.MulBoard(vectorTest);
			boost::dynamic_bitset<uint64_t> vectorResSquared = solutionPeriodMatrix.MulBoard(vectorRes);

			if(vectorRes != vectorResSquared)
			{
				return false;
			}
		}

		//Test that this is indeed the smallest solution period, i.e. there is no smaller value of p' < p
		//such that (A^p')*((A^p')*b) = (A^p')*b, at least divided by 2
		boost::multiprecision::cpp_int power2Factor = 1;
		boost::multiprecision::cpp_int cyclicFactor = solutionPeriod;
		while(cyclicFactor % 2 == 0)
		{
			cyclicFactor = cyclicFactor / 2;
			power2Factor = power2Factor * 2;
		}

		{	
			LOMatrix cyclicPowerMatrix = mat.CalcMatrixPower(cyclicFactor);
			boost::dynamic_bitset<uint64_t> vectorRes = cyclicPowerMatrix.MulBoard(vectorTest);
			boost::dynamic_bitset<uint64_t> vectorResSquared = cyclicPowerMatrix.MulBoard(vectorRes);

			if(vectorRes == vectorResSquared)
			{
				return false;
			}

			if(cyclicFactor != 1)
			{
				LOMatrix power2Matrix = mat.CalcMatrixPower(power2Factor);
				vectorRes = power2Matrix.MulBoard(vectorTest);
				vectorResSquared = power2Matrix.MulBoard(vectorRes);

				if (vectorRes == vectorResSquared)
				{
					return false;
				}
			}
		}

		return true;
	}
}

void PrintOptions(const LaunchOptions& launchOptions)
{
	std::cout << "Launch mode: ";
	switch(launchOptions.LaunchMode)
	{
	case LaunchMode::BuildDirectMatrix:
		std::cout << "save direct matrix" << std::endl;
		break;

	case LaunchMode::BuildInvertedMatrix:
		std::cout << "save inverse matrix" << std::endl;
		break;

	case LaunchMode::CheckSolvability:
		std::cout << "check solvability" << std::endl;
		break;

	case LaunchMode::CalcDefaultClickRuleSolutionPeriod:
		std::cout << "calculate solution period for default Lights Out" << std::endl;
		break;

	case LaunchMode::CalcClickRuleSolutionPeriodAndVerify:
		std::cout << "calculate and verify solution period for default Lights Out" << std::endl;
		break;

	default:
		break;
	}

	//TODO: non-square and 3D boards
	std::cout << std::format("Board size: {}x{}", launchOptions.BoardWidth, launchOptions.BoardWidth) << std::endl;

	std::cout << "Board topology: ";
	switch(launchOptions.Topology)
	{
	case BoardTopology::Square:
		std::cout << "square" << std::endl;
		std::cout << "Click rule filename: " << launchOptions.ClickRuleFilename << std::endl;
		break;

	case BoardTopology::Torus:
		std::cout << "torus" << std::endl;
		std::cout << "Click rule filename: " << launchOptions.ClickRuleFilename << std::endl;
		break;

	case BoardTopology::Matrix:
		std::cout << "matrix-given" << std::endl;
		std::cout << "Matrix filename: " << launchOptions.MatrixFileName << std::endl;
		break;

	default:
		break;
	}

	if(launchOptions.LaunchMode != LaunchMode::BuildDirectMatrix && launchOptions.LaunchMode != LaunchMode::BuildInvertedMatrix)
	{
		return;
	}

	std::cout << "Matrix power: " << launchOptions.MatrixPower << std::endl;

	std::cout << "Borders mode: ";
	switch(launchOptions.SaveMode)
	{
	case SaveMode::SaveNoBorders:
		std::cout << "none" << std::endl;
		break;

	case SaveMode::SaveWithBorders:
		std::cout << "with borders" << std::endl;
		break;

	case SaveMode::SaveWithSmallBorders:
		std::cout << "with small borders" << std::endl;
		break;
			
	default:
		break;
	}

	std::cout << "Out filename: " << launchOptions.OutMatrixFilename << std::endl;
}

//FOR N = 999, THE PREDICTED SOLUTION PERIOD IS COMPARATIVELY SMALL!
//JUST 18 QUADRILLION!!!
//FOR N = 100, FOR EXAMPLE, IT'S 2 QUADRILLION, JUST 4 TIMES SMALLER!
//Of course, there are HUGE numbers. For N = 990, THE SOLUTION PERIOD IS 2 novemquadragintillions!!!
//The smallest solution period around 1000 is for N = 991. This solution period is literally 1984.
//ANOTHER INTERESTING FACT!!!!!!!!!!!!
//The values n > 5 such that nxn solution period is THE LARGEST ONE SO FAR: 6, 10, 12, 18, 22, 28, 36, 46, 52, 58, 60, 66, 70, 78, 82, 100, 102...
//If you divide this by 2, YOU'LL GET QUENEAU NUMBERS: https://oeis.org/A054639 
int main(int argc, char *argv[])
{
	auto launchOptionsOpt = ParseCommandLineArgs(argc, argv);
	if(!launchOptionsOpt.has_value())
	{
		return 1;
	}

	auto launchOptions = launchOptionsOpt.value();
	if(launchOptions.Verbose)
	{
		PrintOptions(launchOptions);
	}

	if(launchOptions.LaunchMode == LaunchMode::CheckSolvability)
	{
		LOMatrix mat;
		std::string resultMessage;

		uint32_t boardSize = launchOptions.BoardWidth;
		resultMessage += std::format("Lights out game {}x{}", boardSize, boardSize);

		switch (launchOptions.Topology)
		{
		case BoardTopology::Square:
		{
			mat.LoadSquareClickRule(launchOptions.ClickRuleFilename, boardSize);
			resultMessage += " on square board";
			break;
		}

		case BoardTopology::Torus:
		{
			mat.LoadToroidClickRule(launchOptions.ClickRuleFilename, boardSize);
			resultMessage += " on toroid board";
			break;
		}

		case BoardTopology::Matrix:
		{
			mat.LoadMatrix(launchOptions.MatrixFileName);
			resultMessage += " on matrix-given board";
			break;
		}

		default:
			break;
		}

		uint32_t quietPatterns = mat.CheckInv();
		if(quietPatterns == 0)
		{
			resultMessage += " is solvable";
		}
		else
		{
			resultMessage += std::format(" is unsolvable ({} quiet patterns)", quietPatterns);
		}

		std::cout << resultMessage << std::endl;
	}
	else if(launchOptions.LaunchMode == LaunchMode::CalcDefaultClickRuleSolutionPeriod || launchOptions.LaunchMode == LaunchMode::CalcClickRuleSolutionPeriodAndVerify)
	{
		uint32_t gameWidth = launchOptions.BoardWidth;

		LOMatrix mat;
		auto solutionPeriod = mat.FindSolutionPeriod(gameWidth);
		std::cout << std::format("SOLUTION PERIOD for default {}x{} Lights Out: {}", gameWidth, gameWidth, solutionPeriod) << std::endl;

		if(launchOptions.LaunchMode == LaunchMode::CalcClickRuleSolutionPeriodAndVerify)
		{
			if(!VerifySolutionPeriod(gameWidth, launchOptions.ClickRuleFilename, solutionPeriod))
			{
				std::cout << "SOLUTION PERIOD VERIFICATION ERROR!" << std::endl;
			}
			else
			{
				std::cout << "Solution period verification success!" << std::endl;
			}
		}
	}
	else
	{
		LOMatrix mat;

		switch (launchOptions.Topology)
		{
		case BoardTopology::Square:
			mat.LoadSquareClickRule(launchOptions.ClickRuleFilename, launchOptions.BoardWidth);
			break;

		case BoardTopology::Torus:
			mat.LoadToroidClickRule(launchOptions.ClickRuleFilename, launchOptions.BoardWidth);
			break;

		case BoardTopology::Matrix:
			mat.LoadMatrix(launchOptions.MatrixFileName);
			break;

		default:
			break;
		}

		if(launchOptions.Verbose)
		{
			std::cout << "Generated succesfully..." << std::endl;
		}

		std::wstring filename;
		if(launchOptions.LaunchMode == LaunchMode::BuildInvertedMatrix)
		{
			mat = mat.Inverto();	
			if(launchOptions.Verbose)
			{
				std::cout << "Diagonalized succesfully..." << std::endl;
			}
		}

		LOMatrix mulMat = mat.CalcMatrixPower(launchOptions.MatrixPower);

		std::string outFilename = launchOptions.OutMatrixFilename;
		if(outFilename.empty())
		{
			outFilename = std::format("LightsOut{}x{}-{}-Power-{}-{}.bmp", launchOptions.BoardWidth, launchOptions.BoardHeight, 
				                                                           launchOptions.LaunchMode == LaunchMode::BuildDirectMatrix ? "Direct" : "Inverse", 
				                                                           launchOptions.MatrixPower, launchOptions.Topology == BoardTopology::Torus ? "Torus" : "Square");
		}

		if(launchOptions.SaveMode == SaveMode::SaveWithBorders)
		{
			mulMat.SaveMatrix(outFilename);
		}
		else if(launchOptions.SaveMode == SaveMode::SaveNoBorders)
		{
			mulMat.SaveMatrixBorderless(outFilename);
		}

		std::cout << "Saved succesfully! Completed." << std::endl;
	}

	return 0;
}
