#include "Graph.hpp"
#include "Solver.hpp"
#include "Generator.hpp"

#include <iostream>
#include <random>
#include <fstream>
#include <experimental/filesystem>
#include <thread>
#include <future>
#include <iomanip>
#include <cstring>
#include <map>

#if _WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#elif __linux__
#include <unistd.h>
#endif

static std::experimental::filesystem::path getExecutableDir()
{
#if _WIN32
	char buf[MAX_PATH + 1];
	GetModuleFileNameA(NULL, buf, sizeof(buf));
	const auto lastSlashPos = std::string(buf).find_last_of("\\/");
	return std::string(buf).substr(0, lastSlashPos);
#elif __linux__
	char buf[256];
	readlink("/proc/self/exe", buf, sizeof(buf));
	const auto lastSlashPos = std::string(buf).find_last_of("\\/");
	return std::string(buf).substr(0, lastSlashPos);
#endif
}

static Graph generateGraph(const GraphGenerator& generator, std::mt19937& random, size_t size)
{
	while (true)
	{
		Graph graph;
		GeneratorContext ctx(graph, random);

		GeneratorParams params(size, -2, 3);

		generator.generate(ctx, params);

		// Make sure the graph is solvable and unsolved.
		if (!graph.isSolvable())
		{
			//std::cout << "Unsolvable\n";
			continue;
		}
		else if (graph.isSolved())
		{
			//std::cout << "Already solved\n";
			continue;
		}

		//std::cout << "Done\n";
		return graph;
	};
}

static bool trySolve(const Graph& graph, const GraphSolver& solver, std::vector<Move>& outMoves, size_t moveLimit)
{
	SolverContext ctx(graph, outMoves, moveLimit);

	int taskResult = -1;
	auto solverTask = std::async(std::launch::async, [&solver, &ctx, &taskResult] ()
	{
		try
		{
			solver.solve(ctx);
			if (!ctx.wasStopped())
			{
				taskResult = 0;
			}
		}
		catch (const std::exception& e)
		{
			std::cerr << e.what() << '\n';
		}
	});

	const std::chrono::milliseconds timeout(1000);
	const std::future_status status = solverTask.wait_for(timeout);
	if (status == std::future_status::timeout || taskResult == -1)
	{
		if (status == std::future_status::timeout)
		{
			std::cout << "timeout\n";
			ctx.stop();
			solverTask.wait();
		}
		outMoves.clear();
		return false;
	}

	return true;
}

static auto enumSolvers()
{
	const auto solverDir = getExecutableDir() / "solvers";

	std::vector<std::unique_ptr<GraphSolver>> outSolvers;
	for (auto solverDirIt : std::experimental::filesystem::directory_iterator(solverDir))
	{
		const auto& solverPath = solverDirIt.path();
#if _WIN32
		if (solverPath.extension() == ".dll")
#elif __linux__
		if (solverPath.extension() == ".so")
#endif
		{
			auto solver = std::make_unique<GraphSolver>(solverPath.string());
			outSolvers.push_back(std::move(solver));
		}
	}
	return outSolvers;
}

static auto enumGenerators()
{
	const auto generatorDir = getExecutableDir() / "generators";

	std::vector<std::unique_ptr<GraphGenerator>> outGenerators;
	for (auto generatorDirIt : std::experimental::filesystem::directory_iterator(generatorDir))
	{
		const auto& solverPath = generatorDirIt.path();
#if _WIN32
		if (solverPath.extension() == ".dll")
#elif __linux__
		if (solverPath.extension() == ".so")
#endif
		{
			auto solver = std::make_unique<GraphGenerator>(solverPath.string());
			outGenerators.push_back(std::move(solver));
		}
	}
	return outGenerators;
}

