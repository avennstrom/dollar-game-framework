#pragma once

#include "Graph.hpp"

#if defined(_MSC_VER)
#define DLLEXPORT __declspec(dllexport)
#elif defined(__GNUC__)
#define DLLEXPORT __attribute__((visibility("default")))
#endif

#define SOLVER_NAME(Name) extern "C" DLLEXPORT const char* SOLVER_getName() { return Name; }
#define SOLVER_DESCRIPTION(Description) extern "C" DLLEXPORT const char* SOLVER_getDescription() { return Description; }
#define SOLVER_FUNC extern "C" DLLEXPORT void SOLVER_solve

struct Move
{
	enum Type
	{
		Give,
		Take,
	};

	Move(Type type, NodeHandle node)
		: type(type)
		, node(node)
	{
	}

	Type type;
	NodeHandle node;
};

class SolverContext final
{
private:
	bool m_shouldStop;
	Graph m_graph;
	std::vector<Move>& m_moves;
	size_t m_moveLimit;

public:
	SolverContext(const Graph& graph, std::vector<Move>& outMoves, size_t moveLimit)
		: m_graph(graph)
		, m_moves(outMoves)
		, m_shouldStop(false)
		, m_moveLimit(moveLimit)
	{
		m_moves.clear();
	}

	SolverContext(const SolverContext&) = delete;

	__forceinline const Graph& graph() const
	{
		return m_graph;
	}

	__forceinline bool isSolvable() const
	{
		return m_graph.isSolvable();
	}

	template<Move::Type type>
	void registerMove(const NodeHandle& handle)
	{
		Move move(type, handle);
		m_moves.push_back(move);
		if constexpr (type == Move::Take)
		{
			m_graph.take(move.node);
		}
		else if constexpr (type == Move::Give)
		{
			m_graph.give(move.node);
		}
	}

	void registerMove(const Move& move)
	{
		m_moves.push_back(move);
		if (move.type == Move::Take)
		{
			m_graph.take(move.node);
		}
		else if (move.type == Move::Give)
		{
			m_graph.give(move.node);
		}
	}

	bool isSolved()
	{
		// Cancel the solve if the move limit has been reached.
		if (m_moves.size() > m_moveLimit)
		{
			m_shouldStop = true;
		}

		return m_shouldStop || m_graph.isSolved();
	}

	__forceinline void stop()
	{
		m_shouldStop = true;
	}

	__forceinline bool wasStopped() const
	{
		return m_shouldStop;
	}
};