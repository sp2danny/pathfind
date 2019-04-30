
#include "fancynumber.hpp"

#include <sstream>
#include <vector>
#include <iomanip>

namespace
{
	struct Formatter
	{
		float factor;
		std::string symbol;
	};

	std::vector<Formatter> general = {
		{1e+24, " Y"},
		{1e+21, " Z"},
		{1e+18, " E"},
		{1e+15, " P"},
		{1e+12, " T"},
		{1e+09, " G"},
		{1e+06, " M"},
		{1e+03, " k"},
		{1, " "},
		{1e-03, " m"},
		{1e-06, " μ"},
		{1e-09, " n"},
		{1e-12, " p"},
		{1e-15, " f"},
		{1e-18, " a"},
		{1e-21, " z"},
		{1e-24, " y"}
	};

	std::vector<Formatter> time = {
		{604800, " w"},
		{86400, " d"},
		{3600, " h"},
		{60, " m"},
		{1, " s"},
		{1e-03, " ms"},
		{1e-06, " μs"},
		{1e-09, " ns"},
		{1e-12, " ps"},
		{1e-15, " fs"},
		{1e-18, " as"},
		{1e-21, " zs"},
		{1e-24, " ys"}
	};

}

//	static const std::vector<std::string> pref = { "", "m", "μ", "n", "p", "f", "a", "z", "y" };
//	static const std::vector<std::string> pref = { "", "k", "M", "G", "T", "P", "E", "Z", "Y" };

std::string FancyNumberFormatter(float f, int p)
{
	float of = f;
	if (f == 0)
		return "0 ";
	bool neg = false;
	if (f<0) { neg = true; f=-f; }
	if (f>=1.0 && f<=1000.0)
	{
		std::stringstream ss;
		ss << std::setprecision(p) << f;
		return (neg?"-":"")+ss.str()+" ";
	}
	else if (f<1.0)
	{
		int i = 0;
		while (f<1.0)
		{
			f *= 1000.0;
			++i;
		}
		static const std::vector<std::string> pref = { "", "m", "μ", "n", "p", "f", "a", "z", "y" };
		if (i >= (int)pref.size())
		{
			std::stringstream ss;
			ss << std::setprecision(p) << std::scientific << of << " ";
			return ss.str();
		}
		std::stringstream ss;
		ss << std::setprecision(p) << f;
		return (neg?"-":"")+ss.str()+" "+pref[i];
	}
	else
	{
		int i = 0;
		while (f>=1000.0)
		{
			f /= 1000.0;
			++i;
		}
		static const std::vector<std::string> pref = { "", "k", "M", "G", "T", "P", "E", "Z", "Y" };
		if (i >= (int)pref.size())
		{
			std::stringstream ss;
			ss << std::setprecision(p) << std::scientific << of << " ";
			return ss.str();
		}
		std::stringstream ss;
		ss << std::setprecision(p) << f;
		return (neg?"-":"")+ss.str()+" "+pref[i];
	}
}


