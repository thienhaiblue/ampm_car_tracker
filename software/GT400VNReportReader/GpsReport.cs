using System;
using System.Text;
using System.Windows.Forms;
using System.IO;
using System.Runtime.InteropServices;
using System.Collections.Generic;
using System.Linq;
using System.Collections;

namespace GT400VNReportReader
{
    #region Framing struct
    [StructLayout(LayoutKind.Sequential, Pack = 1, CharSet = CharSet.Ansi)]
    public struct DRIVER_INFO
    {
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
        public string driverName;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 12)]
        public string licenseNo;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 11)]
        public string issuedDate;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 11)]
        public string expiredDate;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 16)]
        public string phoneNo;
    }

    [StructLayout(LayoutKind.Sequential, Pack = 1, CharSet = CharSet.Ansi)]
    public struct TRACKER_CONFIG_DATA
    {
        // tracker info
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 18)]
        public string imei;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 18)]
        public string trackerId;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 16)]
        public string pass;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 16)]
        public string whiteCallerNo;

        // gprs settings
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 16)]
        public string gprsApn;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 16)]
        public string gprsUser;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 16)]
        public string gprsPass;
        public ushort runReportInterval;
        public ushort stopReportInterval;

        // data server
        public byte currentDataServer;
        // primary
        public UInt32 priDataServerIp;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 31)]
        public string priDataServerDomain;
        public ushort priDataServerPort;
        // secondary
        public UInt32 secDataServerIp;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 31)]
        public string secDataServerDomain;
        public ushort secDataServerPort;
        public byte dataServerUseIp;

        // firmware server
        public byte currentFirmwareServer;
        // primary
        public UInt32 priFirmwareServerIp;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 31)]
        public string priFirmwareServerDomain;
        public ushort priFirmwareServerPort;
        // secondary
        public UInt32 secFirmwareServerIp;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 31)]
        public string secFirmwareServerDomain;
        public ushort secFirmwareServerPort;
        public byte firmwareServerUseIp;

		// info server
		public byte currentInfoServer;
		// primary
		public UInt32 priInfoServerIp;
		[MarshalAs(UnmanagedType.ByValTStr, SizeConst = 31)]
		public string priInfoServerDomain;
		public ushort priInfoServerPort;
		// secondary
		public UInt32 secInfoServerIp;
		[MarshalAs(UnmanagedType.ByValTStr, SizeConst = 31)]
		public string secInfoServerDomain;
		public ushort secInfoServerPort;
		public byte infoServerUseIp;

        // driver info
        public byte driver;
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 10)]
        public DRIVER_INFO[] driverList;

        // vehicle info
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 12)]
        public string plateNo;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 20)]
        public string vin;
        public ushort speedSensorRatio;
        public ushort useGpsSpeed;

        // analog settings
        public ushort a1Mode;
        public ushort a1LowerBound;
        public ushort a1UpperBound;
        public ushort a2Mode;
        public ushort a2LowerBound;
        public ushort a2UpperBound;

        // image server
        public byte currentImageServer;
        // primary
        public UInt32 priImageServerIp;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 31)]
        public string priImageServerDomain;
        public ushort priImageServerPort;
        // secondary
        public UInt32 secImageServerIp;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 31)]
        public string secImageServerDomain;
        public ushort secImageServerPort;
        public byte imageServerUseIp;

        // camera settings
        public byte numCameras;
        public byte cameraCompression;
        public byte cameraWorkingStartTime;
        public byte cameraWorkingStopTime;
        public ushort cameraInterval;
        public ushort enableWarning;
        public ushort cameraEvents;

        // other settings
        public ushort speedLimit;
        public UInt32 drivingTimeLimit;
        public UInt32 totalDrivingTimeLimit;
        public UInt32 baseMileage;
      
        public ushort featureSet;            
        public ushort lastError;
        public ushort testMode;            

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
        public UInt32[] loginKey;
        public byte securityOn;
        public byte securityAction;
        public byte sosKeyPressTimes;
        public byte sosKeyPressPeriod;
        public uint sleepTimer;
        public uint powerLowWarningPeriod;
        public uint batteryLowWarningPeriod;
        public byte powerLowWarningLevel;
        public byte batteryLowWarningLevel;

        public uint autoSecurityTime;
        public uint speedEngineOff;
        public uint powerOffDelayTime;
		public byte accountAlarmCheck;

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 20)]
        public byte[] upgradeTimeStamp;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 200)]
        public string infoStr;

        UInt32 crc;
    }

    [StructLayout(LayoutKind.Sequential, Pack = 1, CharSet = CharSet.Ansi)]
    public struct RECORD_TIME
    { 
        public byte hour;
        public byte min;
        public byte sec;
    }

    [StructLayout(LayoutKind.Sequential, Pack = 1, CharSet = CharSet.Ansi)]
    public struct SPEED_RECORD
    {
        public RECORD_TIME currentTime;
        public RECORD_TIME beginTime;
        public RECORD_TIME endTime;

        [MarshalAs(UnmanagedType.R8)]
        public double lat;
        [MarshalAs(UnmanagedType.R8)]
        public double lng;
        [MarshalAs(UnmanagedType.R8)]
        public double speed;
        [MarshalAs(UnmanagedType.R8)]
        public double mileage;
        public uint drivingTime;
        public uint packingTime;
        public ushort overDrivingTimeCount;
        public uint totalDrivingTime;
        public ushort overTotalDrivingTimeCount;
        public ushort overSpeedCount;
        public ushort doorOpenCount;
        public ushort doorCloseCount;
        public ushort engineOnCount;
        public ushort engineOffCount;
        public ushort parkingCount;
        public byte status;
        public byte crc;
    }

    [StructLayout(LayoutKind.Sequential, Pack = 1, CharSet = CharSet.Ansi)]
    public struct SMS_RECORD
    { 
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst=16)]
        public string smsAddress;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst=16)]
        public string smsMessage;
    }

    #endregion

    public class MySpeedRecord
    {
        public DateTime date;
        public SPEED_RECORD record;

        public MySpeedRecord()
        {
            date = DateTime.Now;
            record = new SPEED_RECORD();
        }

        public MySpeedRecord(DateTime date, SPEED_RECORD record)
        {
            this.date = date;
            this.record = record;
        }

        public MySpeedRecord(DateTime date, byte[] recordBuf)
        {
            this.date = date;
            this.record = (SPEED_RECORD)Lib.ByteArrayToObject(recordBuf, typeof(SPEED_RECORD));
        }

        public DateTime GetCurrentTime()
        {
            DateTime res;

            res = DateTime.MinValue;
            try
            {
                res = new DateTime(date.Year, date.Month, date.Day, record.currentTime.hour, record.currentTime.min, record.currentTime.sec);
            }
            catch (Exception)
            { }

            return res;
        }

        public DateTime GetBeginTime()
        {
            DateTime res;

            if (record.beginTime.hour == 255)
            {
                return DateTime.MinValue;
            }

            res = DateTime.MinValue;
            try
            {
                res = new DateTime(date.Year, date.Month, date.Day, record.beginTime.hour, record.beginTime.min, record.beginTime.sec);
            }
            catch (Exception)
            { }

            return res;
        }

        public DateTime GetEndTime()
        {
            DateTime res;

            if (record.endTime.hour == 255)
            {
                return DateTime.MinValue;
            }

            res = DateTime.MinValue;
            try
            {
                res = new DateTime(date.Year, date.Month, date.Day, record.endTime.hour, record.endTime.min, record.endTime.sec);
            }
            catch (Exception)
            { }
            return res;
        }
    }

    public class IndividualReport
    {
        public int driver;
        public List<MySpeedRecord> speedRecords;

        public IndividualReport()
        {
            driver = 0;
            speedRecords = new List<MySpeedRecord>();
        }

        public void AddReport(MySpeedRecord record)
        {
            speedRecords.Add(record);
        }

        public void AddReport(DateTime date, byte[] recordBuf)
        {
            SPEED_RECORD record;
            MySpeedRecord sRecord;

            record = (SPEED_RECORD)Lib.ByteArrayToObject(recordBuf, typeof(SPEED_RECORD));
            if (record.crc == Lib.CalcCrc(recordBuf, 0, recordBuf.Length - 1))
            {
                if ((record.currentTime.hour < 24) && (record.currentTime.min < 60) && (record.currentTime.sec < 60))
                {
                    if ((record.lat >= -90) && (record.lat <= 90) &&
                       (record.lng >= -180) && (record.lng <= 180))
                    {
                        if (record.speed < 300)
                        {
                            sRecord = new MySpeedRecord(date, record);
                            speedRecords.Add(sRecord);
                        }
                    }
                }
            }
        }

        //public static void GenerateKML(List<IndividualReport[]> reports, string filePath)
        //{
            
        //    StreamWriter sw = new StreamWriter(filePath);
        //    sw.WriteLine("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
        //    sw.WriteLine("<kml xmlns=\"http://earth.google.com/kml/2.1\">");
        //    sw.WriteLine("<Document>");

        //    sw.WriteLine("<Placemark>");
        //    sw.WriteLine("<color>ffffff00</color>");
        //    sw.WriteLine("<width>6</width>");
        //    sw.WriteLine("<MultiGeometry>");
        //    sw.WriteLine("<LineString>");
        //    sw.WriteLine("<coordinates>");
        //    foreach (IndividualReport[] reportArr in reports)
        //    {
        //        foreach (IndividualReport report in reportArr)
        //        {
        //            report.speedRecords.ForEach(spr =>
        //            {
        //                if (spr.record.lat != 0 || spr.record.lng != 0)
        //                {
        //                    sw.Write(Lib.latlngToString(spr.record.lng) + "," + Lib.latlngToString(spr.record.lat) + " ");
        //                }
        //            });
        //        }
        //    }
        //    sw.WriteLine("</coordinates>");
        //    sw.WriteLine("</LineString>");
        //    sw.WriteLine("</MultiGeometry>");
        //    sw.WriteLine("</Placemark>");


        //    sw.WriteLine("<Folder>");
        //    foreach (IndividualReport[] reportArr in reports)
        //    {
        //        foreach (IndividualReport report in reportArr)
        //        {
        //            report.speedRecords.ForEach(spr =>
        //            {
        //                if (spr.record.lat != 0 || spr.record.lng != 0)
        //                {
        //                    sw.WriteLine("<Placemark>");
        //                    sw.WriteLine("<name>" + spr.record.currentTime.hour + ":" + spr.record.currentTime.min + ":" + spr.record.currentTime.sec + " " + spr.date.Day + "/" + spr.date.Month + "/" + spr.date.Year + " " + spr.record.speed.ToString(".#") + " km/h</name>");
        //                    sw.WriteLine("<Point>");
        //                    sw.WriteLine("<coordinates>" + Lib.latlngToString(spr.record.lng) + ", " + Lib.latlngToString(spr.record.lat) + ",0.0</coordinates>");
        //                    sw.WriteLine("</Point>");
        //                    sw.WriteLine("</Placemark>");
        //                }
        //            });
        //        }
        //    }
                    
        //    sw.WriteLine("</Folder>");
        //    sw.WriteLine("</Document>");
        //    sw.WriteLine("</kml>");

        //    sw.Close();
        //}

        public static void GenerateKML(TRACKER_CONFIG_DATA cfg, List<IndividualReport[]> reports, string filePath)
        {
            StreamWriter sw;
            DateTime beginTime, endTime;
            MySpeedRecord beginReport, endReport;
            int beginDriver, endDriver;

            if ((reports == null) && (reports.Count > 0))
            {
                return;
            }

            sw = new StreamWriter(filePath);
            sw.WriteLine("<?xml version=\"1.0\" encoding=\"utf-8\" standalone=\"yes\"?>");
            sw.WriteLine("<kml>");
            sw.WriteLine("<Document>");
            sw.WriteLine("<Style id=\"s_ylw-pushpin\"><IconStyle><scale>1.1</scale><Icon><href>http://maps.google.com/mapfiles/kml/pushpin/ylw-pushpin.png</href></Icon><hotSpot x=\"20\" y=\"2\" xunits=\"pixels\" yunits=\"pixels\"/></IconStyle></Style>");
            sw.WriteLine("<Style id=\"s_blue-pushpin\"><IconStyle><scale>1.1</scale><Icon><href>https://maps.google.com/mapfiles/kml/pushpin/blue-pushpin.png</href></Icon><hotSpot x=\"20\" y=\"2\" xunits=\"pixels\" yunits=\"pixels\"/></IconStyle></Style>");
            sw.WriteLine("<Style id=\"s_ylw-pushpin_hl\"><IconStyle><scale>1.3</scale><Icon><href>http://maps.google.com/mapfiles/kml/pushpin/red-pushpin.png</href></Icon><hotSpot x=\"20\" y=\"2\" xunits=\"pixels\" yunits=\"pixels\"/></IconStyle></Style>");
            sw.WriteLine("<StyleMap id=\"m_ylw-pushpin\"><Pair><key>normal</key><styleUrl>#s_ylw-pushpin</styleUrl></Pair><Pair><key>highlight</key><styleUrl>#s_ylw-pushpin_hl</styleUrl></Pair><Pair><key>highlight1</key><styleUrl>#s_blue-pushpin</styleUrl></Pair></StyleMap>");

            beginTime = DateTime.MaxValue;
            beginReport = null;
            beginDriver = 0;
            for (int i = 0; i < 10; i++)
            {
                IndividualReport report = reports[0][i];
                if ((report.speedRecords != null) && (report.speedRecords.Count > 0))
                {
                    if (report.speedRecords[0].GetCurrentTime() < beginTime)
                    {
                        beginTime = report.speedRecords[0].GetCurrentTime();
                        beginReport = report.speedRecords[0];
                        beginDriver = i;
                    }
                }
            }
            endTime = DateTime.MinValue;
            endReport = null;
            endDriver = 0;
            for (int i = 0; i < 10; i++)
            {
                IndividualReport report = reports[reports.Count - 1][i];
                if ((report.speedRecords != null) && (report.speedRecords.Count > 0))
                {
                    if (report.speedRecords[report.speedRecords.Count - 1].GetCurrentTime() > endTime)
                    { 
                        endTime = report.speedRecords[report.speedRecords.Count - 1].GetCurrentTime();
                        endReport = report.speedRecords[report.speedRecords.Count - 1];
                        endDriver = i;
                    }
                }
            }

            sw.WriteLine("<Placemark>");
            sw.WriteLine("<name>Điểm xuất phát : " + cfg.plateNo + "</name>");
            sw.WriteLine("<description>");
            sw.WriteLine("Thời gian : "+ beginReport.GetCurrentTime().ToString("HH:mm:ss dd/MM/yyyy").Replace('-', '/') + "<br/>");
            sw.WriteLine("Lái xe : " + cfg.driverList[beginDriver].driverName + "<br/>");
            sw.WriteLine("Biển số : " + cfg.plateNo + "<br/>");
            sw.WriteLine("Vận tốc : " + beginReport.record.speed + "<br/>");
            sw.WriteLine("No.Đóng mở : " + beginReport.record.doorOpenCount + "<br/>");
            sw.WriteLine("No.Quá tốc : " + beginReport.record.overSpeedCount + "<br/>");
            sw.WriteLine("No.Dừng đỗ : " + beginReport.record.parkingCount + "<br/>");
            sw.WriteLine("</description>");
            sw.WriteLine("<styleUrl>#s_blue-pushpin</styleUrl>");
            sw.WriteLine("<Point>");
            sw.WriteLine("<coordinates>" + beginReport.record.lng + "," + beginReport.record.lat + "</coordinates>");
            sw.WriteLine("</Point>");
            sw.WriteLine("</Placemark>");

            foreach (IndividualReport[] reportArr in reports)
            {
                for (int i = 0; i < 10; i++)
                {
                    IndividualReport report = reportArr[i];
                    report.speedRecords.ForEach(spr =>
                    {
                        if (spr.record.lat != 0 || spr.record.lng != 0)
                        {
                            sw.WriteLine("<Placemark>");
                            sw.WriteLine("<name>Biển số xe : " + cfg.plateNo + "</name>");
                            sw.WriteLine("<description>");
                            sw.WriteLine("Thời gian : " + spr.GetCurrentTime().ToString("HH:mm:ss dd/MM/yyyy").Replace('-', '/') + "<br/>");
                            sw.WriteLine("Lái xe : " + cfg.driverList[i].driverName + "<br/>");
                            sw.WriteLine("Biển số : " + cfg.plateNo + "<br/>");
                            sw.WriteLine("Vận tốc : " + spr.record.speed + "<br/>");
                            sw.WriteLine("No.Đóng mở : " + spr.record.doorOpenCount + "<br/>");
                            sw.WriteLine("No.Quá tốc : " + spr.record.overSpeedCount + "<br/>");
                            sw.WriteLine("No.Dừng đỗ : " + spr.record.parkingCount + "<br/>");
                            sw.WriteLine("</description>");
                            sw.WriteLine("<styleUrl>#s_ylw-pushpin</styleUrl>");
                            sw.WriteLine("<Point>");
                            sw.WriteLine("<coordinates>" + spr.record.lng + "," + spr.record.lat + "</coordinates>");
                            sw.WriteLine("</Point>");
                            sw.WriteLine("</Placemark>");
                        }
                    });
                }
            }

            sw.WriteLine("<Placemark>");
            sw.WriteLine("<name>Điểm kết thúc : " + cfg.plateNo + "</name>");
            sw.WriteLine("<description>");
            sw.WriteLine("Thời gian : " + endReport.GetCurrentTime().ToString("HH:mm:ss dd/MM/yyyy").Replace('-', '/') + "<br/>");
            sw.WriteLine("Lái xe : " + cfg.driverList[endDriver].driverName + "<br/>");
            sw.WriteLine("Biển số : " + cfg.plateNo + "<br/>");
            sw.WriteLine("Vận tốc : " + endReport.record.speed + "<br/>");
            sw.WriteLine("No.Đóng mở : " + endReport.record.doorOpenCount + "<br/>");
            sw.WriteLine("No.Quá tốc : " + endReport.record.overSpeedCount + "<br/>");
            sw.WriteLine("No.Dừng đỗ : " + endReport.record.parkingCount + "<br/>");
            sw.WriteLine("</description>");
            sw.WriteLine("<styleUrl>#s_blue-pushpin</styleUrl>");
            sw.WriteLine("<Point>");
            sw.WriteLine("<coordinates>" + endReport.record.lng + "," + endReport.record.lat + "</coordinates>");
            sw.WriteLine("</Point>");
            sw.WriteLine("</Placemark>");

            sw.WriteLine("<Placemark>");
            sw.WriteLine("<name>Route</name>");
            sw.WriteLine("<color>ff00ffff</color>");
            sw.WriteLine("<width>6</width>");
            sw.WriteLine("<MultiGeometry>");
            sw.WriteLine("<LineString>");
            sw.WriteLine("<coordinates>");
            foreach (IndividualReport[] reportArr in reports)
            {
                foreach (IndividualReport report in reportArr)
                {
                    report.speedRecords.ForEach(spr =>
                    {
                        if (spr.record.lat != 0 || spr.record.lng != 0)
                        {
                            sw.Write(spr.record.lng + "," + spr.record.lat + ",0.000000" + Environment.NewLine);
                        }
                    });
                }
            }
            sw.WriteLine("</coordinates>");
            sw.WriteLine("</LineString>");
            sw.WriteLine("</MultiGeometry>");
            sw.WriteLine("</Placemark>");

            sw.WriteLine("</Document>");
            sw.WriteLine("</kml>");

            sw.Close();
        }
    }       
}
