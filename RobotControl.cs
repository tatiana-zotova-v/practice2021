using System.Collections;
using System.Collections.Generic;
using System.IO;
using uPLibrary.Networking.M2Mqtt;
using uPLibrary.Networking.M2Mqtt.Messages;
using UnityEngine;
using System;
using System.Net;

public class RobotControl : MonoBehaviour
{
    public int? command = null;

    public GameObject gate;
    private Vector3 gatePos;

    public Camera camera;

    float time = 0;

    private int textureWidth = 480;

    private const string brokerAdress = "localhost";
    private const string imageTopic = "/image";
    private const string posTopic = "/position";
    private const string commandTopic = "/comand";

    public MqttClient client;

    public static WheelCollider leftWheel;
    public static WheelCollider rightWheel;
    public static float maxMotorTorque = 1f;
    public static float maxBrakeTorque = 500f;
    public static float rotationFactor = 5f;

    public static Vector3 startPosition;
    public static Quaternion startRotation;
    public static Vector3 currentPosition;
    public static Quaternion currentRotation;

    public AutoPilot auto = new AutoPilot();
    public enum State
    {
        WAITING = 0,
        PAUSE,
        CHECK_ROTATION,
        CHECK_MOVING,
        FIND_PUCK,
    }

    public static State state;

    void Awake()
    {
        GetWheels();
        camera = GetComponentInChildren<Camera>();
        //CreateClient();
        gatePos = gate.transform.position;
        SetStartPosition();
    }

    void FixedUpdate()
    {
        UpdateCurrentPosition();
        auto.ChangeAction();
        auto.action();
    }

    private void LateUpdate()
    {
        //GetFrame();
    }

    public static void SetState(State _state)
    {
        state = _state;
    }

    private void SetStartPosition()
    {
        startPosition = transform.position;
        startRotation = transform.rotation;
    }

    private void UpdateCurrentPosition()
    {
        currentPosition = transform.position;
        currentRotation = transform.rotation;
    }

    private void GetWheels()
    {
        WheelCollider[] wheels = new WheelCollider[2];
        wheels = GetComponentsInChildren<WheelCollider>();
        leftWheel = wheels[0];
        rightWheel = wheels[1];
    }

    public void CreateClient()
    {
        client = new MqttClient(brokerAdress);

        client.MqttMsgPublishReceived += Client_MqttMsgPublishReceived;

        string clientId = Guid.NewGuid().ToString();
        client.Connect(clientId);
        client.Subscribe(new string[] { imageTopic }, new byte[] { MqttMsgBase.QOS_LEVEL_EXACTLY_ONCE });
        client.Subscribe(new string[] { commandTopic }, new byte[] { MqttMsgBase.QOS_LEVEL_EXACTLY_ONCE });
    }

    private void Client_MqttMsgPublishReceived(object sender, MqttMsgPublishEventArgs e)
    {
        if (e.Topic == commandTopic)
        {
            command = e.Message[0];
        }
    }

    public void GetFrame()
    {
        if ((time += Time.deltaTime) > 1)
        {
            var RT = new RenderTexture(textureWidth, (textureWidth * 3) / 4, 24);
            RenderTexture.active = RT;
            var image = new Texture2D(RT.width, RT.height, TextureFormat.RGB24, false);
            camera.targetTexture = RT;
            camera.Render();
            image.ReadPixels(new Rect(0, 0, textureWidth, (textureWidth * 3) / 4), 0, 0);
            image.Apply();

            byte[] bytes = image.EncodeToJPG();
            //byte[] pixels = image.GetRawTextureData();
            RenderTexture.active = null;
            //camera.targetDisplay = 1;
            //File.WriteAllBytes("D:/Capture/" + counter++ + ".bin", pixels);;
            //File.WriteAllBytes("D:/Capture/" + counter++ + ".jpg", bytes);

            PublishImage(client, bytes);
            time = 0;
        }
    }

    public static void PublishImage(MqttClient client, byte[] message)
    {
        client.Publish(imageTopic, message, MqttMsgBase.QOS_LEVEL_EXACTLY_ONCE, false);
    }

