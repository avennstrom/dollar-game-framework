#include "Generator.hpp"

GraphGenerator::GraphGenerator(const std::string& dllName)
	: m_dll(dllName)
{
	m_fnGetName = m_dll.getProcAddress<FnGetName>("GENERATOR_getName");
	m_fnGetDescription = m_dll.getProcAddress<FnGetDescription>("GENERATOR_getDescription");
	m_fnGenerate = m_dll.getProcAddress<FnGenerate>("GENERATOR_generate");

	if (m_fnGetName == nullptr)
	{
		throw std::runtime_error("Generator hasn't implemented GENERATOR_NAME");
	}

	if (m_fnGetDescription == nullptr)
	{
		throw std::runtime_error("Generator hasn't implemented GENERATOR_DESCRIPTION");
	}

	if (m_fnGenerate == nullptr)
	{
		throw std::runtime_error("Generator hasn't implemented GENERATOR_FUNC");
	}
}

std::string GraphGenerator::getName() const
{
	assert(m_fnGetName);
	return (*m_fnGetName)();
}

std::string GraphGenerator::getDescription() const
{
	assert(m_fnGetDescription);
	return (*m_fnGetDescription)();
}

void GraphGenerator::generate(GeneratorContext& ctx, const GeneratorParams& params) const
{
	assert(m_fnGenerate);
	(*m_fnGenerate)(ctx, params);
}