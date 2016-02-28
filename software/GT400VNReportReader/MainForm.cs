using System;
using System.Threading;
using System.Windows.Forms;
using System.IO;
using System.Globalization;
using System.Runtime.InteropServices;
using System.Net;
using System.Collections.Generic;
using System.Drawing;
using System.IO.Ports;
using System.ComponentModel;
using System.Resources;

namespace GT400VNReportReader
{
    public partial class MainForm : Form
    {
        // variables defined for localized string
        // control strings
        ResourceString APP_CAPTION_STR = new ResourceString();
        ResourceString DATA_SERVER_CMBBOX_ITEM_STR = new ResourceString();
        ResourceString FIRMWARE_SERVER_CMBBOX_ITEM_STR = new ResourceString();
        ResourceString IMAGE_SERVER_CMBBOX_ITEM_STR = new ResourceString();
        ResourceString INTERVAL_TRIGGER_SOURCE_CHKLIST_ITEM_STR = new ResourceString();
        ResourceString CONNECTED_STR = new ResourceString();
        ResourceString DISCONNECTED_STR = new ResourceString();
        ResourceString RETRIEVING_FILE_STR = new ResourceString();
        ResourceString OPEN_FILE_DIALOG_TITLE_STR = new ResourceString();
        ResourceString SAVE_FILE_DIALOG_TITLE_STR = new ResourceString();
        ResourceString OPEN_FOLDER_DIALOG_TITLE_STR = new ResourceString();
        ResourceString DOWNLOADING_FIRMWARE_STR = new ResourceString();
        ResourceString LOG_INTERVAL_1_MIN_STR = new ResourceString();
        ResourceString LOG_INTERVAL_5_MIN_STR = new ResourceString();
        ResourceString LOG_INTERVAL_10_MIN_STR = new ResourceString();
        ResourceString LOG_INTERVAL_30_MIN_STR = new ResourceString();
        // message strings
        ResourceString READ_CONFIG_SUCCESS_STR = new ResourceString();
        ResourceString READ_CONFIG_FAILED_STR = new ResourceString();
        ResourceString WRITE_CONFIG_SUCCESS_STR = new ResourceString();
        ResourceString WRITE_CONFIG_FALIED_STR = new ResourceString();
        ResourceString FORMAT_SDCARD_WARNING_STR = new ResourceString();
        ResourceString KML_SAVE_SUCCESS_STR = new ResourceString();
        ResourceString CONFIG_SAVE_SUCCESS_STR = new ResourceString();
        ResourceString CONFIG_LOADED_STR = new ResourceString();
        ResourceString CANNOT_OPEN_COMPORT_STR = new ResourceString();
        ResourceString FIRMWARE_DOWNLOADED_SUCCESS_STR = new ResourceString();
        ResourceString FIRMWARE_DOWNLOADED_FAILED_STR = new ResourceString();
        ResourceString CANNOT_OPEN_FILE_STR = new ResourceString();
        ResourceString FILE_NAME_IS_INVALID_WARNING_STR = new ResourceString();
        ResourceString SET_CAMERA_ID_SUCCESS_STR = new ResourceString();
        ResourceString SET_CAMERA_ID_FAILED_STR = new ResourceString();
        List<ResourceString> localizedStrings;


        readonly int GPS_CONFIG_DATA_SIZE = Marshal.SizeOf(typeof(TRACKER_CONFIG_DATA));
        readonly int SPEED_RECORD_SIZE = Marshal.SizeOf(typeof(SPEED_RECORD));

        const int DEFAULT_COMM_TIMEOUT = 500;
        const byte COMM_START_CODE = 0xCA;
        const int MAX_COMM_RETRY = 1;

        const byte READ_CONFIG_OPCODE = 0x0C;
        const byte UPLOAD_LOG_OPCODE = 0x0F;
        const byte BOOT_MODE_OPCODE = 0x33;

        const UInt32 DEFAULT_PACKET_NO = 0xA5A5A5A5;
        const UInt32 ERROR_PACKET_NO = 0xFFFFFFFF;
        const UInt32 FINISH_PACKET_NO = 0x5A5A5A5A;
        const int START_CODE_OFFSET = 0;
        const int LENGTH_OFFSET = 1;
        const int OPCODE_OFFSET = 3;
        const int DATA_OFFSET = 4;
        const int PACKET_NO_OFFSET = 4;
        const int DATA_WITH_PACKET_NO_OFFSET = 8;
        const int SIMPLE_CRC_WITH_PACKET_NO_OFFSET = 8;
        const int LENGTH_SIZE = 2;
        const int PACKET_NO_SIZE = 4;
        const int HEADER_SIZE = OPCODE_OFFSET + 1;
        const int CRC_SIZE = 1;
        const int SIMPLE_PACKET_SIZE = HEADER_SIZE + PACKET_NO_SIZE + CRC_SIZE;
        const int MAX_DATA_SIZE = 507;
        const int SMS_SIZE = 32;
        const uint DOOR_STATE_MASK = 0x02;

        const int READ_FROM_DEVICE = 0;
        const int READ_FROM_HARDDISK = 1;

        const int LOG_LENGTH_LOW_THRESHOLD = 10;
        const int LOG_LENGTH_HIGH_THRESHOLD = 50;
        const int LOG_TIME_THRESHOLD = 1000;

        const string GOOGLE_MAP_LINK_PREFIX = "https://maps.google.com/maps?hl=vi&q=";

