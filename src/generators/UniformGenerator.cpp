#include "../GeneratorCommon.hpp"

GENERATOR_NAME("Uniform")
GENERATOR_DESCRIPTION("Simple generator.")

#include <algorithm>
#include <unordered_set>

GENERATOR_FUNC(GeneratorContext& ctx, const GeneratorParams& params)
{
	std::uniform_int_distribution<NodeValue> valueDist(params.minValue(), params.maxValue());
	std::vector<NodeValue> values(params.size());
	for (NodeHandle nodeIt = 0; nodeIt < params.size(); ++nodeIt)
	{
		values[nodeIt] = valueDist(ctx.random());
	}

	std::uniform_int_distribution<NodeHandle> connDist(1, (NodeHandle)params.size() - 1);

	std::set<Edge> edges;
	for (NodeHandle nodeIt = 0; nodeIt < params.size(); ++nodeIt)
	{
		for (int i = 0; i < 2;)
		{
			const NodeHandle other = (nodeIt + connDist(ctx.random())) % params.size();
			assert(nodeIt != other);

			const Edge edge(nodeIt, other);
			if (edges.insert(edge).second)
			{
				++i;
			}
		}
	}

	ctx.graph().init(values, edges);
}