#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "ofxOsc.h"
#include "Skeleton.h"

class SkeletonData {
public:
    ofxOscMessage message;
    float time=0;
};

class Particle {
public:
    ofVec3f pos;
    ofVec3f vel;
    float size = 5;
    bool bRemove = false;
};

class ofApp : public ofBaseApp {
public:
    static bool shouldRemoveParticle( const Particle& p );
    
    void setup();
    void update();
    void draw();
    
    void parseMessage( ofxOscMessage amsg );
    void saveRecording();
    void loadPlaybackData( string afilePath );
    vector<string> split(const string &s, char delim);

    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void mouseEntered(int x, int y);
    void mouseExited(int x, int y);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
    
    ofEasyCam cam;
    
    ofxPanel gui;
    bool bHide;
    ofParameter<bool> bDebug;
    ofParameter<bool> bRecording;
    
    ofxOscReceiver oscRX;
    
    string uniqueFilename="";
    float startRecordingTime=0;
    
    vector< SkeletonData > recordingData;
    bool bUseLiveOsc=false;
    
    vector<SkeletonData> playbackData;
    vector<SkeletonData> playbackDataCached;
    float playbackTimeStart = 0;
    
    map< string, shared_ptr<Skeleton> > skeletons;
    
    vector< Particle > particles;
};