        bool isInTrialPeriod = false;
        Thread downloader;
        IntPtr notificationHandle = IntPtr.Zero;
        HID.Device hidDevice = new HID.Device();
        byte[] hidSendBuffer = new byte[65];
        byte[] hidReceivedBuffer = new byte[65];
        byte[] hidTempBuffer = new byte[1024];
        byte[] hidOutBuffer = new byte[1024];
        byte[] hidInBuffer = new byte[1024];
        byte[] myBuff = new byte[1024];
        ushort vid = 0x2047;
        ushort pid = 0x0301;
        byte[] cfgData = new byte[4096];
        byte[] smsData = new byte[Marshal.SizeOf(typeof(SMS_RECORD))];
        int  offsetData;
        int cfgDataOffset;
        UInt32 cfgDataSize;
        UInt32 cfgDeviceBuffSize;
        UInt32 cfgNumByteToSend;
        UInt32 fDeviceBuffSize;
        TRACKER_CONFIG_DATA cfg;
        string logPath;
        SerialPort serialPort;
        int hidBufIndex = 0;
        int fileDataOffset;
        int logInterval;
        int lastLogOffset;
        bool lastLogRead;
        List<IndividualReport[]> reports;
        IndividualReport[] currentReport;
        int currentDriver;
        DateTime currentReportDate;
        bool[] reportDriverNameAdded;
        DateTime prevLogReceivedTime;
        bool bootMode;

        bool readingReportFile;

        private void MainForm_Load(object sender, EventArgs e)
        {
            registerToolStripMenuItem.Visible = isInTrialPeriod;

            cfg = new TRACKER_CONFIG_DATA();
            cfg.driverList = new DRIVER_INFO[10];
            for (int i = 0; i < 10; i++)
            {
                cfg.driverList[i] = new DRIVER_INFO();
            }

            logIntervalCmbBox.SelectedIndex = 2;

            reports = new List<IndividualReport[]>();
            reportDriverNameAdded = new bool[10];

            prevLogReceivedTime = DateTime.Now;

            bootMode = false;

            // initialize localized strings
            InitLocalizedStrings();

            OpenHIDDevice();
            UpdateUI();
        }

        private void InitLocalizedStrings()
        {
            APP_CAPTION_STR.Name = "APP_CAPTION_STR";
            DATA_SERVER_CMBBOX_ITEM_STR.Name = "DATA_SERVER_CMBBOX_ITEM_STR";
            FIRMWARE_SERVER_CMBBOX_ITEM_STR.Name = "FIRMWARE_SERVER_CMBBOX_ITEM_STR";
            IMAGE_SERVER_CMBBOX_ITEM_STR.Name = "IMAGE_SERVER_CMBBOX_ITEM_STR";
            INTERVAL_TRIGGER_SOURCE_CHKLIST_ITEM_STR.Name = "INTERVAL_TRIGGER_SOURCE_CHKLIST_ITEM_STR";
            CONNECTED_STR.Name = "CONNECTED_STR";
            DISCONNECTED_STR.Name = "DISCONNECTED_STR";
            RETRIEVING_FILE_STR.Name = "RETRIEVING_FILE_STR";
            READ_CONFIG_SUCCESS_STR.Name = "READ_CONFIG_SUCCESS_STR";
            READ_CONFIG_FAILED_STR.Name = "READ_CONFIG_FAILED_STR";
            WRITE_CONFIG_SUCCESS_STR.Name = "WRITE_CONFIG_SUCCESS_STR";
            WRITE_CONFIG_FALIED_STR.Name = "WRITE_CONFIG_FALIED_STR";
            FORMAT_SDCARD_WARNING_STR.Name = "FORMAT_SDCARD_WARNING_STR";
            KML_SAVE_SUCCESS_STR.Name = "KML_SAVE_SUCCESS_STR";
            CONFIG_SAVE_SUCCESS_STR.Name = "CONFIG_SAVE_SUCCESS_STR";
            CONFIG_LOADED_STR.Name = "CONFIG_LOADED_STR";
            CANNOT_OPEN_COMPORT_STR.Name = "CANNOT_OPEN_COMPORT_STR";
            FIRMWARE_DOWNLOADED_SUCCESS_STR.Name = "FIRMWARE_DOWNLOADED_SUCCESS_STR";
            FIRMWARE_DOWNLOADED_FAILED_STR.Name = "FIRMWARE_DOWNLOADED_FAILED_STR";
            CANNOT_OPEN_FILE_STR.Name = "CANNOT_OPEN_FILE_STR";
            FILE_NAME_IS_INVALID_WARNING_STR.Name = "FILE_NAME_IS_INVALID_WARNING_STR";
            OPEN_FILE_DIALOG_TITLE_STR.Name = "OPEN_FILE_DIALOG_TITLE_STR";
            SAVE_FILE_DIALOG_TITLE_STR.Name = "SAVE_FILE_DIALOG_TITLE_STR";
            OPEN_FOLDER_DIALOG_TITLE_STR.Name = "OPEN_FOLDER_DIALOG_TITLE_STR";
            DOWNLOADING_FIRMWARE_STR.Name = "DOWNLOADING_FIRMWARE_STR";
            LOG_INTERVAL_1_MIN_STR.Name = "LOG_INTERVAL_1_MIN_STR";
            LOG_INTERVAL_5_MIN_STR.Name = "LOG_INTERVAL_5_MIN_STR";
            LOG_INTERVAL_10_MIN_STR.Name = "LOG_INTERVAL_10_MIN_STR";
            LOG_INTERVAL_30_MIN_STR.Name = "LOG_INTERVAL_30_MIN_STR";
            SET_CAMERA_ID_SUCCESS_STR.Name = "SET_CAMERA_ID_SUCCESS_STR";
            SET_CAMERA_ID_FAILED_STR.Name = "SET_CAMERA_ID_FAILED_STR";

            localizedStrings = new List<ResourceString>();

            localizedStrings.Add(APP_CAPTION_STR);
            localizedStrings.Add(DATA_SERVER_CMBBOX_ITEM_STR);
            localizedStrings.Add(FIRMWARE_SERVER_CMBBOX_ITEM_STR);
            localizedStrings.Add(IMAGE_SERVER_CMBBOX_ITEM_STR);
            localizedStrings.Add(INTERVAL_TRIGGER_SOURCE_CHKLIST_ITEM_STR);
            localizedStrings.Add(CONNECTED_STR);
            localizedStrings.Add(DISCONNECTED_STR);
            localizedStrings.Add(RETRIEVING_FILE_STR);
            localizedStrings.Add(READ_CONFIG_SUCCESS_STR);
            localizedStrings.Add(READ_CONFIG_FAILED_STR);
            localizedStrings.Add(WRITE_CONFIG_SUCCESS_STR);
            localizedStrings.Add(WRITE_CONFIG_FALIED_STR);
            localizedStrings.Add(FORMAT_SDCARD_WARNING_STR);
            localizedStrings.Add(KML_SAVE_SUCCESS_STR);
            localizedStrings.Add(CONFIG_SAVE_SUCCESS_STR);
            localizedStrings.Add(CONFIG_LOADED_STR);
            localizedStrings.Add(CANNOT_OPEN_COMPORT_STR);
            localizedStrings.Add(FIRMWARE_DOWNLOADED_SUCCESS_STR);
            localizedStrings.Add(FIRMWARE_DOWNLOADED_FAILED_STR);
            localizedStrings.Add(CANNOT_OPEN_FILE_STR);
            localizedStrings.Add(FILE_NAME_IS_INVALID_WARNING_STR);
            localizedStrings.Add(OPEN_FILE_DIALOG_TITLE_STR);
            localizedStrings.Add(SAVE_FILE_DIALOG_TITLE_STR);
            localizedStrings.Add(OPEN_FOLDER_DIALOG_TITLE_STR);
            localizedStrings.Add(DOWNLOADING_FIRMWARE_STR);
            localizedStrings.Add(LOG_INTERVAL_1_MIN_STR);
            localizedStrings.Add(LOG_INTERVAL_5_MIN_STR);
            localizedStrings.Add(LOG_INTERVAL_10_MIN_STR);
            localizedStrings.Add(LOG_INTERVAL_30_MIN_STR);
            localizedStrings.Add(SET_CAMERA_ID_SUCCESS_STR);
            localizedStrings.Add(SET_CAMERA_ID_FAILED_STR);

            LoadLocalizedString("en");
        }

