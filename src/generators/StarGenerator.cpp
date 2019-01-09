#include "../GeneratorCommon.hpp"

GENERATOR_NAME("Star")
GENERATOR_DESCRIPTION("Generates a graph where all nodes are connected to one common (center) node.")

GENERATOR_FUNC(GeneratorContext& ctx, const GeneratorParams& params)
{
	std::uniform_int_distribution<NodeValue> valueDist(params.minValue(), params.maxValue());
	std::vector<NodeValue> values(params.size());
	for (NodeHandle nodeIt = 0; nodeIt < params.size(); ++nodeIt)
	{
		values[nodeIt] = valueDist(ctx.random());
	}

	std::set<Edge> edges;
	for (NodeHandle nodeIt = 1; nodeIt < params.size(); ++nodeIt)
	{
		const Edge edge(0, nodeIt);
		edges.insert(edge);
	}

	ctx.graph().init(values, edges);
}