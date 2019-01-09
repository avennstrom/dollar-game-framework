#include "../GeneratorCommon.hpp"

GENERATOR_NAME("Circular")
GENERATOR_DESCRIPTION("Generates a circular graph.")

GENERATOR_FUNC(GeneratorContext& ctx, const GeneratorParams& params)
{
	std::uniform_int_distribution<NodeValue> valueDist(params.minValue(), params.maxValue());
	std::vector<NodeValue> values(params.size());
	for (NodeHandle nodeIt = 0; nodeIt < params.size(); ++nodeIt)
	{
		values[nodeIt] = valueDist(ctx.random());
	}

	std::set<Edge> edges;
	for (NodeHandle nodeIt = 0; nodeIt < params.size() - 1; ++nodeIt)
	{
		const Edge edge(nodeIt, nodeIt + 1);
		edges.insert(edge);
	}

	// Connect the last and the first node, closing the loop.
	edges.insert(Edge((NodeHandle)params.size() - 1, 0));

	ctx.graph().init(values, edges);
}