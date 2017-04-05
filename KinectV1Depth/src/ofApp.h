#pragma once

#include "ofMain.h"
#include "ofxKinect.h"
#include "ofxOpenCv.h"
#include "ofxGui.h"

class HitBox {
public:
    ofRectangle rectangle;
    float hitPct = 0.0;
};

class ofApp : public ofBaseApp {
public:
    void setup();
    void update();
    void draw();

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
    
    ofxPanel gui;
    bool bHide;
    ofParameter<bool> bDebug;
    
    // kinect
    ofxKinect kinect;
    ofParameter <int> nearClip;
    ofParameter <int> farClip;
    ofParameter <bool> bFlipX, bFlipY;
    
    ofVideoPlayer videoPlayer;
    
    bool bUseLiveKinect;
    
    // cv //
    ofParameter<int> numDilatePasses;
    ofParameter<int> blurAmount;
    ofParameter<int> threshold;
    ofParameter<float> minSize, maxSize;
    
    ofPixels grayPixels;
    ofxCvColorImage colorCv;
    ofxCvGrayscaleImage grayCv;
    ofxCvGrayscaleImage processedCv;
    ofxCvGrayscaleImage prevFrame;
    ofxCvGrayscaleImage historyCv;
    ofxCvContourFinder finder;
    
    ofParameter<int> contourPolySpacing;
    ofParameter<int> contourSmoothing;
    
    ofParameter<int> minPixToActivateBox;
    
    vector< ofPolyline > contours;
    vector< HitBox > hitBoxes;
    
    int incBlobId = 1;
};
