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

        // Software
        public static MainWindow MWInstance;
        static bool reading = false;                                                // Indicate the running state of the software
        public double ArenaBoarderWidth { get { return arenaBoarder.Width; } }      // For other classes to access
        public double ArenaBoarderHeight { get { return arenaBoarder.Height; } }    // For other classes to access
        static bool thisDevice = false;

        // Threading
        Thread serialReadThread;                    // Thread for read serial input
        Thread oneTimeCfgThread;                    // Thread for configuration
        Thread autoFeedThread;                      // Thread for auto feed

        // User Settings
        SaveAndLoad saveAndLoad = new SaveAndLoad();                                // Machinary for save and load an object
        public AllSettings allSets = new AllSettings();                             // The object that stores everything
        public OneUserSettings currentOUS = new OneUserSettings();                  // A reference of "userHashtable[userName]" which stores all settings of the current user
        public LogDay logDay = new LogDay(DateTime.Today);





        // Initialization
        public MainWindow()
        {
            InitializeComponent();
            MWInstance = this;
        }


        static SerialPort serialPort = new SerialPort();
        List<Button> posButtons = new List<Button>();

        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            // Loading all settings
            LoadAllSet();

            // Serial port configuration
            serialPort.BaudRate = 115200;
            serialPort.ReadTimeout = 500;
            serialPort.WriteTimeout = 500;

            // Startup configuration
            oneTimeCfgThread = new Thread(new ThreadStart(OneTimeConfiguration));
            oneTimeCfgThread.IsBackground = true;

            string[] portsAvailable = SerialPort.GetPortNames();
            foreach (string port in portsAvailable)
            {
                comComboBox.Items.Add(port);
                comComboBox.SelectedItem = port;
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
                        oneTimeCfgThread.Start();
                    }
                    else
                    {
                        reading = false;
                        serialReadThread.Join();
                        serialPort.Close();
                    }
                }
                catch (Exception)
                {
                    comComboBox.SelectedIndex = -1;
                }
            }

            // Show Points on UI
            currentAP = new ArenaPosition(allSets.basicParas.ArmLength, 0, 0);                          // Initial arena position
            posCtrlGrid.DataContext = currentAP;
            tempAP = new ArenaPosition(allSets.basicParas.ArmLength, 0, 0);

            double a = allSets.basicParas.ArmLength;
            double l = allSets.basicParas.railLength;
            arenaBoarder.Width = ArenaBoarderHeight * a / (l + 2 * a);
            upperBorder.Height = ArenaBoarderHeight * 2 * a / (l + 2 * a);
            lowerBorder.Height = ArenaBoarderHeight * 2 * a / (l + 2 * a);
            arenaBoarder.CornerRadius = new CornerRadius(0, ArenaBoarderWidth, ArenaBoarderWidth, 0);
            upperBorder.CornerRadius = arenaBoarder.CornerRadius;
            lowerBorder.CornerRadius = arenaBoarder.CornerRadius;
            GetEllipse(currentAP, Brushes.Tomato, "ap");

            // List out pre-set buttons
            posButtons.Add(preSetPos0Button);
            posButtons.Add(preSetPos1Button);
            posButtons.Add(preSetPos2Button);
            posButtons.Add(preSetPos3Button);

        }

        // Load All Setttings from "Settings.bin"
        private void LoadAllSet()
        {
            if (File.Exists(saveAndLoad.settingsFilePath))
            {
                allSets = (AllSettings)saveAndLoad.LoadAll();       // Import AllSettings object containing a hashtable and others settings

                foreach (string s in allSets.userHashtable.Keys)
                    userNameComboBox.Items.Add(s);                  // Display all existing tasks in the hashtable onto the task Listbox
            }
            else                                                    // When the saved "settings.bin" file does not exist
            {
                allSets.userHashtable = new Hashtable();
            }

            deviceParaGrid.DataContext = allSets.basicParas;
        }

        // Add New User
        private void userInfoNewButton_Click(object sender, RoutedEventArgs e)
        {
            string name;
            do
            {
                NewUser nd = new NewUser();
                nd.Owner = this;
                nd.ShowDialog();                                                // Open the "NewUser" window
                name = nd.name;                                                 // Get the input name
                if (name == null)                                               // When user cancels the dialog
                    break;
                if (allSets.userHashtable.ContainsKey(name))                    // Check if the new name exists as a key in hashtable
                    System.Windows.MessageBox.Show("There has already been a \"" + name + "\".\nPlease use another one.");

            } while (allSets.userHashtable.ContainsKey(name));                  // Keep asking if input conflict with existing task names

            if (name != null)
            {
                allSets.userHashtable.Add(name, new OneUserSettings());         // Use this name as key to create a new OneUserSettings object in allSets.usersHashtable
                userNameComboBox.Items.Add(name);
                userNameComboBox.SelectedItem = name;                           // This will trigger "userNameComboBox_SelectionChanged(...)"
            }
        }

        // Change User
        private void userNameComboBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (userNameComboBox.SelectedIndex != -1)
            {
                SaveSettings();                                                                         // Save settings of current user
                currentOUS = (OneUserSettings)allSets.userHashtable[userNameComboBox.SelectedItem];     // Refer currentOUS to the new OneUserSettings object

                // Update data binding after the reference of currentOUS changed
                userGrid.DataContext = currentOUS;
                viewGrid.DataContext = currentOUS;
                autoFeedGrid.DataContext = currentOUS.behaviorParas;
                posLibDataGrid.DataContext = currentOUS.posLibrary;
                logDateComboBox.ItemsSource = currentOUS.logLibrary;

                // Initialize today's log
                bool todayExist = false;
                foreach (LogDay ld in currentOUS.logLibrary)
                {
                    if (ld.dateTime == DateTime.Today)
                    {
                        logDay = ld;
                        todayExist = true; break;
                    }
                }
                if (!todayExist)
                {
                    logDay = new LogDay(DateTime.Today);
                }

                // Display settings on UI
                DisplaySettings();
            }
            else
            {
                userInfoLabel.Content = "Who Are You?";
                autoPosStackPanel.IsEnabled = false;
                preSetStackPanel.IsEnabled = false;
                autoFeedGrid.IsEnabled = false;
                //statGrid.IsEnabled = false;
                logGrid.IsEnabled = false;
                add2LibButton.IsEnabled = false;
            }
        }

        private void DisplaySettings()
        {
            userInfoLabel.Content = "Hello!";
            autoPosStackPanel.IsEnabled = true;
            preSetStackPanel.IsEnabled = true;
            autoFeedGrid.IsEnabled = true;
            //statGrid.IsEnabled = true;
            logGrid.IsEnabled = true;
            add2LibButton.IsEnabled = true;

            // Refresh Pre-set Positions
            for (int i = 0; i < 4; i++)
            {
                if (currentOUS.preSetPosList.ContainsKey(i))
                {
                    PreSetPos psp = (PreSetPos)currentOUS.preSetPosList[i];
                    posButtons[i].Content = psp.posName;
                }
                else
                    posButtons[i].Content = "Position " + (i + 1);
            }

            // Refresh Position Library
            if (currentOUS.posLibrary == null)
                currentOUS.posLibrary = new ObservableCollection<LibPos>();

            RefreshCircles();
        }



        // Delete Current User
        private void userInfoDelButton_Click(object sender, RoutedEventArgs e)
        {
            currentOUS = new OneUserSettings();
            string temp = (string)userNameComboBox.SelectedItem;
            if (!String.IsNullOrEmpty(temp))
            {
                allSets.userHashtable.Remove(temp);
                userNameComboBox.Items.Remove(temp);
                userNameComboBox.SelectedIndex = -1;
            }
        }

        // Save Settings
        private void SaveSettings()
        {
            // Save changed log selectively
            if (currentOUS.logLibrary != null)
            {
                bool todayExist = false;
                foreach (LogDay ld in currentOUS.logLibrary)
                {
                    if (ld.dateTime == DateTime.Today)
                    {
                        todayExist = true; break;
                    }
                }
                if (!todayExist && logDay.sessionList.Count() > 0)
                {
                    currentOUS.logLibrary.Add(logDay);
                }
            }

            // Save the ultimate settings object
            saveAndLoad.SaveAll(allSets);
        }







        #region UI

        // Circles in Arena
        private void RefreshCircles()
        {
            arenaGrid.Children.Clear();

            // Show Library Positions
            if (currentOUS.showLibPos && currentOUS.posLibrary != null)
            {
                foreach (LibPos lp in currentOUS.posLibrary)
                    if (lp.selected)
                        GetEllipse(new ArenaPosition(lp.X, lp.Y, lp.Z), Brushes.LightGray, "lib");
            }

            // Show Pre-set Positions
            if (currentOUS.showPreSet && currentOUS.preSetPosList != null)
            {
                for (int i = 1; i < 6; i++)
                {
                    if (currentOUS.preSetPosList.ContainsKey(i))
                    {
                        PreSetPos psp = (PreSetPos)currentOUS.preSetPosList[i];

                        if (psp.aPos == null)
                            psp.aPos = new ArenaPosition(allSets.basicParas.ArmLength, 0, 0);

                        SolidColorBrush b = Brushes.LightGray;
                        
                        switch (i)
                        {
                            case 4: b = Brushes.AliceBlue; break;
                            case 5: b = Brushes.Gold; break;
                        }

                        GetEllipse(psp.aPos, b, "PreSet");
                    }
                }
            }

            // Show the Origin
            if (currentOUS.showOrigin)
                GetEllipse(new ArenaPosition(allSets.basicParas.ArmLength, 0, 0), Brushes.YellowGreen, "origin");

            // Show Current Position
            GetEllipse(currentAP, Brushes.Tomato, "current");
        }

        private Ellipse GetEllipse(ArenaPosition ap, Brush color, string tag)
        {
            Point targetPt = Converter.Arena2ArenaUI(ap);
            Ellipse circle = new Ellipse();
            circle.Stroke = Brushes.Black;
            circle.StrokeThickness = 1;
            circle.Height = 10;
            circle.Width = 10;
            circle.HorizontalAlignment = System.Windows.HorizontalAlignment.Left;
            circle.VerticalAlignment = System.Windows.VerticalAlignment.Top;
            circle.Margin = new Thickness(targetPt.Y - 5, targetPt.X - 7, 0, 0);
            circle.Fill = color;

            circle.Tag = tag;
            arenaGrid.Children.Add(circle);
            return circle;
        }

        private void TabItem_GotFocus_1(object sender, RoutedEventArgs e)
        {
            RefreshCircles();
        }





        // Open Pre-sets Editing Dialog
        private void preSetPosConfigButton_Click(object sender, RoutedEventArgs e)
        {
            EditPositions ep = new EditPositions();
            ep.Owner = this;
            ep.ShowDialog();
            DisplaySettings();
        }

        // Folder selection
        private void dataDirTextBox_MouseDoubleClick(object sender, MouseButtonEventArgs e)
        {
            string path = OpenF.FolderBrowseDialog();
            if (path != "")
            {
                currentOUS.path = path;
                dataDirTextBox.Text = path;
            }
        }

        // Drag to move the window
        private void Window_MouseLeftButtonDown_1(object sender, MouseButtonEventArgs e)
        {
            try { this.DragMove(); }
            catch (Exception) { }                                       // Strange error occurs when not catching 
        }

        // Prevent empty string in the TextBox
        private void xTextBox_LostFocus(object sender, RoutedEventArgs e)
        {
            System.Windows.Controls.TextBox tb = (System.Windows.Controls.TextBox)sender;
            if (tb.Text == "")
                tb.Text = "0";
        }

        // Afterwork on exit
        private void exitButton_Click(object sender, RoutedEventArgs e)
        {
            reading = false;
            SaveSettings();

            try
            {
                serialReadThread.Join();
                serialPort.Close();
            }
            catch (Exception) { }

            //try
            //{
            //    CInstance.Close();
            //}
            //catch (Exception) { }

            Application.Current.Shutdown();
        }

        private void minimizeButton_Click(object sender, RoutedEventArgs e)
        {
            this.WindowState = WindowState.Minimized;
        }

        private void maximizeButton_Click(object sender, RoutedEventArgs e)
        {
            if (this.WindowState == WindowState.Maximized)
                this.WindowState = WindowState.Normal;
            else if (this.WindowState == WindowState.Normal)
                this.WindowState = WindowState.Maximized;
        }

        #endregion

        private void TabItem_GotFocus_2(object sender, RoutedEventArgs e)
        {
            serialMonitorListBox.SelectedIndex = serialMonitorListBox.Items.Count - 1;
            serialMonitorListBox.ScrollIntoView(serialMonitorListBox.SelectedItem);
            serialMonitorListBox.SelectedIndex = -1;
        }

        private void showPosCheckbox_Click(object sender, RoutedEventArgs e)
        {
            RefreshCircles();
        }




    }
}
