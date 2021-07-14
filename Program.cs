using System;
using System.Collections.Generic;
using uPLibrary.Networking.M2Mqtt;
using uPLibrary.Networking.M2Mqtt.Messages;
using SFML.Graphics;
using SFML.System;
using SFML.Window;
using Emgu.CV;
using Emgu.Util;
using Emgu.CV.Util;
using Emgu.CV.Structure;
using System.Drawing;

namespace window_core
{
    class Program
    {
        public static MainWindow window = new();
        static void Main(string[] args)
        {
            window.InitializeWindow(650, 630);
            Console.WriteLine("Press ESC key to close window");
            window.RunMainCycle();
            Console.WriteLine("Successful exit");
        }
    }

    class MQTTClient
    {
        public MqttClient client { get; private set; }
        private string brokerHostName { get; set; } = "localhost";
        public static string autoTopic { get; private set; } = "/command/auto";
        public static string manualTopic { get; private set; } = "/command/manual";
        public static string modeTopic { get; private set; } = "/command/mode";
        public static byte[] recivedImageBytes { get; private set; }

        public static bool isNewImageBytes { get; set; }

        public void InitializeClient()
        {
            //Console.WriteLine("\tEnter brokerHostName in format X.X.X.X or empty for localhost");
            //string enterLine = Console.ReadLine();
            //if (!(enterLine.Length == 0)) { brokerHostName = enterLine; }
            client = new MqttClient(brokerHostName);
            try
            {
                client.Connect(Guid.NewGuid().ToString());
            }
            catch (uPLibrary.Networking.M2Mqtt.Exceptions.MqttConnectionException e)
            {
                Console.WriteLine(e.Message);
                Console.WriteLine("Check server state");
                Environment.Exit(0);
            }
            Console.WriteLine("\t# topic");
            client.Subscribe(new string[] { "#" }, new byte[] { MqttMsgBase.QOS_LEVEL_EXACTLY_ONCE });
            client.MqttMsgPublishReceived += ReciveMessage;
        }

        public void PublishCommand(string command, string topic)
        {
            byte[] commandToPublish = new byte[command.Length];
            for (int i = 0; i < command.Length; i++)
            {
                commandToPublish[i] = Convert.ToByte(command[i]);
            }
            client.Publish(topic, commandToPublish, MqttMsgBase.QOS_LEVEL_EXACTLY_ONCE, false);
        }

        public void PublishCommand(List<(string, string)> commandSequence)
        {
            foreach (var command in commandSequence)
            {
                byte[] commandToPublish = new byte[command.Item1.Length];
                for (int i = 0; i < command.Item1.Length; i++)
                {
                    commandToPublish[i] = Convert.ToByte(command.Item1[i]);
                }
                client.Publish(command.Item2, commandToPublish, MqttMsgBase.QOS_LEVEL_EXACTLY_ONCE, false);
            }
        }

        private void ReciveMessage(object sender, MqttMsgPublishEventArgs e)
        {
            if (e.Topic == "/image")
            {
                recivedImageBytes = e.Message;
                isNewImageBytes = true;
            }
        }
    }

    class MainWindow
    {
        public static MQTTClient mqttClient = new();

        public static ImageShow image = new();

        public static ImageRecognition reco = new();

        public RenderWindow window;

        public VideoMode mode;
        private string windowTitle { get; set; } = "Operator v5.0";

        private static SFML.Graphics.Color backgroundColor { get; set; } = new SFML.Graphics.Color(47, 48, 50);

        private List<Button> buttons = new List<Button>();

        private void AddButtons()
        {
            buttons.Add(new Button(new Vector2f(305, 495), CommandsEnum.STR_M_F));
            buttons.Add(new Button(new Vector2f(305, 585), CommandsEnum.STR_M_B));
            buttons.Add(new Button(new Vector2f(305, 540), CommandsEnum.PAUSE));
            buttons.Add(new Button(new Vector2f(260, 540), CommandsEnum.ROT_L));
            buttons.Add(new Button(new Vector2f(350, 540), CommandsEnum.ROT_R));
            buttons.Add(new Button(new Vector2f(260, 495), CommandsEnum.AUTO_MODE));
            buttons.Add(new Button(new Vector2f(350, 495), CommandsEnum.MANUAL_MODE));
        }

