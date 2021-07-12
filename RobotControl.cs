using System.Collections;
using System.Collections.Generic;
using System.IO;
using uPLibrary.Networking.M2Mqtt;
using uPLibrary.Networking.M2Mqtt.Messages;
using UnityEngine;
using System;
using System.Globalization;

public class RobotControl : MonoBehaviour
{
    public GameObject gate;
    private Vector3 gatePos;

    public static Vector3 startPosition;
    public static Quaternion startRotation;
    public static Vector3 currentPosition;
    public static Quaternion currentRotation;

    public enum ControlType
    {
        NO = 0,
        AUTO,
        MANUAL
    }

    public static ControlType controlType { get; set; } = ControlType.NO;

    public enum State
    {
        PAUSE = 0,
        //auto
        CHECK_ROTATION = 1,
        CHECK_MOVING = 2,
        STR_M = 3,
        ROT = 4,
        //manual
        STR_M_F = 5,
        STR_M_B = 6,
        ROT_R = 7,
        ROT_L = 8,
    }

    public static State state { get; set; }

    public MQTTClient client;

    public AutoPilot auto;
    public Manual manual;

    public WheelController whellController;
    public RobotCamera robotCamera;

    void Awake()
    {
        client = new MQTTClient();
        client.CreateClient();

        auto = new AutoPilot();
        manual = new Manual();

        whellController = new WheelController();
        GetWheels();

        robotCamera = new RobotCamera();
        robotCamera.GetClient(client);
        GetCamera();

        gatePos = gate.transform.position;
        SetStartPosition();
    }

    void FixedUpdate()
    {
        UpdateCurrentPosition();
        if (controlType == ControlType.MANUAL)
        {
            manual.DoAction();
        }
        if (controlType == ControlType.AUTO)
        {
            auto.DoAction();
        }
    }

    private void LateUpdate()
    {
        robotCamera.GetFrame();
    }

    public class MQTTClient
    {
        private static string brokerAdress { get; set; } = "localhost";
        public static string imageTopic { get; private set; } = "/image";
        public static string autoTopic { get; private set; } = "/command/auto";
        public static string manualTopic { get; private set; } = "/command/manual";
        public static string modeTopic { get; private set; } = "/command/mode";

        private NumberFormatInfo formatInfo = new NumberFormatInfo() { NumberDecimalSeparator = "." };
        private char[] separators = new char[] { ';' };

        public MqttClient client { get; private set; }

        public void CreateClient()
        {
            client = new MqttClient(brokerAdress);

            client.MqttMsgPublishReceived += Receive;
            client.Connect(Guid.NewGuid().ToString());
            client.Subscribe(new string[] { "#" }, new byte[] { MqttMsgBase.QOS_LEVEL_EXACTLY_ONCE });

        }

        private void Receive(object sender, MqttMsgPublishEventArgs e)
        {
            if (e.Topic == autoTopic)
            {
                string Command = "";
                foreach (var item in e.Message)
                {
                    Command += Convert.ToChar(item);
                }
                string[] splitCommand = Command.Split(separators, StringSplitOptions.RemoveEmptyEntries);
                AutoPilot.AddCommandToStateSequence((int.Parse(splitCommand[0]), float.Parse(splitCommand[1], formatInfo)));
            }

            if (e.Topic == manualTopic)
            {
                string Command = "";
                foreach (var item in e.Message)
                {
                    Command += Convert.ToChar(item);
                }
                Manual.SetCommand(int.Parse(Command));
                Debug.Log("Manual Mode " + (int.Parse(Command)).ToString());
            }

            if (e.Topic == modeTopic)
            {
                string Command = "";
                foreach (var item in e.Message)
                {
                    Command += Convert.ToChar(item);
                }
                
                controlType = (ControlType)int.Parse(Command);
                Debug.Log("Command Mode " + controlType.ToString());
            }
        }

        public void PublishImage(byte[] message)
        {
            client.Publish(imageTopic, message, MqttMsgBase.QOS_LEVEL_EXACTLY_ONCE, false);
        }
    }

    public class RobotCamera : MonoBehaviour
    {
        public static Camera camera { get; set; }

        private static float captureTime { get; set; } = 0.2f;
        public static int textureWidth { get; private set; } = 640;
        private float time = 0;

