#include "ofxQuNeo.h"
int pressNum[16] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16}; //pressure input control values
int xNum[16] = {17,19,21,23,25,27,29,31,33,35,37,39,41,43,45,47}; //x control num
int yNum[16] = {18,20,22,24,26,28,30,32,34,36,38,40,42,44,46,48}; //Y control num
int sliderPressNum[12] = {49,50,51,52,53,54,55,56,57,58,69}; //pressure on slider num
int sliderLocNum[12] = {59,60,61,62,63,64,65,66,67,68,70}; //pressure on slider num
int arrowPressNum[15] = {71,72,73,74,75,76,77,78,79,80,81,82,83,84,85}; //pressure on slider num

ofxQuNeo::ofxQuNeo(){
    
}

ofxQuNeo::~ofxQuNeo(){
    
}

void ofxQuNeo::setup(){
    quNeo.openPort("QUNEO");
    quNeo.addListener(this);
    controlVals.assign(127, 0);
    prevControlVals.assign(127, 0);
    velocityVals.assign(127, 0);
    prevVelocity.assign(127, 0);
    
    sender.setup("localhost", 1234);
    
    prevSendTime = 0;
}

void ofxQuNeo::update(){
    

}

void ofxQuNeo::drawInterface(int x, int y){
    
    int numCols = 4;
    int numRows = 4;
    int padWidth = 50;
    int padHeight = 50;
    
    ofPushStyle();
    ofPushMatrix();
    ofTranslate(x,y);
    
    //Draw Pads
    int countpos = 1;
    for(int k = 4; k>0;k--){
        for(int j = 0; j<4;j++){
            ofPushMatrix();
            ofTranslate((j*55)+50, (k*55));
            
            ofSetRectMode(OF_RECTMODE_CORNER);
            ofSetColor(0, 0, 0);
            ofNoFill();
            ofRect(0,0,padWidth, padHeight);
            

            float xScale, yScale, pressureScale;
            pressureScale = ofMap(controlVals[pressNum[countpos-1]], 0, 127, 0, padWidth);
            xScale = ofMap(controlVals[xNum[countpos-1]], 0, 127, 0, padWidth);
            yScale = ofMap(controlVals[yNum[countpos-1]], 127, 0, 0, padHeight);
            
            ofSetRectMode(OF_RECTMODE_CENTER);
            ofSetColor(100, 100, 255);
            ofFill();
            ofRect(padWidth/2,(padWidth/2),pressureScale, pressureScale);
            
            ofSetRectMode(OF_RECTMODE_CORNER);
            //draw x/y stuff
            ofFill();
            ofSetColor(0, 0, 255);
            ofRect(xScale, yScale, 5, 5);
            //name of pad
            ofSetColor(0);
            ofDrawBitmapString(ofToString(countpos), 20,20);
            ofPopMatrix();
            
            countpos++;
        }
    }
    ofPopMatrix();
    
    //Draw Arrows
    ofPushMatrix();
    ofTranslate(x+50, y+320);
    ofDrawBitmapString("Arrows", 0,40);
    for (int i=0; i<15; i++){
        ofPushMatrix();
        ofTranslate(i*15,0);
        ofSetRectMode(OF_RECTMODE_CENTER);
        
        int pressureScale = ofMap(controlVals[arrowPressNum[i]], 0, 127, 0, padHeight);
        
        
        ofNoFill();
        ofSetColor(0, 0, 0);
        ofRect(0,0,10, padHeight);
        
        ofSetColor(100, 100, 255);
        ofFill();
        ofRect(0,-pressureScale/2+padHeight/2,10, pressureScale);
        
        ofPopMatrix();
    }
    ofPopMatrix();
    
    //Draw Sliders Buttons
    ofPushMatrix();
    ofTranslate(x+50, y);
        ofDrawBitmapString("Sliders", 0,40);
        for (int i=0; i<11; i++){
            ofPushMatrix();
            ofTranslate(i*20,0);
                ofSetRectMode(OF_RECTMODE_CENTER);
                
                int pressureScale = ofMap(controlVals[sliderPressNum[i]], 0, 127, 0, padHeight);
                int locScale = ofMap(controlVals[sliderLocNum[i]], 127, 0, 0, padHeight);
                
                ofNoFill();
                ofSetColor(0, 0, 0);
                ofRect(0,0,10, padHeight);
                
                ofSetColor(100, 100, 255);
                ofFill();
                ofRect(0,-pressureScale/2+padHeight/2,10, pressureScale);
                
                ofSetColor(0, 0, 0);
                ofRect(0, locScale-padHeight/2, 10, 5);
            ofPopMatrix();
        }
    ofPopMatrix();
    
    ofPopStyle();
}

