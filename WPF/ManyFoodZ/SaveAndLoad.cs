using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.Serialization;
using System.Runtime.Serialization.Formatters.Binary;
using System.Text;
using System.Threading.Tasks;

namespace ManyFoodZ
{
    class SaveAndLoad
    {
        public string settingsFilePath;

        public SaveAndLoad()
        {
            // Create a folder in My Document for saving settings
            string settingsFolderPath = Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments) +
                Path.DirectorySeparatorChar + "ManyFoodZ";
            if (!Directory.Exists(settingsFolderPath)) { Directory.CreateDirectory(settingsFolderPath); }

            // Get the path of settings file
            settingsFilePath = settingsFolderPath + Path.DirectorySeparatorChar + "Settings.bin";
        }

        public void SaveAll(AllSettings alset)
        {
            BFormatter.save(settingsFilePath, alset);
        }

        public object LoadAll()
        {
            return BFormatter.load(settingsFilePath);
        }
    }
    
    class BFormatter
    {
        /// <summary>
        /// BinaryFormatter序列化方式
        /// </summary>
        /// <param name="filepath"></param>
        /// <param name="obj"></param>
        public static void save(string filepath, object obj)
        {
            IFormatter formatter = new BinaryFormatter();
            Stream fs = null;
            try
            {
                fs = new FileStream(filepath, FileMode.Create, FileAccess.Write, FileShare.Write);
                formatter.Serialize(fs, obj);
            }
            catch (Exception e) { throw e; }
            finally
            { if (fs != null) { fs.Close(); } }
        }


        public static object load(string filepath)
        {
            IFormatter formatter = new BinaryFormatter();
            Stream fs = null;
            try
            {
                fs = new FileStream(filepath, FileMode.Open, FileAccess.Read, FileShare.Read);
                return formatter.Deserialize(fs);
            }
            catch (Exception e)
            {
                throw e;
            }
            finally { if (fs != null) { fs.Close(); } }
        }
    }
}
