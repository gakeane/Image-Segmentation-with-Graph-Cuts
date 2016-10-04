#include "eeImage.h"
#include "Node.h"
#include "graph.h"

typedef Graph<double,double,double> GraphType;					// this creates an alias (GraphType) for the datatype Graph<int, int, int> (i.e. writing GraphType is eqivilent to writing Graph<int, int , int>

class Algorithm {
public:

		// constructors and destructors
	Algorithm();
	~Algorithm();

		// get and set methods
	int getNumBins();
	double getLikelyhoodWeight();
	bool getMaxFlowCalculated();

	void setNumBins(int BW);
	void setLikelihoodWeight(double LW);
	void setMaxFlowCalculated(bool t);

	void deleteGraph();

	void graphCut(const eeImage::Image<double> &colourImg, const eeImage::Image<double> &foreground, const eeImage::Image<double> &background, eeImage::Image<double> &segmentation);

private:

	double calculateSigma(const eeImage::Image<double> &grayImage);
	void convertToGrayscale(const eeImage::Image<double> &colourImg, eeImage::Image<double> &greyImg);
	void getAlphaPixels(const eeImage::Image<double> &colourImg, const eeImage::Image<double> &strokeImage, eeImage::Image<double> &strokePixels);
	void createHistogram(const eeImage::Image<double> &strokePixels, const eeImage::Image<double> &seedPixels, std::vector<double>& histogram);
	void createLikelihoodModel(const std::vector<double>& Histogram, std::vector<double>& Likelihoodmodel);
	void maximumFlow(GraphType* g, const eeImage::Image<double> &grauyImage, eeImage::Image<double> &segmentation);
	void calculateNLinks(GraphType* g, const eeImage::Image<double> &grayImage);
	void calculateTLinks(GraphType* g, const eeImage::Image<double> &grayImage, const eeImage::Image<double> &foreground, const eeImage::Image<double> &background, std::vector<double>& foregroundLikelihood, std::vector<double>& backgroundLikelihood);
	void updateGraph(GraphType* g, const eeImage::Image<double> &foreground, const eeImage::Image<double> &background, eeImage::Image<double> &segmentation);
	void checkNewSeed(const eeImage::Image<double> &currSeeds, eeImage::Image<double> &newSeeds, bool foregroundFlag);
	void checkSeedOverlap(eeImage::Image<double> &foreSeeds, eeImage::Image<double> &backSeeds);
	GraphType* buildGraph(const eeImage::Image<double> &grayImage, const eeImage::Image<double> &foreground, const eeImage::Image<double> &background, std::vector<double>& foregroundLikelihood, std::vector<double>& backgroundLikelihood);
	
	int _numBins;				// stores number of histogram bins for background and foreground likelihood models
	double _likelihoodWeight;	// stores effective weight of likelihood component
	bool _maxFlowCalculated;	// remembers if max flow has been calculated or not
	GraphType* _graph;			// holds calculated graph
	double _K;					// weight assinged to t_links marked as seed (won't change when we reuse tree, at any rate priors will be smaller anyway)

	eeImage::Image<double> _previousforeground;
	eeImage::Image<double> _previousbackground;
};