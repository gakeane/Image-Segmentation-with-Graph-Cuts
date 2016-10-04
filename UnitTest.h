#include "Algortihm.h"
#include "eeImage.h"
#include "5c1UnitTest.h"
#include "graph.h"

#include <iostream>
#include <fstream>
#include <cmath>

typedef Graph<double,double,double> GraphType;	

using namespace std;

namespace UnitTest {

	void readFromFile(eeImage::Image<double>& Image, string filename);

	bool testStrokeData();

	bool testHistogramData();
	bool testHistogramEdgeData();
	bool testUnmarkedInclusion();

	bool testLikelihoodData();
	bool testLikelihoodWithZeros();

	bool testSigmaValue();

	bool calculateNLinks();

	bool calculateTLinks();
};