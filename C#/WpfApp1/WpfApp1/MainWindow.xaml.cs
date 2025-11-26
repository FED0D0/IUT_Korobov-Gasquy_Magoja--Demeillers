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

namespace WpfApp1
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();
        }

        private void TextBox_TextChanged(object sender, TextChangedEventArgs e)
        {

        }
        private int compteurMessages = 1;
        private void EnvoyerMessage()
        {
            textBoxReception.Text += "\nreçu : Message" + compteurMessages;
            compteurMessages++;
            textBoxEmission.Text = "";
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
                e.Handled = true; 
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