    public class AutoPilot
    {
        public List<State> stateSequence = new List<State>(); //очередь состояний, через воторые проходит робот
        private int listIterator = 0;
        private bool inAction = false;

        public static float checkTime = 0;
        public static float rotationAnglePerSecond = 0;
        public static float distancePerSecond = 0;
        public static float pauseTime = 2f;
        //public static bool atHomePosition;

        public delegate void Action();
        public Action action;

        public AutoPilot()
        {
            stateSequence.Add(State.CHECK_ROTATION);
            stateSequence.Add(State.PAUSE);
            stateSequence.Add(State.CHECK_MOVING);
            stateSequence.Add(State.PAUSE);
        }

        public void ChangeAction()
        {
            if (!inAction)
            {
                if (listIterator < stateSequence.Count)
                {
                    SetState(stateSequence[listIterator++]);
                    switch (state)
                    {
                        case State.PAUSE:
                            {
                                action = Pause;
                                break;
                            }

                        case State.CHECK_ROTATION:
                            {
                                action = CheckRotation;
                                break;
                            }

                        case State.CHECK_MOVING:
                            {
                                action = CheckMoving;
                                break;
                            }

                        case State.FIND_PUCK:
                            break;
                        default:
                            break;
                    }
                }
            }
        }

        public static void BrakesOff()
        {
            leftWheel.brakeTorque = 0f;
            rightWheel.brakeTorque = 0f;
        }

        public static void BrakesOn()
        {
            leftWheel.brakeTorque = maxBrakeTorque;
            rightWheel.brakeTorque = maxBrakeTorque;
        }

        public void CheckRotation()
        {
            if (state == State.CHECK_ROTATION)
            {
                inAction = true;
                if (checkTime > 1)
                {
                    BrakesOn();
                    rotationAnglePerSecond = Math.Abs(startRotation.eulerAngles.y - currentRotation.eulerAngles.y) / checkTime;
                    checkTime = 0;
                    inAction = false;
                }
                else
                {
                    BrakesOff();
                    leftWheel.motorTorque = maxMotorTorque / rotationFactor;
                    rightWheel.motorTorque = -maxMotorTorque / rotationFactor;
                    checkTime += Time.fixedDeltaTime;
                }
            }
        }

        public void CheckMoving()
        {
            if (state == State.CHECK_MOVING)
            {
                inAction = true;
                if (checkTime > 1)
                {
                    BrakesOn();
                    distancePerSecond = (startPosition - currentPosition).magnitude / checkTime;
                    checkTime = 0;
                    inAction = false;
                }
                else
                {
                    BrakesOff();
                    leftWheel.motorTorque = maxMotorTorque;
                    rightWheel.motorTorque = maxMotorTorque;
                    checkTime += Time.fixedDeltaTime;
                }
            }
        }

        public void Pause()
        {
            if (state == State.PAUSE)
            {
                inAction = true;
                if (checkTime > pauseTime)
                {
                    inAction = false;
                    checkTime = 0;
                }
                else
                {
                    checkTime += Time.fixedDeltaTime;
                }
            }
        }

        public static void Rotate(float angle) //angle тоже условная величина, будет требоваться корректировка
        {
            if (checkTime > angle / rotationAnglePerSecond)
            {
                BrakesOn();
                checkTime += Time.fixedDeltaTime;
                if (checkTime > 3)
                {
                    checkTime = 0;
                }
            }
            else
            {
                BrakesOff();
                leftWheel.motorTorque = maxMotorTorque / rotationFactor;
                rightWheel.motorTorque = -maxMotorTorque / rotationFactor;
                checkTime += Time.fixedDeltaTime;
            }
        } 
        
        public static void StraightMove(float distance) //distance -- условное расстояние, требует подстроечного коэффициента
        {
            if (checkTime > distance / distancePerSecond)
            {
                BrakesOn();
                checkTime += Time.fixedDeltaTime;
                if (checkTime > 3)
                {
                    checkTime = 0;
                }
            }
            else
            {
                BrakesOff();
                leftWheel.motorTorque = maxMotorTorque;
                rightWheel.motorTorque = maxMotorTorque;
                checkTime += Time.fixedDeltaTime;
            }
        }
    }
}
