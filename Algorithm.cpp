#include "Algortihm.h"

	// constructors and destructors
Algorithm::Algorithm() {

	_numBins = 20;
	_likelihoodWeight = 1.0;
	_maxFlowCalculated = false;
	_K = 0;
}

Algorithm::~Algorithm() {

}

	// get and set methods
int Algorithm::getNumBins() {
	return (_numBins);
}
double Algorithm::getLikelyhoodWeight() {
	return(_likelihoodWeight);
}
bool Algorithm::getMaxFlowCalculated() {
	return(_maxFlowCalculated);
}

void Algorithm::setNumBins(int BW) {
	_numBins = BW;
}
void Algorithm::setLikelihoodWeight(double LW) {
	_likelihoodWeight = LW;
}
void Algorithm::setMaxFlowCalculated(bool t) {
	_maxFlowCalculated = t;
}

void Algorithm::deleteGraph() {	
	_graph->reset();
}

// --------------------------------------------- //
// --------------- Main Function --------------- //
// --------------------------------------------- //

void Algorithm::graphCut(const eeImage::Image<double> &colourImg, const eeImage::Image<double> &foreground, const eeImage::Image<double> &background, eeImage::Image<double> &segmentation) {

		// parameters
	eeImage::Image<double> grayImage;						// stores gray version of passed in image

	eeImage::Image<double> foregroundPixels;					// stores pixels marked as definite foreground
	eeImage::Image<double> backgroundPixels;					// stores pixels marked as definite background

	std::vector<double> foregroundHistogram;					// stores histogram of foreground
	std::vector<double> backgroundHistogram;					// stores histogram of background

	std::vector<double> foregroundLikelihood;					// stores likelihood model of foreground
	std::vector<double> backgroundLikelihood;					// stores likelihood model of background

		// functions
	convertToGrayscale(colourImg, grayImage);						// convert Image to GrayScale

	getAlphaPixels(grayImage, foreground, foregroundPixels);		// Extract the foreground Pixels from the Gray Image
	getAlphaPixels(grayImage, background, backgroundPixels);		// Extract the foreground Pixels from the Gray Image

	// checkSeedOverlap(foregroundPixels, backgroundPixels);

		// crate new graph if this is the first pass
	//if (!_maxFlowCalculated) {

		createHistogram(foregroundPixels, foreground, foregroundHistogram);			// calculate histogram of intensity values marked as foreground
		createHistogram(backgroundPixels, background, backgroundHistogram);			// calculate histogram of intensity values marked as background

		createLikelihoodModel(foregroundHistogram, foregroundLikelihood);	// create liklihood model of foreground from histogram
		createLikelihoodModel(backgroundHistogram, backgroundLikelihood);	// create liklihood model of background from histogram

		_graph = buildGraph(grayImage, foreground, background, foregroundLikelihood, backgroundLikelihood);		// create the graph based on the likelihood models and the prior based on the image intensities
		maximumFlow(_graph, grayImage, segmentation);															// use maximum flow algortihm to segment image

		_maxFlowCalculated = true;
	//}
		// otherwise we update the likelihoods and run graph starting with current max flow
	// else {
	 	// updateGraph(_graph, foreground, background, segmentation);
	//	maximumFlow(_graph, grayImage, segmentation);	
	//}

	_previousforeground.set(foregroundPixels._width, foregroundPixels._height, foregroundPixels._channels, foregroundPixels._data);		// store the current foreground pixels for the next pass
	_previousbackground.set(backgroundPixels._width, backgroundPixels._height, backgroundPixels._channels, backgroundPixels._data);		// store the current background pixels for the next pass

	return;
}


// --------------------------------------------- //
// ------------- Grayscale Function ------------ //
// --------------------------------------------- //

	// function converts an image to grayscale
