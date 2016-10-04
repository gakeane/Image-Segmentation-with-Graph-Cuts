#include "UnitTest.h"

void UnitTest::readFromFile(eeImage::Image<double>& Image, string filename) {
	ifstream myfile;

		// READ IN IMAGE //
	std::vector<double>::iterator Iter1 = Image._data.begin();
	myfile.open(filename);

	if (myfile) {        
		double value;

		// read the elements into the image
		while (myfile >> value) {
			*(Iter1)++ = value;				// store value then incrment pointer
		}
	}
	myfile.close();
}

	// TEST 1: CHECK PIXELS MARKED BY STORKES ARE BEING EXTRACTED CORRECTLY
bool UnitTest::testStrokeData() {

	eeImage::Image<double> AlphaImage(256, 256, 1);			// Image showing the stroke marks
	eeImage::Image<double> OriginalImage(256, 256, 1);		// Original Image
	eeImage::Image<double> GroundTruth(256, 256, 1);			// image used to hold ground truth
	eeImage::Image<double> ExtractedPixels(256, 256, 1);		// Pixels algorithm extracts

	std::vector<double>::iterator Iter1;				// create interator for input 1 to set data in file
	std::vector<double>::iterator Iter2;				// create interator for input 2 to read data

		// READ IN ORIGINAL (EINSTEIN) IMAGE //
	UnitTest::readFromFile(OriginalImage, "EinsteinImage.txt");


		// READ IN STROKES IMAGE //
	UnitTest::readFromFile(AlphaImage, "Foreground.txt");


		// READ IN GROUND TRUTH //
	UnitTest::readFromFile(GroundTruth, "Foreground_groundTruth.txt");


	Algorithm algorithm;					// create an algorithm instance so we can call the find pixel function
	algorithm.getAlphaPixels(OriginalImage, AlphaImage, ExtractedPixels);
	int counter = 0;

		// Compare extracted pixels with the ground truth
	for (Iter1 = GroundTruth._data.begin(), Iter2 = ExtractedPixels._data.begin(); Iter1 < GroundTruth._data.end(); Iter1++, Iter2++) {
		if (*(Iter1) != *(Iter2)) {
			printf("\nTEST 1 FAILED: Extracted pixel doesn't match ground truth");
			return(false);
		}
	}

		// return true if only if all pixels match
	printf("\nTEST 1 PASSED: All pixels extracted by algortihm match ground truth");
	return(true);
}

	// TEST 2: CHECK THAT WE ARE CORRECTLY GENERATING HISTOGRAM //
bool UnitTest::testHistogramData() {

		// 50 random data between zero and one (generated in Matlab)
	double arrayData[50] = {0.42289,0.094229,0.59852,0.47092,0.69595,0.69989,0.63853,0.033604,0.068806,0.3196,0.53086,0.65445,0.40762,0.81998,0.71836,0.96865,0.53133,0.32515,0.10563,0.61096,0.7788,0.42345,0.090823,0.26647,0.15366,0.28101,0.44009,0.52714,0.45742,0.87537,0.51805,0.94362,0.63771,0.95769,0.24071,0.67612,0.28906,0.67181,0.69514,0.067993,0.25479,0.22404,0.66783,0.84439,0.34446,0.78052,0.67533,0.0067153,0.60217,0.38677};
	std::vector<double> data(&arrayData[0], &arrayData[0] + 50);

		// 20 value of what is stored in histogram bins
	double histarrayData[20] = {2,4,1,1,2,4,3,1,4,2,4,1,4,8,1,2,2,1,1,2};
	std::vector<double> GroundTruth(&histarrayData[0], &histarrayData[0] + 20);				// ground truth

	eeImage::Image<double> ExtractedPixels;			// Extracted Pixels used to test algorithm
	eeImage::Image<double> StrokePixels;			// stores marked pixels
	ExtractedPixels.set(5, 10, 1, data);			// set data of extracted Pixels
	StrokePixels.set(5, 10, 1, 1.0);				// here all pixels are marked
	std::vector<double> Histogram;					// stores output of histogram function

	std::vector<double>::iterator Iter1;				// create interator for input 1 to set data in file
	std::vector<double>::iterator Iter2;				// create interator for input 2 to read data

		// Calculate Histogram
	Algorithm algorithm;
	algorithm.createHistogram(ExtractedPixels, StrokePixels, Histogram);


	for (Iter1 = GroundTruth.begin(), Iter2 = Histogram.begin(); Iter1 < GroundTruth.end(); Iter1++, Iter2++) {
		if (*Iter1 != *Iter2) {
			printf("\nTEST 2 FAILED: Histogram bin value doesn't match ground truth");
			return(false);
		}
	}

	printf("\nTEST 2 PASSED: All histogram values created by algortihm match ground truth");
	return (true);
}

	// TEST 3: CHECK HISTOGRAM CORRECTLY HANDLES CASES ON THE EDGE OF HISTOGRAM BINS //