        private void OpenHIDDevice()
        {
            string[] devicePathName = new string[0];

            if (HID.FindDevices(vid, pid, ref devicePathName) > 0)
            {
                hidDevice.pathName = devicePathName[0];

                if (HID.OpenDevice(ref hidDevice))
                {
                    hidDevice.readStream.BeginRead(hidReceivedBuffer, 0, hidReceivedBuffer.Length, new AsyncCallback(hidDevice_DataReceived), null);

                    ReadConfig();
                }
            }

            UpdateUI();
        }

        private void CloseHIDDevice()
        {
            HID.CloseDevice(ref hidDevice);
            UpdateUI();
        }

        private void ReadConfig()
        {
            if (!hidDevice.connected)
            {
                return;
            }

            CreateHIDBuffer(PACKET_NO_SIZE, READ_CONFIG_OPCODE, DEFAULT_PACKET_NO);
            hidOutBuffer[SIMPLE_CRC_WITH_PACKET_NO_OFFSET] = Lib.CalcCrc(hidOutBuffer, PACKET_NO_OFFSET, PACKET_NO_SIZE);
            cfgDataOffset = 0;
            cfgDeviceBuffSize = 0;
            cfgDataSize = 0;
            SendHIDData(hidOutBuffer, 0, SIMPLE_PACKET_SIZE, 1000);
        }

        private void CreateHIDBuffer(int length, byte opcode, UInt32 packetNo)
        {
            Array.Clear(hidOutBuffer, 0, hidOutBuffer.Length);
            hidOutBuffer[START_CODE_OFFSET] = COMM_START_CODE;
            Array.Copy(BitConverter.GetBytes(length), 0, hidOutBuffer, LENGTH_OFFSET, LENGTH_SIZE);
            hidOutBuffer[OPCODE_OFFSET] = opcode;
            Array.Copy(BitConverter.GetBytes(packetNo), 0, hidOutBuffer, PACKET_NO_OFFSET, PACKET_NO_SIZE);
        }

        private void hidDevice_DataReceived(IAsyncResult ar)
        {
            int length;
            try
            {
                length = hidDevice.readStream.EndRead(ar);
                if (!ar.IsCompleted)
                {
                    // read is not complete
                    return;
                }
                Array.Clear(hidTempBuffer, 0, hidTempBuffer.Length);
                Array.Copy(hidReceivedBuffer, 1, hidTempBuffer, 0, length - 1);
                hidBufIndex += length - 1;
                if (hidBufIndex >= (BitConverter.ToInt16(hidTempBuffer, LENGTH_OFFSET) + HEADER_SIZE + CRC_SIZE))
                {
                    hidBufIndex = 0;
                    Array.Copy(hidTempBuffer, hidInBuffer, hidInBuffer.Length);
                    ProcessReceivedHIDData();
                }
                hidDevice.readStream.BeginRead(hidReceivedBuffer, 0, hidReceivedBuffer.Length, new AsyncCallback(hidDevice_DataReceived), null);
            }
            catch
            {
            }
        }

