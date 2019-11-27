using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Forms;
using System.Windows.Media;
using System.Windows.Shapes;

namespace ManyFoodZ
{
    [Serializable]
    public class ArenaPosition
    {
        public double X { set; get; }
        public double Y { set; get; }
        public double Z { set; get; }

        public double V { set; get; }
        public double H { set; get; }
        public double L { set; get; }
        public double R { set; get; }

        public bool outOfRange = false;
        public bool tiltBack = false;


        public ArenaPosition(double x, double y, double z)
        {
            X = x;
            Y = y;
            Z = z;

            double a = MainWindow.MWInstance.allSets.basicParas.ArmLength;
            double l = MainWindow.MWInstance.allSets.basicParas.railLength * 100;

            V = Math.Asin(Z / a);
            H = Math.Asin(Y / a / Math.Cos(V));
            L = 100.0 * (X - a * Math.Cos(H) * Math.Cos(V));

            if (Z > a || L > l || Y < 0 || Z < 0)
                outOfRange = true;
            else if (L < 0)
            {
                H = Math.PI - H;
                L = 100.0 * (X - a * Math.Cos(H) * Math.Cos(V));
                if (L < 0 || L > l)
                    outOfRange = true;
                else
                    tiltBack = true;
            }

            V = V / Math.PI * 180;
            H = H / Math.PI * 180;

            if (H > 90)
                R = 0;
            else if (H > 45)
                R = 45;
            else
                R = H;
        }

        public string Arena2RawCmd()
        {
            string cmdL = Convert.ToString(Math.Round(L, 3));
            string cmdH = Convert.ToString(Math.Round(H, 3));
            string cmdV = Convert.ToString(Math.Round(V, 3));
            string cmdR = Convert.ToString(Math.Round(R, 3));
            return cmdL + "l" + cmdH + "h" + cmdV + "v" + cmdR + "r" + "e";
        }

        public bool Equals(ArenaPosition ap)
        {
            return X == ap.X && Y == ap.Y && Z == ap.Z;
        }
    }

    [Serializable]
    public class RawPosition
    {
        public double L;
        public double H;
        public double V;
        public bool outOfRange = false;
        public bool tiltback = false;

        public RawPosition(double l, double h, double v)
        {
            L = l;
            H = h;
            V = v;

            double length = MainWindow.MWInstance.allSets.basicParas.railLength * 100;

            if (L > length || L < 0 || H < 0 || H > 180 || V < 0 || V > 90)
                outOfRange = true;
            if (H > 90)
                tiltback = true;
        }

        public ArenaPosition Raw2Arena()
        {
            double h = H / 180.0 * Math.PI;
            double v = V / 180.0 * Math.PI;
            double a = (double)MainWindow.MWInstance.allSets.basicParas.ArmLength;

            double X = L / 100 + a * Math.Cos(h) * Math.Cos(v);
            double Y = a * Math.Sin(h) * Math.Cos(v);
            double Z = a * Math.Sin(v);

            return new ArenaPosition(X, Y, Z);
        }
    }

    class RandomPosition
    {
        int lowEdge;
        int highEdge;
        public ArenaPosition ap;

        public RandomPosition(LibPos lp, int accumulator)
        {
            lowEdge = accumulator;
            highEdge = lp.chance + accumulator;
            ap = new ArenaPosition(lp.X, lp.Y, lp.Z);
        }

        public bool LieWithin(double rand)
        {
            if (rand >= lowEdge && rand < highEdge)
                return true;
            else
                return false;
        }
    }

    class SeqPosition
    {
        public ArenaPosition ap;
        public int repeat;

        public SeqPosition(LibPos lp)
        {
            ap = new ArenaPosition(lp.X, lp.Y, lp.Z);
            repeat = lp.repeat;
        }
    }

    class Converter
    {
        public static Point ArenaUI2Arena(Point pt)
        {
            MainWindow win = MainWindow.MWInstance;
            double a = win.allSets.basicParas.ArmLength;
            double l = win.allSets.basicParas.railLength;

            double x = l + a - pt.Y / win.ArenaBoarderHeight * (l + 2 * a);
            double y = pt.X / win.ArenaBoarderWidth * a;

            return new Point(x, y);
        }

        public static Point Arena2ArenaUI(ArenaPosition ap)
        {
            MainWindow win = MainWindow.MWInstance;
            double a = win.allSets.basicParas.ArmLength;
            double l = win.allSets.basicParas.railLength;

            double top = (1 - (ap.X + a) / (l + 2 * a)) * win.ArenaBoarderHeight;
            double left = ap.Y / a * win.ArenaBoarderWidth;

            return new Point(top, left);
        }
    }

    // Open Folder or Open File Dialog
    class OpenF
    {

        public static string FolderBrowseDialog()
        {
            FolderBrowserDialog b = new FolderBrowserDialog();
            DialogResult result = b.ShowDialog();

            if (result == System.Windows.Forms.DialogResult.Cancel)
            {
                return "";
            }
            return b.SelectedPath.Trim();
        }

    }


}