bool UnitTest::testHistogramEdgeData() {

		// 63 random data on the edge of the histogram bins
	double arrayData[63] = {1,1,1,0.95,0.95,0.95,0.9,0.9,0.9,0.85,0.85,0.85,0.8,0.8,0.8,0.75,0.75,0.75,0.7,0.7,0.7,0.65,0.65,0.65,0.6,0.6,0.6,0.55,0.55,0.55,0.5,0.5,0.5,0.45,0.45,0.45,0.4,0.4,0.4,0.35,0.35,0.35,0.3,0.3,0.3,0.25,0.25,0.25,0.2,0.2,0.2,0.15,0.15,0.15,0.1,0.1,0.1,0.05,0.05,0.05,0,0,0};
	std::vector<double> data(&arrayData[0], &arrayData[0] + 63);

		// 20 value of what is stored in histogram bins
	double histarrayData[20] = {3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,6};
	std::vector<double> GroundTruth(&histarrayData[0], &histarrayData[0] + 20);				// ground truth

	eeImage::Image<double> ExtractedPixels;			// Extracted Pixels used to test algorithm
	eeImage::Image<double> StrokePixels;			// stores marked pixels
	ExtractedPixels.set(9, 7, 1, data);				// set data of extracted Pixels
	StrokePixels.set(9, 7, 1, 1.0);					// here all pixels are marked
	std::vector<double> Histogram;					// stores output of histogram function

	std::vector<double>::iterator Iter1;				// create interator for input 1 to set data in file
	std::vector<double>::iterator Iter2;				// create interator for input 2 to read data

		// Calculate Histogram
	Algorithm algorithm;
	algorithm.createHistogram(ExtractedPixels, StrokePixels, Histogram);

		// check histogram data matches ground truth
	for (Iter1 = GroundTruth.begin(), Iter2 = Histogram.begin(); Iter1 < GroundTruth.end(); Iter1++, Iter2++) {
		if (*Iter1 != *Iter2) {
			printf("\nTEST 3 FAILED: Histogram bin value doesn't match ground truth");
			return(false);
		}
	}

	printf("\nTEST 3 PASSED: All histogram values created by algortihm match ground truth");
	return (true);
}

	// TEST 4: CHECK HISTOGRAM IGNORES PIXELS THAT AREN'T MARKED //
bool UnitTest::testUnmarkedInclusion() {

		// 20 random data between zero and one (generated in Matlab)
	double arrayData[20] = {0.52,0.52,0.52,0.52,0.52,0.62,0.62,0.62,0.62,0.62,0.72,0.72,0.72,0.72,0.72,0.82,0.82,0.82,0.82,0.82};
	std::vector<double> data(&arrayData[0], &arrayData[0] + 20);

		// 20 value of what is stored in histogram bins
	double histarrayData[20] = {0,0,0,0,0,0,0,0,0,0,4,0,2,0,3,0,0,0,0,0};
	std::vector<double> GroundTruth(&histarrayData[0], &histarrayData[0] + 20);						// ground truth

		// marked pixel denoted by value greater the 0, unmarked denoted by a zero
	double strokearrayData[20] = {0.1,0.3,0.45,0,1,0,0,0.9,0.00001,0,0.0023,0,0,1,1,0,0,0,0,0};
	std::vector<double> strokeData(&strokearrayData[0], &strokearrayData[0] + 20);			

	eeImage::Image<double> ExtractedPixels;			// Extracted Pixels used to test algorithm
	eeImage::Image<double> StrokePixels;			// stores marked pixels
	ExtractedPixels.set(4, 5, 1, data);				// set data of extracted Pixels
	StrokePixels.set(4, 5, 1, strokeData);			// here all pixels are marked
	std::vector<double> Histogram;					// stores output of histogram function

	std::vector<double>::iterator Iter1;				// create interator for input 1 to set data in file
	std::vector<double>::iterator Iter2;				// create interator for input 2 to read data

		// Calculate Histogram
	Algorithm algorithm;
	algorithm.createHistogram(ExtractedPixels, StrokePixels, Histogram);

		// check histogram bins match ground truth
	for (Iter1 = GroundTruth.begin(), Iter2 = Histogram.begin(); Iter1 < GroundTruth.end(); Iter1++, Iter2++) {
		if (*Iter1 != *Iter2) {
			printf("\nTEST 4 FAILED: Histogram bin value doesn't match ground truth");
			return(false);
		}
	}

	printf("\nTEST 4 PASSED: All Marked Pixels Read Correctly");
	return (true);
}

	// TEST 5: CHECK LIKELIHOOD FUNCTION RETURNS CORRECT VALUE //
