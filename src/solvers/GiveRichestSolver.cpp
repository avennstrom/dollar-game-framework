#include "../SolverCommon.hpp"

SOLVER_NAME("GiveRichest")
SOLVER_DESCRIPTION("Finds the richest node and gives to its neighbors.");

static NodeHandle getRichestNode(const Graph& graph)
{
	const auto minIt = std::max_element(graph.values().cbegin(), graph.values().cend());
	return (NodeHandle)std::distance(graph.values().cbegin(), minIt);
}

SOLVER_FUNC(SolverContext& ctx)
{
	while (!ctx.isSolved())
	{
		const NodeHandle richestNode = getRichestNode(ctx.graph());
		ctx.registerMove<Move::Give>(richestNode);
	}
}
