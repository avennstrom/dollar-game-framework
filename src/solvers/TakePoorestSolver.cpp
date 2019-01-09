#include "../SolverCommon.hpp"

SOLVER_NAME("TakePoorest")
SOLVER_DESCRIPTION("Finds the poorest node and takes from its neighbors.")

static NodeHandle getPoorestNode(const Graph& graph)
{
	const auto minIt = std::min_element(graph.values().cbegin(), graph.values().cend());
	return (NodeHandle)std::distance(graph.values().cbegin(), minIt);
}

SOLVER_FUNC(SolverContext& ctx)
{
	while (!ctx.isSolved())
	{
		const NodeHandle poorestNode = getPoorestNode(ctx.graph());
		ctx.registerMove<Move::Take>(poorestNode);
	}
}