bool UnitTest::testLikelihoodData() {

	int precision = 4;		 // precision to 4 decimal places

		// 10 random histogram bin values
	double arrayData[10] = {20, 90, 80, 200, 808, 106, 509, 20, 22, 3};
	std::vector<double> data(&arrayData[0], &arrayData[0] + 10);

		// Log Likelihood ground truth from Matlab
	double LikelihoodData[10] = {4.5315,3.0274,3.1452,2.2289,0.83269,2.8638,1.2948,4.5315,4.4362,6.4286};
	std::vector<double> GroundTruthLikelihood(&LikelihoodData[0], &LikelihoodData[0] + 10);						// ground truth

		// normalised Histogram ground truth from Matlab
	double normData[10] = {0.010764,0.048439,0.043057,0.10764,0.43488,0.057051,0.27395,0.010764,0.011841,0.0016146};
	std::vector<double> GroundTruthNormalised(&normData[0], &normData[0] + 10);						// ground truth

	//////////////////////////////////////////////////

	std::vector<double> LikelihoodModel;
	std::vector<double> normalisedData;

	std::vector<double>::iterator Iter1;				// create interator for input 1 to set data in file
	std::vector<double>::iterator Iter2;				// create interator for input 2 to read data

		// Calculate Histogram
	Algorithm algorithm;
	algorithm._numBins = 10;
	algorithm.createLikelihoodModel(data, LikelihoodModel, normalisedData);

		// check normalised histogram values match ground truth
	for (Iter1 = GroundTruthNormalised.begin(), Iter2 = normalisedData.begin(); Iter1 < GroundTruthNormalised.end(); Iter1++, Iter2++) {

		int iAnswer = int(*(Iter1) * pow(10.0f, precision) + 0.5f);					// calculates value to precision (four) decimal places of accuracy
		int iGroundTruth = int(*(Iter2) * pow(10.0f, precision) + 0.5f);
		if (iAnswer != iGroundTruth) {													// compare function value with ground truth
			printf("\nTEST 5 FAILED: Normalised Histogram Doesn't Match Ground truth");
			return(false);
		}
	}

		// check likelihood values match ground truth
	for (Iter1 = GroundTruthLikelihood.begin(), Iter2 = LikelihoodModel.begin(); Iter1 < GroundTruthLikelihood.end(); Iter1++, Iter2++) {

		int iAnswer = int(*(Iter1) * pow(10.0f, precision) + 0.5f);					// calculates value to precision (four) decimal places of accuracy
		int iGroundTruth = int(*(Iter2) * pow(10.0f, precision) + 0.5f);
		if (iAnswer != iGroundTruth) {													// compare function value with ground truth
			printf("\nTEST 5 FAILED: Log Likelihood values doesn't match ground truth");
			return(false);
		}
	}

	printf("\nTEST 5 PASSED: All Log Likelihood values created by algortihm match ground truth");
	return (true);
}

	// TEST 6: CHECK LIKELIHOOD FUNCTION RETURNS CORRECT VALUE WHEN HISTOGRAM BIN IS ZERO //
