#include "glove.h"
#include <cmath>
#include <rlgl.h>

// Global state
static CameraController g_cameraController;
static OpenGlovesData g_gloveData;
static ProceduralHand g_hand;
static RenderTexture2D g_renderTarget;
static bool g_windowInitialized = false;
// Toggle with D key
static bool g_showProtocolDiagram = false; 

// Draw a smooth capsule between two points
void DrawSmoothCapsule(Vector3 start, Vector3 end, float radius, Color color)
{
    DrawCylinderEx(start, end, radius, radius, 16, color);
    DrawSphere(start, radius, color);
    DrawSphere(end, radius, color);
}

// Draw a joint sphere with highlight
void DrawJoint(Vector3 pos, float radius, Color baseColor)
{
    DrawSphere(pos, radius, baseColor);
    Color highlight = {
        (unsigned char)fmin(255, baseColor.r + 40),
        (unsigned char)fmin(255, baseColor.g + 40),
        (unsigned char)fmin(255, baseColor.b + 40),
        baseColor.a
    };
    DrawSphere({pos.x, pos.y + radius * 0.3f, pos.z}, radius * 0.6f, highlight);
}

void initProceduralHand(ProceduralHand &hand)
{
    hand.position = {0.0f, 0.0f, 0.0f};

    // Rotate so palm faces camera
    hand.rotation = {90.0f * DEG2RAD, 0.0f, 0.0f}; 
    
    // Teal/cyan color scheme
    Color baseColor = {40, 180, 200, 255};
    Color midColor = {60, 200, 220, 255};
    Color tipColor = {100, 230, 250, 255};
    
    hand.skinColor = baseColor;
    hand.jointColor = midColor;
    
    hand.palmWidth = 0.35f;
    hand.palmHeight = 0.08f;
    hand.palmDepth = 0.40f;
    
    // THUMB
    hand.fingers[0].basePosition = {0.18f, 0.10f, 0.0f};
    hand.fingers[0].baseAngle = 45.0f * DEG2RAD;
    hand.fingers[0].hasMetacarpal = true;
    hand.fingers[0].metacarpal = {0.10f, 0.038f, 0.0f, baseColor};
    hand.fingers[0].proximal = {0.11f, 0.034f, 0.0f, midColor};
    hand.fingers[0].intermediate = {0.09f, 0.030f, 0.0f, tipColor};
    hand.fingers[0].distal = {0.0f, 0.0f, 0.0f, tipColor};
    
    // INDEX
    hand.fingers[1].basePosition = {0.12f, -0.20f, 0.0f};
    hand.fingers[1].baseAngle = 5.0f * DEG2RAD;
    hand.fingers[1].hasMetacarpal = false;
    hand.fingers[1].proximal = {0.16f, 0.032f, 0.0f, baseColor};
    hand.fingers[1].intermediate = {0.10f, 0.028f, 0.0f, midColor};
    hand.fingers[1].distal = {0.08f, 0.024f, 0.0f, tipColor};
    
    // MIDDLE
    hand.fingers[2].basePosition = {0.0f, -0.22f, 0.0f};
    hand.fingers[2].baseAngle = 0.0f;
    hand.fingers[2].hasMetacarpal = false;
    hand.fingers[2].proximal = {0.18f, 0.034f, 0.0f, baseColor};
    hand.fingers[2].intermediate = {0.12f, 0.029f, 0.0f, midColor};
    hand.fingers[2].distal = {0.09f, 0.024f, 0.0f, tipColor};
    
    // RING
    hand.fingers[3].basePosition = {-0.11f, -0.20f, 0.0f};
    hand.fingers[3].baseAngle = -5.0f * DEG2RAD;
    hand.fingers[3].hasMetacarpal = false;
    hand.fingers[3].proximal = {0.16f, 0.031f, 0.0f, baseColor};
    hand.fingers[3].intermediate = {0.10f, 0.027f, 0.0f, midColor};
    hand.fingers[3].distal = {0.08f, 0.023f, 0.0f, tipColor};
    
    // PINKY
    hand.fingers[4].basePosition = {-0.20f, -0.17f, 0.0f};
    hand.fingers[4].baseAngle = -12.0f * DEG2RAD;
    hand.fingers[4].hasMetacarpal = false;
    hand.fingers[4].proximal = {0.12f, 0.026f, 0.0f, baseColor};
    hand.fingers[4].intermediate = {0.08f, 0.022f, 0.0f, midColor};
    hand.fingers[4].distal = {0.06f, 0.019f, 0.0f, tipColor};
    
    hand.isInitialized = true;
    std::cout << "VR glove hand initialized\n";
}

