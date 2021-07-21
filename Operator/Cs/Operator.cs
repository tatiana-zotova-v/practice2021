using System;
using System.Collections.Generic;
using uPLibrary.Networking.M2Mqtt;
using uPLibrary.Networking.M2Mqtt.Messages;
using SFML.Graphics;
using SFML.System;
using SFML.Window;
using Emgu.CV;
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
        public static string imageTopic { get; private set; } = "/image";
        public static string autoTopic { get; private set; } = "/command/auto";
        public static string manualTopic { get; private set; } = "/command/manual";
        public static string modeTopic { get; private set; } = "/command/mode";
        public static string stateTopic { get; private set; } = "/state";
        public static byte[] recivedImageBytes { get; private set; }

        public static bool isNewImageBytes { get; set; }

        public void InitializeClient()
        {
            Console.WriteLine("\tEnter brokerHostName in format X.X.X.X or empty for localhost");
            string enterLine = Console.ReadLine();
            if (!(enterLine.Length == 0)) { brokerHostName = enterLine; }
            client = new MqttClient(brokerHostName);
            try
            {
                client.Connect(Guid.NewGuid().ToString());
            }
            catch (uPLibrary.Networking.M2Mqtt.Exceptions.MqttConnectionException e)
            {
                Console.WriteLine(e.Message + " : check server state");
                Console.WriteLine();
                Environment.Exit(0);
            }
            Console.WriteLine("\t# topic");
            client.Subscribe(new string[] { "#" }, new byte[] { MqttMsgBase.QOS_LEVEL_EXACTLY_ONCE });
            client.MqttMsgPublishReceived += ReciveMessage;
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
            if (e.Topic == imageTopic)
            {
                recivedImageBytes = e.Message;
                isNewImageBytes = true;
            }

            if (e.Topic == stateTopic)
            {
                string state = "";
                foreach (var item in e.Message)
                {
                    state += Convert.ToChar(item);
                }
                RecoObject.command = (Commands)int.Parse(state);
            }
        }
    }

    class MainWindow
    {
        public static MQTTClient mqttClient = new();

        public static ImageShow image = new();

        public static Puck puck = new Puck(new Hsv(75, 100, 95), new Hsv(95, 220, 255), "puck");
        public static Gate gate = new Gate(new Hsv(3, 175, 115), new Hsv(6, 255, 230), "gate");

        public RenderWindow window;

        public Image icon = null;

        public VideoMode mode;
        private string windowTitle { get; set; } = "Operator v5.0";

        private static SFML.Graphics.Color backgroundColor { get; set; } = new SFML.Graphics.Color(47, 48, 50);

        private List<Button> buttons = new List<Button>();

        private void AddButtons()
        {
            buttons.Add(new Button(new Vector2f(305, 495), Commands.STR_M_F, "F"));
            buttons.Add(new Button(new Vector2f(305, 585), Commands.STR_M_B, "B"));
            buttons.Add(new Button(new Vector2f(305, 540), Commands.PAUSE, "S"));
            buttons.Add(new Button(new Vector2f(260, 540), Commands.ROT_L, "L"));
            buttons.Add(new Button(new Vector2f(350, 540), Commands.ROT_R, "R"));
            buttons.Add(new Button(new Vector2f(260, 495), Commands.AUTO_MODE, "A"));
            buttons.Add(new Button(new Vector2f(350, 495), Commands.MANUAL_MODE, "M"));
        }

        private void AddButtonsToDraw()
        {
            if (buttons.Count != 0)
            {
                foreach (var button in buttons)
                {
                    window.Draw(button.buttonBody);
                    window.Draw(button.buttonText);
                }
            }
        }

        public void InitializeWindow(uint x, uint y)
        {
            mode = new VideoMode(x, y);
            window = new RenderWindow(mode, windowTitle, Styles.Close);
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
                    if (RecoObject.command == Commands.END_OF_STATE_SEQUENCE)
                    {
                        List<(string, string)> commands;
                        if (!puck.catched)
                        {
                            puck.CreateImage(MQTTClient.recivedImageBytes);
                            puck.Recognize();
                            commands = puck.FollowTo();
                            puck.CleanAllLists();
                        }
                        else
                        {
                            gate.CreateImage(MQTTClient.recivedImageBytes);
                            gate.Recognize();

                            if (gate.catched)
                            {
                                commands = gate.Stop();
                            }
                            else
                            {
                                commands = gate.FollowTo();
                            }
                            gate.CleanAllLists();
                        }
                        mqttClient.PublishCommand(commands);
                        commands.Clear();
                    }
                    image.UpdateTexture(MQTTClient.recivedImageBytes);
                    RecoObject.command = Commands.IN_SEQUENCE;
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

    public enum Commands
    {
        PAUSE = 0,
        AUTO_MODE = 1,
        MANUAL_MODE = 2,
        STR_M_F = 5,
        STR_M_B = 6,
        ROT_R = 7,
        ROT_L = 8,
        END_OF_STATE_SEQUENCE = 9,
        IN_SEQUENCE = 10,
    }

    public enum FindingStates
    {
        FINDING_PUCK = 0,
        FINDING_GATE = 1
    }

    class Button
    {
        public RectangleShape buttonBody { get; private set; }
        public Text buttonText { get; private set; }
        private static Font font { get; set; }
        private static uint caracterSize { get; set; } = 30;
        private static SFML.Graphics.Color textColor { get; set; } = new SFML.Graphics.Color(229, 228, 226, 192);
        private Vector2f buttonPosition { get; set; }
        private static float caracterSizeRatio { get; set; } = 2.8f;
        private static Vector2f buttonSize { get; set; } = new Vector2f(40, 40);
        private static SFML.Graphics.Color buttonColor { get; set; } = new SFML.Graphics.Color(34, 168, 109, 192);
        public Commands command { get; private set; }
        public float value { get; set; }

        static Button()
        {
            font = new Font("AC_Thermes_Solid.ttf");
        }

        public Button(Vector2f position, Commands commandType, string text)
        {
            command = commandType;
            buttonPosition = position;
            buttonBody = new RectangleShape(buttonSize);
            buttonBody.FillColor = buttonColor;
            buttonBody.Position = buttonPosition;
            buttonBody.Size = buttonSize;
            buttonText = new Text(text, font, caracterSize);
            var textPositionOffset = new Vector2f(buttonPosition.X + buttonSize.X / 2 - caracterSize / caracterSizeRatio, buttonPosition.Y + 2);
            buttonText.Position = textPositionOffset;
            buttonText.FillColor = textColor;
        }

        public void Action(bool isPressed, MQTTClient client)
        {
            if (isPressed)
            {
                List<(string, string)> commands = new List<(string, string)>();
                if (command == Commands.AUTO_MODE || command == Commands.MANUAL_MODE)
                {
                    commands.Add((((int)command).ToString(), MQTTClient.modeTopic));
                    client.PublishCommand(commands);
                }
                else
                {
                    commands.Add((((int)command).ToString(), MQTTClient.manualTopic));
                    client.PublishCommand(commands);
                }
                commands.Clear();
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
            image = new Image(640, 480, new SFML.Graphics.Color(34, 168, 109, 192));
            texture = new Texture(image);
            sprite = new Sprite(texture);
            sprite.Position = new Vector2f(5, 5);
        }

        public void UpdateTexture(byte[] recived)
        {
            if (recived != null)
            {
                image = new Image(recived);
                texture.Update(image);
                sprite.Texture = texture;
            }
            MQTTClient.isNewImageBytes = false;
        }
    }

    public abstract class RecoObject
    {
        public Image<Hsv, byte> image { get; set; } = null;
        public Image<Gray, byte> rangedImage { get; set; } = null;
        public Hsv lower { get; set; } = new Hsv();
        public Hsv upper { get; set; } = new Hsv();
        public VectorOfVectorOfPoint contours { get; set; } = new VectorOfVectorOfPoint();
        public int[,] hierarchy { get; set; } = new int[3, 3];
        public List<Rectangle> rectangles { get; set; } = new List<Rectangle>();
        public string name { get; set; }
        private int noResult { get; set; } = 0;
        private int maxNoResult { get; set; } = 10;
        public bool catched { get; set; }
        public static Commands command { get; set; } = Commands.IN_SEQUENCE;

        public int time = 0;
        public RecoObject(Hsv low, Hsv up, string name)
        {
            lower = low;
            upper = up;
            this.name = name;
        }
        public abstract bool Catch();
        public void CreateImage(byte[] received)
        {
            Mat mat = new Mat();
            CvInvoke.Imdecode(received, Emgu.CV.CvEnum.ImreadModes.Unchanged, mat);
            CvInvoke.CvtColor(mat, mat, Emgu.CV.CvEnum.ColorConversion.Rgb2HsvFull);
            image = mat.ToImage<Hsv, byte>();
            //image.Save("D:\\convert\\" + (time++).ToString() + ".png");
        }

        public void Recognize()
        {
            rangedImage = image.InRange(lower, upper);
            hierarchy = CvInvoke.FindContourTree(rangedImage, contours, Emgu.CV.CvEnum.ChainApproxMethod.ChainApproxSimple);
            for (int i = 0; i < contours.Size; ++i)
            {
                if (0 <= hierarchy[i, 3])
                {
                    continue;
                }
                rectangles.Add(CvInvoke.BoundingRectangle(contours[i]));
            }
        }

        public List<(string, string)> FollowTo()
        {
            if (noResult > maxNoResult)
            {
                List<(string, string)> commands = new List<(string, string)>();
                commands.Add(("0;0.5", MQTTClient.autoTopic));
                commands.Add(("2", MQTTClient.modeTopic));
                Console.WriteLine("Объект не найден, выполнен переход в ручной режим");
                return commands;
            }
            else
            {
                if (rectangles.Count == 1)
                {
                    if (Catch())
                    {
                        List<(string, string)> commands = new List<(string, string)>();
                        catched = true;
                        commands.Add(("0;0.5", MQTTClient.autoTopic));
                        commands.Add(("3;0.35", MQTTClient.autoTopic));
                        Console.WriteLine("Захватил " + name);
                        return commands;
                    }
                    else
                    {
                        List<(string, string)> commands = new List<(string, string)>();
                        catched = false;
                        double xPos = Convert.ToDouble((rectangles[0].X + rectangles[0].Width / 2)) / Convert.ToDouble(image.Width);
                        double angleToTurn = Convert.ToDouble(Math.Atan((xPos - 0.5d) * 0.44d / 0.22d) * 180d / Math.PI);
                        commands.Add(("4;" + (Math.Round(angleToTurn)).ToString(), MQTTClient.autoTopic));
                        commands.Add(("0;0.5", MQTTClient.autoTopic));
                        commands.Add(("3;1", MQTTClient.autoTopic));
                        Console.WriteLine("Распознан " + name + ", поворот на " + (Math.Round(angleToTurn)).ToString());
                        return commands;
                    }
                }
                else
                {
                    List<(string, string)> commands = new List<(string, string)>();
                    commands.Add(("0;0.5", MQTTClient.autoTopic));
                    commands.Add(("4;35", MQTTClient.autoTopic));
                    noResult++;
                    Console.WriteLine("Не распознал " + name + ",продолжаю поиск");
                    return commands;
                }
            }
        }

        public void CleanAllLists()
        {
            contours.Clear();
            rectangles.Clear();
        }
    }

    public class Puck : RecoObject
    {
        public Puck(Hsv low, Hsv up, string name) : base(low, up, name) { }
        
        public override bool Catch()
        {
            if ((double)rectangles[0].Y / image.Height > 0.75 && ((rectangles[0].X + rectangles[0].Width / 2) - image.Width / 2) < 0.1) { return true; }
            else { return false; }
        }
    }

    public class Gate : RecoObject
    {
        public Gate(Hsv low, Hsv up, string name) : base(low, up, name) { }
        public override bool Catch()
        {
            if (((double)rectangles[0].Height / image.Height) > 0.55) { return true; }
            else { return false; }
        }

        public List<(string, string)> Stop()
        {
            List<(string, string)> commands = new List<(string, string)>();
            commands.Add(("3;-2", MQTTClient.autoTopic));
            commands.Add(("2", MQTTClient.modeTopic));
            return commands;
        }
    }
}