        private void UpdateUI()
        {
            if (bootMode)
            {
                connectionStatusLbl.Text = "BOOT MODE";
            }
            else
            {
                commuTypeLbl.Text = "USB";
                if (hidDevice.connected)
                {
                    connectionStatusLbl.Text = CONNECTED_STR.Text;
                    connectionStatusLbl.ForeColor = Color.Green;
                }
                else
                {
                    connectionStatusLbl.Text = DISCONNECTED_STR.Text;
                    connectionStatusLbl.ForeColor = Color.Red;
                }
            }
            connectionStatusLbl.Left = statusGrpBox.Width - connectionStatusLbl.Width - 8;
            commuTypeLbl.Left = statusGrpBox.Width - 185;
        }

        private delegate void InvokerWithOneParam(string o);
        void UpdateDownloadStatus(string str)
        {
            retrieveLogStatusLbl.Text = str;
        }

        void UpdateProgressBar(int min, int max, int value, bool visible)
        {
            retrieveLogPrgBar.Minimum = min;
            retrieveLogPrgBar.Maximum = max;
            retrieveLogPrgBar.Visible = visible;
            retrieveLogPrgBar.Value = value;
            retrieveLogStatusLbl.Visible = visible;
        }

        void UpdateProgressBar(bool visible)
        {
            if (visible == false)
            {
                retrieveLogPrgBar.Value = 0;
                retrieveLogStatusLbl.Text = "0%";
            }
            retrieveLogPrgBar.Visible = visible;
            retrieveLogStatusLbl.Visible = visible;
            retrieveLogPercentLbl.Visible = visible;
        }

        void LoadBootModeLayout()
        {
            connectionStatusLbl.Text = "BOOT MODE";
            connectionStatusLbl.ForeColor = Color.Blue;
        }

        void LoadAppModeLayout()
        {
            UpdateUI();
        }

        public MainForm(bool isInTrial = false)
        {
            InitializeComponent();
            isInTrialPeriod = isInTrial;
        }

        protected override void WndProc(ref Message m)
        {
            if (m.Msg == Win32.WM_DEVICECHANGE)
            {
                if (m.WParam.ToInt32() == Win32.DBT_DEVICEARRIVAL)
                {
                    OpenHIDDevice();
                }
                else if (m.WParam.ToInt32() == Win32.DBT_DEVICEREMOVECOMPLETE)
                {
                    if (HID.DeviceNameMatch(m, hidDevice.pathName))
                    {
                        CloseHIDDevice();
                    }
                }

                UpdateUI();
            }

            base.WndProc(ref m);
        }

       

        private bool ParseIP(string ipAddress, ref UInt32 ip)
        {
            try
            {
                IPAddress address = IPAddress.Parse(ipAddress);
                byte[] b = address.GetAddressBytes();

                ip = BitConverter.ToUInt32(b, 0);
            }
            catch
            {
                return false;
            }

            return true;
        }


        #region download data
        List<byte> LogBuf;
        private void RetrieveLogData(object source)
        {
            string remotePathName;
            DateTime beginTime, endTime;

            beginTime = new DateTime(reportBeginDatePicker.Value.Year, reportBeginDatePicker.Value.Month, reportBeginDatePicker.Value.Day, 0, 0, 0);
            endTime = new DateTime(reportEndDatePicker.Value.Year, reportEndDatePicker.Value.Month, reportEndDatePicker.Value.Day, 23, 59, 59);

            if (beginTime > endTime)
            {
                this.Invoke((MethodInvoker)delegate
                {
                    UpdateProgressBar(false);
                });

                return;
            }
            this.Invoke((MethodInvoker)delegate
            {
                retrieveLogPrgBar.Minimum = 0;
                retrieveLogPrgBar.Maximum = (int)endTime.Subtract(beginTime).TotalMinutes;
            });

            // download hourly speed log files
            currentReport = CreateNewDayReport();
            do
            {
                currentReportDate = beginTime;
                for (currentDriver = 0; currentDriver < 10; currentDriver++)
                {
                    remotePathName = "LOG/" + beginTime.Year.ToString() + "_" + beginTime.Month.ToString("D2") + "/" + beginTime.Day.ToString("D2");
                    remotePathName += "/" + beginTime.Hour.ToString("D") + "_" + currentDriver.ToString() + ".log";

                    this.Invoke(new InvokerWithOneParam(UpdateDownloadStatus), RETRIEVING_FILE_STR.Text + remotePathName);

                    if ((int)source == READ_FROM_DEVICE)
                    {
                        //remotePathName = "/Tracker/" + remotePathName;
                        WaitFor<bool>.Run(new TimeSpan(0, 2, 0), () => ReadLogFromDevice(remotePathName));
                    }
                    else
                    {
                        ReadLogFromHardDisk(remotePathName);
                    }

                    this.Invoke((MethodInvoker)delegate
                    {
                        retrieveLogPrgBar.Value = retrieveLogPrgBar.Maximum - (int)endTime.Subtract(beginTime).TotalMinutes;
                        retrieveLogPercentLbl.Text = (retrieveLogPrgBar.Value * 100 / retrieveLogPrgBar.Maximum).ToString() + "%";
                    });
                }
                beginTime = beginTime.AddHours(1);
                if (beginTime.Day != currentReportDate.Day)
                {
                    for (int i = 0; i < currentReport.Length; i++)
                    {
                        if (currentReport[i].speedRecords.Count != 0)
                        {
                            reports.Add(currentReport);
                            break;
                        }
                    }
                    currentReport = CreateNewDayReport();
                }
            } while (beginTime.Ticks < endTime.Ticks);

            this.Invoke((MethodInvoker)delegate
            {
                UpdateReportUI();
                UpdateProgressBar(false);
                reportGrpBox.Enabled = true;
            });
        }

