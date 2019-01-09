#pragma once

#include <vector>
#include <set>
#include <algorithm>
#include <cassert>
#include <cinttypes>

typedef uint32_t NodeHandle;
typedef int32_t NodeValue;

constexpr NodeHandle NullNode = std::numeric_limits<NodeHandle>::max();

// Basically a vector but without ownership of the memory it's pointing to.
template<typename T>
struct Range
{
private:
	T* first;
	T* last;

public:
	Range()
		: first(nullptr)
		, last(nullptr)
	{
	}

	Range(T* first, size_t count)
		: first(first)
		, last(first + count)
	{
	}

	__forceinline T* begin() const
	{
		return first;
	}

	__forceinline T* end() const
	{
		return last;
	}

	__forceinline T* data() const
	{
		return first;
	}

	__forceinline T& operator[](size_t i)
	{
		assert(i < size());
		return first[i];
	}

	__forceinline const T& operator[](size_t i) const
	{
		assert(i < size());
		return first[i];
	}

	__forceinline bool empty() const
	{
		return first == last;
	}

	__forceinline size_t size() const
	{
		return std::distance(first, last);
	}
};

struct Edge
{
	Edge(NodeHandle a, NodeHandle b)
		: a(a)
		, b(b)
	{
	}

	NodeHandle a;
	NodeHandle b;
};

// Required for std::map
inline bool operator<(const Edge& lhs, const Edge& rhs)
{
	const auto lhs_a = std::min(lhs.a, lhs.b);
	const auto lhs_b = std::max(lhs.a, lhs.b);

	const auto rhs_a = std::min(rhs.a, rhs.b);
	const auto rhs_b = std::max(rhs.a, rhs.b);

	return lhs_a < rhs_a || (!(rhs_a < lhs_a) && lhs_b < rhs_b);
}

// Required for std::unordered_map
namespace std
{
	template<>
	struct hash<Edge>
	{
		constexpr uint32_t operator()(const Edge& edge) const
		{
			return edge.a ^ edge.b;
		}
	};
}

class Graph final
{
private:
	std::vector<NodeValue> m_values;
	std::vector<Range<NodeHandle>> m_connections;
	std::vector<NodeHandle> m_connectionBuffer;

public:
	Graph() = default;
	Graph(const Graph& rhs);

	void init(
		const std::vector<NodeValue>& values,
		const std::set<Edge>& edges);

	void give(NodeHandle node);
	void take(NodeHandle node);

	bool isSolvable() const;
	bool isSolved() const;

	__forceinline size_t size() const
	{
		return m_values.size();
	}

	__forceinline const auto& values() const
	{
		return m_values;
	}

	__forceinline NodeValue getNodeValue(NodeHandle handle) const
	{
		assert(handle != NullNode);
		return m_values[handle];
	}

	__forceinline const auto& connections() const
	{
		return m_connections;
	}

	__forceinline Range<NodeHandle>& getNodeConnections(NodeHandle handle)
	{
		assert(handle != NullNode);
		return m_connections[handle];
	}

	__forceinline const Range<NodeHandle>& getNodeConnections(NodeHandle handle) const
	{
		assert(handle != NullNode);
		return m_connections[handle];
	}
};