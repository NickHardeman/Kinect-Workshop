#include "ofApp.h"
/*
Address: /bodies/{bodyId}/joints/{jointId}
Values: 
 - float:  positionX
 - float:  positionY
 - float:  positionZ
 - string: trackingState (Tracked, NotTracked or Inferred)

Address: /bodies/{bodyId}/hands/{handId} (Left or Right)
Values: 
 - string: handState (Open, Closed, NotTracked, Unknown)
 - string: handConfidence (High, Low)

*/

//--------------------------------------------------------------
bool ofApp::shouldRemoveParticle( const Particle& p ) {
    return p.bRemove;
}

//--------------------------------------------------------------
void ofApp::setup() {
    ofSetFrameRate( 60 );
    
    
    bUseLiveOsc = false;
    // uncomment to use OSC //
    if(bUseLiveOsc) {
        oscRX.setup( 12345 );
    }
    
    ofDirectory tdir;
    tdir.allowExt("txt");
    tdir.listDir("recordings");
    if( tdir.size() ) {
        loadPlaybackData( tdir.getPath( tdir.size()-1 ));
    }
    
    
    gui.setup("Image Processing");
    gui.setPosition(ofGetWidth()-10-gui.getWidth(), 10 );
    gui.add(bDebug.set("Debug", true ));
    if(bUseLiveOsc) gui.add(bRecording.set("Recording", false ));
    
    
    
    
    cam.setAutoDistance( false );
    cam.setDistance( 1000 );
    cam.setNearClip(1);
    cam.setFarClip( 10000 );
    
    cam.setPosition( 0, 0, -1000 );
    cam.lookAt( ofVec3f(), ofVec3f(0,1,0) );
}

//--------------------------------------------------------------
void ofApp::update() {
    
    float etimef = ofGetElapsedTimef();
    
    for( auto it = skeletons.begin(); it != skeletons.end(); it++ ) {
        for( int i = 0; i < Skeleton::TOTAL_JOINTS; i++ ) {
            it->second->getJoint( (Skeleton::JointIndex)i )->prevPos = it->second->getJoint( (Skeleton::JointIndex)i )->pos;
        }
    }
    
    
    if( bUseLiveOsc ) {
        while( oscRX.hasWaitingMessages() ){
            ofxOscMessage msg;
            oscRX.getNextMessage(msg);
            
            parseMessage( msg );
            
            if( bRecording ) {
                if( uniqueFilename == "" ) {
                    uniqueFilename = ofGetTimestampString();
                    startRecordingTime = etimef;
                    recordingData.clear();
                }
            } else {
                // save the file //
                if( uniqueFilename != "" ) {
                    saveRecording();
                }
                
                recordingData.clear();
                uniqueFilename = "";
            }
            
            if( bRecording ) {
                SkeletonData sdata;
                sdata.time = etimef - startRecordingTime;
                sdata.message = msg;
                recordingData.push_back( sdata );
            }
            
        }
    } else {
        if( playbackData.size() == 0 && playbackDataCached.size() ) {
            playbackData = playbackDataCached;
            playbackTimeStart = etimef;
        }
        
        if( playbackData.size() ) {
            float playEndTime = playbackTimeStart + playbackData.back().time;
            float timeSinceStart = etimef - playbackTimeStart;
            int numToKill=0;
            for( int k = 0; k < playbackData.size(); k++ ) {
                if( playbackData[k].time <= timeSinceStart ) {
                    parseMessage( playbackData[k].message );
                    numToKill++;
                } else {
                    break;
                }
            }
            if( numToKill > 0 ) {
                playbackData.erase( playbackData.begin(), playbackData.begin()+numToKill );
            }
        }
        
        
    }
    
    // clean up old skeletons //
    for( auto it = skeletons.begin(); it != skeletons.end(); it++ ) {
        if(etimef - it->second->lastTimeSeen > 2.0 ) {
            skeletons.erase( it );
            break;
        }
    }
    
    // if there are skeletons, add some particles //
    if( skeletons.size() ) {
        for( auto it = skeletons.begin(); it != skeletons.end(); it++ ) {
            // get random joint index //
//            int rindex = ofRandom(0, Skeleton::TOTAL_JOINTS );
//            if( rindex == Skeleton::TOTAL_JOINTS ) rindex = Skeleton::TOTAL_JOINTS-1;
            
            for( int i = 0; i < Skeleton::TOTAL_JOINTS; i++ ) {
                Particle p;
                shared_ptr< Skeleton::Joint > joint = it->second->getJoint( (Skeleton::JointIndex)i );
                p.pos = joint->pos;
                p.vel = (joint->pos - joint->prevPos);// * 3.0;
                p.vel.limit(50);
                p.size = ofRandom( 14, 26 );
                particles.push_back( p );
            }
        }
    }
    
    for( int i = 0; i < particles.size(); i++ ) {
        // gravity
        particles[i].vel *= 0.94f;
        particles[i].vel.y -= 0.02;
        particles[i].pos += particles[i].vel;
        
        particles[i].size -= (1.f/60.f)/0.08f;
        
        if( particles[i].pos.y < -1000 || particles[i].size < 0.1 ) {
            particles[i].bRemove = true;
        }
    }
    
    int tooMany = particles.size()-2000;
    if( tooMany > 0 ) {
        for( int i = 0; i < tooMany; i++ ) {
            particles[i].bRemove = true;
        }
    }
    
    ofRemove( particles, shouldRemoveParticle );
    
//    cout << "Number of skeletons : " << skeletons.size() << " | " << ofGetFrameNum() << endl;
    
}

