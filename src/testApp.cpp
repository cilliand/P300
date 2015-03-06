#include "testApp.h"
#include <sstream>
#include "filters.cpp"


#define SETTINGSTAG "SETTINGS"



//--------------------------------------------------------------
void testApp::setup(){

	oscIn.setup(7400);
    oscStim.setup(7401);
    returnResult.setup("localhost",7402);
    stimCount = 3;
    stimIndexes.resize(stimCount);
    recentEEG.resize(400);
	ofSetVerticalSync(true);
    for(int i=0; i < stimCount * 2; i++) {
        cols.push_back(ofColor(ofRandom(200), ofRandom(200), ofRandom(200)));
    }
    for(int i=0; i < stimCount; i++) {
        stimIndexes[i].clear();
    }
    eeg.clear();
    appPhase = RECORDING;

  //  std::cout << "Start receiving EEG Data! Press any key to stop logging...\n" << std::endl;
}

//--------------------------------------------------------------
void testApp::update(){
    if( oscIn.hasWaitingMessages()) {
        ofxOscMessage msg;
        while(oscIn.getNextMessage(&msg)) {
            //cout << msg.getAddress() << endl;
            if (msg.getAddress() == "/d" && RECORDING == appPhase) {
                float val = msg.getArgAsFloat(0);
                val = dcBlocker.play(val, 0.92);
                val = filterNotch50Hz(val);
                val = lpfilter45(val);
                eeg.push_back(val);
                recentEEG.push_back(val);
                recentEEG.pop_front();
            }
            else if (msg.getAddress() == "/st" && RECORDING == appPhase) {
                int idx = msg.getArgAsInt32(0);
                stimIndexes[idx].push_back(MAX(0,eeg.size()-1));
               // cout << "Stim: " << idx << endl;
            }
            else if (msg.getAddress() == "/start") {
                for(int i=0; i < stimCount; i++) {
                    stimIndexes[i].clear();
                }
                eeg.clear();
                appPhase = RECORDING;
            }
            else if (msg.getAddress() == "/end" && RECORDING == appPhase) {
                //analyse
                analyse();
            }
        }
        while(oscStim.getNextMessage(&msg)) {
            //            cout << msg.getAddress() << endl;
            if (msg.getAddress() == "/d" && RECORDING == appPhase) {
                float val = msg.getArgAsFloat(0);
                val = dcBlocker.play(val, 0.92);
                val = filterNotch50Hz(val);
                val = lpfilter45(val);
                eeg.push_back(val);
                recentEEG.push_back(val);
                recentEEG.pop_front();
            }
            else if (msg.getAddress() == "/st" && RECORDING == appPhase) {
                int idx = msg.getArgAsInt32(0);
                stimIndexes[idx].push_back(MAX(0,eeg.size()-1));
                cout << "Stim: " << idx << endl;
            }
            else if (msg.getAddress() == "/start") {
                for(int i=0; i < stimCount; i++) {
                    stimIndexes[i].clear();
                }
                eeg.clear();
            }
            else if (msg.getAddress() == "/end" && RECORDING == appPhase) {
                //analyse
                analyse();
            }
        }
    }
}


//--------------------------------------------------------------
void testApp::draw(){
    ofBackground(255);
    ofEnableSmoothing();

    switch(appPhase) {
        case RECORDING:
        {
            ofSetColor(0,0,200);
            list<float>::iterator it = recentEEG.begin();
            float inc = ofGetWidth() / (float) recentEEG.size();
            float mid = ofGetHeight() / 2.0;
            int i=0;
            float last = *it;
            it++;
            while(it != recentEEG.end()) {
                //ofCircle(i*inc, mid - (last * mid), 3);
                ofLine(i*inc, mid - (last * mid), (i+1) * inc, mid - (*it * mid));
                last = *it;
                i++;
                it++;
            }
        }
            break;
        case ANALYSING:
            ofBackground(ofRandomuf() * 255, ofRandomuf() * 255, ofRandomuf() * 255);
            break;
        case DONE:
            for(int s=0; s < stimCount; s++) {
                ofSetColor(cols[s]);
                vector<float>::iterator it = stimAvgWindows[s].begin();
                float inc = ofGetWidth() / (float) stimAvgWindows[s].size();
                float mid = ofGetHeight() / 2.0;
                int i=0;
                float last = *it;
                it++;
                while(it != stimAvgWindows[s].end()) {
                    ofLine(i*inc, mid - (last * mid), (i+1) * inc, mid - (*it * mid));
                    last = *it;
                    i++;
                    it++;
                }
            }
//            for(int s=0; s < stimCount; s++) {
//                ofSetColor(cols[s + stimCount]);
//                vector<float>::iterator it = stimAvgWindowsLPF[s].begin();
//                float inc = ofGetWidth() / (float) stimAvgWindowsLPF[s].size();
//                float mid = ofGetHeight() / 2.0;
//                int i=0;
//                float last = *it;
//                it++;
//                while(it != stimAvgWindowsLPF[s].end()) {
//                    ofLine(i*inc, mid - (last * mid), (i+1) * inc, mid - (*it * mid));
//                    last = *it;
//                    i++;
//                    it++;
//                }
//            }
            break;
    }
}