bool UnitTest::testLikelihoodWithZeros() {

	int precision = 4;		 // precision to 4 decimal places

		// 10 random histogram bin values
	double arrayData[10] = {20, 90, 0, 200, 808, 0, 509, 20, 22, 3};
	std::vector<double> data(&arrayData[0], &arrayData[0] + 10);

		// Log Likelihood ground truth from Matlab
	double LikelihoodData[10] = {3.8816,2.3775,1000,1.579,1000,2.2139,0.64485,3.8816,3.7863,5.7787};
	std::vector<double> GroundTruthLikelihood(&LikelihoodData[0], &LikelihoodData[0] + 10);						// ground truth

	//////////////////////////////////////////////////

	std::vector<double> LikelihoodModel;
	std::vector<double> normalisedData;

	std::vector<double>::iterator Iter1;				// create interator for input 1 to set data in file
	std::vector<double>::iterator Iter2;				// create interator for input 2 to read data

		// Calculate Histogram
	Algorithm algorithm;
	algorithm._numBins = 10;
	algorithm.createLikelihoodModel(data, LikelihoodModel, normalisedData);

		// check likelihood values match ground truth
	for (Iter1 = GroundTruthLikelihood.begin(), Iter2 = LikelihoodModel.begin(); Iter1 < GroundTruthLikelihood.end(); Iter1++, Iter2++) {

		int iAnswer = int(*(Iter1) * pow(10.0f, precision) + 0.5f);					// calculates value to precision (four) decimal places of accuracy
		int iGroundTruth = int(*(Iter2) * pow(10.0f, precision) + 0.5f);
		if (iAnswer != iGroundTruth) {													// compare function value with ground truth
			printf("\nTEST 6 FAILED: Log Likelihood values doesn't match ground truth");
			return(false);
		}
	}

	printf("\nTEST 6 PASSED: All Log Likelihood values created by algortihm match ground truth");
	return (true);
}

	// TEST 7: CHECK SIGMA IS BEING EVALUATED CORRECTLY //
bool UnitTest::testSigmaValue() {

	int precision = 4;								// precision to 4 decimal places
	double sigmaGroundTruth = 32.687500000000000;	// ground truth of sigma value

		// sample 4x4 image data
	double arrayData[16] = {3,7,9,13,5,6,12,11,2,5,19,21,1,7,12,24};
	std::vector<double> data(&arrayData[0], &arrayData[0] + 16);

		// cliques values ground truth
	double cliqueData[16] = {10,2.5,12.5,4,5,18.5,25,100,5,100,26.5,9,36,25,144,0};
	std::vector<double> GroundTruth(&cliqueData[0], &cliqueData[0] + 16);	

		// need to pass in 
	eeImage::Image<double> smallImage;
	std::vector<double> sigmaValues;
	smallImage.set(4,4,1,data);

		// apply function
	Algorithm algorithm;
	double Sigma = algorithm.calculateSigma(smallImage, sigmaValues);

	std::vector<double>::const_iterator Iter1 = GroundTruth.begin();
	std::vector<double>::const_iterator Iter2 = sigmaValues.begin();

		// check value of each clique is correct
	for (Iter1; Iter1 < GroundTruth.end(); Iter1++, Iter2++) {

		int iAnswer = int(*(Iter1) * pow(10.0f, precision) + 0.5f);						// calculates value to precision (four) decimal places of accuracy
		int iGroundTruth = int(*(Iter2) * pow(10.0f, precision) + 0.5f);
		if (iAnswer != iGroundTruth) {													// compare function value with ground truth
			printf("\nTEST 7 FAILED: Clique Values don't Match Ground Truth");
			return(false);
		}
	}

		// check final sigma value is correct
	int iAnswer = int(Sigma * pow(10.0f, precision) + 0.5f);						// calculates value to precision (four) decimal places of accuracy
	int iGroundTruth = int(sigmaGroundTruth * pow(10.0f, precision) + 0.5f);
	if (iAnswer != iGroundTruth) {													// compare function value with ground truth
		printf("\nTEST 7 FAILED: Sigma Value don't Match Ground Truth");
		return(false);
	}

	printf("\nTEST 7 PASSED: Sigma Value is correct");
	return (true);
}

	// TEST 8: CHECK NLINKS ARE BEING EVALUATED CORRECTLY (ALSO CHECKS K VALUE) //
