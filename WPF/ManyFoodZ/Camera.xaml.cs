using System;
using System.Collections.Generic;
using System.Linq;
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
using Microsoft.Expression.Encoder.Devices;
using WebcamControl;
using System.IO;
using System.Drawing.Imaging;

namespace ManyFoodZ
{
    /// <summary>
    /// Camera.xaml 的交互逻辑
    /// </summary>
    public partial class Camera : Window
    {

        public Camera()
        {
            InitializeComponent();

            // Prepare Camera
            cameraGrid.DataContext = WebCamCtrl;

            WebCamCtrl.VidFormat = VideoFormat.mp4;
            WebCamCtrl.PictureFormat = ImageFormat.Png;
            WebCamCtrl.FrameRate = 30;
            WebCamCtrl.FrameSize = new System.Drawing.Size(780, 480);

            var vidDevices = EncoderDevices.FindDevices(EncoderDeviceType.Video);
            var audDevices = EncoderDevices.FindDevices(EncoderDeviceType.Audio);

            foreach (EncoderDevice dvc in vidDevices)
                camVideoComboBox.Items.Add(dvc.Name);

            foreach (EncoderDevice dvc in audDevices)
                camAudioComboBox.Items.Add(dvc.Name);

        }

        private void camStartButton_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                WebCamCtrl.StartCapture();
            }
            catch (Microsoft.Expression.Encoder.SystemErrorException)
            {
                MessageBox.Show("Device is in use by another application");
            }
        }
        private void camStopButton_Click(object sender, RoutedEventArgs e)
        {
            WebCamCtrl.StopCapture();
        }
        private void SnapshotButton_Click(object sender, RoutedEventArgs e)
        {
            if (Directory.Exists(MainWindow.MWInstance.imageDir))
            {
                WebCamCtrl.ImageDirectory = MainWindow.MWInstance.imageDir;
                WebCamCtrl.TakeSnapshot();
            }
        }
        private void RecordButton_Click(object sender, RoutedEventArgs e)
        {
            if (Directory.Exists(MainWindow.MWInstance.videoDir))
            {
                WebCamCtrl.VideoDirectory = MainWindow.MWInstance.videoDir;
                WebCamCtrl.StartRecording();
            }
        }
        private void StopRecordButton_Click(object sender, RoutedEventArgs e)
        {
            WebCamCtrl.StopRecording();
        }

        private void Window_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            WebCamCtrl.StopCapture();
            MainWindow.MWInstance.camButton.IsEnabled = true;
        }           
    }
}
