#include "Graph.hpp"

/*bool operator==(const NodeHandle& a, const NodeHandle& b)
{
	return a.index == b.index;
}

bool operator!=(const NodeHandle& a, const NodeHandle& b)
{
	return a.index != b.index;
}

bool operator<(const NodeHandle& a, const NodeHandle& b)
{
	return a.index < b.index;
}

bool operator>(const NodeHandle& a, const NodeHandle& b)
{
	return a.index > b.index;
}*/

void Graph::give(NodeHandle node)
{
	assert(node != NullNode);
	for (NodeHandle connection : m_connections[node])
	{
		++m_values[connection];
	}
	m_values[node] -= static_cast<NodeValue>(m_connections[node].size());
}

void Graph::take(NodeHandle node)
{
	assert(node != NullNode);
	for (NodeHandle connection : m_connections[node])
	{
		--m_values[connection];
	}
	m_values[node] += static_cast<NodeValue>(m_connections[node].size());
}

Graph::Graph(const Graph& rhs)
	: m_connectionBuffer(rhs.m_connectionBuffer)
	, m_values(rhs.m_values)
{
	// The connection ranges have to be moved manually to the new buffer location.
	const ptrdiff_t diff = m_connectionBuffer.data() - rhs.m_connectionBuffer.data();
	m_connections.reserve(rhs.m_connections.size());
	for (const auto& conn : rhs.m_connections)
	{
		m_connections.push_back(Range<NodeHandle>(conn.begin() + diff, conn.size()));
	}
}

void Graph::init(
	const std::vector<NodeValue>& values,
	const std::set<Edge>& edges)
{
	const size_t nodeCount = values.size();
	const size_t edgeCount = edges.size();

	m_values = values;

	std::vector<uint32_t> connectionCount(nodeCount);
	std::fill(connectionCount.begin(), connectionCount.end(), 0);

	for (const auto& edge : edges)
	{
		++connectionCount[edge.a];
		++connectionCount[edge.b];
	}

	m_connections.resize(nodeCount);
	m_connectionBuffer.resize(edgeCount * 2);

	NodeHandle* connectionIt = m_connectionBuffer.data();
	for (size_t i = 0; i < nodeCount; ++i)
	{
		const size_t count = connectionCount[i];
		m_connections[i] = Range<NodeHandle>(connectionIt, count);
		connectionIt += count;
	}

	std::vector<size_t> connectionIterators;
	connectionIterators.resize(nodeCount);
	std::fill(connectionIterators.begin(), connectionIterators.end(), 0);
	for (const auto& edge : edges)
	{
		size_t& it_a = connectionIterators[edge.a];
		size_t& it_b = connectionIterators[edge.b];

		m_connections[edge.a][it_a++] = edge.b;
		m_connections[edge.b][it_b++] = edge.a;
	}
}

/*void Graph::write(std::ostream& stream)
{
	//if (stream.flags() & std::ios::binary)
	{
		const uint64_t nodeCount = static_cast<uint64_t>(m_values.size());
		stream.write((const char*)&nodeCount, sizeof(nodeCount));
		stream.write((const char*)m_values.data(), nodeCount * sizeof(NodeValue));
		for (const auto& connections : m_connections)
		{
			const uint16_t connCount = static_cast<uint16_t>(connections.size());
			stream.write((const char*)&connCount, sizeof(connCount));
			stream.write((const char*)connections.data(), connCount * sizeof(NodeHandle));
		}
	}
}*/

bool Graph::isSolvable() const
{
	std::set<Edge> edges;

	const size_t nodeCount = size();
	for (NodeHandle nodeIt = 0; nodeIt < nodeCount; ++nodeIt)
	{
		for (NodeHandle connection : getNodeConnections(nodeIt))
		{
			edges.emplace(nodeIt, connection);
		}
	}

	const ptrdiff_t edgeCount = static_cast<ptrdiff_t>(edges.size());
	const ptrdiff_t vertexCount = static_cast<ptrdiff_t>(nodeCount);
	const ptrdiff_t genus = edgeCount - vertexCount + 1;

	NodeValue sum = 0;
	for (NodeValue value : m_values)
	{
		sum += value;
	}

	return sum >= genus;
}

bool Graph::isSolved() const
{
	return std::all_of(m_values.cbegin(), m_values.cend(), [] (auto value) {
		return value >= 0;
	});
}