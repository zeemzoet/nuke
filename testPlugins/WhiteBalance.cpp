#include "DDImage/PixelIop.h"
#include "DDImage/Row.h"
#include "DDImage/Knobs.h"

using namespace DD::Image;

static const char* const CLASS = "WhiteBalanceIop"
static const char* const HELP =
"This node set's the whitebalance of the image, "
"based on the users input.";

class WhiteBalanceIop : public PixelIop
{
	
	float value[4];
	
public:
	
	WhiteBalanceIop(Node* node) : PixelIop(node)
	{
		value[0] = value[1] = value[2] = value[3] = 1.0f;
	}
	
	void _validate(bool);
	void in_channels(int, ChannelSet&) const;
	void pixel_engine(const Row&, int, int, int, ChannelMask, Row&);
	bool pass_transform() const { return true; }
	virtual void knobs(Knob_Callback)
	
	static const Iop::Description d;
	
	const char* Class() const { return CLASS; }
	const char* node_help() const { return HELP; }
	
};

void WhiteBalanceIop::_validate(bool for_real)
{
	copy_info();
	for (unsigned i = 0; i < 4; i++) {
		if (value[i] != 1.0f) {
			set_out_channels(Mask_ALL);
			return;
		}
	}
	set_out_channels(Mask_None);
}

void WhiteBalanceIop::in_channels(int input, ChannelSet& mask) const
{
	if ( !(mask.contains(Mask_RGB)))
		mask += (Mask_RGB);
}

void WhiteBalanceIop::pixel_engine(Row& in, int y, int x, int r, ChannelMask channels, Row& out)
{
	foreach(z, channels) {
		
		if(colourIndex(z) >= 3) {
			out.copy(in, z, x, r)
			continue;
		}
		else {
		
			const float c = value[z]
			float avgC = 0.3f * value[0] + 0.59f * value[1] + 0.11f * value[2];
		
			const float inptr* = in[z] + x;
			const float END* = inptr + (r - x);
			float* outptr = out.writable(z) + x;
		
			while (inptr < END)
				c != 0.0f ? *outptr++ = *inptr++ * avgC / c : *outptr++ = *inptr++;
		}
	}
}