void compare(
	GraphGenerator& generator, 
	std::vector<std::unique_ptr<GraphSolver>>& solvers, 
	size_t graphSize,
	size_t iterations)
{
	std::vector<size_t> nodeCounts = {
		graphSize,
	};

	std::vector<std::pair<NodeValue, NodeValue>> valueRanges = {
		{ -2, 2 },
	};

	std::random_device rd;
	std::mt19937 r(rd());

	constexpr size_t moveLimit = 100000000;

	for (const size_t nodeCount : nodeCounts)
	{
		std::cout << nodeCount << " nodes" << std::endl;
		for (const auto& valueRange : valueRanges)
		{
			std::cout << "values [" << valueRange.first << ", " << valueRange.second << "]" << std::endl;

			std::cout << "| ";
			for (size_t solverIt = 0; solverIt < solvers.size(); ++solverIt)
			{
				const size_t w = solvers[solverIt]->getName().length();
				std::cout << std::setw(w) << solvers[solverIt]->getName() << " | ";
			}
			std::cout << '\n';

			std::vector<size_t> totalSolverMoves(solvers.size());
			std::fill(totalSolverMoves.begin(), totalSolverMoves.end(), 0);

			for (size_t sampleIt = 0; sampleIt < iterations; ++sampleIt)
			{
				while (true)
				{
					Graph graph = generateGraph(generator, r, nodeCount);

					std::vector<bool> solveSuccessful(solvers.size());
					std::vector<size_t> solverMoves(solvers.size());
					std::fill(solverMoves.begin(), solverMoves.end(), 0);

					for (size_t solverIt = 0; solverIt < solvers.size(); ++solverIt)
					{
						const GraphSolver& solver = *solvers[solverIt];

						//std::cout << "Solving with " << solver.getName() << '\n';

						std::vector<Move> moves;// = solverMoves[solverIt];
						if (trySolve(graph, solver, moves, moveLimit))
						{
							//std::cout << "Solved in " << moves.size() << " moves by " << solver.getName() << '\n';
							solveSuccessful[solverIt] = true;
							solverMoves[solverIt] = moves.size();
						}
						else
						{
							//std::cout << "Could not be solved by " << solver.getName() << std::endl;
							solveSuccessful[solverIt] = false;
						}
					}

					const bool allSolversSucceeded = std::all_of(solveSuccessful.cbegin(), solveSuccessful.cend(), [] (bool b) { return b; });
					if (allSolversSucceeded)
					{
						// Print results to console.
						std::cout << "| ";
						for (size_t solverIt = 0; solverIt < solvers.size(); ++solverIt)
						{
							const size_t w = solvers[solverIt]->getName().length();
							std::cout << std::setw(w) << solverMoves[solverIt] << " | ";

							totalSolverMoves[solverIt] += solverMoves[solverIt];
						}
						std::cout << '\n';

						//if (solverMoves[0] != solverMoves[1])
						//{
						//	__debugbreak();
						//}

						break;
					}
				}
			}

			std::cout << "#\n";
			std::cout << "# AVERAGE NUM MOVES\n";
			std::cout << "#\n";

			std::cout << "| ";
			for (size_t solverIt = 0; solverIt < solvers.size(); ++solverIt)
			{
				const size_t w = solvers[solverIt]->getName().length();
				std::cout << std::setw(w) << solvers[solverIt]->getName() << " | ";
			}
			std::cout << '\n';

			std::cout << "| ";
			for (size_t solverIt = 0; solverIt < solvers.size(); ++solverIt)
			{
				const size_t w = solvers[solverIt]->getName().length();
				const double avg = (double)totalSolverMoves[solverIt] / iterations;
				std::cout << std::setw(w) << std::fixed << std::setprecision(2) << avg << " | ";
			}
			std::cout << '\n';
		}
	}
}

static void printSolvers()
{
	const auto solvers = enumSolvers();

	size_t maxNameLength = 0;
	for (auto&& solver : solvers)
	{
		maxNameLength = std::max(maxNameLength, solver->getName().length());
	}

	std::cout << "\nSolvers available:\n\n";
	for (auto&& solver : solvers)
	{
		std::cout << "  " << std::setw(maxNameLength) << solver->getName() << " - " << solver->getDescription() << '\n';
	}
}

static void printGenerators()
{
	const auto generators = enumGenerators();

	size_t maxNameLength = 0;
	for (auto&& generator : generators)
	{
		maxNameLength = std::max(maxNameLength, generator->getName().length());
	}

	std::cout << "\nGenerators available:\n\n";
	for (auto&& generator : generators)
	{
		std::cout << "  " << std::setw(maxNameLength) << generator->getName() << " - " << generator->getDescription() << '\n';
	}
}

static void printUsage()
{
	std::cout << "Usage:\n\n";
	std::cout << "  DollarGame <options>\n\n";
	std::cout << "Options:\n\n";
	const size_t w = 12;
	std::cout << "  --solvers   "  << std::setw(w) << "<solvers>" << " - Specify which solvers to run.\n";
	std::cout << "  --generator "  << std::setw(w) << "<generator>" << " - Specify which generator should be used to generate graphs.\n";
	std::cout << "  --graph-size"  << std::setw(w) << "N" << " - Size of the graphs.\n";
	std::cout << "  --iterations"  << std::setw(w) << "N" << " - Number of iterations to run.\n";
	std::cout << '\n';

	printSolvers();

	printGenerators();
}

