#include "GraphCut.h"

GraphCut::GraphCut (Node* node) : Iop (node) {
}

/*! This is a function that creates an instance of the operator, and is
   needed for the Iop::Description to work.
 */
static Iop* GraphCutCreate(Node* node) {

	return new GraphCut(node);
}

/*! The Iop::Description is how NUKE knows what the name of the operator is,
   how to create one, and the menu item to show the user. The menu item may be
   0 if you do not want the operator to be visible.
 */
const Iop::Description GraphCut::description ( CLASS, "Examples/GraphCut", GraphCutCreate );


const char* const GraphCut::CLASS = "GraphCut";
const char* const GraphCut::HELP = "Segmentation Using Graphcut";

const char* GraphCut::input_label(int n, char*) const {
	switch (n) {
		case 0: return "ColourImage";
		case 1: return "ForeGround";
		case 2: return "BackGround";
		default: return "";
	}
}

int GraphCut::maximum_inputs() const {return 3;}
int GraphCut::minimum_inputs() const {return 3;}

void GraphCut::_validate(bool for_real) {
	//the most important thing for this function is to call validate 

	// uncomment for more than 1 input
	for(int i=0; i<inputs(); i++) {
		Op * thisInput = input(i);
		if(thisInput) {
			thisInput->validate(for_real);
		}
	}

	copy_info(0);					// copy bbox channels etc from input0, which will validate it.
	info_.channels(Mask_RGBA);		// these are the channels we are interested in for the Plugin	(Tells channel to use rgb values)

	_isFirstTime = true;
}

void GraphCut::_request(int x, int y, int r, int t, ChannelMask channels, int count)
{
	// for this example, we're only interested in the RGB channels
	input(0)->request(ChannelMask(Mask_RGBA), count);		// this version requests the entire frame		(asks for the rgb value of desired channel)
	input(1)->request(ChannelMask(Mask_Alpha), count);		// we're just going to want the alpha channel of these
	input(2)->request(ChannelMask(Mask_Alpha), count);
	//input(0)->request( x, y, r, t, ChannelMask(Mask_RGB), count );	//this version only requests the portion of the frame indicated by the box (x,y,r,t)
}

ChannelSetInit GraphCut::fetchImage(eeImage::Image<double>& img, int n, int offset, ChannelSet channels) {
	ChannelSet imgChannels = Mask_None;
	bool fetch_src_ok = true;

	Iop * theInput = (Iop *)input(n,offset);
	if(!theInput) {
		error("Iop input not found.");
		fetch_src_ok = false;
		return Mask_None;
	}

	Format format = theInput->format();

	/* these useful format variables are used later */
	const int fx = format.x();
	const int fy = format.y();
	const int fr = format.r();
	const int ft = format.t();

	const int height = ft - fy ;
	const int width = fr - fx ;

	channels &= theInput->info().channels();		// only use the channel we have in our mask and those that are available (8 for just alpha, 7 for RGB, 15 for RGBA, ie set bit for each channel) 

	int nchannels = channels.size();

	if (nchannels == 0)
		return Mask_None;

	/* if everything's good, we're going to copy the data from our Tile */		
	if( fetch_src_ok ) {

		std::vector<double>::iterator imgIt;

		// Grab and lock the input
		if (aborted())
		  return Mask_None;

		Tile tile(*theInput, channels);

		int tilefx = tile.x();
		int tilefr = tilefx + tile.w();
		int tilefy = tile.y();
		int tileft = tilefy + tile.h();

		int chanInd; //this is a channel counter
		Channel chan; //this will represent the actual ID of the channels that are provided by the Tile
		for (chanInd = 0, chan = channels.first(); chanInd < nchannels; chanInd++, chan = channels.next(chan) ) {
			//this calculates which channels are actually in the tile
			if (intersect(tile.channels(),chan))
				imgChannels += (ChannelSetInit)(1 << (chan - 1)); 
			}

			nchannels = imgChannels.size();

			if (nchannels==0) {
				return Mask_None;
			}

			const float** rowChan = new const float*[nchannels];
			img.set(width,height,nchannels);

			for ( int ry = fy; ry < ft; ry++) {
				if (ry<tilefy || ry>=tileft) {
					continue;
				}

			for (chanInd = 0, chan = channels.first(); chanInd < nchannels; chanInd++, chan = channels.next(chan)) {
				//direct each channel pointer to the appropriate memory location if that channel exists   
				if (intersect(tile.channels(),chan)) {
					rowChan[chanInd] = &tile[chan][ry][tilefx];
				}
				else {
					rowChan[chanInd] = NULL;
				}
			}

			//read in the image row by row
			imgIt = img._data.begin() + (tilefx + ry*width)*nchannels;

			for (int rx = tilefx; rx < tilefr; ++rx) {
				for (chanInd = 0; chanInd < nchannels; chanInd++) {
					//only fills the array for channels that exist in the tile
					if(rowChan[chanInd]) {
						*imgIt++ = *(rowChan[chanInd])++;
					}
				}
			}
		} // end main for loop

		delete [] rowChan;

	} // end if statement

	return imgChannels.bitwiseAnd(Mask_All);
}

