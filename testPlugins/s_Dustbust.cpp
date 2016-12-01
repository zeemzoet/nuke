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
#include "DDImage/Thread.h"
#include "DDImage/NukeWrapper.h"

using namespace DD::Image;

#include <algorithm>
#include <vector>

using namespace std;

class s_Dustbust : public Iop
{
private:

	Channel uv[4];
	ChannelSet prevFrame, nextFrame;
	Channel _prevFrame[3];
	Channel _nextFrame[3];
	
	bool _firstTime;
	Lock _lock;
	
	int _size;

public:

  //! Constructor. Initialize user controls to their default values.
  s_Dustbust (Node* node) : Iop (node)
  {
	  uv[0] = uv[1] = uv[2] = uv[3] = Chan_Black;
	  
	  prevFrame = nextFrame = Mask_None;
	  _prevFrame[0] = getChannel("_prevFrame.r");
	  _prevFrame[1] = getChannel("_prevFrame.g");
	  _prevFrame[2] = getChannel("_prevFrame.b");
	  _nextFrame[0] = getChannel("_nextFrame.r");
	  _nextFrame[1] = getChannel("_nextFrame.g");
	  _nextFrame[2] = getChannel("_nextFrame.b");
	  
	  _firstTime = true;
	  
	  _size = 2;
  }

  ~s_Dustbust () {}

  int maximum_inputs() const { return 1; }
  int minimum_inputs() const { return 1; }

  int split_input(int n) const { return 3; }
  const OutputContext& inputContext(int, int, OutputContext&) const;
  
  void _validate(bool) 
  {
	  copy_info();
	  prevFrame.insert(_prevFrame, 3);
	  nextFrame.insert(_nextFrame, 3);
  }
  
  void in_channels(int, ChannelSet& mask) const
  {
	  for(int i = 0; i < 4; i++)
		  mask += (uv[i]);
	  
	  for(int i = 0; i < 3; i++) {
		  mask += _prevFrame[i];
		  mask += _nextFrame[i];
	  }
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
	int X = x;
	
	ChannelSet c(channels);
	in_channels(0, c);
	TileType tile(*input(0), x - 1, y - 1, r + 1, y + 2, c); // *input(0) = input0()
	{
		Guard guard(_lock);
		if (_firstTime) {
			
			if (aborted())
				return;

			// forward motionvectors
			Channel fw_uu = uv[0];
			Channel fw_vv = uv[1];
			if(!intersect(tile.channels(), fw_uu))
				fw_uu = Chan_Black;
			if(!intersect(tile.channels(), fw_vv))
				fw_vv = Chan_Black;

			// backward motionvectors
			Channel bw_uu = uv[2];
			Channel bw_vv = uv[3];
			if(!intersect(tile.channels(), bw_uu))
				bw_uu = Chan_Black;
			if(!intersect(tile.channels(), bw_vv))
				bw_vv = Chan_Black;

			//foreach(z, channels) out.writable(z);
			
			InterestRatchet interestRatchet;
			
			Pixel fw_pixel(channels), bw_pixel(channels);
			
			fw_pixel.setInterestRatchet(&interestRatchet);
			bw_pixel.setInterestRatchet(&interestRatchet);

			for (; x < r; x++) {

				if (aborted()) break;

				int xx = x + 1;
				if (xx >= info_.r())
					xx = info_.r() - 1;

				Vector2 bw_center(tile[bw_uu][y][x] + x + 0.5f,
								  tile[bw_vv][y][x] + y + 0.5f);
								  
				Vector2 bw_du(tile[bw_uu][y][xx] - tile[bw_uu][y][x] + 1,
							  tile[bw_vv][y][xx] - tile[bw_vv][y][x]);
							  
				Vector2 bw_dv(tile[bw_uu][tile.clampy(y + 1)][x] - tile[bw_uu][y][x],
							  tile[bw_vv][tile.clampy(y + 1)][x] - tile[bw_vv][y][x] + 1);

				input(1)->sample(bw_center, bw_du, bw_dv, bw_pixel);

				Vector2 fw_center(tile[fw_uu][y][x] + x + 0.5f,
								  tile[fw_vv][y][x] + y + 0.5f);
								  
				Vector2 fw_du(tile[fw_uu][y][xx] - tile[fw_uu][y][x] + 1,
							  tile[fw_vv][y][xx] - tile[fw_vv][y][x]);
							  
				Vector2 fw_dv(tile[fw_uu][tile.clampy(y + 1)][x] - tile[fw_uu][y][x],
							  tile[fw_vv][tile.clampy(y + 1)][x] - tile[fw_vv][y][x] + 1);

				input(2)->sample(fw_center, fw_du, fw_dv, fw_pixel);

				ChannelSet m(Mask_RGB);
				foreach(z, m) {
					*(out.writable( _prevFrame[colourIndex(z)] ) + x) = bw_pixel[z];
					*(out.writable( _nextFrame[colourIndex(z)] ) + x) = fw_pixel[z];
				}
			}
			
			_firstTime = false;
		}
	} // end of lock

	//reset loop
	x = X;
	
	foreach(z, channels) {
		
		if (colourIndex(z) < 3) {
			for (; x < r; x++) 
				*(out.writable(z) + x) = 0.5f;
		}
		else
			input(0)->get(y, x, r, z, out);
	}		
}

void s_Dustbust::knobs(Knob_Callback f)
{
	Input_Channel_knob(f, uv, 4, 0, "vectors", "Motion Vectors");
}