void Algorithm::convertToGrayscale(const eeImage::Image<double> &colourImage, eeImage::Image<double> &grayImage) {

	grayImage.set(colourImage._width, colourImage._height, 1, 0.0);	

	std::vector<double>::const_iterator colourImageIt;								
	std::vector<double>::iterator grayimageIt;

			// for when we have an RGB image
	if (colourImage._channels == 3) {
		for (colourImageIt = colourImage._data.begin(), grayimageIt = grayImage._data.begin(); colourImageIt < colourImage._data.end(); colourImageIt+=colourImage._channels, grayimageIt++) {
			*grayimageIt = 0.299 * (*colourImageIt) + 0.587 * (*(colourImageIt+1)) + 0.114 * (*(colourImageIt+2));		
		}
	}

			// when we have a grayscale image
	else if (colourImage._channels == 1) {
		for (colourImageIt = colourImage._data.begin(), grayimageIt = grayImage._data.begin(); colourImageIt < colourImage._data.end(); colourImageIt+=colourImage._channels, grayimageIt++) {
			*grayimageIt = (*colourImageIt);		
		}
	}

	else {
		printf("Incorrcet number of channels. There are %f channels \n" ,colourImage._channels);
	}

	return;
}

// ---------------------------------------------- //
// ---- Get the Pixels Identified By Strokes ---- //
// ---------------------------------------------- //

void Algorithm::getAlphaPixels(const eeImage::Image<double> &grayImage, const eeImage::Image<double> &strokeImage, eeImage::Image<double> &strokePixels) {

		// check inputs are the same size
	if ((grayImage._width != strokeImage._width) || (grayImage._height != strokeImage._height)) {
		printf("\nColour Image and backgroun/foreground strokes image not the same size");
		return;
	}

		// check we have correct number of channels
	if ((grayImage._channels != 1) || (strokeImage._channels != 1)) {
		printf("\nIncorrect number of channels in the grayIMage or Foreground/background strokes Image");
		return;
	}

	strokePixels.set(grayImage._width, grayImage._height, 1, 0.0);		// create an image to store the pixels identified by the strokes

	std::vector<double>::const_iterator grayImageIt;			// iterator for gray image
	std::vector<double>::const_iterator strokeImageIt;			// iterator for stroke image
	std::vector<double>::iterator strokePixelsIt;				// iterator for stroke pixels

		// find the pixels marked by a stroke and store them in the stroke pixel image
	for (grayImageIt = grayImage._data.begin(), strokeImageIt = strokeImage._data.begin(), strokePixelsIt = strokePixels._data.begin(); 
		strokeImageIt < strokeImage._data.end(); strokeImageIt++, grayImageIt+=grayImage._channels, strokePixelsIt++) {

		if (*strokeImageIt > 0.1) {
			*(strokePixelsIt) = *(grayImageIt);
		}
	}

	return;
}

// ---------------------------------------------- //
// -------------- Create Histograms ------------- //
// ---------------------------------------------- //

// rounding precision is causing an issue here when unit testing

void Algorithm::createHistogram(const eeImage::Image<double> &strokePixels, const eeImage::Image<double> &seedPixels, std::vector<double>& histogram) {

	double binWidth = 1.0/double(_numBins);			// calculate width of each bin
	histogram.resize(_numBins);						// resize vector so same number of elements as bins (default value of elements in vector is zero :)

		// check stroke pixel image has correct number of channels
	if (strokePixels._channels != 1) {
		printf("\nIncorrect Number of channels in stroke pixel image");
		return;
	}

	std::vector<double>::const_iterator strokePixelsIt;			// itterator to move through stroke pixel Image
	std::vector<double>::const_iterator seedPixelsIt;			// itterator to move through stroke pixel Image

		// find non-zero pixels and increment according bin
	for (strokePixelsIt = strokePixels._data.begin(), seedPixelsIt = seedPixels._data.begin(); strokePixelsIt < strokePixels._data.end(); strokePixelsIt++, seedPixelsIt++) {
		if (*(seedPixelsIt) > 0.1) {
			if (*(strokePixelsIt) >= 1.0) {
				histogram.at(19)++;
			}
			else {
				int histIndex = floor(*(strokePixelsIt)/binWidth);
				histogram.at(histIndex)++;
			}
		}
	}
}