/*! For each line in the area passed to request(), this will be called. It must
   calculate the image data for a region at vertical position y, and between
   horizontal positions x and r, and write it to the passed row
   structure. Usually this works by asking the input for data, and modifying
   it.

 */
void GraphCut::engine ( int y, int x, int r, ChannelMask channels, Row& outRow ) {
	/* engine calls are multi-threaded so any processing must be locked */
	//we dont want each thread to read in the image so we allow the first thread through and lock out the others until we are finished
	if (_isFirstTime) {
		Guard guard(_lock);			// creates a lock (bars all treads except first from executing code in chain braces (forces single treaded processing))

		if(_isFirstTime) {

			/* Do on-demand processing here to calculate data needed to update the plugin output */
			_fetch_src_ok = true;

			if (aborted()) {
				error("Iop aborted.");
				_fetch_src_ok = false;
				return;
			}

			// read in images (sets _src equal to values obtained through getImage
			if(_fetch_src_ok ) {													//image does not have to contain an alpha channel but it must contain rgb channels
				_fetch_src_ok = (fetchImage(_src1,0,0,Mask_RGB) == Mask_RGB);		// reads in image data from input number 0 in _src (Image data structure) (the == Mask_RGB is an error check)
				_fetch_src_ok = (fetchImage(_src2,1,0,Mask_Alpha) == Mask_Alpha);		// change the channel mask here to just alpha
				_fetch_src_ok = (fetchImage(_src3,2,0,Mask_Alpha) == Mask_Alpha);		// chnage the channel mask here to just alpha
			}
      
			if(_fetch_src_ok) {
				_algorithm.graphCut(_src1, _src2, _src3, _dst);
			}

			_isFirstTime = false;
		}
	} // end first time pass

	// now write _dst to the red, green and blue channels. ie. each value of _dst gets assigned to the 3 channels separately
	std::vector< double >::iterator dstDataIt; 
  

	// this bit send s output back to NUKE
	if(_fetch_src_ok) {
		foreach(chan,channels) {					// for each channel for which a new value is requested

			if (intersect(Mask_Alpha, chan)) {		// make sure that it is only a red, green or blue channel (will just change this alpha at the end)
				dstDataIt = _dst._data.begin() + (_dst._width * y + x);

				float * row_out = outRow.writable(chan) + x;					

				for(int xx=0; xx<r-x; xx++) {
					row_out[xx] = float(*dstDataIt++);
				}
			}
			else {									// this is for the case where channels are requested that we dont expect. It shouldn't happen if we did our job right
				ChannelMask chanMask = chan;
				input0().get(y, x, r, chanMask, outRow);		//just copy it from the input
			}
		}		// end foreach
	}
}		// end function

	// Knobs function
void GraphCut::knobs(Knob_Callback f) {

	int binWidth = _algorithm.getNumBins();								// get the bin width of the likelihood distribution histograms
	int tempBinWidth = _algorithm.getNumBins();
	double likelihoodWeight = _algorithm.getLikelyhoodWeight();			// get the effetive weight of the likelihood component
	double tempLikelihoodWeight = _algorithm.getLikelyhoodWeight();

		// knobs for adjusting window size and weights
	Int_knob(f, &binWidth, IRange(0, 1), "Bin Width", "BinWidth");
	Double_knob(f, &likelihoodWeight, IRange(0, 10), "Likelihood Weight", "LikelihoodWeight");

	_algorithm.setNumBins(binWidth);						// set the new bin Width
	_algorithm.setLikelihoodWeight(likelihoodWeight);		// set new Likelihood Weight

		// if we change the binwidth or the likelihood weight we will need to recalculate
	if ((binWidth != tempBinWidth) || (likelihoodWeight != tempLikelihoodWeight)) {
		if (_algorithm.getMaxFlowCalculated() == true) {
			_algorithm.deleteGraph();
		}
		_algorithm.setMaxFlowCalculated(false);
	}

	return;
}