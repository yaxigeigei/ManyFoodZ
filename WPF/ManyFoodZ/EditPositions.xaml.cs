using System;
using System.Collections;
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
using System.Windows.Media.Imaging;
using System.Windows.Shapes;

namespace ManyFoodZ
{
    /// <summary>
    /// EditPositions.xaml 的交互逻辑
    /// </summary>
    public partial class EditPositions : Window
    {
        Hashtable preSets = MainWindow.MWInstance.currentOUS.preSetPosList;
        List<TextBox> posName = new List<TextBox>();
        List<TextBox> posX = new List<TextBox>();
        List<TextBox> posY = new List<TextBox>();
        List<TextBox> posZ = new List<TextBox>();
        
        public EditPositions()
        {
            InitializeComponent();
        }

        private void Window_Loaded_1(object sender, RoutedEventArgs e)
        {
            posName.Add(pos1NameTextBox);
            posName.Add(pos2NameTextBox);
            posName.Add(pos3NameTextBox);
            posName.Add(pos4NameTextBox);

            posX.Add(pos1xTextBox);
            posX.Add(pos2xTextBox);
            posX.Add(pos3xTextBox);
            posX.Add(pos4xTextBox);
            posX.Add(pos5xTextBox);
            posX.Add(posFxTextBox);

            posY.Add(pos1yTextBox);
            posY.Add(pos2yTextBox);
            posY.Add(pos3yTextBox);
            posY.Add(pos4yTextBox);
            posY.Add(pos5yTextBox);
            posY.Add(posFyTextBox);

            posZ.Add(pos1zTextBox);
            posZ.Add(pos2zTextBox);
            posZ.Add(pos3zTextBox);
            posZ.Add(pos4zTextBox);
            posZ.Add(pos5zTextBox);
            posZ.Add(posFzTextBox);

            for (int i = 0; i < 6; i++)
            {
                if (preSets.ContainsKey(i))
                {
                    PreSetPos psp = (PreSetPos)preSets[i];
                    posX[i].Text = Convert.ToString(psp.aPos.X);
                    posY[i].Text = Convert.ToString(psp.aPos.Y);
                    posZ[i].Text = Convert.ToString(psp.aPos.Z);

                    if (i < 4)
                        posName[i].Text = psp.posName;
                }
            }
        }



        private void CloseButton_Click(object sender, RoutedEventArgs e)
        {
            this.Close();
        }

        private void ConfirmButton_Click(object sender, RoutedEventArgs e)
        {
            Save();
            this.Close();
        }

        private void Save()
        {
            for (int i = 0; i < 6; i++)
            {
                string name = Convert.ToString(i);
                if (i < 4)
                    name = posName[i].Text;

                PreSetPos psp = new PreSetPos(name, 
                    Convert.ToDouble(posX[i].Text), Convert.ToDouble(posY[i].Text), Convert.ToDouble(posZ[i].Text));
                
                if (preSets.ContainsKey(i))
                    preSets[i] = psp;
                else
                    preSets.Add(i, psp);
            }
        }

        private void Window_MouseLeftButtonDown_1(object sender, MouseButtonEventArgs e)
        {
            try { this.DragMove(); }
            catch (Exception) { }                                       // Strange error occurs when not catching 
        }

        private void useCurrentAPButton1_Click(object sender, RoutedEventArgs e)
        {
            Button b = (Button)sender;
            int index = Convert.ToInt16(b.Tag);
            posX[index].Text = Convert.ToString(MainWindow.MWInstance.currentAP.X);
            posY[index].Text = Convert.ToString(MainWindow.MWInstance.currentAP.Y);
            posZ[index].Text = Convert.ToString(MainWindow.MWInstance.currentAP.Z);
        }
    }
}
