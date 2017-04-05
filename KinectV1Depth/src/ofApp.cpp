#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {
    ofSetFrameRate( 60 );
    
    gui.setup("Image Processing");
    gui.setPosition( ofGetWidth() - 10 - gui.getWidth(), 10 );
    gui.add(bDebug.set("Debug", true));
    
    gui.add(nearClip.set("NearClip", 500, 500, 8000 ));
    gui.add(farClip.set("FarClip", 4000, 500, 8000));
    gui.add(bFlipX.set("FlipX", false));
    gui.add(bFlipY.set("FlipY", false ));
    
    // cv //
    gui.add(numDilatePasses.set("NumDilates", 2, 0, 6));
    gui.add(blurAmount.set("Blur", 3, 0, 11));
    gui.add(threshold.set("Threshold", 50, 0, 255));
    gui.add(minSize.set("MinBlobSize", 10, 1, 150));
    gui.add(maxSize.set("MaxBlobSize", 600, 1, 600 ));
    
    gui.add(contourPolySpacing.set("PolySpacing", 3, 0, 40));
    gui.add(contourSmoothing.set("ContourSmoothing", 1, 0, 5));
    
    gui.add(minPixToActivateBox.set("NumPixToActivateBox", 20, 1, 100 ));
    
    gui.loadFromFile("settings.xml");
    
    
    bUseLiveKinect = true;
    // init the kinect
    // bool ofxKinect::init(bool infrared, bool video, bool texture)
    kinect.init(false,false,true);
    // try to open the default kinect location //
    if( !kinect.open() ) {
        string tVideoPath = "SinglePerson.mov";
        cout << "could not open kinect! using video from: " << tVideoPath << endl;
        // the kinect failed to open, try loading a video //
        bUseLiveKinect = false;
        videoPlayer.load( tVideoPath );
        videoPlayer.setLoopState( OF_LOOP_NORMAL );
        videoPlayer.play();
    } else {
        cout << "opened kinect serial: " << kinect.getSerial() << endl;
    }
    
    bHide = false;
    
    float numRows = 8;
    float numCols = 12;
    for( int x = 0; x < numCols; x++ ) {
        for( int y = 0; y < numRows; y++ ) {
            HitBox hb;
            hb.rectangle.width = (float)ofGetWidth() / numCols;
            hb.rectangle.height = (float)ofGetHeight() / numRows;
            hb.rectangle.x = hb.rectangle.width * (float)x;
            hb.rectangle.y = hb.rectangle.height * (float)y;
            hitBoxes.push_back( hb );
        }
    }

}

