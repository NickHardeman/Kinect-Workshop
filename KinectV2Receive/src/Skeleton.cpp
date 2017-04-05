//
//  Skeleton.cpp
//  KinectV2Receive
//
//  Created by Nick Hardeman on 4/5/17.
//

#include "Skeleton.h"

//--------------------------------------------------------------
void Skeleton::build() {
    for( int i = 0; i < TOTAL_JOINTS; i++ ) {
        addOrUpdateJoint( getNameForIndex((JointIndex)i), ofVec3f(), false );
    }
}

//--------------------------------------------------------------
void Skeleton::draw() {
    
    drawMesh.clear();
    drawMesh.setMode( OF_PRIMITIVE_LINES );
    
    drawMesh.addVertex( getJoint(SPINE_BASE)->pos );
    drawMesh.addVertex( getJoint(SPINE_MID)->pos );
    drawMesh.addVertex( getJoint(SPINE_MID)->pos );
    drawMesh.addVertex( getJoint(SPINE_SHOULDER)->pos );
    drawMesh.addVertex( getJoint(SPINE_SHOULDER)->pos );
    drawMesh.addVertex( getJoint(NECK)->pos );
    drawMesh.addVertex( getJoint(NECK)->pos );
    drawMesh.addVertex( getJoint(HEAD)->pos );
    
    drawMesh.addVertex( getJoint(SPINE_SHOULDER)->pos );
    drawMesh.addVertex( getJoint(SHOULDER_LEFT)->pos );
    drawMesh.addVertex( getJoint(SHOULDER_LEFT)->pos );
    drawMesh.addVertex( getJoint(ELBOW_LEFT)->pos );
    drawMesh.addVertex( getJoint(ELBOW_LEFT)->pos );
    drawMesh.addVertex( getJoint(WRIST_LEFT)->pos );
    drawMesh.addVertex( getJoint(WRIST_LEFT)->pos );
    drawMesh.addVertex( getJoint(HAND_LEFT)->pos );
    drawMesh.addVertex( getJoint(HAND_LEFT)->pos );
    drawMesh.addVertex( getJoint(HAND_TIP_LEFT)->pos );
    drawMesh.addVertex( getJoint(HAND_LEFT)->pos );
    drawMesh.addVertex( getJoint(THUMB_LEFT)->pos );
    
    drawMesh.addVertex( getJoint(SPINE_BASE)->pos );
    drawMesh.addVertex( getJoint(HIP_LEFT)->pos );
    drawMesh.addVertex( getJoint(HIP_LEFT)->pos );
    drawMesh.addVertex( getJoint(KNEE_LEFT)->pos );
    drawMesh.addVertex( getJoint(KNEE_LEFT)->pos );
    drawMesh.addVertex( getJoint(ANKLE_LEFT)->pos );
    drawMesh.addVertex( getJoint(ANKLE_LEFT)->pos );
    drawMesh.addVertex( getJoint(FOOT_LEFT)->pos );
    
    
    drawMesh.addVertex( getJoint(SPINE_SHOULDER)->pos );
    drawMesh.addVertex( getJoint(SHOULDER_RIGHT)->pos );
    drawMesh.addVertex( getJoint(SHOULDER_RIGHT)->pos );
    drawMesh.addVertex( getJoint(ELBOW_RIGHT)->pos );
    drawMesh.addVertex( getJoint(ELBOW_RIGHT)->pos );
    drawMesh.addVertex( getJoint(WRIST_RIGHT)->pos );
    drawMesh.addVertex( getJoint(WRIST_RIGHT)->pos );
    drawMesh.addVertex( getJoint(HAND_RIGHT)->pos );
    drawMesh.addVertex( getJoint(HAND_RIGHT)->pos );
    drawMesh.addVertex( getJoint(HAND_TIP_RIGHT)->pos );
    drawMesh.addVertex( getJoint(HAND_RIGHT)->pos );
    drawMesh.addVertex( getJoint(THUMB_RIGHT)->pos );
    
    drawMesh.addVertex( getJoint(SPINE_BASE)->pos );
    drawMesh.addVertex( getJoint(HIP_RIGHT)->pos );
    drawMesh.addVertex( getJoint(HIP_RIGHT)->pos );
    drawMesh.addVertex( getJoint(KNEE_RIGHT)->pos );
    drawMesh.addVertex( getJoint(KNEE_RIGHT)->pos );
    drawMesh.addVertex( getJoint(ANKLE_RIGHT)->pos );
    drawMesh.addVertex( getJoint(ANKLE_RIGHT)->pos );
    drawMesh.addVertex( getJoint(FOOT_RIGHT)->pos );
    
    drawMesh.draw();
    
    for( auto& joint : joints ) {
        ofDrawCircle( joint.second->pos, 20 );
        //ofDrawLine( ofVec3f(), joint.second->pos );
    }
}

