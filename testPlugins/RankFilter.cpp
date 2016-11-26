static const char* const CLASS = "RankFilter";

static const char* const HELP =
"Ranks all the values in the chosen kernel"
"and takes the median acoording to the"
"chosen percentage";

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
	int _size;
	float _value, _bias;

public:

	int maximum_inputs() const { return 1; }
	int minimum_inputs() const { return 1; }

	RankFilter (Node* node) : Iop(node)
	{
		_size = 3;
		_value = 1.5f;
		_bias = 0.01f;
	}

	~RankFilter() {}

	void _validate(bool);
	void _request(int, int, int, int, ChannelMask, int);
	void engine(int, int, int, ChannelMask, Row&);
	virtual void knobs(Knob_Callback);

	const char* Class() const { return CLASS; }
	const char* node_help() const { return HELP; }

	static const Iop::Description d; 

};

static Iop* build(Node* node)
{
	return new NukeWrapper(new RankFilter(node));
}

const Iop::Description RankFilter::d (CLASS, "Filter/RankFilter", build);

void RankFilter::_validate(bool for_real)
{
	copy_info();
	info_.pad( _size );
}

void RankFilter::_request(int x, int y, int r, int t, ChannelMask channels, int count)
{
	input(0)->request( x - _size, y - _size, r + _size, t + _size, channels, count);
}

void RankFilter::engine(int y, int x, int r, ChannelMask channels, Row& row)
{

	Tile tile( input0(), x - _size, y - _size, r + _size, y + _size, channels);
	if (aborted()) { return; }

	if (! tile.valid()) {
		foreach(z, channels) {
			float* outptr = row.writable(z) + x;
			memset(&outptr[tile.x()], 0, (tile.r() -  tile.x()) * sizeof(float));
		}

		return;
	}

	foreach(z, channels) {
		
		float* outptr = row.writable(z) + x;

		for(int cur = x; cur < r; cur++) {

			vector<float> v;

			for (int px = -_size; px < _size; px++) {
				for (int py = -_size; py < _size; py++)
					v.push_back(tile[z][tile.clampy(y + py)][tile.clampx(cur + px)]);
			}

			sort(v.begin(),v.end());

			size_t Q1 = v.size() / 4;
			size_t Q2 = v.size() / 2;
			size_t Q3 = Q2 + Q1;

			sort(v.begin(),v.end());
			
			size_t mid = v.size()/2;
			//float q2 = v.size()%2 == 0 ? (v[mid] + v[mid-1])/2 : v[mid];
			float q1, q3;

			if (v.size()%2 == 0) {
				q1 = mid%2 == 0 ? (v[mid/2] + v[mid/2-1])/2 : v[mid/2];
				q3 = mid%2 == 0 ? (v[mid + mid/2] + v[mid + mid/2 -1])/2 : v[mid + mid/2];
			}
			else {
				v.erase(v.begin() + mid);
				q1 = mid%2 == 0 ? (v[mid/2] + v[mid/2-1])/2 : v[mid/2];
				q3 = mid%2 == 0 ? (v[mid + mid/2] + v[mid + mid/2 -1])/2 : v[mid + mid/2];
			}

			float iqr = q3 - q1 + _bias;
			float _lowO = q1 - _value*iqr;
			float _highO = q3 + _value*iqr;

			float pxl = tile[z][y][cur];

			*outptr++ = pxl < _lowO || pxl > _highO ? 1.0f : 0.0f;

		}

	}

}

void RankFilter::knobs(Knob_Callback f)
{
	Int_knob(f, &_size, IRange(1,50,true), "Kernel");
	Float_knob(f, &_value, "Value");
	Float_knob(f, &_bias, "Bias");
}