void updateHandPose(ProceduralHand &hand, const OpenGlovesData &data)
{
    if (!hand.isInitialized) return;
    
    for (int f = 0; f < 5; f++)
    {
        float curl = data.fingerCurl[f];
        float baseCurl = 10.0f * DEG2RAD;
        
        // Thumb
        if (f == 0) 
        {
            hand.fingers[f].metacarpal.curl = baseCurl * 0.5f + curl * 90.0f * DEG2RAD;
            hand.fingers[f].proximal.curl = baseCurl + curl * 130.0f * DEG2RAD;
            hand.fingers[f].intermediate.curl = baseCurl + curl * 110.0f * DEG2RAD;
        }
        else
        {
            hand.fingers[f].proximal.curl = baseCurl + curl * 75.0f * DEG2RAD;
            hand.fingers[f].intermediate.curl = baseCurl * 1.5f + curl * 90.0f * DEG2RAD;
            hand.fingers[f].distal.curl = baseCurl + curl * 65.0f * DEG2RAD;
        }
    }
}

void drawFinger(const ProceduralFinger &finger, Vector3 palmPos, int fingerIndex, Matrix handRotation)
{
    bool isThumb = (fingerIndex == 0);
    
    Vector3 rotatedBase = Vector3Transform(finger.basePosition, handRotation);
    Vector3 pos = Vector3Add(palmPos, rotatedBase);
    
    Vector3 dir = {0.0f, -1.0f, 0.0f};
    
    Matrix splayRot = MatrixRotateZ(finger.baseAngle);
    dir = Vector3Transform(dir, splayRot);
    
    if (isThumb)
    {
        dir = {1.0f, -0.5f, 0.0f};
        dir = Vector3Normalize(dir);
        Matrix splay = MatrixRotateZ(finger.baseAngle);
        dir = Vector3Transform(dir, splay);
    }
    
    // Metacarpal (thumb)
    if (finger.hasMetacarpal && finger.metacarpal.length > 0.01f)
    {
        Vector3 endPos = Vector3Add(pos, Vector3Scale(dir, finger.metacarpal.length));
        DrawSmoothCapsule(pos, endPos, finger.metacarpal.radius, finger.metacarpal.color);
        DrawJoint(endPos, finger.metacarpal.radius * 1.1f, finger.proximal.color);
        pos = endPos;
        
        Vector3 curlAxis = isThumb ? Vector3{0.0f, 0.0f, 1.0f} : Vector3{1.0f, 0.0f, 0.0f};
        // Negative to curl toward palm
        Matrix curlRot = MatrixRotate(curlAxis, -finger.metacarpal.curl);  
        dir = Vector3Transform(dir, curlRot);
    }
    
    // Proximal
    {
        Vector3 endPos = Vector3Add(pos, Vector3Scale(dir, finger.proximal.length));
        DrawSmoothCapsule(pos, endPos, finger.proximal.radius, finger.proximal.color);
        DrawJoint(endPos, finger.proximal.radius * 1.1f, finger.intermediate.color);
        pos = endPos;
        
        Vector3 curlAxis = isThumb ? Vector3{0.0f, 0.0f, 1.0f} : Vector3{1.0f, 0.0f, 0.0f};
        // Negative to curl toward palm
        Matrix curlRot = MatrixRotate(curlAxis, -finger.proximal.curl);  
        dir = Vector3Transform(dir, curlRot);
    }
    
    // Intermediate
    {
        Vector3 endPos = Vector3Add(pos, Vector3Scale(dir, finger.intermediate.length));
        DrawSmoothCapsule(pos, endPos, finger.intermediate.radius, finger.intermediate.color);
        pos = endPos;
        
        if (!isThumb)
        {
            DrawJoint(endPos, finger.intermediate.radius * 1.1f, finger.distal.color);
            Vector3 curlAxis = {1.0f, 0.0f, 0.0f};
            
            // Negative to curl toward palm
            Matrix curlRot = MatrixRotate(curlAxis, -finger.intermediate.curl);  
            dir = Vector3Transform(dir, curlRot);
        }
        else
        {
             // Thumb distal joint (just for tip direction)
            Vector3 curlAxis = {0.0f, 0.0f, 1.0f};
            Matrix curlRot = MatrixRotate(curlAxis, -finger.intermediate.curl); 
            dir = Vector3Transform(dir, curlRot);           
        }
    }
    
    // Distal
    if (!isThumb && finger.distal.length > 0.01f)
    {
        Vector3 endPos = Vector3Add(pos, Vector3Scale(dir, finger.distal.length));
        DrawSmoothCapsule(pos, endPos, finger.distal.radius, finger.distal.color);
        Color brightTip = {150, 240, 255, 255};
        DrawSphere(endPos, finger.distal.radius * 1.2f, brightTip);
    }
    else if (isThumb)
    {
        Color brightTip = {150, 240, 255, 255};
        DrawSphere(pos, finger.intermediate.radius * 1.3f, brightTip);
    }
}