        private bool ReadLogFromDevice(string remotePath)
        {
            byte[] tmp = System.Text.Encoding.ASCII.GetBytes(remotePath);
            int length;

            LogBuf = new List<byte>();
            length = PACKET_NO_SIZE + tmp.Length + 1;
            CreateHIDBuffer(length, UPLOAD_LOG_OPCODE, DEFAULT_PACKET_NO);
            Array.Copy(tmp, 0, hidOutBuffer, DATA_WITH_PACKET_NO_OFFSET, tmp.Length);
            hidOutBuffer[DATA_OFFSET + length - 1] = 0;
            hidOutBuffer[DATA_OFFSET + length] = Lib.CalcCrc(hidOutBuffer, DATA_OFFSET, length);
            fileDataOffset = -1;
            readingReportFile = true;
            lastLogRead = false;
            SendHIDData(hidOutBuffer, 0, HEADER_SIZE + length + CRC_SIZE, 600);

            while (readingReportFile)
            {
                System.Threading.Thread.Sleep(10);
            }

            return true;
        }

        private void ReadLogFromHardDisk(string path)
        {
            string fileName;
            byte[] bytes;
            int offset;
            int length;
            byte[] reportBytes;
            
            lastLogRead = false;
            fileName = logPath + "\\" + path;
            try
            {
                bytes = File.ReadAllBytes(fileName);
                if (bytes != null)
                {
                    offset = 0;
                    length = bytes.Length;
                    lastLogOffset = (length / SPEED_RECORD_SIZE) * SPEED_RECORD_SIZE;
                    if (lastLogOffset == length)
                    {
                        lastLogOffset = length - SPEED_RECORD_SIZE;
                    }
                    reportBytes = new byte[SPEED_RECORD_SIZE];
                    while (offset < length)
                    {
                        Buffer.BlockCopy(bytes, offset, reportBytes, 0, SPEED_RECORD_SIZE);
                        currentReport[currentDriver].AddReport(currentReportDate, reportBytes);
                        offset += logInterval * SPEED_RECORD_SIZE;
                        if (offset == lastLogOffset)
                        {
                            lastLogRead = true;
                        }
                    }
                    // read last log
                    if (!lastLogRead)
                    {
                        Buffer.BlockCopy(bytes, lastLogOffset, reportBytes, 0, SPEED_RECORD_SIZE);
                        currentReport[currentDriver].AddReport(currentReportDate, reportBytes);
                    }
                }
            }
            catch
            { }

            return;
        }
        #endregion

