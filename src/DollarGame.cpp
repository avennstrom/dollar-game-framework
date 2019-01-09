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

Graph generateGraph(GraphGenerator& generator, std::mt19937& random, size_t size)
{
	while (true)
	{
		Graph graph;
		GeneratorContext ctx(graph, random);

		GeneratorParams params(size, -5, 10);

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

void printSolvers()
{
	const auto solvers = enumSolvers();

	size_t maxSolverNameLength = 0;
	for (auto&& solver : solvers)
	{
		maxSolverNameLength = std::max(maxSolverNameLength, solver->getName().length());
	}

	std::cout << "\nAvailable solvers:\n";
	for (auto&& solver : solvers)
	{
		std::cout << std::setw(maxSolverNameLength) << solver->getName() << " - " << solver->getDescription() << '\n';
	}
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

	std::cout << std::endl;
}

int main(int argc, char* argv[])
{
	std::vector<std::string> args;
	args.reserve(argc - 1);

	for (int i = 1; i < argc; ++i)
	{
		args.push_back(argv[i]);
	}

	if (args.empty())
	{
		printUsage();
		return -1;
	}

	std::vector<std::string> argSolvers;
	std::string argGenerator;
	size_t argGraphSize = -1;
	size_t argIterations = -1;

	for (int i = 0; i < args.size();)
	{
		if (args[i] == "--solvers")
		{
			for (++i; i < args.size() && args[i][0] != '-'; ++i)
			{
				argSolvers.push_back(args[i]);
			}
		}
		else if (args[i] == "--generator")
		{
			argGenerator = args[i + 1];
			i += 2;
		}
		else if (args[i] == "--graph-size")
		{
			argGraphSize = std::stoull(args[i + 1]);
			i += 2;
		}
		else if (args[i] == "--iterations")
		{
			argIterations = std::stoull(args[i + 1]);
			i += 2;
		}
		else
		{
			++i;
		}
	}

	if (argIterations == -1)
	{
		std::cerr << "Missing argument: --iterations\n";
		return -1;
	}

	if (argGraphSize == -1)
	{
		std::cerr << "Missing argument: --graph-size\n";
		return -1;
	}

	if (argSolvers.empty())
	{
		std::cerr << "No solvers specified.\n";
		return -1;
	}

	if (argGenerator.empty())
	{
		std::cerr << "No generator specified.\n";
		return -1;
	}

	std::unique_ptr<GraphGenerator> generator;
	{
		auto generators = enumGenerators();
		for (auto&& g : generators)
		{
			if (g->getName() == argGenerator)
			{
				generator = std::move(g);
				break;
			}
		}
	}

	if (!generator)
	{
		std::cerr << "Generator \"" << argGenerator << "\" not found.\n";
	}

	std::vector<std::unique_ptr<GraphSolver>> solvers;
	{
		auto allSolvers = enumSolvers();
		for (const std::string& argSolver : argSolvers)
		{
			bool foundSolver = false;
			for (auto&& solver : allSolvers)
			{
				if (solver->getName() == argSolver)
				{
					solvers.push_back(std::move(solver));
					foundSolver = true;
					break;
				}
			}

			if (!foundSolver)
			{
				std::cerr << "Solver \"" << argSolver << "\" not found.\n";
				return -1;
			}
		}
	}

	compare(*generator, solvers, argGraphSize, argIterations);

    return 0;
}

