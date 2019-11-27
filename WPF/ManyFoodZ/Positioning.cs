using System;
using System.Collections.Generic;
using System.IO.Ports;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Threading;

namespace ManyFoodZ
{
    public partial class MainWindow : Window
    {

        #region POSITIONING


        ArenaPosition tempAP;                                   // New AP value first stores here before validation
        public ArenaPosition currentAP;                         // Represent current valid physical position of small bowl
        Point arenaPt = new Point();                            // Tracing mouse position in arena (unit: pixel)
        static bool executed = false;
        static bool calibrated = false;



        // Common Go to Position Method
        private bool Go2ArenaPos(ArenaPosition ap)
        {
            bool success = false;
            if (!ap.outOfRange && SerialSend(ap.Arena2RawCmd()))
            {
                currentAP = new ArenaPosition(ap.X, ap.Y, ap.Z);
                posCtrlGrid.DataContext = currentAP;
                ShowPosInArena(currentAP);
                success = true;
            }
            return success;
        }

        // To Where User Selected in the Arena
        private void Border_MouseDown_1(object sender, MouseButtonEventArgs e)
        {
            if (e.ChangedButton == MouseButton.Left && e.ClickCount == 2 && !doingAutoFeed)
            {
                Point pt = Converter.ArenaUI2Arena(arenaPt);
                tempAP = new ArenaPosition(Math.Round(pt.X), Math.Round(pt.Y), currentAP.Z);
                if (Go2ArenaPos(tempAP))
                    RefreshCircles();
            }
        }

        // To Where User Determined in TextBoxes and Manipulator
        private void posGoButton_Click(object sender, RoutedEventArgs e)
        {
            Go2ArenaPos(ReadAP());
        }
        private void posZeroButton_Click(object sender, RoutedEventArgs e)
        {
            Go2ArenaPos(new ArenaPosition(allSets.basicParas.ArmLength, 0, 0));
        }
        private void posResetButton_Click(object sender, RoutedEventArgs e)
        {
            SerialSend("RSS");
            currentAP = new ArenaPosition(allSets.basicParas.ArmLength, 0, 0);
            posCtrlGrid.DataContext = currentAP;
            ShowPosInArena(currentAP);
        }
        private void posForButton_Click(object sender, RoutedEventArgs e)
        {
            tempAP = new ArenaPosition(currentAP.X + Convert.ToDouble(posStepSizeTextBox.Text), currentAP.Y, currentAP.Z);
            Go2ArenaPos(tempAP);
        }
        private void posBackButton_Click(object sender, RoutedEventArgs e)
        {
            tempAP = new ArenaPosition(currentAP.X - Convert.ToDouble(posStepSizeTextBox.Text), currentAP.Y, currentAP.Z);
            Go2ArenaPos(tempAP);
        }
        private void posLeftButton_Click(object sender, RoutedEventArgs e)
        {
            tempAP = new ArenaPosition(currentAP.X, currentAP.Y - Convert.ToDouble(posStepSizeTextBox.Text), currentAP.Z);
            Go2ArenaPos(tempAP);
        }
        private void posRightButton_Click(object sender, RoutedEventArgs e)
        {
            tempAP = new ArenaPosition(currentAP.X, currentAP.Y + Convert.ToDouble(posStepSizeTextBox.Text), currentAP.Z);
            Go2ArenaPos(tempAP);
        }
        private void posUpButton_Click(object sender, RoutedEventArgs e)
        {
            tempAP = new ArenaPosition(currentAP.X, currentAP.Y, currentAP.Z + Convert.ToDouble(posStepSizeTextBox.Text));
            Go2ArenaPos(tempAP);
        }
        private void posDownButton_Click(object sender, RoutedEventArgs e)
        {
            tempAP = new ArenaPosition(currentAP.X, currentAP.Y, currentAP.Z - Convert.ToDouble(posStepSizeTextBox.Text));
            Go2ArenaPos(tempAP);
        }

        // To Where User Determined in Pre-set Positions
        private void preSetPosButton_Click(object sender, RoutedEventArgs e)
        {
            Button b = (Button)sender;
            int index = Convert.ToInt32(b.Tag);
            if (currentOUS.preSetPosList.ContainsKey(index))
            {
                PreSetPos psp = (PreSetPos)currentOUS.preSetPosList[index];
                Go2ArenaPos(psp.aPos);
            }
        }
        private void preSetPosFoodButton_Click(object sender, RoutedEventArgs e)
        {
            if (currentOUS.preSetPosList.ContainsKey(5))
            {
                PreSetPos psp = (PreSetPos)currentOUS.preSetPosList[5];
                Go2ArenaPos(new ArenaPosition(psp.aPos.X, psp.aPos.Y, 0));
            }
        }

        // Read Coordinates in TextBoxes
        private ArenaPosition ReadAP()
        {
            return new ArenaPosition(Convert.ToDouble(xTextBox.Text), Convert.ToDouble(yTextBox.Text), Convert.ToDouble(zTextBox.Text));
        }

        // Trace Mouse and Show Corresponding Position in Arena
        private void Border_MouseMove_1(object sender, System.Windows.Input.MouseEventArgs e)
        {
            arenaPt = e.GetPosition(arenaBoarder);
            arenaPt.X -= 1;
            Point pt = Converter.ArenaUI2Arena(arenaPt);
            coordinateLabel.Content = "(" + (int)pt.X + ", " + (int)pt.Y + ", " + (int)currentAP.Z + ")";
        }

        // Show Position in Arena with Circle
        private void ShowPosInArena(ArenaPosition ap)
        {
            MainWindow.MWInstance.Dispatcher.BeginInvoke(
                (ThreadStart)delegate()
                {
                    RefreshCircles();
                });
        }





        private int CalculateLoaderSteps(double step)
        {
            return (int)Math.Round(step * allSets.basicParas.loaderStepperDiv / allSets.basicParas.wheelDivision);
        }

        private void loaderMoveButton_Click(object sender, RoutedEventArgs e)
        {
            SerialSend(Convert.ToDouble(loaderStepSizeTextBox.Text) + "f");
        }

        private void loaderNextButton_Click(object sender, RoutedEventArgs e)
        {
            if (allSets.basicParas.onArmLoader)
            {
                SerialSend(CalculateLoaderSteps(1) + "f");
            }
            else if (currentOUS.preSetPosList.ContainsKey(5))
            {
                PreSetPos psp = (PreSetPos)currentOUS.preSetPosList[5];
                ArenaPosition fPos = psp.aPos;
                ArenaPosition fpPos = new ArenaPosition(fPos.X, fPos.Y, 0);

                if (currentAP.X == fpPos.X && currentAP.Y == fpPos.Y)
                {
                    Go2ArenaPos(fPos);
                    SerialSend(CalculateLoaderSteps(1) + "f");
                    Go2ArenaPos(fpPos);
                }
                else
                {
                    MessageBox.Show("Please move the arm to food grabbing position first.", "Not in Position",
                        MessageBoxButton.OK, MessageBoxImage.Information);
                }
            }
        }


        #endregion


    }
}