//--------------------------------------------------------------
shared_ptr <Skeleton::Joint> Skeleton::getJoint(string jointName){
    for( auto joint : joints ){
        if( joint.first == jointName ){
            return joint.second;
        }
    }
    return shared_ptr<Joint>();
}

//--------------------------------------------------------------
shared_ptr <Skeleton::Joint> Skeleton::getJoint( JointIndex aJointIndex ) {
    return getJoint( getNameForIndex(aJointIndex) );
}

//--------------------------------------------------------------
string Skeleton::getNameForIndex( JointIndex aindex ) {
    switch( aindex ) {
        case SPINE_BASE:
            return "SpineBase";
        case SPINE_MID:
            return "SpineMid";
        case SPINE_SHOULDER:
            return "SpineShoulder";
        case NECK:
            return "Neck";
        case HEAD:
            return "Head";
        case SHOULDER_LEFT:
            return "ShoulderLeft";
        case ELBOW_LEFT:
            return "ElbowLeft";
        case WRIST_LEFT:
            return "WristLeft";
        case HAND_LEFT:
            return "HandLeft";
        case HAND_TIP_LEFT:
            return "HandTipLeft";
        case THUMB_LEFT:
            return "ThumbLeft";
        case SHOULDER_RIGHT:
            return "ShoulderRight";
        case ELBOW_RIGHT:
            return "ElbowRight";
        case WRIST_RIGHT:
            return "WristRight";
        case HAND_RIGHT:
            return "HandRight";
        case HAND_TIP_RIGHT:
            return "HandTipRight";
        case THUMB_RIGHT:
            return "ThumbRight";
        case HIP_LEFT:
            return "HipLeft";
        case KNEE_LEFT:
            return "KneeLeft";
        case ANKLE_LEFT:
            return "AnkleLeft";
        case FOOT_LEFT:
            return "FootLeft";
        case HIP_RIGHT:
            return "HipRight";
        case KNEE_RIGHT:
            return "KneeRight";
        case ANKLE_RIGHT:
            return "AnkleRight";
        case FOOT_RIGHT:
            return "FootRight";
        default:
            return "Unknown";
            
    }
    return "Unknown";
}

//--------------------------------------------------------------
void Skeleton::addOrUpdateJoint(string jointName, ofVec3f position, bool seen){
    
    if( joints.count(jointName) == 0 ){
//        ofLogError() << " all joints should be made at startup! jointName = " << jointName <<  endl;
        //will crash here - so lets make a shared_ptr
        joints[jointName] = shared_ptr <Joint>( new Joint() );
        joints[jointName]->name = jointName;
    }
    
    joints[jointName]->pos      = position * 1000.;
    joints[jointName]->bSeen    = seen;
    joints[jointName]->bNewThisFrame = true;
    
    if( firstTimeSeen < 0 ) {
        firstTimeSeen = lastTimeSeen;
    }
    lastTimeSeen = ofGetElapsedTimef();
}