//--------------------------------------------------------------
void ofApp::parseMessage( ofxOscMessage amsg ) {
    
//    cout << "msg: " << amsg.getAddress() << " | " << ofGetFrameNum() << endl;
    
    string address = amsg.getAddress();
    if( address.size() > 0 && address[0] == '/' ) {
        address = address.substr( 1, address.size()-1 );
    }
    vector< string > parts = split( address, '/' );
    
    if( parts.size() >= 4 ) {
        string bodyId = parts[1];
        string typeName = parts[2];
        if( typeName == "joints") {
            
            ofVec3f tpos;
            tpos.x = amsg.getArgAsFloat(0);
            tpos.y = amsg.getArgAsFloat(1);
            tpos.z = amsg.getArgAsFloat(2);
            
            string status = amsg.getArgAsString(3);
            bool bSeen = (status != "NotTracked" && status != "Unknown");
            
            string jointName = parts[ 3 ];
            
            if( !skeletons.count(bodyId)) {
                skeletons[bodyId] = shared_ptr<Skeleton>(new Skeleton() );
                skeletons[bodyId]->build();
            }
            skeletons[bodyId]->addOrUpdateJoint( jointName, tpos, bSeen );
        }
    }
    
}

//--------------------------------------------------------------
void ofApp::draw() {
    
    cam.begin(); {
        ofEnableDepthTest();
        ofSetColor( 120 );
        for( auto it = skeletons.begin(); it != skeletons.end(); it++ ) {
            it->second->draw();
        }
        
        ofSetColor( 55 );
        for( int i = 0; i < particles.size(); i++ ) {
            ofDrawSphere( particles[i].pos, particles[i].size );
        }
        
        ofDrawGrid( 1000, 10, false, false, true, false );
        ofDisableDepthTest();
    } cam.end();
    
    if( !bHide ){
        gui.draw();
    }
}

//--------------------------------------------------------------
void ofApp::saveRecording() {
    cout << "Saving recording to " << uniqueFilename << endl;
    if( !ofDirectory::doesDirectoryExist("recordings/")) {
        ofDirectory::createDirectory("recordings/");
    }
    
    ofBuffer buffer;
    for( auto a : recordingData ){
        
        buffer.append( ofToString(a.time)+"|" );
        buffer.append( a.message.getAddress() );
        
        for( int i = 0; i < a.message.getNumArgs(); i++ ) {
            buffer.append("|");
            if( a.message.getArgType(i) == OFXOSC_TYPE_FLOAT ) {
                buffer.append( "f"+ofToString(a.message.getArgAsFloat(i),6));
            } else if( a.message.getArgType(i) == OFXOSC_TYPE_STRING ) {
                buffer.append( "s"+a.message.getArgAsString(i) );
            } else if( a.message.getArgType(i) == OFXOSC_TYPE_INT32 ) {
                buffer.append("i"+ofToString(a.message.getArgAsInt(i)) );
            }else {
                buffer.append( "u0" );
            }
        }
        buffer.append("\n");
    }
    
    ofBufferToFile( "recordings/"+uniqueFilename+".txt", buffer );
    recordingData.clear();
}

//--------------------------------------------------------------
void ofApp::loadPlaybackData( string afilePath ) {
    playbackDataCached.clear();
    
    ofBuffer tbuffer = ofBufferFromFile( afilePath );
    if( tbuffer.size() ) {
        for( auto a : tbuffer.getLines() ) {
            string lineStr = a;
            ofStringReplace(lineStr, "\n", "");
            vector <string> results = split(lineStr, '|');
            
            if( results.size() >= 5 ) {
                SkeletonData sdata;
                sdata.time = ofToFloat(results[0]);
                sdata.message.setAddress(results[1]);
                // now set the params //
                for( int k = 2; k < results.size(); k++ ) {
                    if( results[k] == "" ) continue;
                    if( results[k].length() <= 1 ) continue;
                    string vstring = results[k];
                    vstring = vstring.substr(1, vstring.length());
                    if( results[k][0] == 'f' ) {
                        // float
                        sdata.message.addFloatArg( ofToFloat(vstring));
                    } else if( results[k][0] == 's' ) {
                        // string //
                        sdata.message.addStringArg( vstring );
                    } else if( results[k][0] == 'i' ) {
                        sdata.message.addIntArg( ofToInt(vstring) );
                    }
                }
                playbackDataCached.push_back( sdata );
            }
        }
    }
}

//--------------------------------------------------------------
vector<string> ofApp::split(const string &s, char delim) {
    stringstream ss(s);
    string item;
    vector<string> tokens;
    while (getline(ss, item, delim)) {
        tokens.push_back(item);
    }
    return tokens;
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if( key == 'h' ){
        bHide = !bHide;
    }
    if( key == 'd' ) {
        bDebug = !bDebug;
    }
    if( key == ' ' ) {
        bRecording = !bRecording;
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
