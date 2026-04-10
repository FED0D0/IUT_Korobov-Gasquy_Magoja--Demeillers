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
using KeyboardHook_NS;

namespace WpfInterfaceRobot
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>

    public partial class MainWindow : Window
    {
        private GlobalKeyboardHook _globalKeyboardHook;
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
            serialPort1 = new ExtendedSerialPort("COM3", 115200, Parity.None, 8, StopBits.One);
            serialPort1.DataReceived += SerialPort1_DataReceived;
            serialPort1.Open();
            _globalKeyboardHook = new GlobalKeyboardHook();
            _globalKeyboardHook.KeyPressed += _globalKeyboardHook_KeyPressed;
        }
        public void SerialPort1_DataReceived(object sender, DataReceivedArgs e)
        {
            //receivedText += Encoding.UTF8.GetString(e.Data, 0, e.Data.Length);
            for (int i = 0; i < e.Data.Length; i++)
            {
                robot.byteListReceived.Enqueue(e.Data[i]);


            }


        }
        public void TimerAffichage_Tick(object? sender, EventArgs e)
        {


            while (robot.byteListReceived.Count() > 0)
            {
                var b = robot.byteListReceived.Dequeue();
                DecodeMessage(b);
            }

            //if (receivedText != "")
            //    textBoxReception.Text += receivedText;
            //receivedText = "";

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


            frame[index++] = (byte)(msgFunction >> 8);
            frame[index++] = (byte)(msgFunction >> 0);


            frame[index++] = (byte)(msgPayloadLength >> 8);
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
            PayLoad,
            CheckSum
        }

        StateReception rcvState = StateReception.Waiting;

        int msgDecodedFunction = 0;
        int msgDecodedPayLoadLength = 0;
        byte[] msgDecodedPayLoad;
        int msgDecodedPayLoadIndex = 0;

        int calculatedChecksum = 0;
        byte receivedChecksum = 0;
        byte autoControlActivated = 1;
        private void DecodeMessage(byte c)
        {
            switch (rcvState)
            {
                case StateReception.Waiting:
                    if (c == 0xFE)
                    {
                        rcvState = StateReception.FunctionMSB;

                    }
                    break;

                case StateReception.FunctionMSB:
                    msgDecodedFunction = c << 8;
                    rcvState = StateReception.FunctionLSB;
                    break;
                case StateReception.FunctionLSB:
                    msgDecodedFunction += c;
                    rcvState = StateReception.PayloadLengthMSB;
                    break;
                case StateReception.PayloadLengthMSB:
                    msgDecodedPayLoadLength = c << 8;
                    rcvState = StateReception.PayloadLengthLSB;
                    break;
                case StateReception.PayloadLengthLSB:
                    msgDecodedPayLoadLength += c;
                    rcvState = StateReception.PayLoad;
                    msgDecodedPayLoad = new byte[msgDecodedPayLoadLength];
                    msgDecodedPayLoadIndex = 0;
                    break;
                case StateReception.PayLoad:
                    msgDecodedPayLoad[msgDecodedPayLoadIndex] = c;
                    msgDecodedPayLoadIndex++;
                    if (msgDecodedPayLoadIndex >= msgDecodedPayLoadLength)
                    {
                        rcvState = StateReception.CheckSum;
                    }
                    break;
                case StateReception.CheckSum:
                    receivedChecksum = c;
                    calculatedChecksum = CalculateChecksum(msgDecodedFunction, msgDecodedPayLoadLength, msgDecodedPayLoad);

                    if (calculatedChecksum == receivedChecksum)
                    {
                        ProcessDecodedMessage(msgDecodedFunction, msgDecodedPayLoadLength, msgDecodedPayLoad);
                        //Success, on a un message valide
                    }
                    else
                    {
                    }
                    rcvState = StateReception.Waiting;
                    break;

                default:
                    {
                        rcvState = StateReception.Waiting;
                        break;
                    }
            }
        }
        private void _globalKeyboardHook_KeyPressed(object? sender, KeyArgs e)
        {
            switch (e.keyCode)
            {
                case KeyCode.LEFT:
                    UartEncodeAndSendMessage(0x0051, 1, new byte[] { (byte)StateRobot.STATE_TOURNE_SUR_PLACE_GAUCHE });
                    break;
                case KeyCode.RIGHT:
                    UartEncodeAndSendMessage(0x0051, 1, new byte[] { (byte)StateRobot.STATE_TOURNE_SUR_PLACE_DROITE });
                    break;
                case KeyCode.UP:
                    UartEncodeAndSendMessage(0x0051, 1, new byte[] { (byte)StateRobot.STATE_AVANCE });
                    break;
                case KeyCode.DOWN:
                    UartEncodeAndSendMessage(0x0051, 1, new byte[] { (byte)StateRobot.STATE_ATTENTE });
                    break;
                case KeyCode.PAGEDOWN:
                    UartEncodeAndSendMessage(0x0051, 1, new byte[] { (byte)StateRobot.STATE_RECULE });
                    break;
            }
        }
        private void ProcessDecodedMessage(int msgFunction, int msgPayloadLength, byte[] msgPayload)
        {
            switch (msgFunction)
            {
                case (int)RobotFunction.Text:
                    textBoxReception.Text += Encoding.UTF8.GetString(msgPayload);
                    break;
                case (int)RobotFunction.IR:
                    IR_Gauche.Text = "IR Gauche : " + Convert.ToUInt16(msgPayload[0].ToString("X2"), 16);
                    IR_Centre.Text = "IR Centre : " + Convert.ToUInt16(msgPayload[1].ToString("X2"), 16);
                    IR_Droit.Text = "IR Droit : " + Convert.ToUInt16(msgPayload[2].ToString("X2"), 16);
                    break;
                case (int)RobotFunction.Motor:
                    sbyte vitG = (sbyte)msgPayload[0];
                    sbyte vitD = (sbyte)msgPayload[1];
                    Vitesse_Gauche.Text = "Vit Gauche : " + vitG;
                    Vitesse_Droit.Text = "Vit Droit : " + vitD;
                    break;
                case (int)RobotFunction.Depl:
                    int instant = (((int)msgPayload[1]) << 24) + (((int)msgPayload[2]) << 16)
                    + (((int)msgPayload[3]) << 8) + ((int)msgPayload[4]);
                    textBoxReception.Text += "\nRobot␣State : " +
                    ((StateRobot)(msgPayload[0])).ToString() +
                    " - " + instant.ToString() + " ms";
                    Depl.Dispatcher.Invoke(() =>
                    {
                        Depl.Text = "Déplacement en cours : " + ((StateRobot)msgPayload[0]).ToString() +
                                    "\nDurée : " + instant + " ms";
                    });
                    break;
                case (int)RobotFunction.PositionData:
                    {
                        if (msgPayloadLength >= 12)
                        {
                            float x = BitConverter.ToSingle(msgPayload, 4);
                            float y = BitConverter.ToSingle(msgPayload, 8);

                            robot.positionXOdo = x;
                            robot.positionYOdo = y;

                            PositionData.Text = $"Position : X={x}, Y={y}";
                        }
                        else
                        {
                            PositionData.Text = "Erreur : payload trop court pour POSITION_DATA";
                        }
                        break;

                    }
                case (int)RobotFunction.TEST_PID:
                    {
                        if (msgPayloadLength >= 104)
                        {
                            // CONSIGNE
                            float Con_x = BitConverter.ToSingle(msgPayload, 4);
                            float Con_t = BitConverter.ToSingle(msgPayload, 8);
                            asservSpeedDisplay.UpdatePolarSpeedConsigneValues(Con_x, Con_t);

                            // MESURE
                            float Mes_x = BitConverter.ToSingle(msgPayload, 12);
                            float Mes_t = BitConverter.ToSingle(msgPayload, 16);
                            asservSpeedDisplay.UpdatePolarOdometrySpeed(Mes_x, Mes_t);

                            // ERREUR
                            float Err_x = BitConverter.ToSingle(msgPayload, 20);
                            float Err_t = BitConverter.ToSingle(msgPayload, 24);
                            asservSpeedDisplay.UpdatePolarSpeedErrorValues(Err_x, Err_t);

                            // COMMANDE
                            float Com_x = BitConverter.ToSingle(msgPayload, 28);
                            float Com_t = BitConverter.ToSingle(msgPayload, 32);
                            asservSpeedDisplay.UpdatePolarSpeedCommandValues(Com_x, Com_t);

                            // KP
                            float Kp_x = BitConverter.ToSingle(msgPayload, 36);
                            float Kp_t = BitConverter.ToSingle(msgPayload, 40);

                            // KI
                            float Ki_x = BitConverter.ToSingle(msgPayload, 60);
                            float Ki_t = BitConverter.ToSingle(msgPayload, 64);

                            // KD
                            float Kd_x = BitConverter.ToSingle(msgPayload, 84);
                            float Kd_t = BitConverter.ToSingle(msgPayload, 88);
                            asservSpeedDisplay.UpdatePolarSpeedCorrectionGains(Kp_x, Kp_t, Ki_x, Ki_t, Kd_x, Kd_t);

                            // CORRECTION P
                            float Cor_p_x = BitConverter.ToSingle(msgPayload, 44);
                            float Cor_p_t = BitConverter.ToSingle(msgPayload, 48);

                            // CORRECTION I
                            float Cor_i_x = BitConverter.ToSingle(msgPayload, 68);
                            float Cor_i_t = BitConverter.ToSingle(msgPayload, 72);

                            // CORRECTION D
                            float Cor_d_x = BitConverter.ToSingle(msgPayload, 92);
                            float Cor_d_t = BitConverter.ToSingle(msgPayload, 96);

                            asservSpeedDisplay.UpdatePolarSpeedCorrectionValues(Cor_p_x, Cor_p_t, Cor_i_x, Cor_i_t, Cor_d_x, Cor_d_t);

                            // CORRECTION MAX P
                            float Cor_p_max_x = BitConverter.ToSingle(msgPayload, 52);
                            float Cor_p_max_t = BitConverter.ToSingle(msgPayload, 56);

                            // CORRECTION MAX P
                            float Cor_i_max_x = BitConverter.ToSingle(msgPayload, 76);
                            float Cor_i_max_t = BitConverter.ToSingle(msgPayload, 80);

                            // CORRECTION MAX P
                            float Cor_d_max_x = BitConverter.ToSingle(msgPayload, 100);
                            float Cor_d_max_t = BitConverter.ToSingle(msgPayload, 104);

                            asservSpeedDisplay.UpdatePolarSpeedCorrectionLimits(Cor_p_max_x, Cor_p_max_t, Cor_i_max_x, Cor_i_max_t, Cor_d_max_x, Cor_d_max_t);

                        }
                        else
                        {
                            PositionData.Text = "Erreur : payload trop court pour TEST_PID";
                        }
                        break;
                    }


                    //case (int)RobotFunction.Depl:
                    //    Depl.Text = "Consigne De déplacement : " + Convert.ToUInt16(msgPayload[0].ToString("X2"), 16);
                    //    Depl.Text = "Consigne De déplacement : " + Convert.ToUInt16(msgPayload[1].ToString("X2"), 16);
                    //    Depl.Text = "Consigne De déplacement : " + Convert.ToUInt16(msgPayload[2].ToString("X2"), 16);
                    //    Depl.Text = "Consigne De déplacement : " + Convert.ToUInt16(msgPayload[3].ToString("X2"), 16);
                    //    Depl.Text = "Consigne De déplacement : " + Convert.ToUInt16(msgPayload[4].ToString("X2"), 16);
                    //    break;

            }
        }
        public enum StateRobot
        {
            STATE_ATTENTE = 0,
            STATE_ATTENTE_EN_COURS = 1,
            STATE_AVANCE = 2,
            STATE_AVANCE_EN_COURS = 3,
            STATE_TOURNE_GAUCHE = 4,
            STATE_TOURNE_GAUCHE_EN_COURS = 5,
            STATE_TOURNE_DROITE = 6,
            STATE_TOURNE_DROITE_EN_COURS = 7,
            STATE_TOURNE_SUR_PLACE_GAUCHE = 8,
            STATE_TOURNE_SUR_PLACE_GAUCHE_EN_COURS = 9,
            STATE_TOURNE_SUR_PLACE_DROITE = 10,
            STATE_TOURNE_SUR_PLACE_DROITE_EN_COURS = 11,
            STATE_ARRET = 12,
            STATE_ARRET_EN_COURS = 13,
            STATE_RECULE = 14,
            STATE_RECULE_EN_COURS = 15
        }

        enum RobotFunction
        {
            Text = 0x0080,
            LED = 0x0020,
            IR = 0x0030,
            Motor = 0x0040,
            Depl = 0x0050,
            PositionData = 0x0061,
            TEST_PID = 0x0062
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
            UartEncodeAndSendMessage(0x0052, 1, new byte[1] { 0 });
            //UartEncodeAndSendMessage(0x0080, 7, Encoding.UTF8.GetBytes("Bonjour"));
            // UartEncodeAndSendMessage(0x0020, 2, new byte[2] {0, 1});
            //UartEncodeAndSendMessage(0x0030, 3, new byte[3] {30,30,0});
            //UartEncodeAndSendMessage(0x0040, 2, new byte[2] {50,50});
            if (autoControlActivated == 1)
                autoControlActivated = 0;
            else
                autoControlActivated = 1;
            UartEncodeAndSendMessage(0x0052, autoControlActivated, new byte[1] { 1 });
        }

        private void CheckBox_Checked(object sender, RoutedEventArgs e)
        {

        }
    }

}    
    






