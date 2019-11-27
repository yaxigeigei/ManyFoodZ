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
using System.Windows.Media;
using System.Windows.Threading;

namespace ManyFoodZ
{
    public partial class MainWindow : Window
    {

        #region COMMUNICATION

        // Respond to port name selection
        private void comComboBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (comComboBox.SelectedIndex != -1)
            {
                if (!serialPort.IsOpen)
                {
                    serialPort.PortName = (string)comComboBox.SelectedItem;
                }
            }
        }

        // Refresh COM ports everytime user opens the list
        private void comComboBox_DropDownOpened(object sender, EventArgs e)
        {
            comComboBox.Items.Clear();
            string[] portsAvailable = SerialPort.GetPortNames();
            foreach (string port in portsAvailable)
            {
                comComboBox.Items.Add(port);
            }
        }



        // Manual Connect
        private void serialConnectButton_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                serialPort.Open();

                serialReadThread = new Thread(new ThreadStart(SerialRead));
                serialReadThread.IsBackground = true;
                reading = true;
                serialReadThread.Start();

                SerialSend("MFZ");
                Thread.Sleep(50);
                if (thisDevice)
                {
                    serialConnectButton.IsEnabled = false;
                    serialDisconButton.IsEnabled = true;

                    // Startup configuration
                    oneTimeCfgThread = new Thread(new ThreadStart(OneTimeConfiguration));
                    oneTimeCfgThread.IsBackground = true;
                    oneTimeCfgThread.Start();
                }
                else
                {
                    reading = false;
                    serialReadThread.Join();
                    serialPort.Close();
                    MessageBox.Show("Incorrect Device", "Error", MessageBoxButton.OK, MessageBoxImage.Error);
                }
            }
            catch (Exception)
            {
                MessageBox.Show("Connection Failed", "Error", MessageBoxButton.OK, MessageBoxImage.Error);
            }
        }

        // Manual Disconnect
        private void serialDisconButton_Click(object sender, RoutedEventArgs e)
        {
            reading = false;

            try
            {
                serialReadThread.Join();
                serialPort.Close();
                serialConnectButton.IsEnabled = true;
                serialDisconButton.IsEnabled = false;
            }
            catch (Exception) { }
        }

        // Basic Read and Write
        static double incomingVal = 0;
        private static void SerialRead()
        {
            while (reading)
            {
                try
                {
                    string incomingStr = serialPort.ReadLine();
                    if (!String.IsNullOrEmpty(incomingStr))
                    {
                        // Universal Input Processing
                        incomingStr = incomingStr.Substring(0, incomingStr.Length - 1);

                        double numCandidate;
                        bool isNumber = double.TryParse(incomingStr, out numCandidate);

                        bool recordTiming = false;

                        if (isNumber)
                            incomingVal = numCandidate;
                        else
                        {
                            switch (incomingStr)
                            {
                                case "YES":
                                    thisDevice = true; break;
                                case "executed":
                                    executed = true; break;
                                case "calibrated":
                                    calibrated = true; break;
                                default:
                                    recordTiming = true; break;
                            }
                        }

                        MainWindow.MWInstance.serialMonitorListBox.Dispatcher.BeginInvoke((ThreadStart)delegate
                        {
                            ListBoxItem lbi = new ListBoxItem();
                            lbi.Content = incomingStr;
                            lbi.Foreground = Brushes.SeaGreen;
                            MainWindow.MWInstance.serialMonitorListBox.Items.Add(lbi);
                            MainWindow.MWInstance.serialMonitorListBox.ScrollIntoView(lbi);
                        });


                        // Timing Related Input Processing
                        if (recordTiming)
                        {
                            string title = "";
                            SolidColorBrush foreColor = Brushes.Black;

                            switch (incomingStr)
                            {
                                case "fIn":
                                    title = "Food In @ ";
                                    foreColor = Brushes.Goldenrod; break;
                                case "fOut":
                                    title = "Food Out @ ";
                                    foreColor = Brushes.Peru; break;
                                case "drop":
                                    title = "Food Drop @ ";
                                    foreColor = Brushes.SaddleBrown; break;
                                case "grab":
                                    title = "Grab @ ";
                                    foreColor = Brushes.SeaGreen; break;
                                case "ImgTriggered":
                                    title = "Imaging @ ";
                                    foreColor = Brushes.Crimson; break;
                                case "Buzz":
                                    title = "Buzz @ ";
                                    foreColor = Brushes.LightSkyBlue; break;
                                case "dOpen":
                                    title = "Door Open @ ";
                                    foreColor = Brushes.LightSteelBlue; break;
                                case "dClose":
                                    title = "Door Close @ ";
                                    foreColor = Brushes.LightSlateGray; break;
                                default:
                                    recordTiming = false; break;
                            }
                            if (recordTiming)
                            {
                                string timeStamp = DateTime.Now.Minute + ":" + DateTime.Now.Second + ":" + DateTime.Now.Millisecond / 10;
                                MainWindow.MWInstance.serialMonitorListBox.Dispatcher.BeginInvoke((ThreadStart)delegate
                                {
                                    ListBoxItem lbi = new ListBoxItem();
                                    lbi.Content = title + timeStamp + ", " + incomingVal;
                                    lbi.Foreground = foreColor;
                                    MainWindow.MWInstance.behaviorListBox.Items.Add(lbi);
                                    MainWindow.MWInstance.behaviorListBox.ScrollIntoView(lbi);
                                });
                            }
                        }
                    }
                }
                catch (TimeoutException)
                {
                    //serialPort.Close();
                    //Thread.CurrentThread.Abort();
                }
                Thread.Sleep(5);
            }
        }

        private bool SerialSend(string cmd)
        {
            bool success = false;
            try
            {
                serialPort.Write(cmd);
                success = true;

                ListBoxItem lbi = new ListBoxItem();
                lbi.Content = cmd;
                serialMonitorListBox.Items.Add(lbi);
                serialMonitorListBox.ScrollIntoView(lbi);
            }
            catch (InvalidOperationException)
            {
                MessageBox.Show("Connection Lost", "Error", MessageBoxButton.OK, MessageBoxImage.Error);
                serialConnectButton.IsEnabled = true;
                serialPort.Close();
            }
            return success;
        }

        private void serialSendButton_Click(object sender, RoutedEventArgs e)
        {
            SerialSend(serialSendTextBox.Text);
            serialSendTextBox.Clear();
        }
        private void serialSendTextBox_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.Enter)
            {
                SerialSend(serialSendTextBox.Text);
                serialSendTextBox.Clear();
            }
        }
        private void serialClearButton_Click(object sender, RoutedEventArgs e)
        {
            serialMonitorListBox.Items.Clear();
        }
        private void behavListClearButton_Click(object sender, RoutedEventArgs e)
        {
            behaviorListBox.Items.Clear();
        }

        #endregion





        private void imgButton_Click(object sender, RoutedEventArgs e)
        {
            SerialSend("i");
        }
        private void shutOffButton_Click(object sender, RoutedEventArgs e)
        {
            SerialSend("c");
        }
        private void shutOnButton_Click(object sender, RoutedEventArgs e)
        {
            SendCue();
        }
        private void SendCue()
        {
            if (allSets.basicParas.doorbell)
                SerialSend("bo");
            else
                SerialSend("o");
        }
        private void beepButton_Click(object sender, RoutedEventArgs e)
        {
            SerialSend("b");
        }







        #region HARDWARE SETTINGS

        private string hardwareCfgCmd(char device)
        {
            switch (device)
            {
                case 's':
                    return allSets.basicParas.servoH_begin + "SHB" + allSets.basicParas.servoH_end + "SHE" + allSets.basicParas.servoV_begin + "SVB"
                            + allSets.basicParas.servoV_end + "SVE" + allSets.basicParas.servoR_begin + "SRB" + allSets.basicParas.servoR_end + "SRE"
                            + allSets.basicParas.servoD_begin + "SDB" + allSets.basicParas.servoD_end + "SDE" + allSets.basicParas.servoAcc + "SRA";
                case 'm':
                    return allSets.basicParas.motorMaxSpeed + "MMS" + allSets.basicParas.motorAcc + "MLA" + allSets.basicParas.motorStepSize + "MSS"
                            + allSets.basicParas.pulseTime + "MPT";
                case 'd':
                    return allSets.basicParas.multiplierD + "PDM" + allSets.basicParas.dropCritItvl + "CID" + allSets.basicParas.pNumThresholdD + "PND"
                        + allSets.basicParas.counterCritItvl + "CIC" + allSets.basicParas.pNumThresholdC + "PNC" + "FTH";
                case 'f':
                    return allSets.basicParas.toneFrequency + "TFQ" + allSets.basicParas.toneDuration + "TDR";
                default: return "";
            }
        }

        private void OneTimeConfiguration()
        {
            ConfigureDevice(hardwareCfgCmd('d'));
            while (!calibrated)
            {
                Thread.Sleep(10);
            }
            calibrated = false;

            ConfigureDevice(hardwareCfgCmd('m'));
            Thread.Sleep(200);

            ConfigureDevice(hardwareCfgCmd('s'));
            Thread.Sleep(200);

            ConfigureDevice(hardwareCfgCmd('f'));
            Thread.Sleep(200);
        }

        private void applyServoButton_Click(object sender, RoutedEventArgs e)
        {
            ConfigureDevice(hardwareCfgCmd('s'));
        }

        private void applyMotorButton_Click(object sender, RoutedEventArgs e)
        {
            ConfigureDevice(hardwareCfgCmd('m'));
        }

        private void applyDetectorButton_Click(object sender, RoutedEventArgs e)
        {
            MessageBox.Show("Please Keep Quite and Still for 5 Seconds after Clicking OK.", "Freeze", MessageBoxButton.OK, MessageBoxImage.Information);
            ConfigureDevice(hardwareCfgCmd('d'));
        }
        private void applyFeedButton_Click(object sender, RoutedEventArgs e)
        {
            ConfigureDevice(hardwareCfgCmd('f'));
        }

        // Common Configuration method
        private void ConfigureDevice(string cmd)
        {
            MainWindow.MWInstance.Dispatcher.BeginInvoke((ThreadStart)delegate
            {
                SerialSend(cmd);            // Send parameters
            });
        }


        #endregion



    }
}
