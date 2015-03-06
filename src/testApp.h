#pragma once

#include "ofMain.h"

#include "ofxOsc.h"
#include <vector>
#include <list>
#include "ofxXmlSettings.h"
#include "ofxMaxim.h"
#include <fstream>
#include <conio.h>
#include <windows.h>
#include <map>

#include "EmoStateDLL.h"
#include "edk.h"
#include "edkErrorCode.h"
using namespace std;


class testApp : public ofBaseApp{
	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y);
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);

protected:
    ofxOscReceiver oscIn, oscStim;
    ofxOscSender returnResult;
    vector<float> eeg;
    vector<vector<int> > stimIndexes;
    vector<vector<float> > stimAvgWindows, stimAvgWindowsLPF;
    int stimCount;
    int windowSize;
    void analyse();
    list<float> recentEEG;
    enum appPhases {RECORDING, ANALYSING, DONE} appPhase;
    vector<ofColor> cols;

    ofxXmlSettings log;
    maxiDCBlocker dcBlocker;

    void analyseSignal(vector<float> &sig, int winStart, int winLen, vector<vector<float> > &stimAvgWins);


    DataHandle hData;


	std::string input;


};
