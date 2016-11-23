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

#include <algorithm>
#include <vector>

using namespace std;

class RankFilter : public Iop
{
	float _w, _h;
	int _sizeH, _sizeW;
	float _value;

public:

	int maximum_inputs() const { return 1; }
	int minimum_inputs() const { return 1; }

	RankFilter (Node* node) : Iop(node)
	{
		_w = _h = 3;
		_value = 0.5f;
	}

	~RankFilter() {}

	void _validate(bool);
	void _request(int, int, int, int, ChannelMask, int);
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

void _validate(bool for_real)
{
	_sizeH = int(fabs(_h) + 0.5);
	_sizeW = int(fabs(_w) + 0.5);
	copy_info();
	info_.y(info_.y() - _sizeH);
	info_.t(info_.t() + _sizeH);
	info_.x(info_.x() - _sizeW);
	info_.r(info_.r() + _sizeW);
	
	set_out_channels(_sizeH || _sizeW ? Mask_All : Mask_None);
}

void _request(int x, int y, int r, int t, ChannelMask channels, int count)
{
	x -= _sizeW;
	y -= _sizeH;
	r += _sizeW;
	t += _sizeH;
	input(0)->request( x, y, r, t, channels, count);
}

void RankFilter::engine(int y, int x, int r, ChannelMask channels, Row& row)
{

	Tile tile( input0(), x - _sizeW, y - _sizeH, r + _sizeW, y + _sizeH + 1, channels);
	if (aborted()) { return; }
	
	if (!tile.valid()) {
		// if the tile is not valid, just make it black (like your soul).
		
		foreach (z, channels) {
			float *outptr = row.writeable(z) + x;
			memset( &outptr[tile.x()], 0 , (tile.x() - tile.r()) * sizeof(float));
		}
		return;
	}

	foreach(z, channels) {
		
		float* outptr = row.writable(z) + x;

		for(int cur = x; cur < r; cur++) {

			vector<float> v;

			for (int px = -_sizeW; px < _sizeW; px++) {
				for (int py = -_sizeH; py < _sizeH; py++)
					v.push_back(tile[z][tile.clampy(y + py)][tile.clampx(cur + px)]);
			}

			size_t n = v.size() * _value;
			nth_element(v.begin(), v.begin()+n, v.end());

			*outptr++ = v[n];

		}

	}

}