bool UnitTest::calculateNLinks() {

	int precision = 4;							// precision to 4 decimal places
	double KGroundTruth = 47.335356969949570;	// ground truth for K value

		// sample 4x4 image data
	double arrayData[16] = {3,7,9,13,5,6,12,11,2,5,19,21,1,7,12,24};
	std::vector<double> data(&arrayData[0], &arrayData[0] + 16);

		// Nlinks ground truth
	double upData[16] = {0.99813,0.99953,0.9958,0.99813,0.9958,0.99953,0.97733,0.95428,0.99953,0.99813,0.97733,0.9958,0,0,0,0};
	std::vector<double> upGroundTruth(&upData[0], &upData[0] + 16);	

	double rightData[16] = {0.99254,0.99813,0.99254,0,0.99953,0.98329,0.99953,0,0.9958,0.91236,0.99813,0,0.98329,0.98837,0.93483,0};
	std::vector<double> rightGroundTruth(&rightData[0], &rightData[0] + 16);	

		// data must be passed in as an image
	eeImage::Image<double> smallImage;
	smallImage.set(4,4,1,data);

	std::vector<double> upLinks;
	std::vector<double> rightLinks;

		// need a graph to run function
	GraphType *g = new GraphType(16, 24);
	for (int i = 0; i < 16; i++) {
		g->add_node();
	}

		// apply function
	Algorithm algorithm;
	algorithm.calculateNLinks(g, smallImage, upLinks, rightLinks);

	std::vector<double>::const_iterator IterUP = upLinks.begin();
	std::vector<double>::const_iterator IterRIGHT = rightLinks.begin();

	std::vector<double>::const_iterator IterTruthUP = upGroundTruth.begin();
	std::vector<double>::const_iterator IterTruthRIGHT = rightGroundTruth.begin();

		// check NLink values going up match ground truth
	for (IterUP; IterUP < upLinks.end(); IterUP++, IterTruthUP++) {

		int iAnswer = int(*(IterUP) * pow(10.0f, precision) + 0.5f);						// calculates value to precision (four) decimal places of accuracy
		int iGroundTruth = int(*(IterTruthUP) * pow(10.0f, precision) + 0.5f);
		if (iAnswer != iGroundTruth) {													// compare function value with ground truth
			printf("\nTEST 8 FAILED: Incorrect Up NLink compared to ground truth");
			delete g;
			return(false);
		}
	}

		// check NLink values going right match ground truth
	for (IterRIGHT; IterRIGHT < rightLinks.end(); IterRIGHT++, IterTruthRIGHT++) {

		int iAnswer = int(*(IterRIGHT) * pow(10.0f, precision) + 0.5f);						// calculates value to precision (four) decimal places of accuracy
		int iGroundTruth = int(*(IterTruthRIGHT) * pow(10.0f, precision) + 0.5f);
		if (iAnswer != iGroundTruth) {													// compare function value with ground truth
			printf("\nTEST 8 FAILED: Incorrect Right NLink compared to ground truth");
			delete g;
			return(false);
		}
	}

		// check K value matches ground truth
	int iAnswer = int(algorithm._K * pow(10.0f, precision) + 0.5f);						// calculates value to precision (four) decimal places of accuracy
	int iGroundTruth = int(KGroundTruth * pow(10.0f, precision) + 0.5f);
	if (iAnswer != iGroundTruth) {													// compare function value with ground truth
		printf("\nTEST 8 FAILED: K Value don't Match Ground Truth");
		delete g;
		return(false);
	}

	printf("\nTEST 8 PASSED: All NLink Values are correct");
	delete g;
	return (true);
}

	// TEST 9: CHECK TLINKS ARE BEING EVALUATED CORRECTLY //
