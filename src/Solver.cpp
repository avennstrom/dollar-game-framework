#include "Solver.hpp"

GraphSolver::GraphSolver(const std::string& dllName)
	: m_dll(dllName)
{
	m_fnGetName = m_dll.getProcAddress<FnGetName>("SOLVER_getName");
	m_fnGetDescription = m_dll.getProcAddress<FnGetDescription>("SOLVER_getDescription");
	m_fnSolve = m_dll.getProcAddress<FnSolve>("SOLVER_solve");

	if (m_fnGetName == nullptr)
	{
		throw std::runtime_error("Solver hasn't implemented SOLVER_NAME");
	}

	if (m_fnGetDescription == nullptr)
	{
		throw std::runtime_error("Solver hasn't implemented SOLVER_DESCRIPTION");
	}

	if (m_fnSolve == nullptr)
	{
		throw std::runtime_error("Solver hasn't implemented SOLVER_FUNC");
	}
}

std::string GraphSolver::getName() const
{
	assert(m_fnGetName);
	return (*m_fnGetName)();
}

std::string GraphSolver::getDescription() const
{
	assert(m_fnGetDescription);
	return (*m_fnGetDescription)();
}

void GraphSolver::solve(SolverContext& ctx) const
{
	assert(m_fnSolve);

	const auto isEmpty = [] (const auto& v) { return v.empty(); };
	if (std::any_of(ctx.graph().connections().cbegin(), ctx.graph().connections().cend(), isEmpty))
	{
		throw std::invalid_argument("Graph contains dangling nodes");
	}

	if (!ctx.isSolvable())
	{
		throw std::invalid_argument("Graph is unsolvable");
	}

	(*m_fnSolve)(ctx);
}