// ---------------------------------------------- //
// ---- Create likelihood model From Histogram --- //
// ---------------------------------------------- //

void Algorithm::createLikelihoodModel(const std::vector<double>& Histogram, std::vector<double>& Likelihoodmodel) {

	Likelihoodmodel.resize(_numBins);
	int sum = 0;

		// sum histogram for normalisation
	for (int i = 0; i < _numBins; i++) {
		sum += Histogram.at(i);
	}

	for (int i = 0; i < _numBins; i++) {

			// log of zero give indeterminate value, instead just set really big
		if (Histogram.at(i) == 0) {
			Likelihoodmodel.at(i) = 1000;
		}
		else {
			Likelihoodmodel.at(i) = -log(Histogram.at(i)/double(sum));		// calculate log likelihood (energy) based on normalised histogram
		}
	}

	return;
}

// ---------------------------------------------- //
// ------ Calculate Sigma value for N-Lins ------ //
// ---------------------------------------------- //

double Algorithm::calculateSigma(const eeImage::Image<double> &grayImage) {

	std::vector<double>::const_iterator ImageIt;	

	int imgWidth = grayImage._width;
	int imgHeight = grayImage._height;

	int counter = 0;			// reset counter
	double sigma = 0.0;

		// find all the pixels in a clique
	for (ImageIt = grayImage._data.begin(); ImageIt < grayImage._data.end(); ImageIt++) {
			
		double Iv = *(ImageIt);											// get value of pixel we're looking at
		double sum = 0.0;
		int numCliques = 0;

			// add up link
		if (counter < imgWidth*(imgHeight - 1)) {						// check if we are on top row
			double Iw = *(ImageIt + imgWidth);							// offest by width to get above pixel in image
			sum += (Iv - Iw)*(Iv - Iw);									// square value of difference
			numCliques++;
		}

			// add down link
		//if ((counter + 1) > imgWidth) {									// check if we are on bottom row
		//	double Iw = *(ImageIt - imgWidth);							// offest by -widht to get below pixel in image
		//	sum += (Iv - Iw)*(Iv - Iw);									// square value of difference
		//	numCliques++;
		//}

			// add right link
		if (((counter + 1) % imgWidth) != 0) {		// check if we are on right column
			double Iw = *(ImageIt + 1);									// offest by 1 to get pixel to right in image
			sum += (Iv - Iw)*(Iv - Iw);									// square value of difference
			numCliques++;
		}

			// add left link
		//if ((counter % imgWidth) != 0) {								// check if we are on left column
		//	double Iw = *(ImageIt - 1);									// offest by -1 to get pixel to left in image
		//	sum += (Iv - Iw)*(Iv - Iw);									// square value of difference
		//	numCliques++;
		//}

		if (numCliques != 0) {
			sigma += (sum);			// average squared intensity differences of all maximum cliques for each pixel
		}
		counter+=numCliques;
	}

	sigma /= (imgWidth*imgHeight);			// average each pixel
	return (sigma);
}

// ---------------------------------------------- //
// --------------- Create a Grpah --------------- //
// ---------------------------------------------- //

GraphType* Algorithm::buildGraph(const eeImage::Image<double> &grayImage, const eeImage::Image<double> &foreground, const eeImage::Image<double> &background, std::vector<double>& foregroundLikelihood, std::vector<double>& backgroundLikelihood) {
		
		// iterators
	std::vector<double>::const_iterator ImageIt;	
	std::vector<double>::const_iterator PriorCheckIt;	

		// parameters required to set up graph
	int imgWidth = grayImage._width;
	int imgHeight = grayImage._height;

	int numNodes = imgWidth*imgHeight;
	int numEdges = (imgWidth - 1)*(imgHeight) + (imgWidth)*(imgHeight - 1);

	double binWidth = 1.0/double(_numBins);	
	_K = 0;											// large tLink value for pixels marked by strokes (greater than sum of all nLinks)

		// 1. create a graph of size of image
	GraphType *g = new GraphType(numNodes, numEdges);

		// 2. cycle though entire image creating a node for each pixel and adding it to the graph
	for (ImageIt = grayImage._data.begin(); ImageIt < grayImage._data.end(); ImageIt++) {
		g->add_node();		// add a node for each pixel
	}

		// 3. add nLinks to graph (note that adding up and right edge at everynode will cover all 4-connected edges)
	calculateNLinks(g, grayImage);

		// 4. add tLinks to graph
	calculateTLinks(g, grayImage, foreground, background, foregroundLikelihood, backgroundLikelihood);

	return(g);
}