        #region process hid data
        private void ProcessReceivedHIDData()
        {
            byte crc;
            int length;
            byte opcode;
            UInt32 packetNo;
            byte[] bTemp;

            // should have something in the buffer
            length = BitConverter.ToInt16(hidInBuffer, LENGTH_OFFSET);
            if (length <= 0)
            {
                return;
            }
            // validate the CRC
            crc = hidInBuffer[HEADER_SIZE + length];
            if (crc != Lib.CalcCrc(hidInBuffer, DATA_OFFSET, length))
            {
                return;
            }

            opcode = hidInBuffer[OPCODE_OFFSET];
            switch (opcode)
            {
                case BOOT_MODE_OPCODE:
                    bootMode = true;
                    this.Invoke((MethodInvoker)delegate
                    {
                        LoadBootModeLayout();
                    });
                    break;

                case READ_CONFIG_OPCODE:          // config data
                    packetNo = BitConverter.ToUInt32(hidInBuffer, PACKET_NO_OFFSET);

                    if (packetNo == FINISH_PACKET_NO)
                    {
                        cfg = (TRACKER_CONFIG_DATA)Lib.ByteArrayToObject(cfgData, typeof(TRACKER_CONFIG_DATA));
                    }
                    if (packetNo == ERROR_PACKET_NO)
                    {
                        return;
                    }
                    if (packetNo == DEFAULT_PACKET_NO)
                    {
                        cfgDataSize = BitConverter.ToUInt32(hidInBuffer, DATA_WITH_PACKET_NO_OFFSET);
                        cfgDeviceBuffSize = BitConverter.ToUInt32(hidInBuffer, DATA_WITH_PACKET_NO_OFFSET + 4);
                        cfgDataOffset = 0;
                        length = (int)cfgDeviceBuffSize;
                    }
                    else
                    {
                        Array.Copy(hidInBuffer, DATA_WITH_PACKET_NO_OFFSET, cfgData, packetNo, length - PACKET_NO_SIZE);
                        cfgDataOffset = (int)packetNo + length - PACKET_NO_SIZE;
                        length = (int)cfgDeviceBuffSize;
                        if (cfgDataOffset > cfgDataSize - cfgDeviceBuffSize)
                            length = (int)cfgDataSize - cfgDataOffset;
                    }

                    if (cfgDataOffset >= cfgDataSize)         // last packet
                    {
                        cfg = (TRACKER_CONFIG_DATA)Lib.ByteArrayToObject(cfgData, typeof(TRACKER_CONFIG_DATA));
                        // read config successfully
                        if (bootMode)
                        {
                            bootMode = false;
                            this.Invoke((MethodInvoker)delegate
                            {
                                LoadAppModeLayout();
                            });
                        }

                        return;
                    }
                    CreateHIDBuffer(PACKET_NO_OFFSET + 2, READ_CONFIG_OPCODE, (UInt32)cfgDataOffset);
                    Array.Copy(BitConverter.GetBytes(length), 0, hidOutBuffer, DATA_WITH_PACKET_NO_OFFSET, 2);
                    hidOutBuffer[SIMPLE_CRC_WITH_PACKET_NO_OFFSET + 2] = Lib.CalcCrc(hidOutBuffer, PACKET_NO_OFFSET, PACKET_NO_SIZE + 2);
                    SendHIDData(hidOutBuffer, 0, SIMPLE_PACKET_SIZE + 2);
                    break;
                case UPLOAD_LOG_OPCODE:          // file data
                    packetNo = BitConverter.ToUInt32(hidInBuffer, PACKET_NO_OFFSET);
                    if (packetNo == ERROR_PACKET_NO)
                    {
                        // file is not exist
                        readingReportFile = false;
                        return;
                    }
                    else if (packetNo == DEFAULT_PACKET_NO)
                    {
                        cfgDataSize = BitConverter.ToUInt32(hidInBuffer, DATA_WITH_PACKET_NO_OFFSET);
                        cfgDeviceBuffSize = BitConverter.ToUInt32(hidInBuffer, DATA_WITH_PACKET_NO_OFFSET + 4);
                        cfgDataOffset = 0;
                        length = (int)cfgDeviceBuffSize;
                        offsetData = 0;
                        lastLogOffset = (int)(cfgDataSize / SPEED_RECORD_SIZE) * SPEED_RECORD_SIZE;
                        if (lastLogOffset == cfgDataSize)
                        {
                            lastLogOffset = (int)cfgDataSize - SPEED_RECORD_SIZE;
                        }
                    }
                    else
                    {
                        length = BitConverter.ToUInt16(hidInBuffer, LENGTH_OFFSET) - PACKET_NO_SIZE;
                        Array.Copy(hidInBuffer, DATA_WITH_PACKET_NO_OFFSET, myBuff, offsetData, length);
                        offsetData += length;
                        if (offsetData >= SPEED_RECORD_SIZE)
                        {
                            offsetData = 0;
                            bTemp = new byte[SPEED_RECORD_SIZE];
                            Array.Copy(myBuff, 0, bTemp, 0, bTemp.Length);
                            try
                            {
                                currentReport[currentDriver].AddReport(currentReportDate, bTemp);
                            }
                            catch (Exception)
                            { }
                            cfgDataOffset += (int)(logInterval * SPEED_RECORD_SIZE);
                        }
                        if ((cfgDataOffset + offsetData) == lastLogOffset)
                        {
                            lastLogRead = true;
                        }
                        if ((cfgDataOffset + offsetData) >= cfgDataSize)
                        {
                            if (!lastLogRead)
                            {
                                // read the last log in file
                                lastLogRead = true;
                                cfgDataOffset = lastLogOffset;
                                offsetData = 0;
                            }
                            else
                            {
                                // end of file
                                readingReportFile = false;
                                return;
                            }
                        }
                    }
                    CreateHIDBuffer(PACKET_NO_OFFSET, UPLOAD_LOG_OPCODE, (UInt32)(cfgDataOffset + offsetData));
                    hidOutBuffer[SIMPLE_CRC_WITH_PACKET_NO_OFFSET] = Lib.CalcCrc(hidOutBuffer, PACKET_NO_OFFSET, PACKET_NO_SIZE);
                    SendHIDData(hidOutBuffer, 0, SIMPLE_PACKET_SIZE, 1000);
                    break;
                case 0x5A:  // ACK
                    break;
                case 0xA5:  // NACK
                    break;
                default:
                    break;
            }
        }
        #endregion

        private void SendHIDData(byte[] buffer, int offset, int count, int timeout = DEFAULT_COMM_TIMEOUT)
        {
            int retryCount, copyIndex, byteToSend;

            copyIndex = 0;
            while (copyIndex < count)
            {
                byteToSend = count - copyIndex;
                byteToSend = (byteToSend > 64) ? 64 : byteToSend;
                Array.Clear(hidSendBuffer, 0, hidSendBuffer.Length);
                Array.Copy(buffer, offset + copyIndex, hidSendBuffer, 1, byteToSend);
                copyIndex += byteToSend;
                retryCount = 0;
                while (true)
                {
                    try
                    {
                        hidDevice.writeStream.Write(hidSendBuffer, 0, hidSendBuffer.Length);
                        break;
                    }
                    catch (Exception)
                    {
                        if (retryCount++ > MAX_COMM_RETRY)
                        {
                            //ReportHIDWriteTimedOut();
                            return;
                        }
                    }
                }
            }
            if (timeout > DEFAULT_COMM_TIMEOUT)
            { 
                //TODO: start timeout timer here
            }
        }

        private int GetReportInterval()
        { 
            switch (logIntervalCmbBox.SelectedIndex)
            {
                case 0:
                    return 1;
                case 1:
                    return 5;
                case 2:
                    return 10;
                case 3:
                    return 30;
                case 4:
                    return 60;
                case 5:
                    return 60 * 5;
                case 6:
                    return 60 * 10;
                case 7:
                    return 60 * 30;
                default:
                    return 30;
            }
        }

        private void dateTimePickerBegin_CloseUp(object sender, EventArgs e)
        {
            DateTime beginSelect = reportBeginDatePicker.Value;
            reportBeginDatePicker.Value = new DateTime(beginSelect.Year, beginSelect.Month,beginSelect.Day,0,0,0);
        }

        private void dateTimePickerEnd_CloseUp(object sender, EventArgs e)
        {
            DateTime endSelect = reportEndDatePicker.Value;
            reportEndDatePicker.Value = new DateTime(endSelect.Year, endSelect.Month, endSelect.Day, 23, 59, 59);
        }

