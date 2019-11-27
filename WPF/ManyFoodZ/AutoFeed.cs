using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.IO.Ports;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Shapes;
using System.Windows.Threading;
using Microsoft.Research.DynamicDataDisplay.DataSources;

namespace ManyFoodZ
{
    public partial class MainWindow : Window
    {

        #region SERIAL FEED


        // Records
        LogSession logCurrentSession;
        ObservableCollection<SeqPosition> seqPosList;
        ObservableCollection<RandomPosition> randPosList;
        static int totalTrialNum = 0;
        static int currentTrialNum = 0;

        // Flags
        static bool doingAutoFeed = false;
        static bool triggered = false;
        static bool aborted = false;
        static bool paused = false;

        // For calculations
        int chanceTopVal;

        // Timers
        private static Timer imgTimer;



        private void posLibDataGrid_RowEditEnding_1(object sender, DataGridRowEditEndingEventArgs e)
        {
            LibPos lp = new LibPos();
            lp = e.Row.Item as LibPos;
            if (lp.posName == "")
                currentOUS.posLibrary.Remove(lp);
        }

        private void add2LibButton_Click(object sender, RoutedEventArgs e)
        {
            LibPos lp = new LibPos();
            lp.posName = "newly added pos.";
            lp.X = currentAP.X;
            lp.Y = currentAP.Y;
            lp.Z = currentAP.Z;

            currentOUS.posLibrary.Add(lp);
        }





        private void autoFeedStartButton_Click(object sender, RoutedEventArgs e)
        {
            // Auto-feed UI Initialization
            SaveSettings();
            doingAutoFeed = true;
            autoFeedStartButton.IsEnabled = false;
            preSetStackPanel.IsEnabled = false;
            autoFeedGrid.IsEnabled = false;
            deviceParaGrid.IsEnabled = false;

            // Auto-feed Records Initialization
            logCurrentSession = new LogSession(DateTime.Now, allSets.basicParas, currentOUS.behaviorParas);
            totalTrialNum = 0;
            currentTrialNum = 0;

            // Behavior Program Initialization
            if ((bool)seqRadioButton.IsChecked)
            {
                autoFeedThread = new Thread(new ThreadStart(DoSequentialFeed));
                seqPosList = new ObservableCollection<SeqPosition>();

                int length = currentOUS.posLibrary.Count();
                int index = 0;

                for (int i = 0; i < length; i++)
                    foreach (LibPos lp in currentOUS.posLibrary)
                        if (lp.selected && lp.index != 0)
                        {
                            bool find = false;
                            while (!find)
                                if (index == lp.index)
                                {
                                    seqPosList.Add(new SeqPosition(lp));
                                    find = true;
                                }
                                else
                                {
                                    index++;
                                    if (index > length)
                                        break;
                                }
                        }

                foreach (SeqPosition sp in seqPosList)
                {
                    totalTrialNum += sp.repeat;
                }
                totalTrialNum *= currentOUS.behaviorParas.cycNum;
            }
            else
            {
                autoFeedThread = new Thread(new ThreadStart(DoRandomFeed));
                randPosList = new ObservableCollection<RandomPosition>();

                chanceTopVal = 0;
                foreach (LibPos lp in currentOUS.posLibrary)
                    if (lp.selected && lp.chance != 0)
                    {
                        randPosList.Add(new RandomPosition(lp, chanceTopVal));
                        chanceTopVal += lp.chance;
                    }

                totalTrialNum = currentOUS.behaviorParas.trialNum;
            }
            processLabel.Content = "Result of trial " + currentTrialNum + "/" + totalTrialNum + " (last trial) : ";

            autoFeedThread.IsBackground = true;
            autoFeedThread.Start();
        }

        private void DoSequentialFeed()
        {
            for (int i = 0; i < currentOUS.behaviorParas.cycNum; i++)
            {
                foreach (SeqPosition sp in seqPosList)
                {
                    ArenaPosition ap = sp.ap;
                    for (int j = 0; j < sp.repeat; j++)
                        DoFeedCycle(ap);
                }
                Thread.Sleep(TimeSpan.FromSeconds(currentOUS.behaviorParas.cycInterval));
            }
            AfterWork();
        }

        private void DoRandomFeed()
        {
            for (int i = 0; i < currentOUS.behaviorParas.trialNum; i++)
            {
                Random randGenerator = new Random();
                double randNum = randGenerator.Next(0, chanceTopVal - 1);
                foreach (RandomPosition rp in randPosList)
                    if (rp.LieWithin(randNum))
                    {
                        ArenaPosition ap = rp.ap;
                        DoFeedCycle(ap);
                        break;
                    }
            }
            AfterWork();
        }


        private void triggerButton_Click(object sender, RoutedEventArgs e)
        {
            if (doingAutoFeed)
                triggered = true;
        }


