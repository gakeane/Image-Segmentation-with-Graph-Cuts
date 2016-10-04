class Node {
public:

	Node();
	Node(float T_Source, float T_Sink, float N_Up, float N_Right, float N_Left, float N_Down);
	~Node();

	float getTLinkSource();
	float getTLinkSink();

	float getNLinkUp();
	float getNLinkRight();
	float getNLinkLeft();
	float getNLinkDown();

	void setTLinkSource(float T);
	void setTLinkSink(float T);

	void setNLinkUp(float N);
	void setNLinkRight(float N);
	void setNLinkLeft(float N);
	void setNLinkDown(float N);

private:

	float _tLinkSource;
	float _tLinkSink;

	float _nLinkUp;
	float _nLinkRight;
	float _nLinkLeft;
	float _nLinkDown;
};