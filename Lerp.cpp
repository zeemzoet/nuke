#include "DDImage/PixelIop.h"
#include "DDImage/Row.h"
#include "DDImage/Knobs"
#include "DDImage/DDImage"

static const char* const HELP =
"This node remaps the input between 2 values "
"using a linear interpolation algoritmh"

class LerpIop : public PixelIop
{

	double lowValue, highValue;

public:

	LerpIop(Node* node) : PixelIop(node)
	{
		lowValue = 0.0f;
		highValue = 1.0f;
	}

	void _validate(bool);
	void pixel_engine(const Row& in, int y, int x, int r, ChannelMask channels, Row& out);
	
	static const Iop::Description d;
	
	const char* Class() const { return d.name; }
	const char* node_help() const { return HELP; }

};

void LerpIop::_validate(bool for_real)
{
	copy_info();
	if(lowValue == 0.0f && highValue == 1.0f)
		set_out_channels(Mask_None)
	else
		set_out_channels(Mask_All)
}