static void benchmarkSolver(std::map<std::string, std::vector<std::string>> args)
{
	if (args.find("--solver") == args.cend())
	{
		std::cerr << "Missing argument: --solver\n";
		std::exit(-1);
	}

	if (args.find("--generators") == args.cend())
	{
		std::cerr << "Missing argument: --generators\n";
		std::exit(-1);
	}

	if (args.find("--iterations") == args.cend())
	{
		std::cerr << "Missing argument: --iterations\n";
		std::exit(-1);
	}

	if (args.find("--graph-sizes") == args.cend())
	{
		std::cerr << "Missing argument: --graph-sizes\n";
		std::exit(-1);
	}

	const auto& argSolver = args["--solver"][0];
	const auto& argGenerators = args["--generators"];
	const auto& argIterations = args["--iterations"][0];
	const auto& argGraphSizes = args["--graph-sizes"];

	std::unique_ptr<GraphSolver> solver;
	std::vector<std::unique_ptr<GraphGenerator>> generators;
	size_t iterations;
	std::vector<size_t> graphSizes;

	{
		auto allSolvers = enumSolvers();
		for (auto&& s : allSolvers)
		{
			if (s->getName() == argSolver)
			{
				solver = std::move(s);
				break;
			}
		}
	}

	{
		auto allGenerators = enumGenerators();
		for (const auto& argGenerator : argGenerators)
		{
			bool foundGenerator = false;
			for (auto&& generator : allGenerators)
			{
				if (generator && generator->getName() == argGenerator)
				{
					generators.push_back(std::move(generator));
					foundGenerator = true;
					break;
				}
			}

			if (!foundGenerator)
			{
				std::cerr << "Generator \"" << argGenerator << "\" not found.\n";
				exit(-1);
			}
		}
	}

	try
	{
		iterations = std::stoull(argIterations);
	}
	catch (const std::exception&)
	{
		std::cerr << "The --iterations must be a positive integer.\n";
		exit(-1);
	}

	try
	{
		for (const auto& argGraphSize : argGraphSizes)
		{
			graphSizes.push_back(std::stoull(argGraphSize));
		}
	}
	catch (const std::exception&)
	{
		std::cerr << "The --graph-sizes must be positive integers.\n";
		exit(-1);
	}

	std::vector<std::vector<double>> results;
	results.resize(generators.size());
	for (auto&& result : results)
	{
		result.resize(graphSizes.size());
	}

	std::random_device rd;
	std::mt19937 r(rd());

	for (size_t generatorIt = 0; generatorIt < generators.size(); ++generatorIt)
	{
		const auto& generator = generators[generatorIt];
		for (size_t graphSizeIt = 0; graphSizeIt < graphSizes.size(); ++graphSizeIt)
		{
			const size_t graphSize = graphSizes[graphSizeIt];

			size_t totalSolverMoves = 0;

			std::cout << "Generator: " << generator->getName() << " - Size: " << graphSize << "\n";

			for (size_t iteration = 0; iteration < iterations; ++iteration)
			{
				while (true)
				{
					Graph graph = generateGraph(*generator, r, graphSize);

					bool solveSuccessful;

					std::vector<Move> moves;
					if (trySolve(graph, *solver, moves, 1000000))
					{
						solveSuccessful = true;
					}
					else
					{
						solveSuccessful = false;
					}

					if (solveSuccessful)
					{
						totalSolverMoves += moves.size();
						break;
					}
				}
			}

			const double avg = (double)totalSolverMoves / (double)iterations;

			std::cout << "Avg moves: " << std::fixed << std::setprecision(2) << avg << "\n";

			results[generatorIt][graphSizeIt] = avg;
		}
	}

	std::ofstream os("result.csv");
	
	for (auto&& generator : generators)
	{
		os << ';' << generator->getName();
	}
	os << '\n';

	for (size_t graphSizeIt = 0; graphSizeIt < graphSizes.size(); ++graphSizeIt)
	{
		os << graphSizes[graphSizeIt];
		for (size_t generatorIt = 0; generatorIt < generators.size(); ++generatorIt)
		{
			os << ';' << results[generatorIt][graphSizeIt];
		}
		os << '\n';
	}
}

int main(int argc, char* argv[])
{
	if (argc == 1)
	{
		printUsage();
		exit(-1);
	}

	std::map<std::string, std::vector<std::string>> args;

	std::string lastArgName;
	for (int i = 1; i < argc; ++i)
	{
		if (strlen(argv[i]) >= 2 && argv[i][0] == '-' && argv[i][1] == '-')
		{
			lastArgName = argv[i];
			continue;
		}
		else
		{
			args[lastArgName].push_back(argv[i]);
		}
	}

	benchmarkSolver(args);

    return 0;
}

