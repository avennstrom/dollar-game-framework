#pragma once

#include "DllUtils.hpp"
#include "GeneratorCommon.hpp"

class GraphGenerator
{
private:
	typedef const char* (FnGetName)();
	typedef const char* (FnGetDescription)();
	typedef void (FnGenerate)(GeneratorContext&, const GeneratorParams&);

	DllHandle m_dll;

	FnGetName* m_fnGetName;
	FnGetDescription* m_fnGetDescription;
	FnGenerate* m_fnGenerate;

public:
	GraphGenerator(const std::string& dllName);
	GraphGenerator(const GraphGenerator&) = delete;

	std::string getName() const;
	std::string getDescription() const;
	void generate(GeneratorContext& ctx, const GeneratorParams& params) const;
};