        private MQTTClient cachedClient;


        public void GetClient(MQTTClient _client)
        {
            cachedClient = _client;
        }

        public void GetFrame()
        {
            if ((time += Time.deltaTime) > captureTime)
            {
                var RT = new RenderTexture(textureWidth, (textureWidth * 3) / 4, 24);
                RenderTexture.active = RT;
                var image = new Texture2D(RT.width, RT.height, TextureFormat.RGB24, false);
                RobotCamera.camera.targetTexture = RT;
                RobotCamera.camera.Render();
                image.ReadPixels(new Rect(0, 0, textureWidth, (textureWidth * 3) / 4), 0, 0);
                image.Apply();

                byte[] bytes = image.EncodeToJPG();
                //byte[] pixels = image.GetRawTextureData();
                //RenderTexture.active = null;
                //camera.targetDisplay = 1;
                //File.WriteAllBytes("D:/Capture/" + Time.time + ".bin", pixels);;
                //File.WriteAllBytes("D:/Capture/" + Time.time + ".jpg", bytes);

                cachedClient.PublishImage(bytes);
                time = 0;
            }
        }
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

    public class WheelController : MonoBehaviour
    {
        public static WheelCollider leftWheel;
        public static WheelCollider rightWheel;
        private static float maxMotorTorque { get; set; } = 1f;
        private static float maxBrakeTorque { get; set; } = 500f;
        private static float rotationFactor { get; set; } = 5f;
        
        public static void BrakesOn()
        {
            leftWheel.brakeTorque = maxBrakeTorque;
            rightWheel.brakeTorque = maxBrakeTorque;
        }

        public static void BrakesOff()
        {
            leftWheel.brakeTorque = 0f;
            rightWheel.brakeTorque = 0f;
        }

        public static void ForwardMotorTorque()
        {
            leftWheel.motorTorque = maxMotorTorque;
            rightWheel.motorTorque = maxMotorTorque;
        }

        public static void BackwardMotorTorque()
        {
            leftWheel.motorTorque = -maxMotorTorque;
            rightWheel.motorTorque = -maxMotorTorque;
        }

        public static void RightRotateTorque()
        {
            leftWheel.motorTorque = maxMotorTorque / rotationFactor;
            rightWheel.motorTorque = -maxMotorTorque / rotationFactor;
        }

        public static void LeftRotateTorque()
        {
            leftWheel.motorTorque = -maxMotorTorque / rotationFactor;
            rightWheel.motorTorque = maxMotorTorque / rotationFactor;
        }
    }

    public void GetCamera()
    {
        RobotCamera.camera = GetComponentInChildren<Camera>();
    }

    public void GetWheels()
    {
        WheelCollider[] wheels = GetComponentsInChildren<WheelCollider>();
        WheelController.leftWheel = wheels[0];
        WheelController.rightWheel = wheels[1];
    }

    public class AutoPilot
    {
        public static List<(State, float)> stateSequence = new List<(State, float)>(); //the sequence of states that the robot passes through
        public int listIterator { get; private set; }
        public float commandValue { get; private set; }
        public bool inAction { get; private set; }
        public static bool isValueUnderZero { get; private set; }

        private static float checkTime { get; set; } = 0;
        private static float rotationAnglePerSecond { get; set; } = 0;
        private static float distancePerSecond { get; set; } = 0;
        //public static float pauseTime = 2f;

        public delegate void Action(float value);
        public Action action;

        public AutoPilot()
        {
            listIterator = 0;
            AddCommandToStateSequence((0, 5f));
            AddCommandToStateSequence((1, 2f));
            AddCommandToStateSequence((2, 2f));
        }

        public static void AddCommandToStateSequence((int, float) _command)
        {
            isValueUnderZero = (_command.Item2 < 0) ? true : false;
            stateSequence.Add(((State)_command.Item1, _command.Item2));
            //Debug.Log("command " + (State)_command.Item1 + " added with value " + _command.Item2);
        }

