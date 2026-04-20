# Custom OpenGloves Force Feedback Unity Demo

This is a modified version of the [original OpenGloves Force Feedback Unity Demo by LucidVR](https://github.com/LucidVR/opengloves-force-feedback-unity-demo/tree/main). 

## What's New in this Version?

**Improved Grabbing Logic:** In the original branch, force feedback/grabbing actions would sometimes trigger simply by *highlighting* or hovering over an object. We've updated the script (`FFBClient.cs`) so that **grabbing only happens when you are actively grabbing the object**, eliminating accidental triggers from highlighting.

## Prerequisites

Before you start, make sure you have the following ready:
* A working VR headset.
* Your EchoHand hardware turned ON.
* **OpenGloves** installed via Steam. 

## How to Launch

To ensure the environment boots correctly and connects to your hardware, please follow these exact steps:

1. **Close SteamVR and OpenGloves:** Ensure that both SteamVR and OpenGloves are completely **OFF** before you begin. 
2. **Turn on Hardware:** Make sure your VR headset and your EchoHand are turned ON and ready.
3. **Open Unity:** Open this project in the Unity Editor with the version it was created in. 
4. **Press Play:** Hit the **Play** button at the top of the Unity Editor. This will automatically prompt SteamVR and OpenGloves to boot up in the background.

** Troubleshooting:** SteamVR can sometimes take a little too long to boot up. If the Unity environment fails to connect to your gloves or VR on the first try, simply stop the Unity player, wait for SteamVR to finish loading, and press Play again. If issues still persist, restart SteamVR.
