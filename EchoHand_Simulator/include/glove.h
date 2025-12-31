#pragma once

#include <array>
#include <string>
#include <raylib.h>
#include <raymath.h>
#include <iostream>
#include <cstring>

struct OpenGlovesData
{
    // Thumb, Index, Middle, Ring, Pinky (0-1)
    std::array<float, 5> fingerCurl = {0};       
    
    // Individual joints
    std::array<std::array<float, 4>, 5> jointCurl = {{{0}}}; 
    
    // Finger splay
    std::array<float, 5> splay = {0};            
    
    float joystickX = 0.0f;
    float joystickY = 0.0f;
    bool joystickButton = false;
    
    bool buttonTrigger = false;
    bool buttonA = false;
    bool buttonB = false;
    bool buttonGrab = false;
    bool buttonSystem = false;
    bool buttonCalibrate = false;
    
    float triggerAnalog = 0.0f;
};

// Camera controller
struct CameraController
{
    Camera3D camera = {
        {0.0f, 0.5f, 3.0f},  
        {0.0f, 0.5f, 0.0f},  
        {0.0f, 1.0f, 0.0f},  
        45.0f,               
        CAMERA_PERSPECTIVE   
    };
    
    Vector3 position = {0.0f, 0.5f, 3.0f};
    float yaw = 0.0f;
    float pitch = 0.0f;
    float moveSpeed = 3.0f;
    float lookSpeed = 0.003f;
    bool isLooking = false;
};


// Finger segment (one bone)
struct FingerSegment
{
    float length = 0.0f;
    float radius = 0.0f;
    
    // Current curl angle in radians
    float curl = 0.0f;     
    Color color = WHITE;
};

// Complete finger (3-4 segments)
struct ProceduralFinger
{
    // Position relative to palm
    Vector3 basePosition = {0, 0, 0};  
    
    // Splay angle
    float baseAngle = 0.0f;            
    
    // Thumb has metacarpal
    bool hasMetacarpal = false;        
    
    // Only for thumb
    FingerSegment metacarpal;          
    FingerSegment proximal;
    FingerSegment intermediate;
    FingerSegment distal;
};

// Complete hand
struct ProceduralHand
{
    float palmWidth = 0.35f;
    float palmHeight = 0.08f;
    float palmDepth = 0.40f;
    
    // Thumb, Index, Middle, Ring, Pinky
    ProceduralFinger fingers[5];       
    
    Vector3 position = {0, 0, 0};
    
    // Euler angles
    Vector3 rotation = {0, 0, 0};      
    
    Color skinColor = WHITE;
    Color jointColor = WHITE;
    
    bool isInitialized = false;
};

// Legacy HandModel for compatibility (not used in procedural version)
struct HandModel
{
    bool isLoaded = false;
};

void initializeWindow();
bool userAttemptedClose();
void initGloveRendering();
void cleanupGloveRendering();
void updateGloveObject(const char *payload);
void updateGloveObject(const std::string &payload);
void drawGloveObject();

// Internal functions
void initProceduralHand(ProceduralHand &hand);
void updateHandPose(ProceduralHand &hand, const OpenGlovesData &data);
void drawProceduralHand(const ProceduralHand &hand);
void parseOpenGlovesPayload(const char *payload, OpenGlovesData &data);
void updateCamera(CameraController &controller, float deltaTime);
void drawUI(const OpenGlovesData &data);