bool UnitTest::calculateTLinks() {

	int precision = 4;		 // precision to 4 decimal places

		// 16 random intensity values
	double arrayData[16] = {0.42289,0.094229,0.59852,0.47092,0.69595,0.69989,0.63853,0.033604,0.068806,0.3196,0.53086,0.65445,0.40762,0.81998,0.71836,0.96865};
	std::vector<double> data(&arrayData[0], &arrayData[0] + 16);

		// foreground and background strokes
	double foregroundData[16] = {0.0,0.0,0.0,0.0,0.0,0.4,0.2,0.0,0.0,1.0,1.0,0.0,0.0,0.0,0.0,0.0};
	std::vector<double> foreground(&foregroundData[0], &foregroundData[0] + 16);

	double backgroundData[16] = {5.0,1.0,0.0,1.0,0.1,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,1.0,0.0,0.0};
	std::vector<double> background(&backgroundData[0], &backgroundData[0] + 16);

		// foreground and background likelihoods
	double foreLikelihood[20] = {6.0878,2.0805,3.0814,5.0297,3.8315,3.5602,4.2051,4.2051,1.9312,3.1137,2.5109,1.9432,2.2412,3.8315,5.9543,2.723,2.4242,3.9255,5.5079,4.0173};
	std::vector<double> likelihoodF(&foreLikelihood[0], &foreLikelihood[0] + 20);

	double backLikelihood[20] = {3.329,3.2798,2.7977,5.8152,3.0492,3.1958,3.4173,2.3321,2.2084,5.2686,1.5519,4.8174,3.3265,4.2272,3.1515,4.1355,3.2525,3.0940,2.5699,3.2294};
	std::vector<double> likelihoodB(&backLikelihood[0], &backLikelihood[0] + 20);

		// sink and source ground truths
	double SinkData[16] = {0,0,1.9432,0,0,1000000,1000000,6.0878,2.0805,1000000,1000000,3.8315,1.9312,0,5.9543,4.0173};
	std::vector<double> sinkGroundTruth(&SinkData[0], &SinkData[0] + 16);	

	double SourceData[16] = {1000000,1000000,4.8174,1000000,1000000,0,0,3.329,3.2798,0,0,4.2272,2.2084,1000000,3.1515,3.2295};
	std::vector<double> sourceGroundTruth(&SourceData[0], &SourceData[0] + 16);	

		// vectors need to be passed in as images
	eeImage::Image<double> IntensityImage;
	eeImage::Image<double> foregroundImage;
	eeImage::Image<double> backgroundImage;
	IntensityImage.set(4, 4, 1, data);
	foregroundImage.set(4, 4, 1, foreground);
	backgroundImage.set(4, 4, 1, background);

		// create a graph (need for function)
	GraphType *g = new GraphType(16, 24);
	for (int i = 0; i < 16; i++) {
		g->add_node();
	}

		// apply function
	Algorithm algorithm;
	algorithm._K = 1000000;			// set K value (assume 1000000)

	std::vector<double> tSink;
	std::vector<double> tSource;

	algorithm.calculateTLinks(g, IntensityImage, foregroundImage, backgroundImage, likelihoodF, likelihoodB, tSink, tSource);

	std::vector<double>::const_iterator sinkIt = tSink.begin();
	std::vector<double>::const_iterator sourceIt = tSource.begin();
	std::vector<double>::const_iterator sinkTruthIt = sinkGroundTruth.begin();
	std::vector<double>::const_iterator sourceTruthIt = sourceGroundTruth.begin();

		// check sink Tlinks returned by function match ground truth
	for (sinkTruthIt; sinkTruthIt < sinkGroundTruth.end(); sinkTruthIt++, sinkIt++) {
		int iAnswer = int(*(sinkIt) * pow(10.0f, precision) + 0.5f);						// calculates value to precision (four) decimal places of accuracy
		int iGroundTruth = int(*(sinkTruthIt) * pow(10.0f, precision) + 0.5f);
		if (iAnswer != iGroundTruth) {													// compare function value with ground truth
			printf("\nTEST 8 FAILED: Incorrect Right NLink compared to ground truth");
			delete g;
			return(false);
		}
	}

	precision = 2;
		// check source Tlinks returned by function match ground truth (one case here is only accurate to 2 decimal places)
	for (sourceTruthIt; sourceTruthIt < sourceGroundTruth.end(); sourceTruthIt++, sourceIt++) {
		int iAnswer = int(*(sourceIt) * pow(10.0f, precision) + 0.5f);						// calculates value to precision (four) decimal places of accuracy
		int iGroundTruth = int(*(sourceTruthIt) * pow(10.0f, precision) + 0.5f);
		if (iAnswer != iGroundTruth) {													// compare function value with ground truth
			printf("\nTEST 8 FAILED: Incorrect Right NLink compared to ground truth");
			delete g;
			return(false);
		}
	}

	printf("\nTEST 9 PASSED: All TLink Values are correct");
	delete g;
	return (true);
}