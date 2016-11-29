#define _SECURE_SCL 0

static const char* const CLASS = "WorldToNormals";
static const char* const HELP = "Procedurally creates a normal pass of the world position info";
static const char* const bitmaskKnobNames[] = { "x", "y", "z", 0};

#include "DDImage/Iop.h"
#include "DDImage/Tile.h"
#include "DDImage/Row.h"
#include "DDImage/Knobs.h"

#define FLIPX 1 //2^0
#define FLIPY 2 //2^1
#define FLIPZ 4 //2^2

using namespace DD::Image;

class WorldToNormals : public Iop
{
private:

	int _size;
	enum _direction {UP, SIDE};

	ChannelSet Xco;
	ChannelSet Yco;
	Channel xcoords[3];
	Channel ycoords[3];

	unsigned int _bitmaskKnob;

	void getXYcoordinates(int y, int x, int r, ChannelMask channels, Row &out, _direction dir);

public:

	WorldToNormals(Node *node) : Iop(node)
	{
		_size = 2;

		Xco = Mask_None;
		Yco = Mask_None;
		xcoords[0] = getChannel("xco.x");
		xcoords[1] = getChannel("xco.y");
		xcoords[2] = getChannel("xco.z");
		ycoords[0] = getChannel("yco.x");
		ycoords[1] = getChannel("yco.y");
		ycoords[2] = getChannel("yco.z");

		_bitmaskKnob = 0;
	}

	~WorldToNormals() {}

	void _validate(bool);
	void _request(int x, int y, int r, int t, ChannelMask channels, int count);
	void engine(int y, int x, int r, ChannelMask channels, Row &out);

	void knobs(Knob_Callback f);

	const char* Class() const { return CLASS; }
	const char* node_help() const { return HELP; }

	static const Description d;
};

//NUKE node creation

static Op* build(Node *node) { return new WorldToNormals(node); }

const Op::Description WorldToNormals::d ( CLASS, "Image/WorldToNormals", build);

//NUKE call order

void WorldToNormals::_validate(bool for_real)
{
	copy_info(Mask_RGB);
	info_.pad( _size);

	Xco.insert(xcoords, 3);
	Yco.insert(ycoords, 3);

	//set_out_channels(Mask_All); //only output the RGB channels
}

void WorldToNormals::_request(int x, int y, int r, int t, ChannelMask channels, int count)
{
	input0().request( x - _size, y - _size, r + _size, t + _size, channels, count);
}

void WorldToNormals::engine(int y, int x, int r, ChannelMask channels, Row &row)
{
	ChannelSet m(channels);
	m -= Mask_RGB;
	input0().get(y,x,r,m,row); //pass every non-RGB channel trough

	getXYcoordinates(y,x,r,channels,row,UP);
	getXYcoordinates(y,x,r,channels,row,SIDE);

	const float *xx = row[xcoords[0]] + x;
	const float *xy = row[xcoords[1]] + x;
	const float *xz = row[xcoords[2]] + x;
	
	const float *yx = row[ycoords[0]] + x;
	const float *yy = row[ycoords[1]] + x;
	const float *yz = row[ycoords[2]] + x;

	if (xx == NULL || xy == NULL || xz == NULL)
		row.erase(Xco);
	else if (yx == NULL || yy == NULL || yz == NULL)
		row.erase(Yco);
	else
	{
		float *TOr = row.writable(Chan_Red) + x;
		float *TOg = row.writable(Chan_Green) + x;
		float *TOb = row.writable(Chan_Blue) + x;
		const float *END = TOr + (r - x);

		while (TOr < END){
			Vector3 sideVec(*xx++,*xy++,*xz++);	//create the sideVector
			Vector3 upVec(*yx++,*yy++,*yz++); //create the upVector

			Vector3 crossVec = upVec.cross(sideVec);
			crossVec.normalize();
			Vector3 flipCrossVec(crossVec);
			flipCrossVec.negate();

			(_bitmaskKnob & FLIPX) == FLIPX ? *TOr++ = flipCrossVec[0] : *TOr++ = crossVec[0];
			(_bitmaskKnob & FLIPY) == FLIPY ? *TOg++ = flipCrossVec[1] : *TOg++ = crossVec[1];
			(_bitmaskKnob & FLIPZ) == FLIPZ ? *TOb++ = flipCrossVec[2] : *TOb++ = crossVec[2];
		}
	}
}

void WorldToNormals::knobs(Knob_Callback f)
{
	Bitmask_knob(f, &_bitmaskKnob, bitmaskKnobNames, "Flip");
}

//Private functions

void WorldToNormals::getXYcoordinates(int y, int x, int r, ChannelMask channels, Row &row, _direction dir)
{
	Tile tile( input0(), x - _size , y - _size , r + _size, y + _size + 1 , channels);
	if ( aborted() ) 
		return;

	ChannelSet m(Mask_RGB);

	foreach(z, m)
	{
		float *outptr = 0;
		if (dir == UP)
			outptr = row.writable(ycoords[colourIndex(z)]) + x;
		else
			outptr = row.writable(xcoords[colourIndex(z)]) + x;
		
		for( int p = x; p < r; p++ ) //p = current pixel
		{
			float value = 0;

			if( intersect( tile.channels(), z))
			{
			for( int px = -_size; px <= _size; px++ ){		//this loops from -2 to 2
				for( int py = -_size; py <= _size; py++ ){	//so we have a nice 3D plot fallof
					
					float weight = 0;

					if (dir == UP)
						weight = py * exp( -pow(float(px), 2.0f) -pow(float(py), 2.0f));
					else
						weight = px * exp( -pow(float(px), 2.0f) -pow(float(py), 2.0f));

					value += (tile[z][ tile.clampy( y + py ) ][ tile.clampx( x + p + px ) ]) * weight;
				}}
			}
			*outptr++ = value;
		}
	}
}