        public void DoAction()
        {
            if (!inAction && listIterator < stateSequence.Count)
            {
                state = (stateSequence[listIterator].Item1);
                commandValue = stateSequence[listIterator].Item2;
                listIterator++;

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

                    case State.STR_M:
                        {
                            action = StraightMove;
                            break;
                        }
                    case State.ROT:
                        {
                            action = Rotation;
                            break;
                        }
                    default:
                        break;
                }
            }
            action(commandValue);
        }

        public void CheckRotation(float checkTimeValue)
        {
            if (state == State.CHECK_ROTATION)
            {
                inAction = true;
                if (checkTime > checkTimeValue)
                {
                    WheelController.BrakesOn();
                    rotationAnglePerSecond = Math.Abs(startRotation.eulerAngles.y - currentRotation.eulerAngles.y) / checkTime;
                    checkTime = 0;
                    inAction = false;
                    AddCommandToStateSequence((0, 1f));
                }
                else
                {
                    WheelController.BrakesOff();
                    WheelController.RightRotateTorque();
                    checkTime += Time.fixedDeltaTime;
                }
            }
        }

        public void CheckMoving(float checkTimeValue)
        {
            if (state == State.CHECK_MOVING)
            {
                inAction = true;
                if (checkTime > checkTimeValue)
                {
                    WheelController.BrakesOn();
                    distancePerSecond = (startPosition - currentPosition).magnitude / checkTime;
                    checkTime = 0;
                    inAction = false;
                    AddCommandToStateSequence((0, 1f));
                }
                else
                {
                    WheelController.BrakesOff();
                    WheelController.ForwardMotorTorque();
                    checkTime += Time.fixedDeltaTime;
                }
            }
        }

        private void Pause(float pauseTime)
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

        public void Rotation(float rotationAngleValue)
        {
            if (state == State.ROT)
            {
                inAction = true;
                if (checkTime > Math.Abs(rotationAngleValue) / rotationAnglePerSecond)
                {
                    WheelController.BrakesOn();
                    AddCommandToStateSequence((0, 1f));
                    checkTime = 0;
                    inAction = false;
                }
                else
                {
                    WheelController.BrakesOff();
                    if (isValueUnderZero)
                    {
                        WheelController.LeftRotateTorque();
                    }
                    else
                    {
                        WheelController.RightRotateTorque();
                    }
                    checkTime += Time.fixedDeltaTime;
                }
            }
        }

        public void StraightMove(float distanceValue)
        {
            if (state == State.STR_M)
            {
                inAction = true;
                if (checkTime > Math.Abs(distanceValue) / distancePerSecond)
                {
                    WheelController.BrakesOn();
                    AddCommandToStateSequence((0, 1f));
                    checkTime = 0;
                    inAction = false;
                }
                else
                {
                    WheelController.BrakesOff();
                    if (isValueUnderZero)
                    {
                        WheelController.BackwardMotorTorque();
                    }
                    else
                    {
                        WheelController.ForwardMotorTorque();
                    }
                    checkTime += Time.fixedDeltaTime;
                }
            }
        }
    }

    public class Manual
    {
        public delegate void Action();
        public Action action;
        private static State state { get; set; } = State.PAUSE;

        public void DoAction()
        {
            switch (state)
            {
                case State.PAUSE:
                    {
                        action = Pause;
                        break;
                    }
                case State.STR_M_F:
                    {
                        action = ForwardMove;
                        break;
                    }
                case State.STR_M_B:
                    {
                        action = BackwardMove;
                        break;
                    }
                case State.ROT_L:
                    {
                        action = RotateLeft;
                        break;
                    }
                case State.ROT_R:
                    {
                        action = RotateRight;
                        break;
                    }
                default:
                    break;
            }
            action();
        }

        public static void SetCommand(int stateNum)
        {
            state = (State)stateNum;
        }

        public void Pause()
        {
            WheelController.BrakesOn();
        }

        public void RotateLeft()
        {
            WheelController.BrakesOff();
            WheelController.LeftRotateTorque();
        }

        public void RotateRight()
        {
            WheelController.BrakesOff();
            WheelController.RightRotateTorque();
        }

        public void ForwardMove()
        {
            WheelController.BrakesOff();
            WheelController.ForwardMotorTorque();
        }

        public void BackwardMove()
        {
            WheelController.BrakesOff();
            WheelController.BackwardMotorTorque();
        }
    }
}

