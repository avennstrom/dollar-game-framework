#pragma once

#include "Graph.hpp"

#include <random>

#if defined(_MSC_VER)
#define DLLEXPORT __declspec(dllexport)
#elif defined(__GNUC__)
#define DLLEXPORT __attribute__((visibility("default")))
#endif

#define GENERATOR_NAME(Name) extern "C" DLLEXPORT const char* GENERATOR_getName() { return Name; }
#define GENERATOR_DESCRIPTION(Description) extern "C" DLLEXPORT const char* GENERATOR_getDescription() { return Description; }
#define GENERATOR_FUNC extern "C" DLLEXPORT void GENERATOR_generate

class GeneratorContext final
{
private:
	Graph& m_graph;
	std::mt19937& m_random;

public:
	GeneratorContext(Graph& graph, std::mt19937& random)
		: m_graph(graph)
		, m_random(random)
	{
	}

	GeneratorContext(const GeneratorContext&) = delete;

	__forceinline Graph& graph()
	{
		return m_graph;
	}

	__forceinline std::mt19937& random()
	{
		return m_random;
	}
};

class GeneratorParams final
{
private:
	size_t m_size;
	NodeValue m_minValue;
	NodeValue m_maxValue;

public:
	GeneratorParams(size_t size, NodeValue minValue, NodeValue maxValue)
		: m_size(size)
		, m_minValue(minValue)
		, m_maxValue(maxValue)
	{
	}

	__forceinline size_t size() const
	{
		return m_size;
	}

	__forceinline NodeValue minValue() const
	{
		return m_minValue;
	}

	__forceinline NodeValue maxValue() const
	{
		return m_maxValue;
	}
};