//--------------------------------------------------------------
void ofApp::update() {
    
    bool bReceivedNewFrame = false;
    
    if( bUseLiveKinect ) {
        kinect.update();
        // only perform cpu intense cv operations when new data has been received //
        if( kinect.isFrameNew() ) {
            if( nearClip != kinect.getNearClipping() || farClip != kinect.getFarClipping() ){
                kinect.setDepthClipping(nearClip, farClip);
            }
            bReceivedNewFrame = true;
            grayPixels = kinect.getDepthPixels();
            grayCv.setFromPixels( grayPixels );
        }
    } else {
        videoPlayer.update();
        if( videoPlayer.isFrameNew() ) {
            bReceivedNewFrame = true;
            ofPixels& videoPixels = videoPlayer.getPixels();
            // allocate the cv images to the size of the video pixels //
            if( colorCv.getWidth() != videoPixels.getWidth() ){
                // we aren't going to draw these cv images, so do not use a texture
                // to help reduce overhead //
                colorCv.setUseTexture(false);
                colorCv.allocate(videoPixels.getWidth(), videoPixels.getHeight());
                grayCv.setUseTexture(false);
                grayCv.allocate(videoPixels.getWidth(), videoPixels.getHeight());
            }
            // the video comes in as RGB
            colorCv.setFromPixels(videoPixels);
            // ofxCvGrayscaleImage can be set from an ofxOpenCvColorImage
            // converts from color to gray
            grayCv      = colorCv;
            //grayPixels  = grayCv.getPixels();
        }
    }
    
    if( bReceivedNewFrame ) {
        if( bFlipX || bFlipY ){
            grayCv.mirror(bFlipY, bFlipX);
        }
        
        prevFrame = processedCv;
        
        // set the cv image from the pixels we have extracted //
//        processedCv.setFromPixels( grayPixels );
        if( processedCv.getWidth() == 0 ) {
            processedCv.allocate( grayCv.getWidth()/2, grayCv.getHeight()/2 );
        }
        // perform operations at a smaller size //
        processedCv.scaleIntoMe( grayCv, CV_INTER_LINEAR );
        
        if( blurAmount > 0 ) {
            //processedCv.blurGaussian( preBlurAmount*2+1 );
        }
        
        processedCv.threshold( threshold );
        
        for( int i = 0; i < numDilatePasses; i++ ) {
            processedCv.dilate();
            if(blurAmount>0)processedCv.blurGaussian(blurAmount*2+1);
            processedCv.erode_3x3();
        }
        
        if( prevFrame.getWidth() && processedCv.getWidth() ) {
            historyCv.absDiff( prevFrame, processedCv );
        }
        
//        findContours( ofxCvGrayscaleImage&  input,
//                     int minArea,
//                     int maxArea,
//                     int nConsidered,
//                     bool bFindHoles,
//                     bool bUseApproximation)
        if( maxSize < minSize ) {
            maxSize = minSize;
        }
        finder.findContours( processedCv, minSize*minSize, maxSize*maxSize, 20, true, false);
        
        contours.clear();
        // copy the contours and apply some smoothing //
        vector< ofxCvBlob >& blobs = finder.blobs;
        for( int d = 0; d < blobs.size(); d++ ) {
            ofPolyline tempPoly;
            tempPoly.addVertices( blobs[d].pts );
            if( tempPoly.getPerimeter() > contourPolySpacing ) {
                tempPoly = tempPoly.getResampledBySpacing( contourPolySpacing );
            }
            if( contourSmoothing > 0 ) {
                tempPoly = tempPoly.getSmoothed( contourSmoothing );
            }
            contours.push_back( tempPoly );
        }
        
        // all of the contours are relative to the width and height of the kinect depth image
        // convert them to screen space //
        if( !bDebug ) {
            float xscale = (float)ofGetWidth() / processedCv.getWidth();
            float yscale = (float)ofGetHeight() / processedCv.getHeight();
            for( int i = 0; i < contours.size(); i++ ) {
                // loop through all of the vertices and scale them //
                for( int j = 0; j < contours[i].getVertices().size(); j++ ) {
                    contours[i].getVertices()[j].x *= xscale;
                    contours[i].getVertices()[j].y *= yscale;
                }
            }
            
            float rxscale = processedCv.getWidth() / (float)ofGetWidth();
            float ryscale = processedCv.getHeight() / (float)ofGetHeight();
            // check the motion history to see if one of the boxes should be hit //
            for( int i = 0; i < hitBoxes.size(); i++ ) {
                ofRectangle tempRect = hitBoxes[i].rectangle;
                tempRect.x *= rxscale;
                tempRect.width *= rxscale;
                tempRect.y *= ryscale;
                tempRect.height *= ryscale;
                if (historyCv.countNonZeroInRegion( tempRect.x, tempRect.y, tempRect.width, tempRect.height ) > minPixToActivateBox*minPixToActivateBox ) {
                    hitBoxes[i].hitPct += 0.1;
                } else {
                    hitBoxes[i].hitPct -= 0.01;
                }
                hitBoxes[i].hitPct = ofClamp(hitBoxes[i].hitPct, 0.0, 1.0 );
            }
        }
    }
}

//--------------------------------------------------------------
void ofApp::draw() {
    ofSetColor( 255 );
    if( bDebug ) {
        if( bUseLiveKinect ) {
            kinect.drawDepth( 10, 10, kinect.getWidth(), kinect.getHeight() );
        } else {
            videoPlayer.draw( 10, 10, videoPlayer.getWidth(), videoPlayer.getHeight() );
        }
        if( processedCv.bAllocated ) {
            ofPushMatrix(); {
                ofTranslate( 660, 10 );
                processedCv.draw( 0, 0 );
                ofSetColor( ofColor::pink );
                for( int i = 0; i < contours.size(); i++ ) {
                    contours[i].draw();
                }
                ofSetColor(255);
                historyCv.draw( 0, processedCv.getHeight() + 20 );
            } ofPopMatrix();
        }
    } else {
        
        ofSetColor( 40 );
        for( int i = 0; i < contours.size(); i++ ) {
            contours[i].draw();
        }
        for( int i = 0; i < hitBoxes.size(); i++ ) {
            if( hitBoxes[i].hitPct > 0.0 ) {
                ofSetColor(hitBoxes[i].rectangle.x / (float)ofGetWidth() * 150 + 100,
                           hitBoxes[i].rectangle.y / (float)ofGetHeight() * 150 + 100, 30, 225.f * hitBoxes[i].hitPct );
                ofDrawRectangle( hitBoxes[i].rectangle );
            }
            
            ofSetColor( 120 );
            ofNoFill();
            ofDrawRectangle( hitBoxes[i].rectangle );
            ofFill();
        }
    }
    
    if( !bHide ){
        gui.draw();
    }
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if( key == 'h' ){
        bHide = !bHide;
    }
    if( key == 'd' ) {
        bDebug = !bDebug;
    }
    if(key == 's') {
        gui.saveToFile("settings.xml");
    }
    if(key == 'l') {
        gui.loadFromFile("settings.xml");
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