// ---------------------------------------------- //
// ------------ Calculate Maximum Flow ---------- //
// ---------------------------------------------- //

void Algorithm::maximumFlow(GraphType* g, const eeImage::Image<double> &grayImage, eeImage::Image<double> &segmentation) {

	int flow = g -> maxflow();								// apply max flow algorithm
	int numNodes = grayImage._width*grayImage._height;		// number of nodes in graph

	std::vector<double>::const_iterator ImageIt;							// itterator to move through gary image
	segmentation.set(grayImage._width, grayImage._height, 1, 0.0);			// temp means of setting segmentation image
	std::vector<double>::iterator segmentIt = segmentation._data.begin();	
	ImageIt = grayImage._data.begin();

	for (int i = 0; i < numNodes; i++, segmentIt++, ImageIt++) {
		// keep if pixel is connected to source
		if (g->what_segment(i) == GraphType::SOURCE) {				// check whether first node is Source or Sink
			// printf("node %i is in the SOURCE set\n", i);
			// *(segmentIt) = *(ImageIt);
			*(segmentIt) = 1.0;									// set output to binary
		}

		// remove if pixel connected to sink
		else {
			// printf("node %i is in the SINK set\n", i);
			*(segmentIt) = 0.0;
		}
	}
}

// ---------------------------------------------- //
// ---------- Calculate NLinks for Node --------- //
// ---------------------------------------------- //

void Algorithm::calculateNLinks(GraphType* g, const eeImage::Image<double> &grayImage) {

	std::vector<double>::const_iterator ImageIt;

	int counter = 0;
	int imgWidth = grayImage._width;
	int imgHeight = grayImage._height;

	double sigma = calculateSigma(grayImage);				// get the value of sigma

	// std::vector<std::vector<double>> NLinks(imgWidth*imgHeight, std::vector<double>(2,0));
	// std::vector<std::vector<double>>::iterator truthIt;
	// truthIt = NLinks.begin();

		// 3. add nLinks to graph (note that adding up and right edge at everynode will cover all 4-connected edges)
	for (ImageIt = grayImage._data.begin(); ImageIt < grayImage._data.end(); ImageIt++) {
			
		double Iv = *(ImageIt);									// get value of pixel we're looking at

			// add up link
		if (counter < imgWidth*(imgHeight - 1)) {						// check if we are on top row
			double Iw = *(ImageIt + imgWidth);							// offest by width to get above pixel in image
			double nLink;
			if (Iw != Iv) {
				nLink = exp(-((Iv - Iw)*(Iv - Iw))/(2*sigma*sigma));
				g -> add_edge(counter, counter+imgWidth, nLink, nLink);		// add the edge to the graph
				// (*(truthIt)).at(0) = nLink;
			}
			else {
				nLink = 1;
			}
			_K += (nLink*2);												// sum all the nLinks
		}

			// add right link
		if (((counter + 1) % imgWidth) != 0) {							// check if we are on right column (need to account for special case when counter = 0)
			double Iw = *(ImageIt + 1);									// offest by 1 to get pixel to right in image
			double nLink;
			if (Iw != Iv) {
				nLink = exp(-((Iv - Iw)*(Iv - Iw))/(2*sigma*sigma));		// calculate value of edge (large value the closer the pixels are together (greater cost to cut)
				g -> add_edge(counter, counter+1, nLink, nLink);			// add the edge to the graph
				// (*(truthIt)).at(1) = nLink;
			}
			else {
				nLink = 1;
			}
			_K += (nLink*2);												// sum all the nLinks
		}

		counter++;
	}
}