        private void MainForm_FormClosing(object sender, FormClosingEventArgs e)
        {
            Environment.Exit(0);
        }

        private void outputReportToKMLBtn_Click(object sender, EventArgs e)
        {
            string fileName;
            DialogResult res;

            if (reports.Count == 0)
            {
                return;
            }
            saveFileDialog1.Filter = "KML file (*.KML)|*.KML";
            res = saveFileDialog1.ShowDialog();
            if (res != DialogResult.OK)
            {
                return;
            }
            fileName = saveFileDialog1.FileName;
            IndividualReport.GenerateKML(cfg, reports, fileName);
            //"Completed! The KML file is saved at: "
            MessageBox.Show(KML_SAVE_SUCCESS_STR.Text + fileName, APP_CAPTION_STR.Text);
        }

        private void readReportFromDeviceBtn_Click(object sender, EventArgs e)
        {
            if (!hidDevice.connected)
            {
                return;
            }
            logInterval = GetReportInterval();
            ClearAllReport();
            reportGrpBox.Enabled = false;
            UpdateProgressBar(true);
            downloader = new System.Threading.Thread(RetrieveLogData);
            downloader.Start(READ_FROM_DEVICE);
        }

        private void readReportFromHardDiskBtn_Click(object sender, EventArgs e)
        {
            DialogResult res;

            res = selectFolderDialog.ShowDialog();
            if (res != DialogResult.OK)
            {
                return;
            }
            logInterval = GetReportInterval();
            logPath = selectFolderDialog.SelectedPath;
            ClearAllReport();
            reportGrpBox.Enabled = false;
            UpdateProgressBar(true);
            downloader = new System.Threading.Thread(RetrieveLogData);
            downloader.Start(READ_FROM_HARDDISK);
        }

        private void clearReportBtn_Click(object sender, EventArgs e)
        {
            ClearAllReport();
        }

        private void ClearAllReport()
        {
            reports.Clear();
            ClearReportGirdView();
            reportDriverNameCmbBox.Items.Clear();
            Array.Clear(reportDriverNameAdded, 0, 10);
        }

        private void UpdateReportUI()
        {
            GenerateReport();
        }

        private void GenerateReport()
        {
            MySpeedRecord record;
            bool driverNameSelected;

            driverNameSelected = false;
            for (int i = 0; i < 10; i++)
            {
                if ((cfg.driverList[i].driverName == null) || (cfg.driverList[i].driverName.Equals(string.Empty)))
                {
                    reportDriverNameCmbBox.Items.Add("Driver " + (i + 1).ToString());
                }
                else
                {
                    reportDriverNameCmbBox.Items.Add(cfg.driverList[i].driverName);
                }
            }

            foreach (IndividualReport[] reportArr in reports)
            {
                for (currentDriver = 0; currentDriver < 10; currentDriver++)
                {
                    if (reportArr[currentDriver].speedRecords.Count == 0)
                    {
                        continue;
                    }
                    if (!driverNameSelected)
                    {
                        driverNameSelected = true;
                        reportDriverNameCmbBox.SelectedIndex = currentDriver;
                    }
                    record = reportArr[currentDriver].speedRecords[reportArr[currentDriver].speedRecords.Count - 1];
                    summaryReportGridView.Rows.Add(summaryReportGridView.Rows.Count + 1,
                                                    cfg.plateNo,
                                                    cfg.driverList[currentDriver].driverName,
                                                    cfg.driverList[currentDriver].licenseNo,
                                                    (record.GetBeginTime() != DateTime.MinValue) ? record.GetBeginTime().ToString("dd/MM/yyyy HH:mm:ss") : "n.a",
                                                    (record.GetEndTime() != DateTime.MinValue) ? record.GetEndTime().ToString("dd/MM/yyyy HH:mm:ss") : "n.a",
                                                    record.record.overSpeedCount,
                                                    record.record.doorOpenCount,
                                                    record.record.doorCloseCount,
                                                    record.record.engineOnCount,
                                                    record.record.engineOffCount,
                                                    record.record.parkingCount,
                                                    new TimeSpan(0, 0, (int)record.record.drivingTime).ToString(),
                                                    record.record.overDrivingTimeCount,
                                                    new TimeSpan(0, 0, (int)record.record.totalDrivingTime).ToString(),
                                                    record.record.overTotalDrivingTimeCount,
                                                    record.record.mileage);
                }
            }
        }

        private void ClearReportGirdView()
        {
            detailReportGridView.Rows.Clear();
            summaryReportGridView.Rows.Clear();
        }

        private void exitToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Environment.Exit(0);
        }

        private void usbMenuItem_Click(object sender, EventArgs e)
        {
            if (hidDevice.connected)
            {
                return;
            }

            commuTypeLbl.Text = "USB";
            HID.RegisterDeviceNotification(this.Handle, ref notificationHandle);
            OpenHIDDevice();

            // close COM port
            if ((serialPort != null) && (serialPort.IsOpen))
            {
                serialPort.Close();
                serialPort = null;
            }
        }

        private void reportDriverNameCmbBox_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (reportDriverNameCmbBox.Items.Count == 0)
            {
                return;
            }

