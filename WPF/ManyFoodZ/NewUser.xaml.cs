using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;

namespace ManyFoodZ
{
    /// <summary>
    /// NewUser.xaml 的交互逻辑
    /// </summary>
    public partial class NewUser : Window
    {

        public string name;

        public NewUser()
        {
            InitializeComponent();
        }

        private void Window_Loaded_1(object sender, RoutedEventArgs e)
        {
            DoubleAnimation da = new DoubleAnimation(0, 1, TimeSpan.FromMilliseconds(300));
            WindowBorder.BeginAnimation(OpacityProperty, da);
        }

        private void Window_MouseLeftButtonDown(object sender, MouseButtonEventArgs e)
        {
            try { this.DragMove(); }
            catch (Exception) { }                                       // Strange error occurs when not catching 
        }

        private void CancelButton_Click(object sender, RoutedEventArgs e)
        {
            this.Close();
        }

        private void OkButton_Click(object sender, RoutedEventArgs e)
        {
            Confirmation();
        }

        private void textBox1_KeyDown(object sender, KeyEventArgs e)
        {
            if (Keyboard.IsKeyDown(Key.Enter))
                Confirmation();
        }

        private void Confirmation()
        {
            string test = textBox1.Text;
            if (!String.IsNullOrEmpty(test))
            {
                name = test;
                this.Close();
            }
        }

    }
}