void drawPalm(const ProceduralHand &hand, Matrix handRotation)
{
    Vector3 pos = hand.position;
    Color palmColor = hand.skinColor;
    Color darkColor = {30, 140, 160, 255};
    Color lightColor = {80, 210, 230, 255};
    
    float w = hand.palmWidth;
    float h = hand.palmHeight;
    
    // Flat palm box
    rlPushMatrix();
    rlTranslatef(pos.x, pos.y, pos.z);
    rlRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    DrawCube({0, 0, 0}, w * 1.2f, w * 1.0f, h * 1.5f, palmColor);
    rlPopMatrix();
    
    // Rounded edges
    float edgeR = h * 0.8f;
    
    Vector3 leftEdge = Vector3Transform({-w * 0.5f, 0.0f, 0.0f}, handRotation);
    DrawCapsule(Vector3Add(pos, Vector3Add(leftEdge, Vector3Transform({0, -w*0.35f, 0}, handRotation))),
                Vector3Add(pos, Vector3Add(leftEdge, Vector3Transform({0, w*0.35f, 0}, handRotation))),
                edgeR, 8, 8, darkColor);
    
    Vector3 rightEdge = Vector3Transform({w * 0.5f, 0.0f, 0.0f}, handRotation);
    DrawCapsule(Vector3Add(pos, Vector3Add(rightEdge, Vector3Transform({0, -w*0.35f, 0}, handRotation))),
                Vector3Add(pos, Vector3Add(rightEdge, Vector3Transform({0, w*0.35f, 0}, handRotation))),
                edgeR, 8, 8, darkColor);
    
    // Wrist
    Vector3 wristStart = Vector3Add(pos, Vector3Transform({0.0f, w * 0.4f, 0.0f}, handRotation));
    Vector3 wristEnd = Vector3Add(pos, Vector3Transform({0.0f, w * 0.7f, 0.0f}, handRotation));
    DrawCylinderEx(wristStart, wristEnd, w * 0.35f, w * 0.28f, 12, darkColor);
    
    // Thumb mount
    Vector3 thumbMount = Vector3Transform({w * 0.45f, 0.12f, 0.0f}, handRotation);
    DrawSphere(Vector3Add(pos, thumbMount), w * 0.18f, lightColor);
}

void drawProceduralHand(const ProceduralHand &hand)
{
    if (!hand.isInitialized) return;
    
    Matrix handRotation = MatrixRotateX(hand.rotation.x);
    
    drawPalm(hand, handRotation);
    
    for (int f = 0; f < 5; f++)
    {
        drawFinger(hand.fingers[f], hand.position, f, handRotation);
    }
}

