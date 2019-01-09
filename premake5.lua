solution "DollarGame"
	
configurations { "Debug", "Release" }
platforms { "Windows", "Linux" }

startproject "DollarGame"

project "DollarGameLib"
	location "%{sln.location}/build"
	kind "StaticLib"
	architecture "x86_64"
	language "C++"
	targetdir "%{sln.location}/lib/%{cfg.shortname}"
	--objdir "%{sln.location}/build/%{cfg.shortname}" -- Weird things happen when uncommenting this. Should investigate.
	flags "FatalWarnings"

	files { 
		"src/Graph.hpp",
		"src/Graph.cpp",
		"src/SolverCommon.hpp",
		"src/GeneratorCommon.hpp",
	}

	excludes {
		"src/DollarGame.cpp",
	}

project "DollarGame"
	location "%{sln.location}/build"
	kind "ConsoleApp"
	architecture "x86_64"
	language "C++"
	targetdir "%{sln.location}/bin/%{cfg.buildcfg}/"
	--objdir "%{sln.location}/build/%{cfg.shortname}" -- Weird things happen when uncommenting this. Should investigate.
	debugdir "%{cfg.targetdir}"
	flags "FatalWarnings"

	files { 
		"src/DollarGame.cpp",
		"src/Solver.hpp",
		"src/Solver.cpp",
		"src/Generator.hpp",
		"src/Generator.cpp",
		"src/DllUtils.hpp",
		"src/DllUtils.cpp",
	}

	libdirs {
		"%{sln.location}/lib/%{cfg.shortname}",
	}

	links {
		"DollarGameLib",
	}

	filter "platforms:Linux"
		links {
			"stdc++fs",
			"pthread",
			"dl",
		}
		buildoptions {
			"-pthread",
		}

group "Solvers"

for _, file in ipairs(os.matchfiles("src/solvers/*.cpp")) do
	local basename = string.explode(file, "/")[3]
	local ident = string.explode(basename, "%.")[1]

	project ( ident )
		location "%{sln.location}/build"
		kind "SharedLib"
		architecture "x86_64"
		language "C++"
		targetname (ident)
		targetdir "%{sln.location}/bin/%{cfg.buildcfg}/solvers"
		implibdir "%{sln.location}/build/%{cfg.targetname}-%{cfg.shortname}"
		objdir "%{sln.location}/build/%{cfg.targetname}-%{cfg.shortname}"
		files { file }
		libdirs "%{sln.location}/lib/%{cfg.shortname}"
		links "DollarGameLib"
		flags "FatalWarnings"
end

group "Generators"

for _, file in ipairs(os.matchfiles("src/generators/*.cpp")) do
	local basename = string.explode(file, "/")[3]
	local ident = string.explode(basename, "%.")[1]

	project ( ident )
		location "%{sln.location}/build"
		kind "SharedLib"
		architecture "x86_64"
		language "C++"
		targetname (ident)
		targetdir "%{sln.location}/bin/%{cfg.buildcfg}/generators"
		implibdir "%{sln.location}/build/%{cfg.targetname}-%{cfg.shortname}"
		objdir "%{sln.location}/build/%{cfg.targetname}-%{cfg.shortname}"
		files { file }
		libdirs "%{sln.location}/lib/%{cfg.shortname}"
		links "DollarGameLib"
		flags "FatalWarnings"
end

project "*"
	filter "configurations:Debug"
		defines "_DEBUG"
		symbols "On"
		optimize "Off"
	filter "configurations:not Debug"
		defines "NDEBUG"
		symbols "Off"
		optimize "On"
	filter "platforms:Windows"
		buildoptions "/std:c++latest"
	filter "platforms:Linux"
		buildoptions "-std=c++17"
		defines "__forceinline=__attribute__((always_inline))"