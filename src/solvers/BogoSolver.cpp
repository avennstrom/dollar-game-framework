#include "../SolverCommon.hpp"

SOLVER_NAME("BogoSolver")
SOLVER_DESCRIPTION("Performs random moves. Probably not going to solve any graph ever.")

#include <random>

SOLVER_FUNC(SolverContext& ctx)
{
	std::random_device rd;
	std::mt19937 r(rd());

	std::uniform_int_distribution<NodeHandle> nodeDist(0, (NodeHandle)ctx.graph().size() - 1);
	std::bernoulli_distribution typeDist;

	while (!ctx.isSolved())
	{
		const NodeHandle node = nodeDist(r);
		const Move::Type moveType = static_cast<Move::Type>(typeDist(r));
		const Move move(moveType, node);
		ctx.registerMove(move);
	}
}