        private void AddButtonsToDraw()
        {
            if (buttons.Count != 0)
            {
                foreach (var button in buttons)
                {
                    window.Draw(button.buttonBody);
                }
            }
        }

        public void InitializeWindow(uint x, uint y)
        {
            mode = new VideoMode(x, y);
            window = new RenderWindow(mode, windowTitle);
            window.KeyPressed += WindowKeyPressed;
            window.MouseButtonPressed += WindowMouseButtonPressed;
            window.Closed += WindowClosed;
            AddButtons();
            mqttClient.InitializeClient();
        }

        private void WindowClosed(object sender, EventArgs e)
        {
            var window = (Window)sender;
            window.Close();
        }

        public void RunMainCycle()
        {
            while (window.IsOpen)
            {
                if (MQTTClient.isNewImageBytes)
                {
                    image.UpdateTexture(MQTTClient.recivedImageBytes);
                    reco.CreateImg();
                    reco.AngleTurn(reco.RefreshHSV());
                }
                window.Clear(backgroundColor);
                window.Draw(image.sprite);
                AddButtonsToDraw();

                window.DispatchEvents();
                window.Display();
            }
        }

        private void WindowMouseButtonPressed(object sender, MouseButtonEventArgs e)
        {
            foreach (var button in buttons)
            {
                button.Action(button.IsPressed(e.X, e.Y), mqttClient);
                if (button.IsPressed(e.X, e.Y))
                {
                    Console.WriteLine("\tButton " + button.command.ToString());
                }
            }
        }

        private void WindowKeyPressed(object sender, KeyEventArgs e)
        {
            var window = (Window)sender;
            if (e.Code == Keyboard.Key.Escape)
            {
                window.Close();
            }
        }
    }

    enum CommandsEnum
    {
        PAUSE = 0,
        //CHECK_ROTATION = 1,
        //CHECK_MOVING = 2,
        AUTO_MODE = 1,
        MANUAL_MODE = 2,
        STR_M_F = 5,
        STR_M_B = 6,
        ROT_R = 7,
        ROT_L = 8,

    }

    class Button
    {
        public RectangleShape buttonBody;

        private Vector2f buttonPosition { get; set; }
        private static Vector2f buttonSize { get; set; } = new Vector2f(40, 40);
        private static SFML.Graphics.Color buttonColor { get; set; } = new SFML.Graphics.Color(34, 168, 109, 192);
        //public static Color buttonBlinkColor { get; private set; } = new Color(12, 135, 80, 64);
        public CommandsEnum command { get; private set; }
        public float value { get; set; }

        public Button(Vector2f position, CommandsEnum commandType)
        {
            command = commandType;
            buttonPosition = position;
            buttonBody = new RectangleShape(buttonSize);
            buttonBody.FillColor = buttonColor;
            buttonBody.Position = buttonPosition;
            buttonBody.Size = buttonSize;
        }

        public void Action(bool isPressed, MQTTClient client)
        {
            if (isPressed)
            {
                if (command == CommandsEnum.AUTO_MODE || command == CommandsEnum.MANUAL_MODE)
                {
                    client.PublishCommand(((int)command).ToString(), MQTTClient.modeTopic);
                }
                else
                {
                    client.PublishCommand(((int)command).ToString(), MQTTClient.manualTopic);
                }
            }
        }

        public bool IsPressed(int x, int y)
        {
            return ((x > buttonPosition.X && x < buttonPosition.X + buttonSize.X) &&
                    (y > buttonPosition.Y && y < buttonPosition.Y + buttonSize.Y)) ? true : false;
        }
    }

    class ImageShow
    {
        public Image image { get; private set; }
        public Texture texture { get; private set; }
        public Sprite sprite { get; private set; }