        private void DoFeedCycle(ArenaPosition ap)
        {
            executed = false;

            currentTrialNum++;

            PreSetPos foodPos = (PreSetPos)currentOUS.preSetPosList[5];
            ArenaPosition preFoodPos = new ArenaPosition(allSets.basicParas.ArmLength, 0, 0);
            preFoodPos.L = foodPos.aPos.L;
            preFoodPos.H = foodPos.aPos.H;
            preFoodPos.R = foodPos.aPos.R;

            BehaviorRecord behaviorRecord = new BehaviorRecord(currentTrialNum, ap, foodPos.aPos);

            // Move to Food Loading Preparation Position
            InvokeGo2ArenaPos(preFoodPos);
            CheckExecuted();

            // Food Catching Position
            InvokeGo2ArenaPos(foodPos.aPos);
            CheckExecuted();

            // Get One New Food
            InvokeSerialSend(CalculateLoaderSteps(1) + "f");
            CheckExecuted();
            InvokeGo2ArenaPos(preFoodPos);
            CheckExecuted();

            // Waiting for Trial Begin Trigger
            if (currentOUS.behaviorParas.manualBegin)
            {
                // Note
                while (!triggered)
                {
                    Thread.Sleep(10);
                    CheckPoint();
                }
                // Remove Note
                triggered = false;
            }

            // Timing of Imaging Start
            if (currentOUS.behaviorParas.trigImg)
            {
                imgTimer = new Timer(InvokeSerialSend, "i", TimeSpan.FromMilliseconds(currentOUS.behaviorParas.imgDelay), TimeSpan.FromMilliseconds(-1));
            }

            // Move to Grabbing Position 
            InvokeGo2ArenaPos(ap);
            CheckExecuted();
            InvokePrint("Trial Starts @ " + DateTime.Now.Minute + ":" + DateTime.Now.Second + ":" + DateTime.Now.Millisecond / 10);

            // Timing of Cue
            if (currentOUS.behaviorParas.manualCue)
            {
                // Note
                while (!triggered)
                {
                    Thread.Sleep(10);
                    CheckPoint();
                }
                // Remove Note
                triggered = false;
            }
            else
            {
                int halfWindow = (int)Math.Round(currentOUS.behaviorParas.cueWindow / 2);
                Random randGenerator = new Random();
                double randCueDelay = currentOUS.behaviorParas.cueDelay + randGenerator.Next(-halfWindow, halfWindow);
                Thread.Sleep(TimeSpan.FromMilliseconds(randCueDelay));
            }
            MainWindow.MWInstance.Dispatcher.Invoke((ThreadStart)delegate
            {
                SendCue();
            });

            // Timing of Trial End
            if (currentOUS.behaviorParas.manualEnd)
            {
                // Note
                while (!triggered)
                {
                    Thread.Sleep(10);
                    CheckPoint();
                }
                // Remove Note
                triggered = false;
            }
            else
            {
                Thread.Sleep(TimeSpan.FromMilliseconds(currentOUS.behaviorParas.reachLimit));
            }
            InvokePrint("Trial Ends @ " + DateTime.Now.Minute + ":" + DateTime.Now.Second + ":" + DateTime.Now.Millisecond / 10);

            // Cancel Timers (Even when they exceed waiting time)
            try
            {
                imgTimer.Dispose();
            }
            catch (Exception) { }


            // Move to Food Loading Preparation Position
            InvokeGo2ArenaPos(preFoodPos);
            CheckExecuted();
            InvokeSerialSend("c");


            //logCurrentSession.behaviorList.Add(behaviorRecord);
        }





        private void InvokeGo2ArenaPos(ArenaPosition ap)
        {
            MainWindow.MWInstance.Dispatcher.Invoke((ThreadStart)delegate
            {
                Go2ArenaPos(ap);
            });
        }
        private void InvokeSerialSend(object cmd)
        {
            MainWindow.MWInstance.Dispatcher.Invoke((ThreadStart)delegate
            {
                SerialSend((string)cmd);
            });
        }
        private void InvokePrint(string stamp)
        {
            MainWindow.MWInstance.Dispatcher.Invoke((ThreadStart)delegate
            {
                ListBoxItem lbi = new ListBoxItem();
                lbi.Content = stamp;
                lbi.Foreground = Brushes.Gray;
                behaviorListBox.Items.Add(lbi);
                behaviorListBox.ScrollIntoView(lbi);
            });
        }





        private void CheckExecuted()
        {
            CheckPoint();
            while (!executed)
            {
                Thread.Sleep(10);
                CheckPoint();
            }
            executed = false;
        }
        private void CheckPoint()
        {
            if (paused)
            {
                MainWindow.MWInstance.Dispatcher.BeginInvoke((ThreadStart)delegate()
                    {
                        autoFeedPauseButton.IsEnabled = true;
                    });
                Thread.CurrentThread.Suspend();
            }
            if (aborted)
            {
                try
                {
                    aborted = false;
                    Thread.CurrentThread.Abort();
                }
                catch (Exception) { }
            }
        }

        private void autoFeedAbortButton_Click(object sender, RoutedEventArgs e)
        {
            aborted = true;
            AfterWork();
        }

        private void autoFeedPauseButton_Click(object sender, RoutedEventArgs e)
        {
            string temp = (string)autoFeedPauseButton.Content;
            if (temp.Equals("Continue"))
            {
                autoFeedThread.Resume();
                paused = false;
                autoFeedAbortButton.IsEnabled = true;
                autoFeedPauseButton.Content = "Pause";
            }
            else if (temp.Equals("Pause"))
            {
                paused = true;
                autoFeedPauseButton.IsEnabled = false;
                autoFeedPauseButton.Content = "Continue";
                autoFeedAbortButton.IsEnabled = false;
            }
        }

        private void AfterWork()
        {
            MainWindow.MWInstance.Dispatcher.BeginInvoke((ThreadStart)delegate()
                {
                    if (logCurrentSession.behaviorList.Count() > 0)
                    {
                        logDay.sessionList.Add(logCurrentSession);
                    }

                    autoFeedStartButton.IsEnabled = true;
                    autoFeedAbortButton.IsEnabled = true;
                    preSetStackPanel.IsEnabled = true;
                    deviceParaGrid.IsEnabled = true;
                    autoFeedGrid.IsEnabled = true;
                    doingAutoFeed = false;
                });
        }



        #endregion

    }
}