//--------------------------------------------------------------
void ofxQuNeo::newMidiMessage(ofxMidiMessage& msg){
	// make a copy of the latest message
	
    

    quNeoMessage = msg;
    
    for (int i=0; i<127; i++) { //this could be optimized...
        if (i==quNeoMessage.pitch && prevVelocity[i] != quNeoMessage.velocity ) {
            velocityVals[i] = quNeoMessage.velocity;
            ofxOscMessage m;
            m.setAddress("/quVel/"+ofToString(i));
            m.addIntArg(velocityVals[i]);
            sender.sendMessage(m);
            
            prevVelocity[i] = velocityVals [i]; //we need to ignore messages with values of 0, which get sent with all the other control data
        }
        
        
        //cout<<"Pitch "<<quNeoMessage.pitch<<endl;
    }
    

    

    for (int i=0; i<127; i++) {
        if (i==quNeoMessage.control) {
            controlVals[i] = quNeoMessage.value; //just make a vector of all the control vals...
            
            if (prevSendTime+0.002 < ofGetElapsedTimef() && controlVals[i]!=prevControlVals[i]) { //need to rate limit incoming data before sending over osc - but we need to keep values that are equal to 0
                
                //sendPadInfo
                for (int j=0; j<16; j++){
                    if (i==pressNum[j]) {
                        ofxOscMessage m;
                        m.setAddress("/quP/"+ofToString(j+1));
                        m.addIntArg(controlVals[i]);
                        sender.sendMessage(m);
                    }
                    if (i==xNum[j]) {
                        ofxOscMessage m;
                        m.setAddress("/quX/"+ofToString(j+1));
                        m.addIntArg(controlVals[i]);
                        sender.sendMessage(m);
                    }
                    if (i==yNum[j]) {
                        ofxOscMessage m;
                        m.setAddress("/quY/"+ofToString(j+1));
                        m.addIntArg(controlVals[i]);
                        sender.sendMessage(m);
                    }
                    
                    prevControlVals[i] = controlVals[i];
                }
                
                for (int j=0; j<11; j++){
                    if (i==sliderPressNum[j]) {
                        ofxOscMessage m;
                        m.setAddress("/quSlideP/"+ofToString(j+1));
                        m.addIntArg(controlVals[i]);
                        sender.sendMessage(m);
                    }
                    if(i==sliderLocNum[j]){
                        ofxOscMessage m;
                        m.setAddress("/quSlideLoc/"+ofToString(j+1));
                        m.addIntArg(controlVals[i]);
                        sender.sendMessage(m);
                    }
                }
                
                for (int j=0; j<11; j++){
                    if (i==arrowPressNum[j]) {
                        ofxOscMessage m;
                        m.setAddress("/quArrowP/"+ofToString(j+1));
                        m.addIntArg(controlVals[i]);
                        sender.sendMessage(m);
                    }
                }
                
                
                prevSendTime = ofGetElapsedTimef();
            
            }
        }
    }
    
    
    

}

void ofxQuNeo::drawDebug(){
    
    ofSetColor(0);
	
	// draw the last recieved message contents to the screen
	text << "Received: " << ofxMidiMessage::getStatusString(quNeoMessage.status);
	ofDrawBitmapString(text.str(), 20, 20);
	text.str(""); // clear
	
	text << "channel: " << quNeoMessage.channel;
	ofDrawBitmapString(text.str(), 20, 34);
	text.str(""); // clear
	
	text << "pitch: " << quNeoMessage.pitch;
	ofDrawBitmapString(text.str(), 20, 48);
	text.str(""); // clear
	ofRect(20, 58, ofMap(quNeoMessage.pitch, 0, 127, 0, ofGetWidth()-40), 20);
	
	text << "velocity: " << quNeoMessage.velocity;
	ofDrawBitmapString(text.str(), 20, 96);
	text.str(""); // clear
	ofRect(20, 105, ofMap(quNeoMessage.velocity, 0, 127, 0, ofGetWidth()-40), 20);
	
	text << "control: " << quNeoMessage.control;
	ofDrawBitmapString(text.str(), 20, 144);
	text.str(""); // clear
	ofRect(20, 154, ofMap(quNeoMessage.control, 0, 127, 0, ofGetWidth()-40), 20);
	
	text << "value: " << quNeoMessage.value;
	ofDrawBitmapString(text.str(), 20, 192);
	text.str(""); // clear

	if(quNeoMessage.status == MIDI_PITCH_BEND) {
		ofRect(20, 202, ofMap(quNeoMessage.value, 0, MIDI_MAX_BEND, 0, ofGetWidth()-40), 20);
	}
	else {
		ofRect(20, 202, ofMap(quNeoMessage.value, 0, 127, 0, ofGetWidth()-40), 20);
	}
	
	text << "delta: " << quNeoMessage.deltatime;
	ofDrawBitmapString(text.str(), 20, 240);
	text.str(""); // clear
}

void ofxQuNeo::exit(){
    quNeo.closePort();
	quNeo.removeListener(this);
}