void updateCamera(CameraController &controller, float deltaTime)
{
    float speed = controller.moveSpeed * deltaTime;
    if (IsKeyDown(KEY_LEFT_SHIFT)) speed *= 2.5f;

    Vector3 forward = Vector3Normalize(Vector3Subtract(controller.camera.target, controller.camera.position));
    Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, controller.camera.up));

    if (IsKeyDown(KEY_W)) controller.position = Vector3Add(controller.position, Vector3Scale(forward, speed));
    if (IsKeyDown(KEY_S)) controller.position = Vector3Subtract(controller.position, Vector3Scale(forward, speed));
    if (IsKeyDown(KEY_A)) controller.position = Vector3Subtract(controller.position, Vector3Scale(right, speed));
    if (IsKeyDown(KEY_D) && !IsKeyPressed(KEY_D)) controller.position = Vector3Add(controller.position, Vector3Scale(right, speed));
    if (IsKeyDown(KEY_SPACE)) controller.position.y += speed;
    if (IsKeyDown(KEY_LEFT_CONTROL)) controller.position.y -= speed;

    if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON))
    {
        if (!controller.isLooking)
        {
            controller.isLooking = true;
            DisableCursor();
        }

        Vector2 mouseDelta = GetMouseDelta();
        controller.yaw -= mouseDelta.x * controller.lookSpeed;
        controller.pitch -= mouseDelta.y * controller.lookSpeed;

        if (controller.pitch > 89.0f * DEG2RAD) controller.pitch = 89.0f * DEG2RAD;
        if (controller.pitch < -89.0f * DEG2RAD) controller.pitch = -89.0f * DEG2RAD;
    }
    else
    {
        if (controller.isLooking)
        {
            controller.isLooking = false;
            EnableCursor();
        }
    }

    float wheel = GetMouseWheelMove();
    if (wheel != 0.0f)
    {
        controller.camera.fovy -= wheel * 3.0f;
        if (controller.camera.fovy < 20.0f) controller.camera.fovy = 20.0f;
        if (controller.camera.fovy > 100.0f) controller.camera.fovy = 100.0f;
    }

    controller.camera.position = controller.position;

    Vector3 lookDir = {
        cosf(controller.pitch) * sinf(controller.yaw),
        sinf(controller.pitch),
        cosf(controller.pitch) * cosf(controller.yaw)
    };
    controller.camera.target = Vector3Add(controller.position, lookDir);
}

void parseOpenGlovesPayload(const char *payload, OpenGlovesData &data)
{
    if (!payload || strlen(payload) == 0) return;

    std::string payloadStr(payload);

    std::cout << "Received payload: " << payloadStr << std::endl;
    
    // Reset buttons to false before parsing
    data.joystickButton = false;
    data.buttonTrigger = false;
    data.buttonA = false;
    data.buttonB = false;
    data.buttonGrab = false;
    data.buttonSystem = false;
    data.buttonCalibrate = false;
    
    size_t pos = 0;
    while (pos < payloadStr.length())
    {
        while (pos < payloadStr.length() && !isalpha(payloadStr[pos])) pos++;
        if (pos >= payloadStr.length()) break;

        char key = payloadStr[pos++];
        
        size_t valueStart = pos;
        while (pos < payloadStr.length() && (isdigit(payloadStr[pos]) || payloadStr[pos] == '.')) pos++;
        
        float value = 0;
        if (valueStart < pos)
        {
            std::string sub = payloadStr.substr(valueStart, pos - valueStart);
            char* endPtr;
            value = strtof(sub.c_str(), &endPtr);
        }

        switch (key)
        {
        case 'A': data.fingerCurl[0] = value / 4095.0f; break;
        case 'B': data.fingerCurl[1] = value / 4095.0f; break;
        case 'C': data.fingerCurl[2] = value / 4095.0f; break;
        case 'D': data.fingerCurl[3] = value / 4095.0f; break;
        case 'E': data.fingerCurl[4] = value / 4095.0f; break;
        case 'F': data.joystickX = value / 4095.0f * 2.0f - 1.0f; break;
        case 'G': data.joystickY = value / 4095.0f * 2.0f - 1.0f; break;
        case 'H': data.joystickButton = value > 0 || valueStart == pos; break;
        case 'I': data.buttonTrigger = value > 0 || valueStart == pos; break;
        case 'J': data.buttonA = value > 0 || valueStart == pos; break;
        case 'K': data.buttonB = value > 0 || valueStart == pos; break;
        case 'L': data.buttonGrab = value > 0 || valueStart == pos; break;
        case 'N': data.buttonSystem = value > 0 || valueStart == pos; break;
        case 'O': data.buttonCalibrate = value > 0 || valueStart == pos; break;
        case 'P': data.triggerAnalog = value / 4095.0f; break;
        default: break;
        }
    }
}


