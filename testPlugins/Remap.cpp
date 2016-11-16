#include "DDImage/PixelIop.h"
#include "DDImage/Row.h"
#include "DDImage/Knobs"
#include "DDImage/DDImage"

static const char* const HELP =
"This node remaps the input between 2 values "
"using a linear interpolation algoritmh"

class RemapIop : public PixelIop
{

	double lowValue, highValue;

public:

	RemapIop(Node* node) : PixelIop(node)
	{
		lowValue = 0.0f;
		highValue = 1.0f;
	}

	void _validate(bool);
	void in_channels(int, ChannelSet&) const;
	void pixel_engine(const Row&, int, int, int, ChannelMask, Row&);
	
	static const Iop::Description d;
	
	const char* Class() const { return d.name; }
	const char* node_help() const { return HELP; }

};

void RemapIop::_validate(bool for_real)
{
	copy_info();
	if(lowValue == 0.0f && highValue == 1.0f)
		set_out_channels(Mask_None)
	else
		set_out_channels(Mask_All)
}

void RemapIop::in_channels(int input, ChannelSet &mask)
{
	// mask is unchanged, PixelIop's variant on request
}

void RemapIop::pixel_engine(const Row& in, int y, int x, int r, ChannelMask channels, Row& out)
{
	foreach (z, channels){
		const float* inptr = in[z] + x;
		const float* END = inptr + (r - x)
		float* outptr = out.writable(z) + x
		
		if(highValue != 0){
			const float c = lowValue/highValue;
			while(inptr < END)
				*outptr++ = lowValue + *inptr - c * *inptr++
		}
	}
}