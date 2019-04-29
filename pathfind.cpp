
#include <vector>
#include <iostream>
#include <map>
#include <algorithm>
#include <string>
#include <random>
#include <sstream>
#include <chrono>
#include <iomanip>

using namespace std;

struct Pos
{
	short x,y;
	Pos  operator  + (Pos p) const { return Pos(x+p.x, y+p.y); }
	bool operator  < (Pos p) const { return (y==p.y) ? (x<p.x) : (y<p.y); }
	bool operator != (Pos p) const { return (y!=p.y) || (x!=p.x); }
	Pos(short x=0, short y=0) : x(x), y(y) {}
};

struct Map
{
	int width, height;
	std::vector<char> data;
	Pos start;
	char& at(int x, int y) { return data.at(y*width+x); }
	char at(int x, int y) const { return data.at(y*width+x); }
	char& at(Pos p) { return data.at(p.y*width+p.x); }
	char at(Pos p) const { return data.at(p.y*width+p.x); }
	bool valid(Pos p) const { return p.x>=0 && p.x<width && p.y>=0 && p.y<height; }
	void print(std::ostream&);
};

void Map::print(std::ostream& out)
{
	for (short y=0; y<height; ++y)
	{
		for (short x=0; x<width; ++x)
		{
			if (Pos{x,y} != start)
				out << at(x,y);
			else
				out << 'S';
		}
		out << endl;
	}
}

const char FLOOR = '1' ;
const char WALL  = '0' ;
const char GOAL  = 'G' ;

enum Dir { d_beg, d_up=d_beg, d_rg, d_dn, d_lf, d_end };

Pos deltas[d_end] = { {0,-1}, {+1,0}, {0,+1}, {-1,0} };

Dir& operator ++ (Dir& d) { d = (Dir)(1+(int)d); return d; }

Dir other(Dir d)
{
	switch(d)
	{
	case d_up: return d_dn;
	case d_rg: return d_lf;
	case d_dn: return d_up;
	case d_lf: return d_rg;
	default:   return d_end;
	}
}

struct SearchMapItem
{
	bool traversble;
	bool goal;
	bool visited;
	Dir came_from;
	bool paths[d_end];
};

map<Pos,SearchMapItem> search_map;
typedef map<Pos,SearchMapItem>::iterator SMII;

std::random_device rdev;
std::default_random_engine generator{rdev()};

Map RandomMap(int w, int h)
{
	Map m;
	m.width = w; m.height = h;
	m.data.resize(w*h, WALL);

	auto fillp = [&]() -> int
	{
		int cnt = 0;
		for (auto c : m.data)
		{
			if (c==WALL)
				++cnt;
		}
		return (cnt*100) / m.data.size();
	};

	while (fillp() > 50)
	{
		int startx = uniform_int_distribution<int>(1,w-2)(generator);
		int starty = uniform_int_distribution<int>(1,h-2)(generator);
		int length = uniform_int_distribution<int>(0,(w+h)/3)(generator);
		static bool lft = true;
		if (lft)
		{
			for(int i = 0; i<length; ++i)
			{
				int x = startx+i;
				if (x >= (w-1)) break;
				m.at(x, starty) = FLOOR;
			}
		} else {
			for(int i = 0; i<length; ++i)
			{
				int y = starty+i;
				if (y >= (h-1)) break;
				m.at(startx, y) = FLOOR;
			}
		}
		lft = !lft;
	}
	m.start.x = uniform_int_distribution<int>(1,w-2)(generator);
	m.start.y = uniform_int_distribution<int>(1,h-2)(generator);
	m.at(m.start.x, m.start.y) = FLOOR;
	int gx = uniform_int_distribution<int>(1,w-2)(generator);
	int gy = uniform_int_distribution<int>(1,h-2)(generator);
	m.at(gx, gy) = GOAL;
	return m;
}

std::string FancyNumberFormatter(float f, int p=4)
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

Map RandomMapOld(int w, int h)
{
	Map m;
	m.width = w; m.height = h;
	m.data.reserve(w*h);
	for (int y=0; y<h; ++y) for (int x=0; x<w; ++x)
	{
		char c;
		int i = uniform_int_distribution<int>(0,99)(generator);
		if (i<3)
			c = GOAL;
		else if (i<75)
			c = WALL;
		else
			c = FLOOR;
		m.data.push_back(c);
	}
	m.start.x = uniform_int_distribution<int>(0,w-1)(generator);
	m.start.y = uniform_int_distribution<int>(0,h-1)(generator);
	return m;
}

void MakeMap(const Map& map)
{
	search_map.clear();
	Pos p;
	for (p.y=0; p.y<map.height; ++p.y) for (p.x=0; p.x<map.width; ++p.x) 
	{
		SearchMapItem smi;
		smi.visited = false;
		smi.came_from = d_end;
		if (map.at(p) == WALL)
		{
			smi.traversble = false;
		}
		else if (map.at(p) == GOAL)
		{
			smi.traversble = true;
			smi.goal = true;
		}
		else if (map.at(p) == FLOOR)
		{
			smi.traversble = true;
			smi.goal = false;
			for (Dir d = d_beg; d != d_end; ++d)
			{
				Pos p2 = p + deltas[d];
				smi.paths[d] = map.valid(p2) && (map.at(p2) != WALL);
			}
		}
		search_map[p] = smi;
	}
}