void drawUI(const OpenGlovesData &data)
{
    int y = 10;
    int lineHeight = 20;

    DrawText("OpenGloves Echo Hand Simulator", 10, y, 20, RAYWHITE);
    y += 30;

    DrawText("Controls:", 10, y, 16, GRAY);
    y += lineHeight;
    DrawText("  WASD: Move Camera", 10, y, 14, LIGHTGRAY);
    y += lineHeight;
    DrawText("  Right Mouse + Drag: Look Around", 10, y, 14, LIGHTGRAY);
    y += lineHeight;
    DrawText("  Mouse Wheel: Adjust FOV", 10, y, 14, LIGHTGRAY);
    y += lineHeight;
    DrawText("  Shift: Move Faster", 10, y, 14, LIGHTGRAY);
    y += lineHeight;
    DrawText("  Space/Ctrl: Move Up/Down", 10, y, 14, LIGHTGRAY);
    y += lineHeight;
    DrawText("  P: Toggle Protocol Diagram", 10, y, 14, LIGHTGRAY);
    y += 30;

    DrawText("Finger Curls:", 10, y, 16, YELLOW);
    y += lineHeight;
    
    const char* names[] = {"Thumb", "Index", "Middle", "Ring", "Pinky"};
    Color barColors[] = {ORANGE, GREEN, BLUE, PURPLE, PINK};
    
    for (int i = 0; i < 5; i++)
    {
        int pct = (int)(data.fingerCurl[i] * 100);
        char buf[32];
        snprintf(buf, sizeof(buf), "  %s: %d%%", names[i], pct);
        DrawText(buf, 10, y, 12, WHITE);
        
        DrawRectangle(100, y + 2, 80, 10, DARKGRAY);
        DrawRectangle(100, y + 2, (int)(data.fingerCurl[i] * 80), 10, barColors[i]);
        y += 14;
    }
    y += 10;

    DrawText("Joystick:", 10, y, 16, YELLOW);
    y += lineHeight;
    char buf[64];
    snprintf(buf, sizeof(buf), "  X: %.2f  Y: %.2f", data.joystickX, data.joystickY);
    DrawText(buf, 10, y, 12, WHITE);
    y += lineHeight;
    snprintf(buf, sizeof(buf), "  Button: %s", data.joystickButton ? "PRESSED" : "Released");
    DrawText(buf, 10, y, 12, data.joystickButton ? GREEN : GRAY);
    y += 25;

    DrawText("Buttons:", 10, y, 16, YELLOW);
    y += lineHeight;
    snprintf(buf, sizeof(buf), "  Trigger: %s (%.0f%%)", 
             data.buttonTrigger ? "PRESSED" : "Released", data.triggerAnalog * 100);
    DrawText(buf, 10, y, 12, data.buttonTrigger ? GREEN : GRAY);
    y += lineHeight;
    // Button A
    snprintf(buf, sizeof(buf), "  A: %s", data.buttonA ? "PRESSED" : "Released");
    DrawText(buf, 10, y, 12, data.buttonA ? GREEN : GRAY);

    // Button B
    snprintf(buf, sizeof(buf), "  B: %s", data.buttonB ? "PRESSED" : "Released");
    DrawText(buf, 130, y, 12, data.buttonB ? GREEN : GRAY);
    y += lineHeight;
    snprintf(buf, sizeof(buf), "  Grab: %s", 
             data.buttonGrab ? "PRESSED" : "Released");
    DrawText(buf, 10, y, 12, WHITE);
}

