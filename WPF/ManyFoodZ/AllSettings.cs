using System;
using System.Collections;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ManyFoodZ
{
    [Serializable]
    public class AllSettings
    {
        public HardwareParas basicParas = new HardwareParas();
        public Hashtable userHashtable = new Hashtable();
    }

    [Serializable]
    public class HardwareParas
    {
        // Servos
        public int servoH_begin { get; set; }
        public int servoH_end { get; set; }
        public int servoV_begin { get; set; }
        public int servoV_end { get; set; }
        public int servoR_begin { get; set; }
        public int servoR_end { get; set; }
        public int servoD_begin { get; set; }
        public int servoD_end { get; set; }
        public int servoAcc { get; set; }

        // Arm
        public double ArmLength { get { return Math.Sqrt(Math.Pow(armXoffset, 2) + Math.Pow(armZoffset, 2)); } }
        public double armXoffset { get; set; }
        public double armZoffset { get; set; }

        // Stepper Motor
        public int motorStepSize { get; set; }
        public int pulseTime { get; set; }
        public int motorMaxSpeed { get; set; }
        public int motorAcc { get; set; }
        public double railLength { get; set; }

        // Piezos
        public int multiplierD { get; set; }
        public int pNumThresholdD { get; set; }
        public int dropCritItvl { get; set; }

        // Counters
        public int pNumThresholdC { get; set; }
        public int counterCritItvl { get; set; }

        // Food Loader
        public bool onArmLoader { get; set; }
        public double wheelDivision { get; set; }
        public double loaderStepperDiv { get; set; }

        // Cue
        public bool doorbell { get; set; }
        public double toneFrequency { get; set; }
        public double toneDuration { get; set; }

        public HardwareParas()
        {
            servoH_begin = 570;     // us
            servoH_end = 2150;      // us
            servoV_begin = 640;     // us
            servoV_end = 2250;      // us
            servoR_begin = 600;     // us
            servoR_end = 2200;      // us
            servoD_begin = 600;     // us
            servoD_end = 2200;      // us
            servoAcc = 90;          // degree/s^2

            motorStepSize = 10;     // um
            pulseTime = 100;        // us
            motorMaxSpeed = 30;     // mm/s
            motorAcc = 60;          // mm/s^2
            railLength = 85;        // mm

            armXoffset = 50;        // mm
            armZoffset = 0;         // mm

            multiplierD = 150;      // %
            pNumThresholdD = 3;
            dropCritItvl = 150;     // ms

            pNumThresholdC = 2;
            counterCritItvl = 50;   // ms

            onArmLoader = false;
            loaderStepperDiv = 2048;
            wheelDivision = 64;     // mm

            doorbell = true;
            toneFrequency = 1000;   // Hz
            toneDuration = 500;     // ms
        }

        public string Specs()
        {
            string specs = "\nHARDWARE PARAMETERS\n";
            specs += "\nPositioning System";
            specs += "\nServos' angular acceleration : " + servoAcc + " mm/s^2";
            specs += "\nRail platform's maximum speed : " + motorMaxSpeed + " mm/s";
            specs += "\nRail platform's acceleration : " + motorAcc + " mm/s^2";
            specs += "\nArm Length : " + ArmLength + " mm";
            specs += "\nRail Length (available) : " + railLength + " mm";

            specs += "\n\nDetection System";
            specs += "\nDrop detection threshold : " + multiplierD + "% (positive signal >= baseline * this value)";
            specs += "\nDrop reliability : " + pNumThresholdD + " (minimal consecutive signals of a drop)";
            specs += "\nDrop critical interval : " + dropCritItvl + " ms (maximum interval between two consecutive signal)";
            specs += "\nCounter reliability : " + pNumThresholdC + " (minimal consecutive signals of food missing or grab)";
            specs += "\nCounter critical interval : " + counterCritItvl + " ms (maximum interval between two consecutive signal)";

            specs += "\n\nFeed and Feedback System";
            if (doorbell)
            {
                specs += "\nFrequency of Tone : " + toneFrequency + " Hz";
                specs += "\nDuration of Tone : " + toneDuration + " ms";
            }

            specs += "\n\n\n\n\n";
            return specs;
        }
    }

    [Serializable]
    public class OneUserSettings
    {
        public string path { get; set; }                                                        // Data directory
        public BehaviorParas behaviorParas = new BehaviorParas();                               // Behavior parameters
        public Hashtable preSetPosList = new Hashtable();                                       // Pre-sets locations
        public ObservableCollection<LibPos> posLibrary = new ObservableCollection<LibPos>();    // Position library
        public ObservableCollection<LogDay> logLibrary = new ObservableCollection<LogDay>();    // Experiment records

        public bool showPreSet { get; set; }
        public bool showLibPos { get; set; }
        public bool showOrigin { get; set; }

        public OneUserSettings()
        {
            path = "";
            showPreSet = false;
            showLibPos = false;
            showOrigin = true;
        }
    }

    [Serializable]
    public class BehaviorParas
    {
        // Serial feed basic settings
        public double imgDelay { get; set; }
        public double cueDelay { get; set; }
        public double cueWindow { get; set; }
        public double reachLimit { get; set; }
        public double graspLimit { get; set; }
        public double trialInterval { get; set; }

        public bool trigImg { get; set; }
        public bool manualBegin { get; set; }
        public bool manualCue { get; set; }
        public bool manualEnd { get; set; }
        public bool trigOption3 { get; set; }

        // Random mode settings
        public int trialNum { get; set; }
        public bool evenOdd { get; set; }

        // Sequence mode settings
        public int cycNum { get; set; }
        public int cycInterval { get; set; }

        public BehaviorParas()
        {
            trigImg = true;
            manualBegin = false;
            manualCue = false;
            manualEnd = false;
            trigOption3 = false;

            imgDelay = 0;           // ms
            cueDelay = 3000;        // ms
            cueWindow = 1000;       // ms
            reachLimit = 5000;      // ms
            graspLimit = 2000;      // ms
            trialInterval = 10;     // s

            trialNum = 10;
            evenOdd = false;
            cycNum = 1;
            cycInterval = 15;
        }

        public string Specs()
        {
            string specs = "\nBASIC SETTINGS\n";
            if (manualBegin)
                specs += "\nTriggered feed\t";
            else
                specs += "\n";
            if (manualEnd)
                specs += "Triggered back";
            else
                specs += "Cue delay : " + cueDelay + " s";
            if (trigImg)
                specs += "\nImaging delay : " + imgDelay + " ms";
            if (trigOption3)
                specs += "\nGrasp limit : " + graspLimit + " ms";
            if (manualCue)
                specs += "\nTrial interval : " + trialInterval + " ms\n";
            else
                specs += "\n";

            specs += "\nRANDOM MODE SETTINGS\n";


            specs += "\nSEQUENCE MODE SETTINGS\n";



            return specs;
        }
    }

    [Serializable]
    class PreSetPos
    {
        public string posName;
        public ArenaPosition aPos;

        public PreSetPos(string n, double x, double y, double z)
        {
            posName = n;
            aPos = new ArenaPosition(x, y, z);
        }
    }

    [Serializable]
    public class LibPos
    {
        public string posName { get; set; }
        public double X { get; set; }
        public double Y { get; set; }
        public double Z { get; set; }
        public bool selected { get; set; }
        public int chance { get; set; }
        public int repeat { get; set; }
        public int index { get; set; }
    }

    [Serializable]
    public class LogDay
    {
        public DateTime dateTime { get; set; }
        public ObservableCollection<LogSession> sessionList { get; set; }
        public string comment { get; set; }

        public LogDay(DateTime dt)
        {
            dateTime = dt;
            sessionList = new ObservableCollection<LogSession>();
        }
    }

    [Serializable]
    public class LogSession
    {
        public int sessionIndex { get; set; }
        public string sessionName { get; set; }
        public DateTime time { get; set; }
        public HardwareParas hardwareParas { get; set; }
        public BehaviorParas behaviorParas { get; set; }
        public bool isRandomMode { get; set; }
        public ObservableCollection<BehaviorRecord> behaviorList { get; set; }

        public LogSession(DateTime t, HardwareParas hp, BehaviorParas bp)
        {
            sessionName = "";
            time = t;
            hardwareParas = hp;
            behaviorParas = bp;
            behaviorList = new ObservableCollection<BehaviorRecord>();

            sessionIndex = 1;
            foreach (LogSession ls in MainWindow.MWInstance.logDay.sessionList)
            {
                if (sessionIndex <= ls.sessionIndex)
                    sessionIndex = ls.sessionIndex + 1;
            }
        }

        public double HitRate()
        {
            double hits = 0;
            double trialNum = 0;
            foreach (BehaviorRecord br in behaviorList)
            {
                trialNum++;
                if (br.hit == true)
                {
                    hits++;
                }
            }
            return hits / trialNum * 100;
        }

        public string Summary()
        {
            string summary = "1";
            return summary;
        }

        internal string Details()
        {
            string details = "";

            foreach (var record in behaviorList)
            {
                details += record.GetText();
            }

            return details;
        }
    }

    [Serializable]
    public class BehaviorRecord
    {
        public int trialIndex { set; get; }
        public ArenaPosition ap { set; get; }
        public ArenaPosition foodPosition { set; get; }
        public bool? hit { set; get; }
        public int numAttempts { set; get; }
        public int eventIndex { set; get; }

        public BehaviorRecord(int index, ArenaPosition a, ArenaPosition f)
        {
            trialIndex = index;
            ap = a;
            foodPosition = f;
            numAttempts = 0;
            eventIndex = 0;
        }

        public string GetText()
        {
            string text = "\nTrial #" + trialIndex;
            if (hit == true)
                text += " Hit";
            else if (hit == false)
                text += " Fail";

            text += "\nArena position (X, Y, Z) : " + ap.X + ", " + ap.Y + ", " + ap.Z + " mm";
            text += "\nTotal number of attempts : " + numAttempts;
            if (eventIndex != 0)
                text += "\nEvent occured at attempt : " + eventIndex;
            text += "\n";

            return text;
        }
    }

}