// ---------------------------------------------- //
// ---------- Calculate TLinks for Node --------- //
// ---------------------------------------------- //

void Algorithm::calculateTLinks(GraphType* g, const eeImage::Image<double> &grayImage, const eeImage::Image<double> &foreground, const eeImage::Image<double> &background, std::vector<double>& foregroundLikelihood, std::vector<double>& backgroundLikelihood) {

	std::vector<double>::const_iterator foregroundIt;	
	std::vector<double>::const_iterator backgroundIt;	
	std::vector<double>::const_iterator ImageIt;

	int counter = 0;								// reset counter
	foregroundIt = foreground._data.begin();
	backgroundIt = background._data.begin();

	double binWidth = 1.0/double(_numBins);	

		// 4. add tLinks to graph
	for (ImageIt = grayImage._data.begin(); ImageIt < grayImage._data.end(); ImageIt++, foregroundIt++, backgroundIt++) {

			// check if pixel marked as foreground
		if (*(foregroundIt) > 0.1) {
			g -> add_tweights(counter, _K, 0);			// somewhere I swapped the tLinks (as a quick fix I have swapped the values here
		}

			// check if pixel marked as background
		else if (*(backgroundIt) > 0.1) {
			g -> add_tweights(counter, 0, _K);
		}

			// calculate source and sink tLink for unmarked pixels
		else {
			int histIndex = floor(*(ImageIt)/binWidth);					// get index of probability in likelihood model
			double tLinkSink = _likelihoodWeight*foregroundLikelihood.at(histIndex);
			double tLinkSource = _likelihoodWeight*backgroundLikelihood.at(histIndex);
			g -> add_tweights(counter, tLinkSource, tLinkSink);
		}
		counter++;
	}
}

// ---------------------------------------------- //
// ----------- Update Graph Likelihood ---------- //
// ---------------------------------------------- //

	// this needs a little more work but it does work
void Algorithm::updateGraph(GraphType* g, const eeImage::Image<double> &foreground, const eeImage::Image<double> &background, eeImage::Image<double> &segmentation) {

	segmentation.set(foreground._width, foreground._height, 1, 0.0);		// will return segmented image

	std::vector<double>::const_iterator foregroundIt;	
	std::vector<double>::const_iterator backgroundIt;
	std::vector<double>::iterator segmentIt = segmentation._data.begin();	

		// find what are the new forwground and background seeds
	eeImage::Image<double> newforeground;
	eeImage::Image<double> newbackground;

	checkNewSeed(foreground, newforeground, 1);
	checkNewSeed(background, newbackground, 0);

		// initalise counters and itterators
	int counter = 0;								// reset counter
	foregroundIt = newforeground._data.begin();
	backgroundIt = newbackground._data.begin();

		// crate a list of all the pixels that could potentailly change
	Block<GraphType::node_id>* changed_list = new Block<GraphType::node_id>(foreground._width*foreground._height);		// create a list of changed nodes
	changed_list->New(foreground._width*foreground._height);															// set the size of changed list so it includes every pixel

	GraphType::node_id* ptr;			// create a pointer to move through the changed list

		// move through all the nodes that could have changed value and add them to changed list
	for (ptr = changed_list->ScanFirst(); ptr; ptr = changed_list->ScanNext()) {
		*ptr = counter;
		// printf("\nvalue in List: %i", *ptr);
		counter++;
	}

	counter = 0;
		// cycle thorugh all the pixels
	for (foregroundIt = newforeground._data.begin(); foregroundIt < newforeground._data.end(); foregroundIt++, backgroundIt++) {

			// check if pixel marked as foreground that wasn't before
		if (*(foregroundIt) > 0.1) {
			g -> add_tweights(counter, _K, 0);
			g -> mark_node(counter);
		}
		// else if (*(foregroundIt) == -1) {
			// update using Log Likelihood
		// }

			// check if pixel marked as background that wasn't before
		else if (*(backgroundIt) > 0.1) {
			g -> add_tweights(counter, 0, _K);
			g -> mark_node(counter);
		}
		// else if (*(backgroundIt) == -1) {
			// update using Log Likelihood
		// }
		counter++;
	}

	g->maxflow(true, changed_list);		// rerun maxflow algorithm
	// GraphType::node_id* ptr;			// create a pointer to move trhough the changed list

		// move through all the nodes that could have changed value
	for (ptr = changed_list->ScanFirst(); ptr; ptr = changed_list->ScanNext()) {		// for loop with terminate condition of ptr = NULL
		GraphType::node_id i = *ptr; 
		assert(i >= 0 && i < nodeNum);
		g->remove_from_changed_list(i);

			// do something with node i...
		if (g->what_segment(i) == GraphType::SOURCE) {				// check whether first node is Source or Sink
			// printf("node %i is in the SOURCE set\n", i);
			// *(segmentIt) = *(ImageIt);
			*(segmentIt) = 1.0f;									// set output to binary
		}

		// remove if pixel connected to sink
		else {
			// printf("node %i is in the SINK set\n", i);
			*(segmentIt) = 0.0;
		}
		if ((segmentIt + 1 == segmentation._data.end())) {
			printf("\nExited Update Early");
			break;
		}
		segmentIt++;
	}
	delete changed_list;
}