// Protocol diagram overlay
void drawProtocolDiagram()
{
    int x = 350;
    int y = 40;
    int lineHeight = 18;

    DrawRectangle(x - 10, y - 10, 900, 640, Fade(BLACK, 0.92f));
    DrawRectangleLines(x - 10, y - 10, 900, 640, YELLOW);

    DrawText("OpenGloves Protocol Reference", x + 20, y, 20, YELLOW);
    y += 35;

    DrawText("FINGER CURLS (Scalar 0-4095):", x, y, 14, LIME);
    y += lineHeight;
    DrawText("  A = Thumb curl    B = Index curl    C = Middle curl", x, y, 12, WHITE);
    y += lineHeight;
    DrawText("  D = Ring curl     E = Pinky curl", x, y, 12, WHITE);
    y += 25;

    DrawText("INDIVIDUAL JOINTS (Scalar 0-4095):", x, y, 14, LIME);
    y += lineHeight;
    DrawText("  (AAA)-(AAC) = Thumb joints 0-2", x, y, 12, WHITE);
    y += lineHeight;
    DrawText("  (BAA)-(BAD) = Index joints 0-3", x, y, 12, WHITE);
    y += lineHeight;
    DrawText("  (CAA)-(CAD) = Middle joints 0-3", x, y, 12, WHITE);
    y += lineHeight;
    DrawText("  (DAA)-(DAD) = Ring joints 0-3", x, y, 12, WHITE);
    y += lineHeight;
    DrawText("  (EAA)-(EAD) = Pinky joints 0-3", x, y, 12, WHITE);
    y += 25;

    DrawText("SPLAY (Scalar):", x, y, 14, LIME);
    y += lineHeight;
    DrawText("  (AB) = Thumb    (BB) = Index    (CB) = Middle", x, y, 12, WHITE);
    y += lineHeight;
    DrawText("  (DB) = Ring     (EB) = Pinky", x, y, 12, WHITE);
    y += 25;

    DrawText("JOYSTICK:", x, y, 14, LIME);
    y += lineHeight;
    DrawText("  F = X axis (scalar)    G = Y axis (scalar)", x, y, 12, WHITE);
    y += lineHeight;
    DrawText("  H = Button (boolean - no value needed)", x, y, 12, WHITE);
    y += 25;

    DrawText("BUTTONS (Boolean - no value needed):", x, y, 14, LIME);
    y += lineHeight;
    DrawText("  I = Trigger    J = A Button    K = B Button", x, y, 12, WHITE);
    y += lineHeight;
    DrawText("  L = Grab       N = System Menu", x, y, 12, WHITE);
    y += lineHeight;
    DrawText("  O = Calibrate", x, y, 12, WHITE);
    y += 25;

    DrawText("TRIGGER VALUE:", x, y, 14, LIME);
    y += lineHeight;
    DrawText("  P = Trigger analog value (scalar 0-4095)", x, y, 12, WHITE);
    y += 30;

    DrawText("EXAMPLE PACKET:", x, y, 14, YELLOW);
    y += lineHeight;
    DrawText("  A2048B3000C1500D800E400F2048G2048HIJ\\n", x, y, 12, SKYBLUE);
    y += 20;
    DrawText("  (Thumb half curl, Index mostly curl, Middle some curl,", x, y, 11, GRAY);
    y += lineHeight;
    DrawText("   Ring little curl, Pinky less curl, Joystick centered,", x, y, 11, GRAY);
    y += lineHeight;
    DrawText("   Joystick pressed, Trigger pressed, A pressed)", x, y, 11, GRAY);
    y += 25;

    DrawText("Press 'P' to close", x + 320, y, 14, YELLOW);
}


void initGloveRendering()
{
    std::cout << "Initializing VR glove rendering...\n";
    
    g_renderTarget = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
    initProceduralHand(g_hand);
    
    g_cameraController.position = {0.0f, 0.3f, 1.2f};
    g_cameraController.yaw = PI;
    g_cameraController.pitch = -0.2f;
    g_cameraController.camera.position = g_cameraController.position;
    g_cameraController.camera.target = {0.0f, 0.0f, -0.2f};
    g_cameraController.camera.fovy = 50.0f;
    
    std::cout << "VR glove rendering ready!\n";
}

void cleanupGloveRendering()
{
    UnloadRenderTexture(g_renderTarget);
}

void updateGloveObject(const char *payload)
{
    parseOpenGlovesPayload(payload, g_gloveData);
    updateHandPose(g_hand, g_gloveData);
}

void updateGloveObject(const std::string &payload)
{
    updateGloveObject(payload.c_str());
}

void initializeWindow()
{
    if (g_windowInitialized) return;
    
    InitWindow(1280, 720, "OpenGloves VR Hand Simulator");
    SetTargetFPS(60);
    
    initGloveRendering();
    g_windowInitialized = true;
}

bool userAttemptedClose()
{
    return WindowShouldClose();
}

void drawGloveObject()
{
    float dt = GetFrameTime();
    updateCamera(g_cameraController, dt);
    
    // Toggle protocol diagram with D key
    if (IsKeyPressed(KEY_P))
    {
        g_showProtocolDiagram = !g_showProtocolDiagram;
    }

    BeginTextureMode(g_renderTarget);
    ClearBackground({20, 25, 35, 255});

    BeginMode3D(g_cameraController.camera);
    
    DrawGrid(10, 0.2f);
    drawProceduralHand(g_hand);

    EndMode3D();
    EndTextureMode();

    BeginDrawing();
    DrawTextureRec(g_renderTarget.texture,
                   {0, 0, (float)g_renderTarget.texture.width, -(float)g_renderTarget.texture.height},
                   {0, 0}, WHITE);
    
    drawUI(g_gloveData);
    
    if (g_showProtocolDiagram)
    {
        drawProtocolDiagram();
    }
    
    DrawFPS(GetScreenWidth() - 90, 10);
    EndDrawing();
}