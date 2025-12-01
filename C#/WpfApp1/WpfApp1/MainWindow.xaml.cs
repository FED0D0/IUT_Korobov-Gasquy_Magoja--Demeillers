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
namespace WpfApp1
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        ExtendedSerialPort serialPort1;
        string receivedText = "";
        DispatcherTimer timerAffichage;
        public MainWindow()
        {
            InitializeComponent();
            serialPort1 = new ExtendedSerialPort("COM6", 115200, Parity.None, 8, StopBits.One);
            serialPort1.DataReceived += SerialPort1_DataReceived;
            serialPort1.Open();
            timerAffichage = new DispatcherTimer();
            timerAffichage.Interval = new TimeSpan(0, 0, 0, 0, 100);
            timerAffichage.Tick += TimerAffichage_Tick;
            timerAffichage.Start();

        }
        public void SerialPort1_DataReceived(object sender, DataReceivedArgs e)
        {
            receivedText += Encoding.UTF8.GetString(e.Data, 0, e.Data.Length);

        }
        private void TimerAffichage_Tick(object? sender, EventArgs e)
        {

            if (receivedText != "")
                textBoxReception.Text += receivedText;
            receivedText = "";
            
        }

        

        private void TextBox_TextChanged(object sender, TextChangedEventArgs e)
        {

        }
        private int compteurMessages = 1;
        private void EnvoyerMessage()
        {
            //textBoxReception.Text += "\nenvoyé : Message" + compteurMessages;
            compteurMessages++;
            textBoxEmission.Text = "";

            serialPort1.WriteLine(textBoxEmission.Text);
        }
        private void buttonEnvoyer_Click(object sender, RoutedEventArgs e)
        {
            
            if (buttonEnvoyer.Background != Brushes.Beige)
            {
                buttonEnvoyer.Background = Brushes.Beige;   
            }
            else
            {
                buttonEnvoyer.Background = Brushes.RoyalBlue;
            }
            EnvoyerMessage();


            string message = textBoxEmission.Text;
        }

        private void textBoxEmission_KeyUp(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.Enter)
            {
                EnvoyerMessage();
                //e.Handled = true; 
            }
        }
        /* string message = textBoxEmission.Text;   

if (!string.IsNullOrWhiteSpace(message))
{
    textBoxReception.Text += "\nReçu : " + message;  
    textBoxEmission.Text = ""; 
}*/

    }
    }
