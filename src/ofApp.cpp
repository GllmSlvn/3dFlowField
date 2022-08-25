#include "ofApp.h"

int   nPoints = 4096;   // points to draw
float complexity = 6;   // wind complexity
float pollenMass = .8;  // pollen mass
float timeSpeed = .02;  // wind variation speed
float phase = TWO_PI;   // separate u-noise from v-noise
float windSpeed = 40;   // wind vector magnitude for debug
int   step = 100;       // spatial sampling rate for debug
bool  debugMode = false;

// PARAM
Params param;

void Params::setup(){
eCenter = { ofGetWidth() / 2, ofGetHeight() / 2 ,ofGetWidth() / 2 };
eRad = 50;
velRad = 200;
lifeTime = 1.0;
f = 10;
rotate = 90;
};

glm::vec3 ofApp::getField(const glm::vec3& position){
    float normx = ofNormalize(position.x, 0, ofGetWidth());
    float normy = ofNormalize(position.y, 0, ofGetHeight());
    float normz = ofNormalize(position.z, 0, ofGetWidth()); // profondeur = largeur
    float u = ofNoise(t + phase, normx * complexity + phase, normy * complexity + phase, normz * complexity + phase);
    float v = ofNoise(t - phase, normx * complexity - phase, normy * complexity + phase, normz * complexity + phase);
    float w = ofNoise(t + phase, normx * complexity + phase, normy * complexity - phase, normz * complexity - phase);
    return glm::vec3(u,v,w);
}

glm::vec3 randomPointInCircle( float maxRad ){
    glm::vec3 pnt;
    
    // Cercle
    float rad = ofRandom( 0, maxRad );
    float angle = ofRandom( 0, M_TWO_PI );
    pnt.x = cos( angle ) * rad;
    pnt.y = sin( angle ) * rad;
    pnt.z = sin( angle ) * rad;
    
    
    /*
     // Ligne -> distribution gaussienne
     float rand = 0;
     for(int i = 0; i < 6; i++){
     rand += ofRandom(-maxRad, maxRad);// * (1.0/maxRad);
     }
     
     float w = 1. * (rand - 3.0);
     
     pnt.x = w;
     pnt.y = 0;
     */
    
    /*
     // Souris
     pnt.x = ofGetMouseX();
     pnt.y = ofGetMouseY();
     */
    
    return pnt;
}
//--------------------------------------------------------------
//----------------------  Particle  ----------------------------
//--------------------------------------------------------------

Particle::Particle() {
    live = false;
}

//--------------------------------------------------------------
void Particle::setup() {
    //pos = param.eCenter + randomPointInCircle( param.eRad );
    pos = glm::vec3(ofRandom(0, ofGetWidth()), ofRandom(0, ofGetHeight()), ofRandom(0, ofGetWidth()));
    vel = glm::vec3(ofRandom(-10,10), ofRandom(-10,-10), ofRandom(-10,-10));
    force = glm::vec3(ofRandom(-10,10), ofRandom(-10,10), ofRandom(-10,10));
    
    time = 0;
    lifeTime = 1.0;
    live = true;
}

//--------------------------------------------------------------
void Particle::update( float dt ){
    if ( live ) {
        
        // on applique la force
        vel += force;
        
        //Update pos
        pos += vel * dt;    //Euler method
        
        //Update time and check if particle should die
        time += dt;
        if ( time >= lifeTime ) {
            live = false;   //Particle is now considered as died
        }
    }
}

//--------------------------------------------------------------
void Particle::edges(){
    
    if(pos.x >= ofGetWidth() || pos.x <= 0) {
        vel.x *= -1;
        pos.x = ofClamp(pos.x, 0, ofGetWidth());
    }
    if(pos.y >= ofGetHeight() || pos.y <= 0) {
        vel.y *= -1;
        pos.y = ofClamp(pos.y, 0, ofGetHeight());
    }
}

//--------------------------------------------------------------
void Particle::draw(){
    if ( live ) {
        float size = 1;
        
        //Compute color
        ofColor color = ofColor::blue;
        float hue = ofMap( time, 0, lifeTime, 200, 100 );
        color.setHue( hue );
        
        float brightness = ofMap(time, 0, lifeTime, 255, 0);
        color.setBrightness( brightness );
        
        ofSetColor( color );
        ofDrawCircle( pos, size );  //Draw particle
    }
}