void testApp::analyseSignal(vector<float> &sig, int winStart, int winLen, vector<vector<float> > &stimAvgWins) {
    stimAvgWins.resize(stimCount);
    for(int i=0; i < stimCount; i++) {
        stimAvgWins[i].resize(winLen, 0);
    }

    vector<int> validWindowCount(stimCount,0);
    //sum area of each stimulus
    for(int s=0; s < stimCount; s++) {
        for(int i=0; i < stimIndexes[s].size(); i++) {
            int eegIdx = stimIndexes[s][i];
            if (eegIdx + winStart + winLen < sig.size()) {
                int winIdx = 0;
                for(int e=eegIdx + winStart; e < eegIdx + winStart + winLen; e++) {
                    stimAvgWins[s][winIdx++] += sig[e];
                }
                validWindowCount[s]++;
            }
        }
    }
    vector<float> totalArea(stimCount,0);
    for(int s=0; s < stimCount; s++) {
        if (validWindowCount[s] > 0) {
            for(int i=0; i < stimAvgWins[s].size(); i++) {
                stimAvgWins[s][i] /= validWindowCount[s];
                totalArea[s] += fabs(stimAvgWins[s][i]);
            }
        }
    }

    //cout << "Results: \n";
    int lastArea = -1;
    int best = -1;
    for(int s=0; s < stimCount; s++) {
        cout << ", " << s << ":";
        cout << ", " << validWindowCount[s];
        cout << ", " << totalArea[s];
//        cout << "Stimulus " << s << ":\n";
//        cout << "#: " << validWindowCount[s] << endl;
//        cout << "Average area: " << totalArea[s] << endl;

        if(totalArea[s] > lastArea){
            best = s;
        }
        lastArea = totalArea[s];
    }

    ofxOscMessage m;
    m.setAddress("/r");
    m.addFloatArg(best);
    returnResult.sendMessage(m);
    cout << ", " << best << ", " << std::endl;
}


void testApp::analyse() {
    appPhase = ANALYSING;
    int sampleRate = 128;
    float winLenMs = 500;
    float winStartMs = 100;
    int winLen = (winLenMs / 1000.0 * sampleRate);
    int winStart = (winStartMs / 1000.0 * sampleRate);

//    log.addTag(SETTINGSTAG);
//    log.pushTag(SETTINGSTAG, 0);
//    log.addAttribute(SETTINGSTAG, "STIMCOUNT", stimCount, 0);
//    log.addAttribute(SETTINGSTAG, "SAMPLERATE", sampleRate, 0);
//    log.popTag();
    stringstream data;
    for(int e=0; e < eeg.size(); e++)
        data << eeg[e] << ",";
    log.addValue("RAW", data.str());
    for(int s=0; s < stimCount; s++) {
        data.str("");
        for(int i=0; i < stimIndexes[s].size(); i++) {
            data << stimIndexes[s][i] << ",";
        }
        log.addValue("INDEXES", data.str());
        log.addAttribute("INDEXES", "STIM", s, s);
    }
    cout << winLen << ", " << winStart;
    //cout << "Window: len: " << winLen << ", offset: " << winStart << endl;
    //cout << "Analysing raw...";

    analyseSignal(eeg, winStart, winLen, stimAvgWindows);

//    cout << "Analysing filtered data...\n";
//    vector<float> eegFilt = eeg;
//    for(int i=0; i < eegFilt.size(); i++) {
//        eegFilt[i] = lpfilter45(eegFilt[i]);
//    }
//    analyseSignal(eegFilt, winStart, winLen, stimAvgWindowsLPF);

    stringstream filename;
    filename << "eegLog-";
    filename << ofGetSystemTime() << ".xml";
    log.saveFile(ofToDataPath(filename.str()));
    appPhase = DONE;
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){

}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y){

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){

}
