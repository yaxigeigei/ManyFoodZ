using System;
using System.Collections;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.IO;
using System.IO.Ports;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Drawing.Imaging;
using Microsoft.Research.DynamicDataDisplay;
using Microsoft.Research.DynamicDataDisplay.DataSources;


namespace ManyFoodZ
{

    public partial class MainWindow : Window
    {
        LogDay logDayDisplay;
        LogSession logSessionDisplay;

        private void logDateComboBox_SelectionChanged_1(object sender, SelectionChangedEventArgs e)
        {
            if (logDateComboBox.SelectedIndex != -1)
            {
                foreach (LogDay ld in currentOUS.logLibrary)
                {
                    if ((DateTime)logDateComboBox.SelectedValue == ld.dateTime)
                        logDayDisplay = ld;
                }
            }
            logSessionComboBox.ItemsSource = logDayDisplay.sessionList;
        }

        private void logSessionComboBox_SelectionChanged_1(object sender, SelectionChangedEventArgs e)
        {
            if (logSessionComboBox.SelectedIndex != -1)
            {
                foreach (LogSession ls in logDayDisplay.sessionList)
                {
                    if ((int)logSessionComboBox.SelectedValue == ls.sessionIndex)
                        logSessionDisplay = ls;
                }
            }
            logNameTextbox.DataContext = logSessionDisplay.sessionName;
            RefreshCatDisplay();
        }

        private void logCatComboBox_SelectionChanged_1(object sender, SelectionChangedEventArgs e)
        {
            RefreshCatDisplay();
        }

        private void RefreshCatDisplay()
        {
            try
            {
                switch (logCatComboBox.SelectedIndex)
                {
                    case 0: logInfoTextBox.Text = logSessionDisplay.Summary(); break;
                    case 1: logInfoTextBox.Text = logSessionDisplay.Details(); break;
                    case 2: logInfoTextBox.Text = logSessionDisplay.behaviorParas.Specs(); break;
                    case 3: logInfoTextBox.Text = logSessionDisplay.hardwareParas.Specs(); break;
                    default: break;
                }
            }
            catch (Exception)
            {
                
            }
        }


    }
}
