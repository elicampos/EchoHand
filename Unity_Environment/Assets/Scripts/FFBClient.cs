using System.Diagnostics;
using UnityEngine;
using Valve.VR;
using Valve.VR.InteractionSystem;

public class FFBClient : MonoBehaviour
{
    private FFBManager _ffbManager;
    private void Awake()
    {
        _ffbManager = GameObject.FindObjectOfType<FFBManager>();
    }

    private void OnAttachedToHand(Hand hand)
    {
        UnityEngine.Debug.Log("Restricting servo movement as an object has been grabbed.");
        SteamVR_Skeleton_Pose_Hand skeletonPoseHand;
        if (hand.handType == SteamVR_Input_Sources.LeftHand)
        {
            skeletonPoseHand = GetComponent<Interactable>().skeletonPoser.skeletonMainPose.leftHand;
        }
        else
        {
            skeletonPoseHand = GetComponent<Interactable>().skeletonPoser.skeletonMainPose.rightHand;
        }
        
        _ffbManager.SetForceFeedbackFromSkeleton(hand, skeletonPoseHand);
    }

    private void OnDetachedFromHand(Hand hand)
    {
        UnityEngine.Debug.Log("Resetting servo positions as object is dropped.");
        _ffbManager.RelaxForceFeedback(hand);
    }
}