// ---------------------------------------------- //
// -------------- Check If New Seed ------------- //
// ---------------------------------------------- //

void Algorithm::checkNewSeed(const eeImage::Image<double> &currSeeds, eeImage::Image<double> &newSeeds, bool foregroundFlag) {

	newSeeds.set(currSeeds._width, currSeeds._height, 1, 0.0);		// this will store the new pixels marked as seed

	std::vector<double>::const_iterator currentSeedsIt = currSeeds._data.begin();		// itterator for the current seed pixels
	std::vector<double>::iterator newSeedsIt = newSeeds._data.begin();					// itterator for the new Seeds

	std::vector<double>::const_iterator previousSeedsIt;								// itterator for the previous seed pixels

		// check if we're working with foreground or background pixels then set itterator to appropriate image
	if (foregroundFlag) {
		previousSeedsIt = _previousforeground._data.begin();
	}
	else {
		previousSeedsIt = _previousbackground._data.begin();
	}

		// find which pixels are the new seeds
	for (currentSeedsIt; currentSeedsIt < currSeeds._data.end(); currentSeedsIt++, previousSeedsIt++,  newSeedsIt++) {
			// seed added
		if (*(currentSeedsIt) > *(previousSeedsIt)) {
			*(newSeedsIt) = 1.0;
		}
			// seed removed
		else if (*(currentSeedsIt) < *(previousSeedsIt)) {
			// *(newSeedsIt) = -1.0;			// will require a likelihood model for this case
			*(newSeedsIt) = 0.0;				// ignore this case for now
		}
			// previously assigned seed
		else {
			*(newSeedsIt) = 0.0;
		}
	}
}

// ---------------------------------------------- //
// ----- Check Background/Foreground Overlap ---- //
// ---------------------------------------------- //

void Algorithm::checkSeedOverlap(eeImage::Image<double> &foreSeeds, eeImage::Image<double> &backSeeds) {
	

	std::vector<double>::iterator foreSeedsIt = foreSeeds._data.begin();		// itterator for the foreground seed pixels
	std::vector<double>::iterator backSeedsIt = backSeeds._data.begin();		// itterator for the background seed pixels

		// check if sed pixels overlap, if they do then set so pixel is not a seed in background or foreground
	for (foreSeedsIt; foreSeedsIt < foreSeeds._data.begin(); foreSeedsIt++, backSeedsIt++) {
		if ((*(foreSeedsIt) > 0.0) && (*(backSeedsIt) > 0.0)) {
			*foreSeedsIt = 0.0;
			*backSeedsIt = 0.0;
		}
	}
}