struct SearchResult
{
	bool found;
	bool error;
	std::vector<Dir> path;
	std::string message;
	int operator<=>(const SearchResult&) const;

};

int SearchResult::operator<=>(const SearchResult& rhs) const
{
	auto& lhs = *this;
	if (lhs.error != rhs.error)
		return lhs.error ? -1 : +1;
	if (lhs.found != rhs.found)
		return rhs.found ? -1 : +1;
	return ((int)lhs.path.size()) - ((int)rhs.path.size());
}

SearchResult FindGoalFrom(Pos start)
{
	vector<SMII> found;

	{
		SMII smii = search_map.find(start);

		if (smii == search_map.end()) { return { false, true, {}, "starting outside map"s }; }
		if (smii->second.goal)        { return { true, false, {}, "already at target"s }; }
		if (!smii->second.traversble) { return { false, true, {}, "starting in a wall"s }; }

		smii->second.visited = true;
		found.push_back(smii);
	}

	bool did_find = false;

	while (!did_find)
	{
		vector<SMII> candidates;

		for (SMII smii : found)
		{
			for (Dir d = d_beg; d != d_end; ++d)
			{
				if (!smii->second.paths[d]) continue;
				Pos p = smii->first + deltas[d];
				//if (!valid(p)) continue;
				SMII cand = search_map.find(p);
				if (cand == search_map.end()) continue;
				if (cand->second.visited) continue;
				cand->second.came_from = d;
				candidates.push_back(cand);
			}
		}

		if (candidates.empty()) break;

		for (SMII smii : candidates)
		{
			smii->second.visited = true;
			found.push_back(smii);
			if (smii->second.goal) { did_find = true; break; }
		}
	}

	if (!did_find) { return { false, false, {}, "no goal reachable"s }; }

	SMII pos = found.back();

	vector<Dir> path;

	while (pos->first != start)
	{
		Dir d = pos->second.came_from;
		path.push_back(d);
		Pos p = pos->first + deltas[ other(d) ];
		pos = search_map.find(p);
	}

	const char* dir_names[] = { "Up", "Right", "Down", "Left" } ;
	auto last = d_end;
	int cnt = 0, n = path.size();

	stringstream ss;
	for (int i = n-1; i >= 0; --i)
	{
		auto curr = path[i];
		if (last != curr)
		{
			if (cnt) ss << " " << cnt << ((cnt==1)?" step.\n":" steps.\n");
			ss << "Walk " << dir_names[curr];
			cnt = 1;
			last = curr;
		} else {
			++cnt;
		}
	}
	if (cnt) ss << " " << cnt << ((cnt==1)?" step.\n":" steps.\n");
	ss << "Then you are at goal in total " << to_string(n) << " steps" << endl;
	return {true, false, path, ss.str()};
}

bool operator<(const SearchResult& lhs, const SearchResult& rhs)
{ return (lhs<=>rhs) < 0; }

namespace {
	int attempt = 0;
	int error = 0;
	int succ = 0;
	long long accu = 0;
	void upd(const SearchResult& sr)
	{
		++attempt;
		if (sr.error)
			return (void)++error;
		if (sr.found)
		{
			++succ;
			accu += sr.path.size();
		}
	}
	std::chrono::duration<long double> dur, gen;
}

int main()
{

	Map bsf_map = RandomMap(25,25);
	MakeMap(bsf_map);
	SearchResult bsf_sr = FindGoalFrom(bsf_map.start);
	upd(bsf_sr);

	for (long long i=0; ;++i)
	{
		auto t1 = std::chrono::high_resolution_clock::now();
		auto m = RandomMap(25,25);
		auto t2 = std::chrono::high_resolution_clock::now();
		gen += (t2-t1);
		t1 = std::chrono::high_resolution_clock::now();
		MakeMap(m);
		auto sr = FindGoalFrom(m.start);
		t2 = std::chrono::high_resolution_clock::now();
		dur += (t2-t1);
		upd(sr);
		bool dop = false;
		if (bsf_sr < sr)
		{
			bsf_map = m;
			bsf_sr = sr;
			bsf_map.print(cout);
			cout << sr.message << endl;
			dop = true;
		}
		if ((i%128)==0) dop=true;
		if (dop)
		{
			cout << "errorp  : " << (error*100.0f) / attempt << endl;
			cout << "findp   : " << (succ*100.0f) / attempt << endl;
			cout << "apl     : " << (accu*1.0f) / succ << endl;
			cout << "avg gen : " << FancyNumberFormatter(gen.count() / attempt) << "s" << endl;
			cout << "avg dur : " << FancyNumberFormatter(dur.count() / attempt) << "s" << endl;
		}
	}
}

