#pragma once

#include "ofMain.h"
#include "ofxGui.h"

//Particle class
class Particle {
public:
    Particle();
    void setup();
    void update( float dt );
    void draw();
    void edges();
    
    // ajout d'une fonction follow()
    //std::vector<glm::vec2> follow();
    
    glm::vec3 pos, vel, force; //Position
    
    float time;                //Time of living
    float lifeTime;            //Allowed lifetime
    bool live;                 //Is particle live
};

//Control parameters class
class Params {
public:
    void setup();
    glm::vec3 eCenter;    //Emitter center
    float eRad;         //Emitter radius
    float velRad;       //Initial velocity limit
    float lifeTime;     //Lifetime in seconds
    float f;
    
    float rotate;   //Direction rotation speed in angles per second
};

extern Params param; //Déclaration d'une variable globale

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();
    
        void drawField();
    
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
		
    glm::vec3 getField(const glm::vec3& position);
    
    float t;
    float width, height, depth;
    
    ofCamera cam;
    
    std::vector<Particle> p;
    ofFbo fbo;
    float history;
    float time0;
    
    float bornRate;       //Particles born rate per second
    float bornCount;
    
    ofxPanel gui;
    ofxVec3Slider posCam;
    ofxVec3Slider lookatCam;
};
