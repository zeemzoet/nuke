static const char* const CLASS = "RankFilter";

static const char* const HELP =
"Ranks all the values in the chosen kernel"
"and takes the median acoording to the"
"chosen percentage"

#include "DDImage/Iop.h"
#include "DDImage/NukeWrapper.h"
#include "DDImage/Row.h"
#include "DDImage/Tile.h"
#include "DDImage/Knobs.h"

using namespace DD::Image;

#include <algortihm>
#include <vector>

using namespace std;

class RankFilter : public Iop
{
	int _size;
	float _value;

public:

	int maximum_inputs() const { return 1; }
	int minimum_inputs() const { return 1; }

	RankFilter (Node* node) : Iop(node)
	{
		_size = 3;
		_value = 0.5f;
	}

	~RankFilter() {}

	void _validate(bool);
	void _request(int, int, int, int, ChannelMask channels, int count);
	void engine(int, int, int, ChannelMask, Row&);

	const char* Class() const { return CLASS; }
	const char* node_help() const { return HELP; }

	static const Iop::Description d; 

};

static Iop* build(Node* node)
{
	return new NukeWrapper(new RankFilter(node));
}

const Iop::Description RankFilter::d (CLASS, "Filter/RankFilter", build);

void RankFilter::engine(int y, int x, int r, ChannelMask channels, Row& row)
{

	Tile tile( input0(), x - _size, y - _size, r + _size, y+_size, channels);
	if (aborted()) { return; }

	foreach(z, channels) {
		
		float* outptr = row.writable(z) + x;

		for(int cur = x; cur < r; cur++) {

			vector<float> v;

			for (int px = -_size; px < _size; px++) {
				for (int py = -_size; py < _size; py++)
					v.push_back(tile[ z ][ tile.clampy(y+py) ][ tile.clampx(cur + px) ]);
			}

			size_t n = v.size() * _value;
			nth_element(v.begin(), v.begin()+n, v.end());

			*outptr++ = v[n]

		}

	}

}