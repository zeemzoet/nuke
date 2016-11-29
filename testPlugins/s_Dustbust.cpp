static const char* const CLASS = "s_Dustbust";
static const char* const HELP =
"s_Dustbust tries to remove dust in an"
"intelligent way, using simple math and"
"motionvectors.";

// Standard plug-in include files.
#include "DDImage/Iop.h"
#include "DDImage/Row.h"
#include "DDImage/Tile.h"
#include "DDImage/Knobs.h"
#include "DDImage/MultiTile.h"
#include "DDImage/Pixel.h"
#include "DDImage/Vector2.h"
#include "DDImage/NukeWrapper.h"

using namespace DD::Image;

class s_Dustbust : public Iop
{

	Channel uv[4];

public:

  //! Constructor. Initialize user controls to their default values.
  s_Dustbust (Node* node) : Iop (node)
  {
	  uv[0] = uv[1] = uv[2] = uv[3] = Chan_Black;
  }

  ~s_Dustbust () {}

  int maximum_inputs() const { return 1; }
  int minimum_inputs() const { return 1; }

  int split_input(int n) const { return 3; }
  const OutputContext& inputContext(int, int, OutputContext&) const;
  
  void _validate(bool) { copy_info(); }
  void in_channels(int, ChannelSet& mask) const
  {
	mask += (uv[0]);
	mask += (uv[1]);
	mask += (uv[2]);
	mask += (uv[3]);
  }
  void _request(int x, int y, int r, int t, ChannelMask channels, int count)
  {
	  ChannelSet c(channels);
	  in_channels(0, c);
	  int X = input0().info().x();
	  int Y = input0().info().y();
	  int R = input0().info().r();
	  int T = input0().info().t();
	  for(int i = 0; i <= 2; i++)
		  input(i)->request( X, Y, R, T, c, count*2 );
  }
  
  mFnDDImageMultiTileGenerateEngineInline(input0())
  template<class TileType> void doEngine(int, int, int, ChannelMask, Row&);

  virtual void knobs(Knob_Callback);

  //! Return the name of the class.
  const char* Class() const { return CLASS; }
  const char* node_help() const { return HELP; }

private:

  //! Information to the plug-in manager of DDNewImage/NUKE.
  static const Iop::Description d;
}; 

const OutputContext& s_Dustbust::inputContext(int i, int n, OutputContext& context) const
{
	context = outputContext();
	switch(n) {
	case 0:
		break;
	case 1:
		context.setFrame(context.frame() - 1);
		break;
	case 2:
		context.setFrame(context.frame() + 1);
		break;
	}
	return context;
}

static Iop* build(Node* node)
{
	return (new NukeWrapper( new s_Dustbust(node)))->noMix()->noMask();
}

const Iop::Description s_Dustbust::d ( CLASS, "Filter/s_Dustbust", build );

template<class TileType> void s_Dustbust::doEngine(int y, int x, int r, ChannelMask channels, Row& out)
{
	ChannelSet c(channels);
	in_channels(0, c);
	TileType tile(*input(0), x - 1, y - 1, r + 1, y + 2, c); // *input(0) = input0()
	//TileType prevTile(*input(1), x - 1, y - 1, r + 1, y + 2, c);
	//TileType nextTile(*input(2), x - 1, y - 1, r + 1, y + 2, c);
	
	if (aborted())
		return;

	// forward motionvectors
	Channel fw_uu = uv[0];
	Channel fw_vv = uv[1];
	if(!intersect(tile.channels(), fw_uu))
		fw_uu = Chan_Black;
	if(!intersect(tile.channels(), fw_vv))
		fw_vv = Chan_Black;

	const typename TileType::RowPtr fwU0 = tile[fw_uu][y];
	const typename TileType::RowPtr fwV0 = tile[fw_vv][y];
	const typename TileType::RowPtr fwU1 = tile[fw_uu][tile.clampy(y + 1)];
	const typename TileType::RowPtr fwV1 = tile[fw_vv][tile.clampy(y + 1)];

	// backward motionvectors
	Channel bw_uu = uv[2];
	Channel bw_vv = uv[3];
	if(!intersect(tile.channels(), bw_uu))
		bw_uu = Chan_Black;
	if(!intersect(tile.channels(), bw_vv))
		bw_vv = Chan_Black;

	const typename TileType::RowPtr bwU0 = tile[bw_uu][y];
	const typename TileType::RowPtr bwV0 = tile[bw_vv][y];
	const typename TileType::RowPtr bwU1 = tile[bw_uu][tile.clampy(y + 1)];
	const typename TileType::RowPtr bwV1 = tile[bw_vv][tile.clampy(y + 1)];

	foreach(z, channels) out.writable(z);
	InterestRatchet interestRatchet;
	Pixel fw_pixel(channels);
	Pixel bw_pixel(channels);
	fw_pixel.setInterestRatchet(&interestRatchet);
	bw_pixel.setInterestRatchet(&interestRatchet);

	for(; x < r; x++) {

		if (aborted()) break;

		int xx = x + 1;
		if (xx >= info_.r())
			xx = info_.r() - 1;

		Vector2 bw_center(bwU0[x] + x + 1.5f,
					      bwV0[x] + y + 1.5f);
		Vector2 bw_du(bwU0[xx] - bwU0[x] + 2,
					  bwV0[xx] - bwV0[x] + 1);
		Vector2 bw_dv(bwU1[x] - bwU0[x] + 1,
			          bwV1[x] - bwV0[x] + 2);

		input(1)->sample(bw_center, bw_du, bw_dv, bw_pixel);

		Vector2 fw_center(fwU0[x] + x + 0.5f,
					      fwV0[x] + y + 0.5f);
		Vector2 fw_du(fwU0[xx] - fwU0[x] + 1,
				      fwV0[xx] - fwV0[x]);
		Vector2 fw_dv(fwU1[x] - fwU0[x],
			          fwV1[x] - fwV0[x] + 1);

		input(2)->sample(fw_center, fw_du, fw_dv, fw_pixel);

		foreach(z, channels)
			((float*)(out[z]))[x] = (fw_pixel[z] + bw_pixel[z])/2.0f;
	}
}

void s_Dustbust::knobs(Knob_Callback f)
{
	Input_Channel_knob(f, uv, 4, 0, "vectors", "Motion Vectors");
}