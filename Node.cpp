#include "Node.h"

	// constructors and destructors
Node::Node() {

	_tLinkSource = 0;
	_tLinkSink = 0;

	_nLinkUp = 0;
	_nLinkRight = 0;
	_nLinkLeft = 0;
	_nLinkDown = 0;
}

Node::Node(float T_Source, float T_Sink, float N_Up, float N_Right, float N_Left, float N_Down) {

	_tLinkSource = T_Source;
	_tLinkSink = T_Sink;

	_nLinkUp = N_Up;
	_nLinkRight = N_Right;
	_nLinkLeft = N_Left;
	_nLinkDown = N_Down;
}

Node::~Node() {

}

	// get methods
float Node::getTLinkSource() {
	return(_tLinkSource);
}

float Node::getTLinkSink() {
	return(_tLinkSink);
}

float Node::getNLinkUp() {
	return(_nLinkUp);
}

float Node::getNLinkRight() {
	return(_nLinkRight);
}

float Node::getNLinkLeft() {
	return(_nLinkLeft);
}

float Node::getNLinkDown() {
	return(_nLinkDown);
}

	// set moethods
void Node::setTLinkSource(float T) {
	_tLinkSource = T;
}

void Node::setTLinkSink(float T) {
	_tLinkSink = T;
}

void Node::setNLinkUp(float N) {
	_nLinkUp = N;
}

void Node::setNLinkRight(float N) {
	_nLinkRight = N;
}

void Node::setNLinkLeft(float N) {
	_nLinkLeft = N;
}

void Node::setNLinkDown(float N) {
	_nLinkDown = N;
}