        public ImageShow()
        {
            image = new SFML.Graphics.Image("C:\\Users\\Kactus\\source\\repos\\operator5\\bin\\Debug\\net5.0\\defaultImage.jpg");
            texture = new Texture(image);
            sprite = new Sprite(texture);
            sprite.Position = new Vector2f(5, 5);
        }

        public void UpdateTexture(byte[] recived)
        {
            if (recived != null)
            {
                image = new SFML.Graphics.Image(recived);
                texture.Update(image);
                sprite.Texture = texture;
            }
            MQTTClient.isNewImageBytes = false;
        }
    }

    class ImageRecognition
    {
        public Mat img { get; private set; } = new Mat();
        public int noResultFound { get; private set; } = 0;

        public static int time = 0;

        public void CreateImg()
        {
            CvInvoke.Imdecode(MQTTClient.recivedImageBytes, Emgu.CV.CvEnum.ImreadModes.Unchanged, img);
            img.Save("D:\\convert\\in" + time++.ToString() + ".jpg");
        }

        public Mat RefreshHSV()
        {
            Mat newMat = new Mat();
            CvInvoke.CvtColor(img, newMat, Emgu.CV.CvEnum.ColorConversion.YCrCb2Rgb);
            
            Mat result = new Mat();
            newMat.Save("D:\\convert\\hsv" + time++.ToString() + ".jpg");
            CvInvoke.InRange(newMat, new ScalarArray(new MCvScalar(200, 65, 30)), new ScalarArray(new MCvScalar(250, 100, 100)), result);
            CvInvoke.Erode(result, result, null, new Point(-1, -1), 1, Emgu.CV.CvEnum.BorderType.Constant, CvInvoke.MorphologyDefaultBorderValue);
            CvInvoke.Dilate(result, result, null, new Point(-1, -1), 1, Emgu.CV.CvEnum.BorderType.Constant, CvInvoke.MorphologyDefaultBorderValue);
                       
            return result;
        }

        public /*List<(string, string)>*/ void AngleTurn(Mat findIn)
        {
            VectorOfVectorOfPoint contours = new VectorOfVectorOfPoint();
            int[,] hierarchy = CvInvoke.FindContourTree(findIn, contours, Emgu.CV.CvEnum.ChainApproxMethod.ChainApproxSimple);
            List<Rectangle> rectangles = new List<Rectangle>();

            for (int i = 0; i < contours.Size; ++i)
            {
                if (0 <= hierarchy[i, 3])
                {
                    continue;
                }
                rectangles.Add(CvInvoke.BoundingRectangle(contours[i]));
            }

            if (noResultFound > 5)
            {
                List<(string, string)> defaultCommands = new List<(string, string)>();
                defaultCommands.Add(("0;1", MQTTClient.autoTopic));
                defaultCommands.Add(("2", MQTTClient.modeTopic));
                //return defaultCommands;
                Console.WriteLine("не нашел, перешел в мануал");
            }
            else
            {
                if (rectangles.Count <= 0 || rectangles.Count >= 2)
                {
                    List<(string, string)> commands = new List<(string, string)>();
                    commands.Add(("4;60", MQTTClient.autoTopic));
                    commands.Add(("0;1", MQTTClient.autoTopic));
                    noResultFound++;
                    //return commands;
                    Console.WriteLine("<= 0 команда поворот вправо на 60 и пауза");
                }
                else
                {
                    List<(string, string)> commands = new List<(string, string)>();
                    double xUVpos = Convert.ToDouble((rectangles[0].X + rectangles[0].Width / 2)) / 640d;
                    double angleToTurn = Convert.ToSingle(Math.Atan((xUVpos - 0.5d) * 0.6d / 0.3d) * 180d / Math.PI);
                    commands.Add(("4;" + Math.Round(angleToTurn, 2).ToString(), MQTTClient.autoTopic));
                    commands.Add(("0;1", MQTTClient.autoTopic));
                    //return commands;
                    Console.WriteLine("поворот на " + Math.Round(angleToTurn, 2).ToString());
                }
            }
        }
    }
}