            currentDriver = reportDriverNameCmbBox.SelectedIndex;
            detailReportGridView.Rows.Clear();
            foreach (IndividualReport[] reportArr in reports)
            {
                foreach (MySpeedRecord record in reportArr[currentDriver].speedRecords)
                {
                    detailReportGridView.Rows.Add(detailReportGridView.Rows.Count + 1,
                                                    record.GetCurrentTime(),
                                                    record.record.lng,
                                                    record.record.lat,
                                                    record.record.speed,
                                                    ((record.record.status | DOOR_STATE_MASK) != 0) ? "Open" : "Close",
                                                    record.record.mileage,
                                                    GOOGLE_MAP_LINK_PREFIX + record.record.lat + "," + record.record.lng);
                }
            }
        }

        private IndividualReport[] CreateNewDayReport()
        {
            IndividualReport[] rp;

            rp = new IndividualReport[10];
            for (int i = 0; i < rp.Length; i++)
            {
                rp[i] = new IndividualReport();
                rp[i].driver = i;
            }

            return rp;
        }

        private void englishToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ChangeLanguage("en");
        }

        private void vietnameseToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ChangeLanguage("vi");
        }

        ResourceString testStr = new ResourceString();

        private void ChangeLanguage(string lang)
        {
            ComponentResourceManager resources = new ComponentResourceManager(typeof(MainForm));
            CultureInfo culture = new CultureInfo(lang);

            foreach (Control c in this.Controls)
            {

                ApplyControlResources(c, resources, culture);
            }
            foreach (ToolStripMenuItem item in menuStrip1.Items)
            {
                ApplyMenuItemResources(item, resources, culture);
            }
            foreach (TabPage tab in tabMain.TabPages)
            {
                resources.ApplyResources(tab, tab.Name, culture);
            }
            foreach (DataGridViewColumn col in summaryReportGridView.Columns)
            {
                resources.ApplyResources(col, col.Name, culture);
            }
            foreach (DataGridViewColumn col in detailReportGridView.Columns)
            {
                resources.ApplyResources(col, col.Name, culture);
            }

            LoadLocalizedString(lang);
            UpdateUI();
            ApplyLayoutChanges();
        }

        private void LoadLocalizedString(string lang)
        {
            ResourceManager rm;

            if (lang.Equals("vi"))
            {
                rm = Resources.ViStrings.ResourceManager;
            }
            else
            {
                rm = Resources.EnStrings.ResourceManager;
            }

            for (int i = 0; i < localizedStrings.Count; i++)
            {
                localizedStrings[i].Text = rm.GetString(localizedStrings[i].Name);
            }

            logIntervalCmbBox.Items[4] = LOG_INTERVAL_1_MIN_STR.Text;
            logIntervalCmbBox.Items[5] = LOG_INTERVAL_5_MIN_STR.Text;
            logIntervalCmbBox.Items[6] = LOG_INTERVAL_10_MIN_STR.Text;
            logIntervalCmbBox.Items[7] = LOG_INTERVAL_30_MIN_STR.Text;
            openFileDialog1.Title = APP_CAPTION_STR.Text + " - " + OPEN_FILE_DIALOG_TITLE_STR.Text;
            saveFileDialog1.Title = APP_CAPTION_STR.Text + " - " + SAVE_FILE_DIALOG_TITLE_STR.Text;
            selectFolderDialog.Description = APP_CAPTION_STR.Text + " - " + OPEN_FOLDER_DIALOG_TITLE_STR.Text;
        }

        private void ApplyControlResources(Control control, ComponentResourceManager resources, CultureInfo culture)
        {
            resources.ApplyResources(control, control.Name, culture);
            foreach (Control c in control.Controls)
            {
                ApplyControlResources(c, resources, culture);
            }
        }

        private void ApplyMenuItemResources(ToolStripMenuItem item, ComponentResourceManager resources, CultureInfo culture)
        {
            resources.ApplyResources(item, item.Name, culture);
            foreach (ToolStripMenuItem i in item.DropDownItems)
            {
                ApplyMenuItemResources(i, resources, culture);
            }
        }

        private void ApplyLayoutChanges()
        {
            tabMain.Height = statusGrpBox.Top - 106;
            tabMain.Width = statusGrpBox.Width - 10;
        }

        private List<byte> ReadAllByteInFile(string fileName)
        {
            BinaryReader reader;
            long fileSize;
            List<byte> buffer;
            byte[] tempBuf;
            int bufIndex;

            reader = null;
            try
            {
                reader = new BinaryReader(new FileStream(fileName, FileMode.Open));
            }
            catch (Exception)
            {
                return null;
            }
            if (reader == null)
            {
                return null;
            }

            fileSize = reader.BaseStream.Length;
            buffer = new List<byte>();
            bufIndex = 0;
            while (bufIndex < fileSize)
            {
                tempBuf = reader.ReadBytes(1024);
                buffer.AddRange(tempBuf);
                bufIndex += tempBuf.Length;
            }
            reader.Close();
            reader = null;

            return buffer;
        }

        private void registerToolStripMenuItem_Click(object sender, EventArgs e)
        {
            RegisterForm rForm;
            
            rForm = new RegisterForm();
            rForm.ShowDialog();
            registerToolStripMenuItem.Visible = !rForm.isRegistered;
            rForm.Dispose();
            rForm = null;
        }

        private void detailReportGridView_CellContentClick(object sender, DataGridViewCellEventArgs e)
        {
            if ((e.ColumnIndex == googleLinkCol.Index) && (e.RowIndex >= 0))
            {
                System.Diagnostics.Process.Start(detailReportGridView.Rows[e.RowIndex].Cells[e.ColumnIndex].Value.ToString());
            }
        }
    }

    public class ResourceString
    {
        string name;
        string text;

        public string Name
        {
            get { return name; }
            set { name = value; }
        }

        public string Text
        {
            get { return text; }
            set { text = value; }
        }
    }
}
