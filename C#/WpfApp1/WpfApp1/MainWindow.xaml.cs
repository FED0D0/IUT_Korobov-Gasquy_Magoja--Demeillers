using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using ExtendedSerialPort_NS;
using System.IO.Ports;
using System.Windows.Threading;
using WpfApp1;
using System.Reflection;
namespace WpfApp1
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        bool toogle;
        string receivedText;
        ExtendedSerialPort serialPort1;
        DispatcherTimer timerAffichage;
        Robot robot = new Robot();
        public MainWindow()
        {
            timerAffichage = new DispatcherTimer();
            timerAffichage.Interval = new TimeSpan(0, 0, 0, 0, 100);
            timerAffichage.Tick += TimerAffichage_Tick;
            timerAffichage.Start();
            InitializeComponent();
            serialPort1 = new ExtendedSerialPort("COM9", 115200, Parity.None, 8, StopBits.One);
            serialPort1.DataReceived += SerialPort1_DataReceived;
            serialPort1.Open();

        }
        public void SerialPort1_DataReceived(object sender, DataReceivedArgs e)
        {
            //receivedText += Encoding.UTF8.GetString(e.Data, 0, e.Data.Length);
            for(int i = 0; i< e.Data.Length; i++)
            {
                robot.byteListReceived.Enqueue(e.Data[i]);


            }
            
                
        }
        public void TimerAffichage_Tick(object? sender, EventArgs e)
        {
            for (int i = 0; i < robot.byteListReceived.Count() ; i++)
            {

                textBoxReception.Text += "0x" + robot.byteListReceived.Dequeue().ToString("X2") + " ";
            }

            if (receivedText != "")
                textBoxReception.Text += receivedText;
            receivedText = "";

        }

        public byte CalculateChecksum(int msgFunction, int msgPayloadLength, byte[] msgPayload)
        {
            byte checksum = 0;
            checksum ^= 0xFE;
            checksum ^= (byte)(msgFunction >> 8);
            checksum ^= (byte)(msgFunction >> 0);
            checksum ^= (byte)(msgPayloadLength >> 8);
            checksum ^= (byte)(msgPayloadLength >> 0);
            for (int i = 0; i < msgPayloadLength; i++)
            {
                checksum ^= msgPayload[i];
            }

            return checksum;
        }
        void UartEncodeAndSendMessage(int msgFunction, int msgPayloadLength, byte[] msgPayload)
        {
            
            int totalLength = 1 + 2 + 2 + msgPayloadLength + 1;
            byte[] frame = new byte[totalLength];

            int index = 0;
            frame[index++] = 0xFE;

            
            frame[index++] = (byte)(msgFunction>>8);
            frame[index++] = (byte)(msgFunction >> 0);


            frame[index++] = (byte)(msgPayloadLength >>8);
            frame[index++] = (byte)(msgPayloadLength >> 0);

            for (int i = 0; i < msgPayloadLength; i++)
                frame[index++] = msgPayload[i];

      
            frame[index++] = CalculateChecksum(msgFunction, msgPayloadLength, msgPayload);

            
            serialPort1.Write(frame, 0, frame.Length);
        }
        public enum StateReception
        {
            Waiting,
            FunctionMSB,
            FunctionLSB,
            PayloadLengthMSB,
            PayloadLengthLSB,
            Payload,
            CheckSum
        }

        StateReception rcvState = StateReception.Waiting;
        int msgDecodedFunction = 0;
        int msgDecodedPayloadLength = 0;
        byte[] msgDecodedPayload;
        int msgDecodedPayloadIndex = 0;

        int calculatedChecksum = 0;
        byte receivedChecksum = 0;

        private void DecodeMessage(byte c)
        {
            switch (rcvState)
            {
                case StateReception.Waiting:
                    calculatedChecksum = 0;
                    rcvState = StateReception.FunctionMSB;
                    break;

                case StateReception.FunctionMSB:
                    msgDecodedFunction = c << 8;
                    calculatedChecksum += c;
                    rcvState = StateReception.FunctionLSB;
                    break;

                case StateReception.FunctionLSB:
                    msgDecodedFunction |= c;
                    calculatedChecksum += c;
                    rcvState = StateReception.PayloadLengthMSB;
                    break;

                case StateReception.PayloadLengthMSB:
                    msgDecodedPayloadLength = c << 8;
                    calculatedChecksum += c;
                    rcvState = StateReception.PayloadLengthLSB;
                    break;

                case StateReception.PayloadLengthLSB:
                    msgDecodedPayloadLength |= c;
                    calculatedChecksum += c;

                    if (msgDecodedPayloadLength < 0 || msgDecodedPayloadLength > 1024)
                    { rcvState = StateReception.Waiting; break; }

                    msgDecodedPayload = new byte[msgDecodedPayloadLength];
                    msgDecodedPayloadIndex = 0;

                    if (msgDecodedPayloadLength == 0) {
                        rcvState = StateReception.CheckSum;
                    }
                    else {
                        rcvState = StateReception.Payload;
                    }
                    break;

                case StateReception.Payload:
                    msgDecodedPayload[msgDecodedPayloadIndex++] = c;
                    calculatedChecksum += c;

                    if (msgDecodedPayloadIndex >= msgDecodedPayloadLength)
                        rcvState = StateReception.CheckSum;

                    break;

                case StateReception.CheckSum:
                    receivedChecksum = c;
                    calculatedChecksum &= 0xFF;

                    if (calculatedChecksum == receivedChecksum)
                    {
                        // Message validé
                        // (Tu mettras ton traitement ici)
                    }

                    rcvState = StateReception.Waiting;
                    break;

                default:
                    rcvState = StateReception.Waiting;
                    break;
            }
        }

        enum RobotFunction { Text = 0x0080, LED = 0x0020, IR = 0x0030, Motor = 0x0040 }

        // Variables internes
        string txt = ""; bool[] leds = new bool[8]; int[] ir = new int[3]; int motorL, motorR;

        void ProcessDecodedMessage(int f, int len, byte[] p)
        {
            switch ((RobotFunction)f)
            {
                case RobotFunction.Text: txt = System.Text.Encoding.UTF8.GetString(p, 0, len); break;
                case RobotFunction.LED: if (len >= 2) { int n = p[0]; if (n < leds.Length) leds[n] = p[1] != 0; } break;
                case RobotFunction.IR: if (len >= 3) for (int i = 0; i < 3; i++) ir[i] = p[i]; break;
                case RobotFunction.Motor: if (len >= 2) { motorL = p[0]; motorR = p[1]; } break;
            }
        }

        // Simulation loopback
        void TestLoopback()
        {
            byte[][] msgs ={
        new byte[]{0x00,0x80,0x00,0x05,(byte)'H',(byte)'e',(byte)'l',(byte)'l',(byte)'o',0xF3},
        new byte[]{0x00,0x20,0x00,0x02,0x01,0x01,0x23},
        new byte[]{0x00,0x30,0x00,0x03,0x10,0x20,0x30,0x63},
        new byte[]{0x00,0x40,0x00,0x02,0x50,0x60,0xF6}
         };
            foreach (var m in msgs) foreach (var b in m) DecodeMessage(b);
        }


        private void TextBox_TextChanged(object sender, TextChangedEventArgs e)
        {

        }

        private void buttonEnvoyer_Click(object sender, RoutedEventArgs e)
        {

            textBoxReception.Text += ("Reçu : " + textBoxEmission.Text + "\n");
            textBoxEmission.Text = "";

            if (toogle == false)
            {
                buttonEnvoyer.Background = Brushes.RoyalBlue;
                toogle = !toogle;
            }
            else
            {
                buttonEnvoyer.Background = Brushes.Beige;
                toogle = !toogle;

            }
        }
        private void buttonClear_Click(object sender, RoutedEventArgs e)
        {
            buttonClear.Background = Brushes.RoyalBlue;
            textBoxReception.Clear();
        }
        private void textBoxEmission_KeyUp(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.Enter)
            {

                //TextBoxréception.Text += ("Reçu : " + textBoxEmission.Text);
                serialPort1.WriteLine(textBoxEmission.Text);
                //receivedText =textBoxEmission.Text ;

                textBoxEmission.Text = "";
            }
        }

        private void Button_Click(object sender, RoutedEventArgs e)
        {
            //À l’oscilloscope

            //Le signal présente une trame série classique:
            //start bit → 8 bits data → stop bit.
            //Les transitions logiques correspondent bien aux valeurs envoyées.
            //Le signal électrique est propre et cohérent.
            /*byte[] byteList = new byte[20];

            for (int i = 0; i < byteList.Length; i++)
                byteList[i] = (byte)(2 * i);

            if (serialPort1.IsOpen)
                serialPort1.Write(byteList, 0, byteList.Length);*/

            var payload = Encoding.UTF8.GetBytes("Bonjour");
            UartEncodeAndSendMessage(0x0080, payload.Length, payload);
 

        } 

    }
}    
    






