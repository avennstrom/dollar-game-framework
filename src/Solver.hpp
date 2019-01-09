#pragma once

#include "DllUtils.hpp"
#include "SolverCommon.hpp"

class GraphSolver
{
private:
	typedef const char* (FnGetName)();
	typedef const char* (FnGetDescription)();
	typedef void (FnSolve)(SolverContext&);

	DllHandle m_dll;

	FnGetName* m_fnGetName;
	FnGetDescription* m_fnGetDescription;
	FnSolve* m_fnSolve;

public:
	GraphSolver(const std::string& dllName);

	std::string getName() const;
	std::string getDescription() const;
	void solve(SolverContext& ctx) const;
};