//--------------------------------------------------------------
void ofApp::setup(){
    ofBackground(0);
   
    width = ofGetWidth(), height = ofGetHeight(), depth = ofGetWidth();
    fbo.allocate( width, height, GL_RGB32F_ARB );
    
    //Fill buffer with white color
    fbo.begin();
    ofClear(0,0,0);
    fbo.end();
    
    history = 0.9;
    bornRate = 5000;
    
    bornCount = 0;
    time0 = ofGetElapsedTimef();
    
    // gui setup
    gui.setup();
    gui.add(posCam.setup("posCam",
        ofVec3f(ofGetWidth()/2,ofGetHeight()/2,1200),
        ofVec3f(-10000,-10000,-10000),
        ofVec3f(10000,10000,10000)));
    gui.add(lookatCam.setup("lookat",
        ofVec3f(300,300,0),
        ofVec3f(-1000,-1000,-1000),
        ofVec3f(1000,1000,1000)));
    
    // cam setting
    cam.setPosition(posCam);
    cam.lookAt(lookatCam);
}

//--------------------------------------------------------------
void ofApp::update(){
    
    ofSetWindowTitle(ofToString(ofGetFrameRate()));
    
    cam.setPosition(posCam);
    cam.lookAt(lookatCam);
    
    t = ofGetFrameNum() * timeSpeed;
    
    //Compute dt
    float time = ofGetElapsedTimef();
    float dt = ofClamp( time - time0, 0, 0.1 );
    time0 = time;
    
    //Delete inactive particles
    int i=0;
    while (i < p.size()) {
        if ( !p[i].live ) {
            p.erase( p.begin() + i );
        }
        else {
            i++;
        }
    }
    
    //Born new particles
    bornCount += dt * bornRate;      //Update bornCount value
    if ( bornCount >= 1 ) {          //It's time to born particle(s)
        int bornN = int( bornCount );//How many born
        bornCount -= bornN;          //Correct bornCount value
        for (int i=0; i<bornN; i++) {
            Particle newP;
            newP.setup();            //Start a new particle
            p.push_back( newP );     //Add this particle to array
        }
    }
    
    //Update the particles
    for (int i=0; i<p.size(); i++) {
        p[i].vel = getField(p[i].pos) * 100.;
        // fonction follow aiderait + force dans les particules = plus naturel
        //p[i].follow(flo);
        p[i].update( dt );
        p[i].edges();
    }
    
    param.eCenter = {mouseX, mouseY,0 };
}

//--------------------------------------------------------------
void ofApp::draw(){
    
   
    ofBackground( 0 );  //Set white background
    
    //1. Drawing to buffer
    fbo.begin();
    
    ofEnableAlphaBlending();
    
    float alpha = (1-history) * 255;
    ofSetColor( 0,0,0, alpha );
    ofFill();
    ofDrawRectangle( 0, 0, ofGetWidth(), ofGetHeight() );
    
    cam.begin();
    //Draw the particles
    ofFill();
    ofEnableBlendMode(OF_BLENDMODE_ADD);
    for (int i=0; i<p.size(); i++) {
        p[i].draw();
    }
   
    //drawField();
    
    cam.end();
    ofDisableAlphaBlending();
    fbo.end();
    
    //2. Draw buffer on the screen
    ofSetColor( 255, 255, 255 );
    
    fbo.draw( 0, 0 );
    
    gui.draw();
}

//--------------------------------------------------------------
void ofApp::drawField(){
    
     // draw a vector field for the debug screen
    
    for(int i = 0; i < width; i += step) {
        for(int j = 0; j < height; j += step) {
            for(int k = 0; k < depth; k += step){
                glm::vec3 field = getField(glm::vec3(i, j, k));
                ofPushMatrix();
                ofTranslate(i, j, k);
                ofSetColor(255);
                ofDrawLine(glm::vec3(0, 0, 0), glm::vec3(ofLerp(-windSpeed, windSpeed, field.x), ofLerp(-windSpeed, windSpeed, field.y), ofLerp(-windSpeed, windSpeed, field.z)));
                ofPopMatrix();
            }
        }
    }
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

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
