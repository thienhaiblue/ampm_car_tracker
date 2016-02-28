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
using System.Linq;

namespace GT400VNTrackerConfiguration
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

        const byte DISABLE_LOG_OPCODE = 0x01;
        const byte ENABLE_LOG_OPCODE = 0x02;
        const byte READ_LOG_OPCODE = 0x03;
        const byte ENABLE_GPS_LOG_OPCODE = 0x04;
        const byte ENABLE_GSM_LOG_OPCODE = 0x05;
        const byte ENABLE_MCU_LOG_OPCODE = 0x06;
        const byte READ_CONFIG_OPCODE = 0x0C;
        const byte WRITE_CONFIG_OPCODE = 0x0D;

        const byte READ_FIRMWARE_VERSION_OPCODE = 0x11;
        const byte DOWNLOAD_FIRMWARE_OPCODE = 0x12;
        const byte SET_CAMERA_ID_OPCODE = 0x13;
        const byte SEND_SMS_OPCODE = 0x14;

        const byte BOOT_MODE_OPCODE = 0x33;
        const byte JUMP_TO_APP_OPCODE = 0xA5;
        const byte UPLOAD_LOG_OPCODE = 0x0F;
        const byte RESET_FACTORY_OPCODE = 0xFD;
        const byte RESET_DEVICE_OPCODE = 0xFE;
        const byte PRINT_REPORT_OPCODE = 0x91;
        const byte FORMAT_SDCARD_OPCODE = 0xFB;

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

        const int READ_FROM_DEVICE = 0;
        const int READ_FROM_HARDDISK = 1;

        const int USB_COMMUNICATION = 0;
        const int COM_COMMUNICATION = 1;

        const int DATA_SERVER_SELECTED = 0;
        //const int FIRMWARE_SERVER_SELECTED = 1;
        //const int INFO_SERVER_SELECTED = 2;
        const int IMAGE_SERVER_SELECTED = 1;

        const int SERVER_USE_IP = 1;
        const int USE_PRIMARY_SERVER = 0;
        const int GPS_SPEED_SOURCE = 1;
        const int ANALOG_INPUT_1 = 0;
        const int ANALOG_INPUT_2 = 1;
        const int ANALOG_PERCENTAGE_MODE = 1;

        const int CAMERA_TRIGGER_INTERVAL = 0;
        const int CAMERA_TRIGGER_DI1 = 1;
        const int CAMERA_TRIGGER_DI2 = 2;
        const int CAMERA_TRIGGER_DI3 = 3;
        const int CAMERA_TRIGGER_DI4 = 4;
        const int INTERVAL_TRIGGER_MASK = 0x0100;
        const int DI1_TRIGGER_MASK = 0x01 | 0x02;
        const int DI2_TRIGGER_MASK = 0x04 | 0x08;
        const int DI3_TRIGGER_MASK = 0x10 | 0x20;
        const int DI4_TRIGGER_MASK = 0x40 | 0x80;

        const UInt32 DOOR_STATE_MASK = 0x02;

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
        byte[] comInBuffer = new byte[1024];
        byte[] comOutBuffer = new byte[1024];
        byte[] comTempBuffer = new byte[1024];
        byte[] myBuff = new byte[1024];
        ushort vid1 = 0x2047;
        ushort pid1 = 0x0301;
        ushort vid2 = 0x2047;
        ushort pid2 = 0x0319;
        ushort boot_vid = 0xC251;
        ushort boot_pid = 0x2201;
        // boot id for test only
        //ushort boot_vid = 0x1256;
        //ushort boot_pid = 0x170A;
        byte[] cfgData = new byte[4096];
        byte[] smsData = new byte[Marshal.SizeOf(typeof(SMS_RECORD))];
        int  offsetData;
        int cfgDataOffset;
        UInt32 cfgDataSize;
        UInt32 cfgDeviceBuffSize;
        UInt32 cfgNumByteToSend;
        UInt32 fDeviceBuffSize;
        TRACKER_CONFIG_DATA cfg;
        TRACKER_CONFIG_DATA tempCfg;
        string firmwareVersion;
        BinaryReader firmwareReader;
        bool downloadingFirmware;
        string logPath;
        SerialPort serialPort;
        int communicationType;
        int hidBufIndex = 0;
        int comBufIndex = 0;
        int fileDataOffset;
        int logDataLength;
        int logInterval;
        int lastLogOffset;
        bool lastLogRead;
        List<IndividualReport[]> reports;
        IndividualReport[] currentReport;
        int currentDriver;
        DateTime currentReportDate;
        bool configUpdateDisabled;
        System.Timers.Timer comTimeoutTimer;
        int currentOperation;
        byte[] comRetryBuffer;
        int comRetryTimeout;
        int comRetryCount;
        bool[] reportDriverNameAdded;
        string conLogStr;
        DateTime prevLogReceivedTime;
        bool bootMode;

        bool readLastLog;
        bool readingReportFile;

        private void MainForm_Load(object sender, EventArgs e)
        {
            registerToolStripMenuItem.Visible = isInTrialPeriod;
            configUpdateDisabled = true;

            communicationType = USB_COMMUNICATION;
            serialPort = new SerialPort();

            cfg = new TRACKER_CONFIG_DATA();
            cfg.driverList = new DRIVER_INFO[10];
            tempCfg = new TRACKER_CONFIG_DATA();
            tempCfg.driverList = new DRIVER_INFO[10];
            for (int i = 0; i < 10; i++)
            {
                cfg.driverList[i] = new DRIVER_INFO();
                tempCfg.driverList[i] = new DRIVER_INFO();
            }

            commTypeCmbBox.SelectedIndex = USB_COMMUNICATION;
            logIntervalCmbBox.SelectedIndex = 2;
            selectLogTypeCmbBox.SelectedIndex = 0;

            reports = new List<IndividualReport[]>();
            reportDriverNameAdded = new bool[10];

            comTimeoutTimer = new System.Timers.Timer();
            comTimeoutTimer.AutoReset = false;
            comTimeoutTimer.Elapsed += new System.Timers.ElapsedEventHandler(comTimeoutTimer_Elapsed);

            firmwareVersion = "";
            firmwareVersionLbl.Text = "";

            prevLogReceivedTime = DateTime.Now;

            bootMode = false;

            // initialize localized strings
            InitLocalizedStrings();

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

            ChangeLanguage("vi");
        }

        private void comTimeoutTimer_Elapsed(object sender, System.Timers.ElapsedEventArgs e)
        {
            if (comRetryCount++ > MAX_COMM_RETRY)
            {
                comRetryCount = 0;
                this.Invoke((MethodInvoker)delegate
                {
                    ReportCOMWriteTimedOut();
                });
                return;
            }
            SendCOMData(comRetryBuffer, 0, comRetryBuffer.Length, comRetryTimeout);
        }

        private void ReportCOMWriteTimedOut()
        {
            if (currentOperation == UPLOAD_LOG_OPCODE)
            {
                readingReportFile = false;
                return;
            }
            if (currentOperation == READ_CONFIG_OPCODE)
            {
                MessageBox.Show(READ_CONFIG_FAILED_STR.Text, APP_CAPTION_STR.Text);
                EnableDeviceCmd(true);
                return;
            }
            if (currentOperation == WRITE_CONFIG_OPCODE)
            {
                MessageBox.Show(WRITE_CONFIG_FALIED_STR.Text, APP_CAPTION_STR.Text);
                EnableDeviceCmd(true);
                return;
            }
            if (currentOperation == DOWNLOAD_FIRMWARE_OPCODE)
            {
                if (downloadingFirmware)
                {
                    downloadingFirmware = false;
                    EnableDeviceCmd(true);
                    UpdateProgressBar(false);
                    MessageBox.Show(FIRMWARE_DOWNLOADED_FAILED_STR.Text, APP_CAPTION_STR.Text);
                }
                return;
            }
        }

        private void OpenHIDDevice()
        {
            string[] devicePathName = new string[0];

            if (HID.FindDevices(vid1, pid1, ref devicePathName) > 0)
            {
                hidDevice.pathName = devicePathName[0];

                if (HID.OpenDevice(ref hidDevice))
                {
                    hidDevice.readStream.BeginRead(hidReceivedBuffer, 0, hidReceivedBuffer.Length, new AsyncCallback(hidDevice_DataReceived), null);
                    ReadConfig();
                }
            }
            else if (HID.FindDevices(vid2, pid2, ref devicePathName) > 0)
            {
                hidDevice.pathName = devicePathName[0];

                if (HID.OpenDevice(ref hidDevice))
                {
                    hidDevice.readStream.BeginRead(hidReceivedBuffer, 0, hidReceivedBuffer.Length, new AsyncCallback(hidDevice_DataReceived), null);
                    ReadConfig();
                }
            }
            else if (HID.FindDevices(boot_vid, boot_pid, ref devicePathName) > 0)
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
            if (communicationType == USB_COMMUNICATION)
            {
                if (!hidDevice.connected)
                {
                    return;
                }

                CreateHIDBuffer(PACKET_NO_SIZE, READ_CONFIG_OPCODE, DEFAULT_PACKET_NO);
                hidOutBuffer[SIMPLE_CRC_WITH_PACKET_NO_OFFSET] = calcCRC(hidOutBuffer, PACKET_NO_OFFSET, PACKET_NO_SIZE);
                cfgDataOffset = 0;
                cfgDeviceBuffSize = 0;
                cfgDataSize = 0;
                SendHIDData(hidOutBuffer, 0, SIMPLE_PACKET_SIZE, 1000);
            }
            else
            {
                if (!serialPort.IsOpen)
                {
                    return;
                }

                CreateCOMBuffer(PACKET_NO_SIZE, READ_CONFIG_OPCODE, DEFAULT_PACKET_NO);
                comOutBuffer[SIMPLE_CRC_WITH_PACKET_NO_OFFSET] = calcCRC(comOutBuffer, PACKET_NO_OFFSET, PACKET_NO_SIZE);
                cfgDataOffset = 0;
                cfgDeviceBuffSize = 0;
                cfgDataSize = 0;
                SendCOMData(comOutBuffer, 0, SIMPLE_PACKET_SIZE, 1000);
            }
        }

        private void SendSMS()
        {
            SMS_RECORD sms;

            if (communicationType == USB_COMMUNICATION)
            {
                if (!hidDevice.connected)
                {
                    return;
                }

                sms = new SMS_RECORD();
                sms.smsAddress = smsAddressTxt.Text;
                sms.smsMessage = smsMessageTxt.Text;
                CreateHIDBuffer(PACKET_NO_SIZE + SMS_SIZE, SEND_SMS_OPCODE, DEFAULT_PACKET_NO);
                Buffer.BlockCopy(Lib.ObjectToByteArray(sms), 0, hidOutBuffer, DATA_WITH_PACKET_NO_OFFSET, SMS_SIZE);
                hidOutBuffer[DATA_WITH_PACKET_NO_OFFSET + SMS_SIZE] = calcCRC(hidOutBuffer, DATA_OFFSET, PACKET_NO_SIZE + SMS_SIZE);
                SendHIDData(hidOutBuffer, 0, HEADER_SIZE + PACKET_NO_SIZE + SMS_SIZE + CRC_SIZE, 1000);
            }
            else
            {
                if (!serialPort.IsOpen)
                {
                    return;
                }

                sms = new SMS_RECORD();
                sms.smsAddress = smsAddressTxt.Text;
                sms.smsMessage = smsMessageTxt.Text;
                CreateCOMBuffer(PACKET_NO_SIZE + SMS_SIZE, SEND_SMS_OPCODE, DEFAULT_PACKET_NO);
                Buffer.BlockCopy(Lib.ObjectToByteArray(sms), 0, comOutBuffer, DATA_WITH_PACKET_NO_OFFSET, SMS_SIZE);
                comOutBuffer[DATA_WITH_PACKET_NO_OFFSET + SMS_SIZE] = calcCRC(comOutBuffer, DATA_OFFSET, PACKET_NO_SIZE + SMS_SIZE);
                SendCOMData(comOutBuffer, 0, HEADER_SIZE + PACKET_NO_SIZE + SMS_SIZE + CRC_SIZE, 1000);
            }
        }

        private void CreateHIDBuffer(int length, byte opcode, UInt32 packetNo)
        {
            Array.Clear(hidOutBuffer, 0, hidOutBuffer.Length);
            hidOutBuffer[START_CODE_OFFSET] = COMM_START_CODE;
            Array.Copy(BitConverter.GetBytes(length), 0, hidOutBuffer, LENGTH_OFFSET, LENGTH_SIZE);
            hidOutBuffer[OPCODE_OFFSET] = opcode;
            Array.Copy(BitConverter.GetBytes(packetNo), 0, hidOutBuffer, PACKET_NO_OFFSET, PACKET_NO_SIZE);
            currentOperation = opcode;
        }

        private void CreateCOMBuffer(int length, byte opcode, UInt32 packetNo)
        {
            Array.Clear(comOutBuffer, 0, comOutBuffer.Length);
            comOutBuffer[START_CODE_OFFSET] = COMM_START_CODE;
            Array.Copy(BitConverter.GetBytes(length), 0, comOutBuffer, LENGTH_OFFSET, LENGTH_SIZE);
            comOutBuffer[OPCODE_OFFSET] = opcode;
            Array.Copy(BitConverter.GetBytes(packetNo), 0, comOutBuffer, PACKET_NO_OFFSET, PACKET_NO_SIZE);
            currentOperation = opcode;
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
                if (communicationType != USB_COMMUNICATION)
                {
                    return;
                }
                hidDevice.readStream.BeginRead(hidReceivedBuffer, 0, hidReceivedBuffer.Length, new AsyncCallback(hidDevice_DataReceived), null);
            }
            catch
            {
            }
        }

        private void UpdateTextBox()
        {
            int length;
            string str;

            if (communicationType == USB_COMMUNICATION)
            {
                length = BitConverter.ToInt16(hidInBuffer, LENGTH_OFFSET);
                str = System.Text.Encoding.ASCII.GetString(hidInBuffer, DATA_OFFSET, length);
                if (DateTime.Now.CompareTo(prevLogReceivedTime.AddMilliseconds(LOG_TIME_THRESHOLD)) < 0)
                {
                    conLogStr += str;
                    return;
                }
                else
                {
                    prevLogReceivedTime = DateTime.Now;
                    str = conLogStr + str;
                    conLogStr = "";
                }
            }
            else
            {
                length = BitConverter.ToInt16(comInBuffer, LENGTH_OFFSET);
                str = System.Text.Encoding.ASCII.GetString(comInBuffer, DATA_OFFSET, length);
            }

            lock (logTxtBox)
            {
                logTxtBox.Text += str;
                logTxtBox.SelectionStart = logTxtBox.Text.Length;
                logTxtBox.ScrollToCaret();
            }
        }

        private void UpdateConfigUI()
        {
            configUpdateDisabled = true;

            tempCfg = cfg;

            // GPRS config
            gprsApnTxtBox.Text = cfg.gprsApn;
            gprsUserTxtBox.Text = cfg.gprsUser;
            gprsPwdTxtBox.Text = cfg.gprsPass;
            gprsRunReportIntervalTxtBox.Text = cfg.runReportInterval.ToString();
            gprsStopReportIntervalTxtBox.Text = cfg.stopReportInterval.ToString();

            // server parameter
			serverSelectionCmbBox.SelectedIndex = 0;
			serverSelection_Changed(null, null);

            // vehicle config
            trackerIdTxtBox.Text = cfg.trackerId;
            plateNoTxtBox.Text = cfg.plateNo;
            vinNumTxtBox.Text = cfg.vin;
            bossPhoneTxtBox.Text = cfg.whiteCallerNo;
            tyreRateTxtBox.Text = cfg.speedSensorRatio.ToString();
            if (cfg.useGpsSpeed == GPS_SPEED_SOURCE)
            {
                gpsSpeedSourceRBtn.Checked = true;
            }
            else
            {
                pulseSpeedSourceRBtn.Checked = true;
            }

            // current driver config
            if (cfg.driver < 10)
            {
                driverNoCmbBox.SelectedIndex = cfg.driver;
            }

			// feature set
            
			// other settings
            speedLimitTxtBox.Text = cfg.speedLimit.ToString();
            drivingTimeLimitTxtBox.Text = ((float)cfg.drivingTimeLimit / 3600).ToString();
            totalDrivingTimeLimitTxtBox.Text = ((float)cfg.totalDrivingTimeLimit / 3600).ToString();
            baseMileageTxtBox.Text = cfg.baseMileage.ToString();
			accountAlarmCheckTxt.Text = cfg.accountAlarmCheck.ToString();

            cameraChannelUDBox.Value = 100;
            compressionRateTxt.Text = cfg.cameraCompression.ToString();
            cameraIntervalTxt.Text = cfg.cameraInterval.ToString();
            cameraWorkingStartTxt.Text = cfg.cameraWorkingStartTime.ToString();
            cameraWorkingStopTxt.Text = cfg.cameraWorkingStopTime.ToString();
            if ((cfg.cameraEvents & INTERVAL_TRIGGER_MASK) == INTERVAL_TRIGGER_MASK)
            {
                triggerSourceChkList.SetItemCheckState(CAMERA_TRIGGER_INTERVAL, CheckState.Checked);
            }
            else
            {
                triggerSourceChkList.SetItemCheckState(CAMERA_TRIGGER_INTERVAL, CheckState.Unchecked);
            }
            if ((cfg.cameraEvents & DI1_TRIGGER_MASK) == DI1_TRIGGER_MASK)
            {
                triggerSourceChkList.SetItemCheckState(CAMERA_TRIGGER_DI1, CheckState.Checked);
            }
            else
            {
                triggerSourceChkList.SetItemCheckState(CAMERA_TRIGGER_DI1, CheckState.Unchecked);
            }
            if ((cfg.cameraEvents & DI2_TRIGGER_MASK) == DI2_TRIGGER_MASK)
            {
                triggerSourceChkList.SetItemCheckState(CAMERA_TRIGGER_DI2, CheckState.Checked);
            }
            else
            {
                triggerSourceChkList.SetItemCheckState(CAMERA_TRIGGER_DI2, CheckState.Unchecked);
            }
            if ((cfg.cameraEvents & DI3_TRIGGER_MASK) == DI3_TRIGGER_MASK)
            {
                triggerSourceChkList.SetItemCheckState(CAMERA_TRIGGER_DI3, CheckState.Checked);
            }
            else
            {
                triggerSourceChkList.SetItemCheckState(CAMERA_TRIGGER_DI3, CheckState.Unchecked);
            }
            if ((cfg.cameraEvents & DI4_TRIGGER_MASK) == DI4_TRIGGER_MASK)
            {
                triggerSourceChkList.SetItemCheckState(CAMERA_TRIGGER_DI4, CheckState.Checked);
            }
            else
            {
                triggerSourceChkList.SetItemCheckState(CAMERA_TRIGGER_DI4, CheckState.Unchecked);
            }

            analogInputNoCmbBox.SelectedIndex = 1;
            analogInputNoCmbBox.SelectedIndex = 0;

            printHeadTxtBox.Text = cfg.infoStr;

            imeiLbl.Text = "IMEI: " + cfg.imei;

            configUpdateDisabled = false;
        }

        private void UpdateUI()
        {
            if (bootMode)
            {
                if (hidDevice.connected)
                {
                    connectionStatusLbl.Text = "BOOT MODE";
                    connectionStatusLbl.ForeColor = Color.Blue;
                }
                else
                {
                    connectionStatusLbl.Text = DISCONNECTED_STR.Text;
                    connectionStatusLbl.ForeColor = Color.Red;
                }
                runAppBtn.Enabled = true;
            }
            else
            {
                if (communicationType == USB_COMMUNICATION)
                {
                    commTypeCmbBox.SelectedIndex = 0;
                    commuTypeLbl.Text = "USB";
                    if (hidDevice.connected)
                    {
                        deviceCmdGBox.Enabled = true;
                        connectionStatusLbl.Text = CONNECTED_STR.Text;
                        connectionStatusLbl.ForeColor = Color.Green;
                    }
                    else
                    {
                        deviceCmdGBox.Enabled = false;
                        connectionStatusLbl.Text = DISCONNECTED_STR.Text;
                        connectionStatusLbl.ForeColor = Color.Red;
                    }
                }
                else
                {
                    commTypeCmbBox.SelectedIndex = 1;
                    if ((serialPort != null) && serialPort.IsOpen)
                    {
                        deviceCmdGBox.Enabled = true;
                        commuTypeLbl.Text = serialPort.PortName;
                        comPortCmbBox.Text = serialPort.PortName;
                        connectionStatusLbl.Text = CONNECTED_STR.Text;
                        connectionStatusLbl.ForeColor = Color.Green;
                    }
                    else
                    {
                        deviceCmdGBox.Enabled = false;
                        commuTypeLbl.Text = "COM";
                        connectionStatusLbl.Text = DISCONNECTED_STR.Text;
                        connectionStatusLbl.ForeColor = Color.Red;
                    }
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
            commGrpBox.Enabled = false;
            deviceCmdGBox.Enabled = false;
            tabMain.SelectedTab = settingTabPage;
            setCameraIdBtn.Enabled = false;
            sendSMSBtn.Enabled = false;
            readFirmwareVersionBtn.Enabled = false;
            runAppBtn.Enabled = true;
            connectionStatusLbl.Text = "BOOT MODE";
            connectionStatusLbl.ForeColor = Color.Blue;
        }

        void LoadAppModeLayout()
        {
            commGrpBox.Enabled = true;
            deviceCmdGBox.Enabled = true;
            setCameraIdBtn.Enabled = true;
            sendSMSBtn.Enabled = true;
            readFirmwareVersionBtn.Enabled = true;
            runAppBtn.Enabled = false;
            UpdateUI();
        }

        public MainForm(bool isInTrial = false)
        {
            Thread.CurrentThread.CurrentUICulture = new CultureInfo("vi");
            InitializeComponent();
            isInTrialPeriod = isInTrial;
        }

        protected override void WndProc(ref Message m)
        {
            if (m.Msg == Win32.WM_DEVICECHANGE)
            {
                if (communicationType == USB_COMMUNICATION)
                {
                    if (m.WParam.ToInt32() == Win32.DBT_DEVICEARRIVAL)
                    {
                        if ((hidDevice == null) || (!hidDevice.connected))
                        {
                            OpenHIDDevice();
                        }
                    }
                    else if (m.WParam.ToInt32() == Win32.DBT_DEVICEREMOVECOMPLETE)
                    {
                        if (HID.DeviceNameMatch(m, hidDevice.pathName))
                        {
                            CloseHIDDevice();
                        }
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
            currentReportDate = DateTime.MinValue;
            currentReport = CreateNewDayReport();
            do
            {
                currentReportDate = beginTime;
                for (currentDriver = 0; currentDriver < 10; currentDriver++)
                {
                    remotePathName = "LOG/" + beginTime.Year.ToString() + "_" + beginTime.Month.ToString("D2") + "/" + beginTime.Day.ToString("D2");
                    remotePathName += "/" + beginTime.Hour.ToString("D") + "_" + currentDriver.ToString() + ".log";

                    this.Invoke(new InvokerWithOneParam(UpdateDownloadStatus), RETRIEVING_FILE_STR.Text + remotePathName);

                    ReadLogFromHardDisk(remotePathName);

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
                EnableDeviceCmd(true);
				UpdateReportUI();
                UpdateProgressBar(false);
            });
        }

        private void EnableCommTypeChange(bool enabled)
        {
            commGrpBox.Enabled = enabled;
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
                case READ_LOG_OPCODE:             // debug data
                    this.Invoke(new MethodInvoker(UpdateTextBox));
                    break;

                case BOOT_MODE_OPCODE:
                    bootMode = true;
                    this.Invoke((MethodInvoker)delegate
                    {
                        LoadBootModeLayout();
                    });
                    break;

                case WRITE_CONFIG_OPCODE:
                    comTimeoutTimer.Stop();
                    packetNo = BitConverter.ToUInt32(hidInBuffer, PACKET_NO_OFFSET);

                    if (packetNo == FINISH_PACKET_NO)
                    {
                        MessageBox.Show(WRITE_CONFIG_SUCCESS_STR.Text, APP_CAPTION_STR.Text);
                        this.Invoke((MethodInvoker)delegate
                        {
                            EnableDeviceCmd(true);
                        });
                        return;
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
                        cfgNumByteToSend = BitConverter.ToUInt32(hidInBuffer, DATA_WITH_PACKET_NO_OFFSET);
                        cfgDataOffset = (int)packetNo;
                        length = (int)cfgNumByteToSend;
                    }

                    if (cfgDataOffset >= cfgDataSize)         // last packet
                    {
                        MessageBox.Show(WRITE_CONFIG_SUCCESS_STR.Text, APP_CAPTION_STR.Text);
                        this.Invoke((MethodInvoker)delegate
                        {
                            EnableDeviceCmd(true);
                        });
                        return;
                    }
                    CreateHIDBuffer(PACKET_NO_SIZE + length, WRITE_CONFIG_OPCODE, (UInt32)cfgDataOffset);
                    Array.Copy(cfgData, cfgDataOffset, hidOutBuffer, DATA_WITH_PACKET_NO_OFFSET, length);
                    hidOutBuffer[DATA_WITH_PACKET_NO_OFFSET + length] = calcCRC(hidOutBuffer, DATA_OFFSET, PACKET_NO_SIZE + length);
                    SendHIDData(hidOutBuffer, 0, HEADER_SIZE + PACKET_NO_SIZE + length + CRC_SIZE);
                    break;

                case READ_CONFIG_OPCODE:          // config data
                    comTimeoutTimer.Stop();
                    packetNo = BitConverter.ToUInt32(hidInBuffer, PACKET_NO_OFFSET);

                    if (packetNo == FINISH_PACKET_NO)
                    {
                        cfg = (TRACKER_CONFIG_DATA)Lib.ByteArrayToObject(cfgData, typeof(TRACKER_CONFIG_DATA));
                        this.Invoke(new MethodInvoker(UpdateConfigUI));
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
                        this.Invoke(new MethodInvoker(UpdateConfigUI));
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
                    hidOutBuffer[SIMPLE_CRC_WITH_PACKET_NO_OFFSET + 2] = calcCRC(hidOutBuffer, PACKET_NO_OFFSET, PACKET_NO_SIZE + 2);
                    SendHIDData(hidOutBuffer, 0, SIMPLE_PACKET_SIZE + 2);
                    break;
                case UPLOAD_LOG_OPCODE:          // file data
                    comTimeoutTimer.Stop();
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

                case READ_FIRMWARE_VERSION_OPCODE:
                    firmwareVersion = System.Text.Encoding.ASCII.GetString(hidInBuffer, 3, length);
                    this.Invoke((MethodInvoker)delegate
                    {
                        firmwareVersionLbl.Text = firmwareVersion;
                    });
                    break;

                case DOWNLOAD_FIRMWARE_OPCODE:
                    comTimeoutTimer.Stop();
                    if (!downloadingFirmware)
                    {
                        return;
                    }
                    packetNo = BitConverter.ToUInt32(hidInBuffer, PACKET_NO_OFFSET);
                    if (packetNo == FINISH_PACKET_NO)
                    {
                        // complete
                        downloadingFirmware = false;
                        this.Invoke((MethodInvoker)delegate
                        {
                            EnableDeviceCmd(true);
                            UpdateProgressBar(false);
                        });
                        MessageBox.Show(FIRMWARE_DOWNLOADED_SUCCESS_STR.Text, APP_CAPTION_STR.Text);
                        try
                        {
                            firmwareReader.Close();
                            return;
                        }
                        catch (Exception)
                        { }
                    }
                    if (packetNo == DEFAULT_PACKET_NO)
                    {
                        fDeviceBuffSize = BitConverter.ToUInt32(hidInBuffer, DATA_WITH_PACKET_NO_OFFSET + 4);
                        packetNo = 0;
                    }
                    this.Invoke((MethodInvoker)delegate
                    {
                        retrieveLogPrgBar.Value = (int)(packetNo / 100);
                        retrieveLogPercentLbl.Text = (retrieveLogPrgBar.Value * 100 / retrieveLogPrgBar.Maximum).ToString() + "%";
                    });
                    try
                    {
                        byte[] temp;

                        firmwareReader.BaseStream.Seek(packetNo, SeekOrigin.Begin);
                        //bTemp = firmwareReader.ReadBytes((int)fDeviceBuffSize);
                        temp = firmwareReader.ReadBytes((int)fDeviceBuffSize);
                        bTemp = Enumerable.Repeat<byte>(0xFF, (int)fDeviceBuffSize).ToArray();
                        Buffer.BlockCopy(temp, 0, bTemp, 0, temp.Length);
                        length = bTemp.Length + 4;
                        CreateHIDBuffer(length, DOWNLOAD_FIRMWARE_OPCODE, packetNo);
                        Array.Copy(bTemp, 0, hidOutBuffer, DATA_WITH_PACKET_NO_OFFSET, bTemp.Length);
                        hidOutBuffer[length + HEADER_SIZE] = calcCRC(hidOutBuffer, DATA_OFFSET, length);
                        SendHIDData(hidOutBuffer, 0, length + HEADER_SIZE + CRC_SIZE);
                    }
                    catch (Exception)
                    {
                        downloadingFirmware = false;
                        this.Invoke((MethodInvoker)delegate
                        {
                            EnableDeviceCmd(true);
                            UpdateProgressBar(false);
                        });
                        MessageBox.Show(FIRMWARE_DOWNLOADED_FAILED_STR.Text, APP_CAPTION_STR.Text);
                        return;
                    }
                    break;

                case SET_CAMERA_ID_OPCODE:
                    packetNo = BitConverter.ToUInt32(hidInBuffer, PACKET_NO_OFFSET);
                    if (packetNo == FINISH_PACKET_NO)
                    {
                        MessageBox.Show(SET_CAMERA_ID_SUCCESS_STR.Text, APP_CAPTION_STR.Text);
                    }
                    else
                    {
                        MessageBox.Show(SET_CAMERA_ID_FAILED_STR.Text, APP_CAPTION_STR.Text);
                    }
                    break;
                case 0x5A:  // ACK
                    break;
                case 0xA5:  // NACK
                    break;
            }
        }
        #endregion

        #region process com data
        private void ProcessReceivedCOMData()
        {
            byte crc;
            int length;
            byte opcode;
            UInt32 packetNo;
            byte[] bTemp;

            // should have something in the buffer
            length = BitConverter.ToInt16(comInBuffer, LENGTH_OFFSET);
            if (length <= 0)
            {
                return;
            }
            // validate the CRC
            crc = comInBuffer[HEADER_SIZE + length];
            if (crc != calcCRC(comInBuffer, DATA_OFFSET, length))
            {
                return;
            }

            opcode = comInBuffer[OPCODE_OFFSET];
            switch (opcode)
            {
                case READ_LOG_OPCODE:             // debug data
                    this.Invoke(new MethodInvoker(UpdateTextBox));
                    break;

                case WRITE_CONFIG_OPCODE:
                    comTimeoutTimer.Stop();
                    packetNo = BitConverter.ToUInt32(comInBuffer, PACKET_NO_OFFSET);
               
                    if (packetNo == FINISH_PACKET_NO)
                    {
                        MessageBox.Show(WRITE_CONFIG_SUCCESS_STR.Text, APP_CAPTION_STR.Text);
                        this.Invoke((MethodInvoker)delegate
                        {
                            EnableDeviceCmd(true);
                        });
                        return;
                    }
                    if (packetNo == ERROR_PACKET_NO)
                    {
                        return;
                    }
                    if (packetNo == DEFAULT_PACKET_NO)
                    {
                        cfgDataSize = BitConverter.ToUInt32(comInBuffer, DATA_WITH_PACKET_NO_OFFSET);
                        cfgDeviceBuffSize = BitConverter.ToUInt32(comInBuffer, DATA_WITH_PACKET_NO_OFFSET + 4);
                        cfgDataOffset = 0;
                        length = (int)cfgDeviceBuffSize;
                    }
                    else
                    {
                        cfgNumByteToSend = BitConverter.ToUInt32(comInBuffer, DATA_WITH_PACKET_NO_OFFSET);
                        cfgDataOffset = (int)packetNo;
                        length = (int)cfgNumByteToSend;
                    }

                    if (cfgDataOffset >= cfgDataSize)         // last packet
                    {
                        MessageBox.Show(WRITE_CONFIG_SUCCESS_STR.Text, APP_CAPTION_STR.Text);
                        this.Invoke((MethodInvoker)delegate
                        {
                            EnableDeviceCmd(true);
                        });
                        return;
                    }
                    CreateCOMBuffer(PACKET_NO_SIZE + length, WRITE_CONFIG_OPCODE, (UInt32)cfgDataOffset);
                    Array.Copy(cfgData, cfgDataOffset, comOutBuffer, DATA_WITH_PACKET_NO_OFFSET, length);
                    comOutBuffer[DATA_WITH_PACKET_NO_OFFSET + length] = calcCRC(comOutBuffer, DATA_OFFSET, PACKET_NO_SIZE + length);
                    SendCOMData(comOutBuffer, 0, HEADER_SIZE + PACKET_NO_SIZE + length + CRC_SIZE);
                    break;

                case READ_CONFIG_OPCODE:          // config data
                    comTimeoutTimer.Stop();
                    packetNo = BitConverter.ToUInt32(comInBuffer, PACKET_NO_OFFSET);
               
                    if (packetNo == FINISH_PACKET_NO)
                    {
                        cfg = (TRACKER_CONFIG_DATA)Lib.ByteArrayToObject(cfgData, typeof(TRACKER_CONFIG_DATA));
                        this.Invoke(new MethodInvoker(UpdateConfigUI));
                    }
                    if (packetNo == ERROR_PACKET_NO)
                    {
                        return;
                    }
                    if (packetNo == DEFAULT_PACKET_NO)
                    {
                        cfgDataSize = BitConverter.ToUInt32(comInBuffer, DATA_WITH_PACKET_NO_OFFSET);
                        cfgDeviceBuffSize = BitConverter.ToUInt32(comInBuffer, DATA_WITH_PACKET_NO_OFFSET + 4);
                        cfgDataOffset = 0;
                        length = (int)cfgDeviceBuffSize;
                    }
                    else
                    {
                        Array.Copy(comInBuffer, DATA_WITH_PACKET_NO_OFFSET, cfgData, packetNo, length - PACKET_NO_SIZE);
                        cfgDataOffset = (int)packetNo + length - PACKET_NO_SIZE;
                        length = (int)cfgDeviceBuffSize;
                        if (cfgDataOffset > cfgDataSize - cfgDeviceBuffSize)
                            length = (int)cfgDataSize - cfgDataOffset;
                    }

                    if (cfgDataOffset >= cfgDataSize)         // last packet
                    {
                        cfg = (TRACKER_CONFIG_DATA)Lib.ByteArrayToObject(cfgData, typeof(TRACKER_CONFIG_DATA));
                        this.Invoke(new MethodInvoker(UpdateConfigUI));
                        return;
                    }
                    CreateCOMBuffer(PACKET_NO_SIZE + 2, READ_CONFIG_OPCODE, (UInt32)cfgDataOffset);
                    Array.Copy(BitConverter.GetBytes(length), 0, comOutBuffer, DATA_WITH_PACKET_NO_OFFSET, 2);
                    comOutBuffer[SIMPLE_CRC_WITH_PACKET_NO_OFFSET + 2] = calcCRC(comOutBuffer, PACKET_NO_OFFSET, PACKET_NO_SIZE + 2);
                    SendCOMData(comOutBuffer, 0, SIMPLE_PACKET_SIZE + 2);
                    break;

                case READ_FIRMWARE_VERSION_OPCODE:
                    firmwareVersion = System.Text.Encoding.ASCII.GetString(comInBuffer, 3, length);
                    this.Invoke((MethodInvoker)delegate
                    {
                        firmwareVersionLbl.Text = firmwareVersion;
                    });
                    break;

                case DOWNLOAD_FIRMWARE_OPCODE:
                    comTimeoutTimer.Stop();
                    if (!downloadingFirmware)
                    {
                        return;
                    }
                    packetNo = BitConverter.ToUInt32(comInBuffer, PACKET_NO_OFFSET);
                    if (packetNo == FINISH_PACKET_NO)
                    {
                        // complete
                        downloadingFirmware = false;
                        this.Invoke((MethodInvoker)delegate
                        {
                            EnableDeviceCmd(true);
                            UpdateProgressBar(false);
                        });
                        MessageBox.Show(FIRMWARE_DOWNLOADED_SUCCESS_STR.Text, APP_CAPTION_STR.Text);
                        try
                        {
                            firmwareReader.Close();
                            return;
                        }
                        catch (Exception)
                        { }
                    }
                    if (packetNo == DEFAULT_PACKET_NO)
                    {
                        fDeviceBuffSize = BitConverter.ToUInt32(comInBuffer, DATA_WITH_PACKET_NO_OFFSET + 4);
                        packetNo = 0;
                    }
                    this.Invoke((MethodInvoker)delegate
                    {
                        retrieveLogPrgBar.Value = (int)(packetNo / 100);
                        retrieveLogPercentLbl.Text = (retrieveLogPrgBar.Value * 100 / retrieveLogPrgBar.Maximum).ToString() + "%";
                    });
                    try
                    {
                        byte[] temp;

                        firmwareReader.BaseStream.Seek(packetNo, SeekOrigin.Begin);
                        //bTemp = firmwareReader.ReadBytes((int)fDeviceBuffSize);
                        temp = firmwareReader.ReadBytes((int)fDeviceBuffSize);
                        bTemp = Enumerable.Repeat<byte>(0xFF, (int)fDeviceBuffSize).ToArray();
                        Buffer.BlockCopy(temp, 0, bTemp, 0, temp.Length);
                        length = bTemp.Length + 4;
                        CreateCOMBuffer(length, DOWNLOAD_FIRMWARE_OPCODE, packetNo);
                        Array.Copy(bTemp, 0, comOutBuffer, DATA_WITH_PACKET_NO_OFFSET, bTemp.Length);
                        comOutBuffer[length + HEADER_SIZE] = calcCRC(comOutBuffer, DATA_OFFSET, length);
                        SendCOMData(comOutBuffer, 0, length + HEADER_SIZE + CRC_SIZE);
                    }
                    catch (Exception)
                    {
                        downloadingFirmware = false;
                        this.Invoke((MethodInvoker)delegate
                        {
                            EnableDeviceCmd(true);
                            UpdateProgressBar(false);
                        });
                        MessageBox.Show(FIRMWARE_DOWNLOADED_FAILED_STR.Text, APP_CAPTION_STR.Text);
                        return;
                    }
                    break;

                case SET_CAMERA_ID_OPCODE:
                    packetNo = BitConverter.ToUInt32(comInBuffer, PACKET_NO_OFFSET);
                    if (packetNo == FINISH_PACKET_NO)
                    {
                        MessageBox.Show(SET_CAMERA_ID_SUCCESS_STR.Text, APP_CAPTION_STR.Text);
                    }
                    else
                    {
                        MessageBox.Show(SET_CAMERA_ID_FAILED_STR.Text, APP_CAPTION_STR.Text);
                    }
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

        private void SendCOMData(byte[] buffer, int offset, int count, int timeout = DEFAULT_COMM_TIMEOUT)
        {
            int retryCount;

            serialPort.WriteTimeout = timeout;
            retryCount = 0;
            while (true)
            {
                try
                {
                    serialPort.Write(buffer, offset, count);
                    break;
                }
                catch (Exception)
                {
                    if (retryCount++ > MAX_COMM_RETRY)
                    {
                        ReportCOMWriteTimedOut();
                        return;
                    }
                }
            }
            if (timeout > DEFAULT_COMM_TIMEOUT)
            {
                comRetryBuffer = new byte[count];
                Array.Copy(buffer, offset, comRetryBuffer, 0, count);
                comRetryTimeout = timeout;
                comTimeoutTimer.Stop();
                comTimeoutTimer.Interval = timeout;
                comTimeoutTimer.Start();
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

        #region Event handler
        private void printReportBtn_Clicked(object sender, EventArgs e)
        {
            if (communicationType == USB_COMMUNICATION)
            {
                if (!hidDevice.connected)
                {
                    return;
                }

                CreateHIDBuffer(PACKET_NO_SIZE + 6, PRINT_REPORT_OPCODE, DEFAULT_PACKET_NO);
                hidOutBuffer[DATA_WITH_PACKET_NO_OFFSET] = (byte)(reportBeginDatePicker.Value.Year - 2000);
                hidOutBuffer[DATA_WITH_PACKET_NO_OFFSET + 1] = (byte)reportBeginDatePicker.Value.Month;
                hidOutBuffer[DATA_WITH_PACKET_NO_OFFSET + 2] = (byte)reportBeginDatePicker.Value.Day;

                hidOutBuffer[DATA_WITH_PACKET_NO_OFFSET + 3] = (byte)(reportEndDatePicker.Value.Year - 2000);
                hidOutBuffer[DATA_WITH_PACKET_NO_OFFSET + 4] = (byte)reportEndDatePicker.Value.Month;
                hidOutBuffer[DATA_WITH_PACKET_NO_OFFSET + 5] = (byte)reportEndDatePicker.Value.Day;
                hidOutBuffer[DATA_WITH_PACKET_NO_OFFSET + 6] = calcCRC(hidOutBuffer, DATA_OFFSET, PACKET_NO_SIZE + 6);
                SendHIDData(hidOutBuffer, 0, HEADER_SIZE + PACKET_NO_SIZE + 6 + CRC_SIZE);
            }
            else
            {
                if (!serialPort.IsOpen)
                {
                    return;
                }

                CreateCOMBuffer(PACKET_NO_SIZE + 6, PRINT_REPORT_OPCODE, DEFAULT_PACKET_NO);
                comOutBuffer[DATA_WITH_PACKET_NO_OFFSET] = (byte)(reportBeginDatePicker.Value.Year - 2000);
                comOutBuffer[DATA_WITH_PACKET_NO_OFFSET + 1] = (byte)reportBeginDatePicker.Value.Month;
                comOutBuffer[DATA_WITH_PACKET_NO_OFFSET + 2] = (byte)reportBeginDatePicker.Value.Day;

                comOutBuffer[DATA_WITH_PACKET_NO_OFFSET + 3] = (byte)(reportEndDatePicker.Value.Year - 2000);
                comOutBuffer[DATA_WITH_PACKET_NO_OFFSET + 4] = (byte)reportEndDatePicker.Value.Month;
                comOutBuffer[DATA_WITH_PACKET_NO_OFFSET + 5] = (byte)reportEndDatePicker.Value.Day;
                comOutBuffer[DATA_WITH_PACKET_NO_OFFSET + 6] = calcCRC(comOutBuffer, DATA_OFFSET, PACKET_NO_SIZE + 6);
                SendCOMData(comOutBuffer, 0, HEADER_SIZE + PACKET_NO_SIZE + 6 + CRC_SIZE);
            }
        }

        private void readSettingsBtn_Clicked(object sender, EventArgs e)
        {
            ReadConfig();
        }

        private void GetConfigFromUI()
        {
            cfg = tempCfg;
        }

        private void writeSettingsBtn_Clicked(object sender, EventArgs e)
        {
            GetConfigFromUI();

            if (communicationType == USB_COMMUNICATION)
            {
                if (!hidDevice.connected)
                {
                    return;
                }

                EnableDeviceCmd(true);
                cfgData = Lib.ObjectToByteArray(cfg);

                CreateHIDBuffer(PACKET_NO_SIZE, WRITE_CONFIG_OPCODE, DEFAULT_PACKET_NO);
                hidOutBuffer[SIMPLE_CRC_WITH_PACKET_NO_OFFSET] = calcCRC(hidOutBuffer, PACKET_NO_OFFSET, PACKET_NO_SIZE);
                cfgDataOffset = 0;
                cfgDeviceBuffSize = 0;
                cfgDataSize = 0;
                SendHIDData(hidOutBuffer, 0, SIMPLE_PACKET_SIZE, 1000);
            }
            else
            {
                if (!serialPort.IsOpen)
                {
                    return;
                }

                EnableDeviceCmd(false);
                cfgData = Lib.ObjectToByteArray(cfg);

                CreateCOMBuffer(PACKET_NO_SIZE, WRITE_CONFIG_OPCODE, DEFAULT_PACKET_NO);
                comOutBuffer[SIMPLE_CRC_WITH_PACKET_NO_OFFSET] = calcCRC(comOutBuffer, PACKET_NO_OFFSET, PACKET_NO_SIZE);
                cfgDataOffset = 0;
                cfgDeviceBuffSize = 0;
                cfgDataSize = 0;
                SendCOMData(comOutBuffer, 0, SIMPLE_PACKET_SIZE, 1000);
            }
        }

        private void EnableDeviceCmd(bool enabled)
        {
            deviceCmdGBox.Enabled = enabled;
            EnableCommTypeChange(enabled);
            reportGrpBox.Enabled = enabled;
            readFirmwareVersionBtn.Enabled = enabled;
            downloadFirmwareBtn.Enabled = enabled;
        }

        private void resetCPUBtn_Clicked(object sender, EventArgs e)
        {
            if (communicationType == USB_COMMUNICATION)
            {
                if (!hidDevice.connected)
                {
                    return;
                }

                CreateHIDBuffer(PACKET_NO_SIZE, RESET_DEVICE_OPCODE, DEFAULT_PACKET_NO);
                hidOutBuffer[SIMPLE_CRC_WITH_PACKET_NO_OFFSET] = calcCRC(hidOutBuffer, PACKET_NO_OFFSET, PACKET_NO_SIZE);
                SendHIDData(hidOutBuffer, 0, SIMPLE_PACKET_SIZE);
            }
            else
            {
                if (!serialPort.IsOpen)
                {
                    return;
                }

                CreateCOMBuffer(PACKET_NO_SIZE, RESET_DEVICE_OPCODE, DEFAULT_PACKET_NO);
                comOutBuffer[SIMPLE_CRC_WITH_PACKET_NO_OFFSET] = calcCRC(comOutBuffer, PACKET_NO_OFFSET, PACKET_NO_SIZE);
                SendCOMData(comOutBuffer, 0, SIMPLE_PACKET_SIZE);
            }
        }

        private void factoryResetBtn_Clicked(object sender, EventArgs e)
        {
            if (communicationType == USB_COMMUNICATION)
            {
                if (!hidDevice.connected)
                {
                    return;
                }

                CreateHIDBuffer(PACKET_NO_OFFSET, RESET_FACTORY_OPCODE, DEFAULT_PACKET_NO);
                hidOutBuffer[SIMPLE_CRC_WITH_PACKET_NO_OFFSET] = calcCRC(hidOutBuffer, PACKET_NO_OFFSET, PACKET_NO_SIZE);
                SendHIDData(hidOutBuffer, 0, SIMPLE_PACKET_SIZE);
            }
            else
            {
                if (!serialPort.IsOpen)
                {
                    return;
                }

                CreateCOMBuffer(PACKET_NO_SIZE, RESET_FACTORY_OPCODE, DEFAULT_PACKET_NO);
                comOutBuffer[SIMPLE_CRC_WITH_PACKET_NO_OFFSET] = calcCRC(comOutBuffer, PACKET_NO_OFFSET, PACKET_NO_SIZE);
                SendCOMData(comOutBuffer, 0, SIMPLE_PACKET_SIZE);
            }
        }
        #endregion

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

        private void formatSDCardBtn_Clicked(object sender, EventArgs e)
        {
            if ((communicationType == USB_COMMUNICATION) && (!hidDevice.connected))
            {
                return;
            }
            if ((communicationType == COM_COMMUNICATION) && ((serialPort == null) || (!serialPort.IsOpen)))
            {
                return;
            }
            // "Format the SD card will take several minutes to complete. Do not disconnect the power supply during the formatting operation!"
            if (MessageBox.Show(FORMAT_SDCARD_WARNING_STR.Text, APP_CAPTION_STR.Text, MessageBoxButtons.OKCancel, MessageBoxIcon.Warning) != System.Windows.Forms.DialogResult.OK) return;

            if (communicationType == USB_COMMUNICATION)
            {
                CreateHIDBuffer(PACKET_NO_SIZE, FORMAT_SDCARD_OPCODE, DEFAULT_PACKET_NO);
                hidOutBuffer[SIMPLE_CRC_WITH_PACKET_NO_OFFSET] = calcCRC(hidOutBuffer, PACKET_NO_OFFSET, PACKET_NO_SIZE);
                SendHIDData(hidOutBuffer, 0, SIMPLE_PACKET_SIZE);
            }
            else
            {
                CreateCOMBuffer(PACKET_NO_SIZE, FORMAT_SDCARD_OPCODE, DEFAULT_PACKET_NO);
                comOutBuffer[SIMPLE_CRC_WITH_PACKET_NO_OFFSET] = calcCRC(comOutBuffer, PACKET_NO_OFFSET, PACKET_NO_SIZE);
                SendCOMData(comOutBuffer, 0, SIMPLE_PACKET_SIZE);
            }
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
            EnableDeviceCmd(false);
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

        private void serialPort_DataReceived(object sender, SerialDataReceivedEventArgs e)
        {
            byte[] b;
            int length;
            int copyIndex;

            length = serialPort.BytesToRead;
            b = new byte[length];
            serialPort.Read(b, 0, length);
            if (comBufIndex == 0)
            {
                copyIndex = -1;
                for (int i = 0; i < b.Length; i++)
                {
                    if (b[i] == COMM_START_CODE)
                    {
                        copyIndex = i;
                        break;
                    }
                }
                if (copyIndex < 0)
                {
                    return;
                }
                Array.Clear(comTempBuffer, 0, comTempBuffer.Length);
                Array.Copy(b, copyIndex, comTempBuffer, 0, b.Length - copyIndex);
                comBufIndex += b.Length - copyIndex;
            }
            else
            {
                Array.Copy(b, 0, comTempBuffer, comBufIndex, b.Length);
                comBufIndex += b.Length;
            }
            if (comBufIndex >= (BitConverter.ToInt16(comTempBuffer, LENGTH_OFFSET) + HEADER_SIZE + CRC_SIZE))
            {
                comBufIndex = 0;
                Array.Copy(comTempBuffer, comInBuffer, comInBuffer.Length);
                ProcessReceivedCOMData();
            }
        }

        private byte calcCRC(byte[] buf, int index, int length)
        {
            byte crc;

            crc = 0;
            for (int i = index; i < index + length; i++)
            {
                crc += buf[i];
            }

            return crc;
        }

        private uint calcCRC32(byte[] buf, int index, int length)
        {
            uint crc;

            crc = 0;
            for (int i = index; i < index + length; i++)
            {
                crc += buf[i];
            }

            return crc;
        }

        private void gprsSettings_Changed(object sender, EventArgs e)
        {
            if (configUpdateDisabled)
            {
                return;
            }

            // gprs config
            tempCfg.gprsApn = gprsApnTxtBox.Text;
            tempCfg.gprsUser = gprsUserTxtBox.Text;
            tempCfg.gprsPass = gprsPwdTxtBox.Text;
            try
            {
                tempCfg.runReportInterval = ushort.Parse(gprsRunReportIntervalTxtBox.Text);
                tempCfg.stopReportInterval = ushort.Parse(gprsStopReportIntervalTxtBox.Text);
            }
            catch (Exception)
            { }
        }

        private void serverParameter_Changed(object sender, EventArgs e)
        {
            uint priIp, secIp;
            string priDomain, secDomain;
            ushort priPort, secPort;
            bool useIp;

            useIp = serverUseIpRBtn.Checked;
            priIpTxtBox.Enabled = secIpTxtBox.Enabled = useIp;
            priDomainTxtBox.Enabled = secDomainTxtBox.Enabled = !useIp;

            if (configUpdateDisabled)
            {
                return;
            }

            priIp = secIp = priPort = secPort = 0;
            try
            {
                ParseIP(priIpTxtBox.Text, ref priIp);
                ParseIP(secIpTxtBox.Text, ref secIp);
                priPort = ushort.Parse(priPortTxtBox.Text);
                secPort = ushort.Parse(secPortTxtBox.Text);
            }
            catch (Exception)
            { }
            priDomain = priDomainTxtBox.Text;
            secDomain = secDomainTxtBox.Text;

            switch (serverSelectionCmbBox.SelectedIndex)
            { 
                case DATA_SERVER_SELECTED:
                    tempCfg.priDataServerIp = priIp;
                    tempCfg.priDataServerDomain = priDomain;
                    tempCfg.priDataServerPort = priPort;
                    tempCfg.secDataServerIp = secIp;
                    tempCfg.secDataServerDomain = secDomain;
                    tempCfg.secDataServerPort = secPort;
                    tempCfg.dataServerUseIp = (byte)(useIp ? SERVER_USE_IP : 0);
                    break;
                //case FIRMWARE_SERVER_SELECTED:
                //    tempCfg.priFirmwareServerIp = priIp;
                //    tempCfg.priFirmwareServerDomain = priDomain;
                //    tempCfg.priFirmwareServerPort = priPort;
                //    tempCfg.secFirmwareServerIp = secIp;
                //    tempCfg.secFirmwareServerDomain = secDomain;
                //    tempCfg.secFirmwareServerPort = secPort;
                //    tempCfg.firmwareServerUseIp = (byte)(useIp ? SERVER_USE_IP : 0);
                //    break;
                //case INFO_SERVER_SELECTED:
                //    tempCfg.priInfoServerIp = priIp;
                //    tempCfg.priInfoServerDomain = priDomain;
                //    tempCfg.priInfoServerPort = priPort;
                //    tempCfg.secInfoServerIp = secIp;
                //    tempCfg.secInfoServerDomain = secDomain;
                //    tempCfg.secInfoServerPort = secPort;
                //    tempCfg.infoServerUseIp = (byte)(useIp ? SERVER_USE_IP : 0);
                //    break;
                case IMAGE_SERVER_SELECTED:
                    tempCfg.priImageServerIp = priIp;
                    tempCfg.priImageServerDomain = priDomain;
                    tempCfg.priImageServerPort = priPort;
                    tempCfg.secImageServerIp = secIp;
                    tempCfg.secImageServerDomain = secDomain;
                    tempCfg.secImageServerPort = secPort;
                    tempCfg.imageServerUseIp = (byte)(useIp ? SERVER_USE_IP : 0);
                    break;
				default:
					break;
            }
        }

        private void serverSelection_Changed(object sender, EventArgs e)
        {
            bool useIp = true;

            configUpdateDisabled = true;

            switch (serverSelectionCmbBox.SelectedIndex)
            { 
                case DATA_SERVER_SELECTED:
                    priIpTxtBox.Text = (new IPAddress(tempCfg.priDataServerIp)).ToString();
                    secIpTxtBox.Text = (new IPAddress(tempCfg.secDataServerIp)).ToString();
                    priDomainTxtBox.Text = tempCfg.priDataServerDomain;
                    secDomainTxtBox.Text = tempCfg.secDataServerDomain;
                    priPortTxtBox.Text = tempCfg.priDataServerPort.ToString();
                    secPortTxtBox.Text = tempCfg.secDataServerPort.ToString();
                    useIp = (tempCfg.dataServerUseIp == SERVER_USE_IP);
                    break;
                //case FIRMWARE_SERVER_SELECTED:
                //    priIpTxtBox.Text = (new IPAddress(tempCfg.priFirmwareServerIp)).ToString();
                //    secIpTxtBox.Text = (new IPAddress(tempCfg.secFirmwareServerIp)).ToString();
                //    priDomainTxtBox.Text = tempCfg.priFirmwareServerDomain;
                //    secDomainTxtBox.Text = tempCfg.secFirmwareServerDomain;
                //    priPortTxtBox.Text = tempCfg.priFirmwareServerPort.ToString();
                //    secPortTxtBox.Text = tempCfg.secFirmwareServerPort.ToString();
                //    useIp = (tempCfg.firmwareServerUseIp == SERVER_USE_IP);
                //    break;
                //case INFO_SERVER_SELECTED:
                //    priIpTxtBox.Text = (new IPAddress(tempCfg.priInfoServerIp)).ToString();
                //    secIpTxtBox.Text = (new IPAddress(tempCfg.secInfoServerIp)).ToString();
                //    priDomainTxtBox.Text = tempCfg.priInfoServerDomain;
                //    secDomainTxtBox.Text = tempCfg.secInfoServerDomain;
                //    priPortTxtBox.Text = tempCfg.priInfoServerPort.ToString();
                //    secPortTxtBox.Text = tempCfg.secInfoServerPort.ToString();
                //    useIp = (tempCfg.infoServerUseIp == SERVER_USE_IP);
                //    break;
                case IMAGE_SERVER_SELECTED:
                    priIpTxtBox.Text = (new IPAddress(tempCfg.priImageServerIp)).ToString();
                    secIpTxtBox.Text = (new IPAddress(tempCfg.secImageServerIp)).ToString();
                    priDomainTxtBox.Text = tempCfg.priImageServerDomain;
                    secDomainTxtBox.Text = tempCfg.secImageServerDomain;
                    priPortTxtBox.Text = tempCfg.priImageServerPort.ToString();
                    secPortTxtBox.Text = tempCfg.secImageServerPort.ToString();
                    useIp = (tempCfg.imageServerUseIp == SERVER_USE_IP);
                    break;
				default:
					break;
            }

            serverUseIpRBtn.Checked = useIp;
            serverUseDomainRBtn.Checked = !useIp;
            priIpTxtBox.Enabled = secIpTxtBox.Enabled = useIp;
            priDomainTxtBox.Enabled = secDomainTxtBox.Enabled = !useIp;

            configUpdateDisabled = false;
        }

        private void vehicleParameter_Changed(object sender, EventArgs e)
        {
            tyreRateTxtBox.Enabled = !gpsSpeedSourceRBtn.Checked;
            if (configUpdateDisabled)
            {
                return;
            }
            
            tempCfg.trackerId = trackerIdTxtBox.Text;
            tempCfg.plateNo = plateNoTxtBox.Text;
            tempCfg.vin = vinNumTxtBox.Text;
            tempCfg.whiteCallerNo = bossPhoneTxtBox.Text;
            try
            {
                tempCfg.speedSensorRatio = ushort.Parse(tyreRateTxtBox.Text);
            }
            catch (Exception)
            { }

            tempCfg.useGpsSpeed = (ushort)(gpsSpeedSourceRBtn.Checked ? GPS_SPEED_SOURCE : 0);
        }

        private void driverListInfo_Changed(object sender, EventArgs e)
        {
            int driver;

            if (configUpdateDisabled)
            {
                return;
            }

            driver = driverNoCmbBox.SelectedIndex;
            tempCfg.driverList[driver].driverName = driverNameTxtBox.Text;
            tempCfg.driverList[driver].licenseNo = licenseNoTxtBox.Text;
            tempCfg.driverList[driver].issuedDate = issuedDateTxtBox.Text;
            tempCfg.driverList[driver].expiredDate = expiredDateTxtBox.Text;
            tempCfg.driverList[driver].phoneNo = phoneNoTxtBox.Text;
        }

        private void driverListIndex_Changed(object sender, EventArgs e)
        {
            int driver;

            configUpdateDisabled = true;

            driver = driverNoCmbBox.SelectedIndex;
            driverNameTxtBox.Text = tempCfg.driverList[driver].driverName;
            licenseNoTxtBox.Text = tempCfg.driverList[driver].licenseNo;
            issuedDateTxtBox.Text = tempCfg.driverList[driver].issuedDate;
            expiredDateTxtBox.Text = tempCfg.driverList[driver].expiredDate;
            phoneNoTxtBox.Text = tempCfg.driverList[driver].phoneNo;

            configUpdateDisabled = false;
        }

        private void analogParameter_Changed(object sender, EventArgs e)
        {
            int analog;

            if (configUpdateDisabled)
            {
                return;
            }

            analog = analogInputNoCmbBox.SelectedIndex;
            try
            {
                switch (analog)
                {
                    case ANALOG_INPUT_1:
                        tempCfg.a1Mode = (ushort)(analogPercentageModeChkBox.Checked ? ANALOG_PERCENTAGE_MODE : 0);
                        tempCfg.a1UpperBound = ushort.Parse(analogUpperBoundTxtBox.Text);
                        tempCfg.a1LowerBound = ushort.Parse(analogLowerBoundTxtBox.Text);
                        break;
                    case ANALOG_INPUT_2:
                        tempCfg.a2Mode = (ushort)(analogPercentageModeChkBox.Checked ? ANALOG_PERCENTAGE_MODE : 0);
                        tempCfg.a2UpperBound = ushort.Parse(analogUpperBoundTxtBox.Text);
                        tempCfg.a2LowerBound = ushort.Parse(analogLowerBoundTxtBox.Text);
                        break;
                }
            }
            catch (Exception)
            { }
        }

        private void analogSelection_Changed(object sender, EventArgs e)
        {
            int analog;

            configUpdateDisabled = true;

            analog = analogInputNoCmbBox.SelectedIndex;
            switch (analog)
            { 
                case ANALOG_INPUT_1:
                    analogPercentageModeChkBox.Checked = (tempCfg.a1Mode == ANALOG_PERCENTAGE_MODE);
                    analogUpperBoundTxtBox.Text = tempCfg.a1UpperBound.ToString();
                    analogLowerBoundTxtBox.Text = tempCfg.a1LowerBound.ToString();
                    break;
                case ANALOG_INPUT_2:
                    analogPercentageModeChkBox.Checked = (tempCfg.a2Mode == ANALOG_PERCENTAGE_MODE);
                    analogUpperBoundTxtBox.Text = tempCfg.a2UpperBound.ToString();
                    analogLowerBoundTxtBox.Text = tempCfg.a2LowerBound.ToString();
                    break;
            }

            configUpdateDisabled = false;
        }

        private void printHead_Changed(object sender, EventArgs e)
        {
            if (configUpdateDisabled)
            {
                return;
            }

            tempCfg.infoStr = printHeadTxtBox.Text;
        }

        private void drivingSettings_Changed(object sender, EventArgs e)
        {
            if (configUpdateDisabled)
            {
                return;
            }

            try
            {
                tempCfg.speedLimit = ushort.Parse(speedLimitTxtBox.Text);
                tempCfg.drivingTimeLimit = (UInt32)(float.Parse(drivingTimeLimitTxtBox.Text) * 3600);
                tempCfg.totalDrivingTimeLimit = (UInt32)(float.Parse(totalDrivingTimeLimitTxtBox.Text) * 3600);
                tempCfg.baseMileage = (UInt32)(float.Parse(baseMileageTxtBox.Text));
				byte accountAlarmCheck = 0;
				byte.TryParse(accountAlarmCheckTxt.Text, out accountAlarmCheck);
				if ((accountAlarmCheck >= 0) && (accountAlarmCheck <= 23))
				{
					tempCfg.accountAlarmCheck = accountAlarmCheck;
				}
            }
            catch (Exception)
            { }
        }

        private void commTypeCmbBox_SelectedIndexChanged(object sender, EventArgs e)
        {
            communicationType = commTypeCmbBox.SelectedIndex;
            if (communicationType == USB_COMMUNICATION)
            {
                comPortCmbBox.Enabled = false;
                openPortBtn.Enabled = false;
                if (hidDevice.connected)
                {
                    return;
                }

                commuTypeLbl.Text = "USB";
                communicationType = USB_COMMUNICATION;
                HID.RegisterDeviceNotification(this.Handle, ref notificationHandle);
                OpenHIDDevice();

                // close COM port
                if ((serialPort != null) && (serialPort.IsOpen))
                {
                    serialPort.Close();
                    serialPort = null;
                }
            }
            else
            {
                comPortCmbBox.Items.Clear();
                comPortCmbBox.Enabled = true;
                openPortBtn.Enabled = true;
                foreach (string portName in SerialPort.GetPortNames())
                {
                    comPortCmbBox.Items.Add(portName);
                }
                if (comPortCmbBox.Items.Count > 0)
                {
                    comPortCmbBox.SelectedIndex = 0;
                }

                // close HID device
                if ((hidDevice != null) && (hidDevice.connected))
                {
                    CloseHIDDevice();
                    HID.UnregisterDeviceNotification(notificationHandle);
                }
            }

            UpdateUI();
        }

        private void openPortBtn_Click(object sender, EventArgs e)
        {
            if ((serialPort != null) && (serialPort.IsOpen))
            {
                serialPort.Close();
            }

            // open COM Port
            serialPort = new SerialPort(comPortCmbBox.Text);
            serialPort.BaudRate = 115200;
            //serialPort.WriteTimeout = 1000;
            try
            {
                serialPort.Open();
            }
            catch (Exception)
            {
                // "Cannot open port "
                MessageBox.Show(CANNOT_OPEN_COMPORT_STR.Text + serialPort.PortName, APP_CAPTION_STR.Text);
                return;
            }
            commuTypeLbl.Text = serialPort.PortName;
            communicationType = COM_COMMUNICATION;

            // stop USB communication
            CloseHIDDevice();
            HID.UnregisterDeviceNotification(notificationHandle);

            // start COM communication
            comBufIndex = 0;
            serialPort.DataReceived += new SerialDataReceivedEventHandler(serialPort_DataReceived);
            ReadConfig();
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

        private void selectLogTypeCmbBox_SelectedIndexChanged(object sender, EventArgs e)
        {
            int logOpcode;

            switch (selectLogTypeCmbBox.SelectedIndex)
            { 
                case 0:
                    logOpcode = DISABLE_LOG_OPCODE;
                    break;
                case 1:
                    logOpcode = ENABLE_GPS_LOG_OPCODE;
                    break;
                case 2:
                    logOpcode = ENABLE_GSM_LOG_OPCODE;
                    break;
                case 3:
                    logOpcode = ENABLE_MCU_LOG_OPCODE;
                    break;
                case 4:
                    logOpcode = ENABLE_LOG_OPCODE;
                    break;
                default:
                    logOpcode = DISABLE_LOG_OPCODE;
                    break;
            }
            if (communicationType == USB_COMMUNICATION)
            {
                if (!hidDevice.connected)
                {
                    return;
                }

                CreateHIDBuffer(PACKET_NO_SIZE, (byte)logOpcode, DEFAULT_PACKET_NO);
                hidOutBuffer[SIMPLE_CRC_WITH_PACKET_NO_OFFSET] = calcCRC(hidOutBuffer, PACKET_NO_OFFSET, PACKET_NO_SIZE);
                SendHIDData(hidOutBuffer, 0, SIMPLE_PACKET_SIZE);
            }
            else
            {
                if (!serialPort.IsOpen)
                {
                    return;
                }
                
                CreateCOMBuffer(PACKET_NO_SIZE, (byte)logOpcode, DEFAULT_PACKET_NO);
                comOutBuffer[SIMPLE_CRC_WITH_PACKET_NO_OFFSET] = calcCRC(comOutBuffer, PACKET_NO_OFFSET, PACKET_NO_SIZE);
                SendCOMData(comOutBuffer, 0, SIMPLE_PACKET_SIZE);
            }
        }

        private void saveSettingsBtn_Click(object sender, EventArgs e)
        {
            string fileName;
            DialogResult res;
            BinaryWriter writer;

            saveFileDialog1.Filter = "CFG file (*.CFG)|*.CFG";
            res = saveFileDialog1.ShowDialog();
            if (res != DialogResult.OK)
            {
                return;
            }
            fileName = saveFileDialog1.FileName;
            writer = new BinaryWriter(new FileStream(fileName, FileMode.CreateNew));
            cfg = tempCfg;
            writer.Write(Lib.ObjectToByteArray(cfg));
            writer.Close();
            MessageBox.Show(CONFIG_SAVE_SUCCESS_STR.Text, APP_CAPTION_STR.Text);
        }

        private void loadSettingsBtn_Click(object sender, EventArgs e)
        {
            string fileName;
            DialogResult res;
            BinaryReader reader;
            byte[] b;

            openFileDialog1.Filter = "CFG file (*.CFG)|*.CFG";
            res = openFileDialog1.ShowDialog();
            if (res != DialogResult.OK)
            {
                return;
            }
            fileName = openFileDialog1.FileName;
            if (!File.Exists(fileName))
            {
                return;
            }
            try
            {
                reader = new BinaryReader(new FileStream(fileName, FileMode.Open));
                b = reader.ReadBytes(GPS_CONFIG_DATA_SIZE);
                cfg = (TRACKER_CONFIG_DATA)Lib.ByteArrayToObject(b, typeof(TRACKER_CONFIG_DATA));
                reader.Close();
                UpdateConfigUI();
                MessageBox.Show(CONFIG_LOADED_STR.Text, APP_CAPTION_STR.Text);
            }
            catch (Exception)
            { }
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

            serverSelectionCmbBox.Items[0] = DATA_SERVER_CMBBOX_ITEM_STR.Text;
            //serverSelectionCmbBox.Items[1] = FIRMWARE_SERVER_CMBBOX_ITEM_STR.Text;
            serverSelectionCmbBox.Items[1] = IMAGE_SERVER_CMBBOX_ITEM_STR.Text;
            triggerSourceChkList.Items[0] = INTERVAL_TRIGGER_SOURCE_CHKLIST_ITEM_STR.Text;
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
            string toolTipText;

            resources.ApplyResources(control, control.Name, culture);
            toolTipText = resources.GetString(control.Name + ".ToolTip", culture);
            if (toolTipText != null)
            {
                toolTip1.SetToolTip(control, toolTipText);
                Console.WriteLine(toolTipText);
            }
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

        private void readFirmwareVersionBtn_Click(object sender, EventArgs e)
        {
            if (communicationType == USB_COMMUNICATION)
            {
                if (!hidDevice.connected)
                {
                    return;
                }

                CreateHIDBuffer(PACKET_NO_SIZE, READ_FIRMWARE_VERSION_OPCODE, DEFAULT_PACKET_NO);
                hidOutBuffer[SIMPLE_CRC_WITH_PACKET_NO_OFFSET] = calcCRC(hidOutBuffer, PACKET_NO_OFFSET, PACKET_NO_SIZE);
                SendHIDData(hidOutBuffer, 0, SIMPLE_PACKET_SIZE);
            }
            else
            {
                if (!serialPort.IsOpen)
                {
                    return;
                }

                CreateCOMBuffer(PACKET_NO_SIZE, READ_FIRMWARE_VERSION_OPCODE, DEFAULT_PACKET_NO);
                comOutBuffer[SIMPLE_CRC_WITH_PACKET_NO_OFFSET] = calcCRC(comOutBuffer, PACKET_NO_OFFSET, PACKET_NO_SIZE);
                SendCOMData(comOutBuffer, 0, SIMPLE_PACKET_SIZE);
            }
        }

        private void browseFirmwareBtn_Click(object sender, EventArgs e)
        {
            string fileName;
            DialogResult res;

            openFileDialog1.Filter = "BIN file (*.BIN)|*.BIN";
            res = openFileDialog1.ShowDialog();
            if (res != DialogResult.OK)
            {
                return;
            }
            fileName = openFileDialog1.FileName;
            if (!File.Exists(fileName))
            {
                return;
            }
            firmwarePathTxtBox.Text = fileName;
        }

        private void downloadFirmwareBtn_Click(object sender, EventArgs e)
        {
            string fileName;
            byte[] fileSize;
            byte[] fileCRC;

            fileName = firmwarePathTxtBox.Text;
            if (!File.Exists(fileName))
            {
                MessageBox.Show(FILE_NAME_IS_INVALID_WARNING_STR.Text, APP_CAPTION_STR.Text);
                return;
            }
            if (communicationType == USB_COMMUNICATION)
            {
                if (!hidDevice.connected)
                {
                    return;
                }

                if (firmwareReader != null)
                {
                    try
                    {
                        firmwareReader.Close();
                    }
                    catch (Exception)
                    { }
                }

                fileCRC = BitConverter.GetBytes(CalcFileCRC(firmwarePathTxtBox.Text));
                try
                {
                    firmwareReader = new BinaryReader(new FileStream(firmwarePathTxtBox.Text, FileMode.Open));
                }
                catch (Exception)
                {
                    MessageBox.Show(CANNOT_OPEN_FILE_STR.Text + fileName, APP_CAPTION_STR.Text);
                    return;
                }

                downloadingFirmware = true;
                EnableDeviceCmd(false);
                fileSize = BitConverter.GetBytes(firmwareReader.BaseStream.Length);
                CreateHIDBuffer(PACKET_NO_SIZE + 8, DOWNLOAD_FIRMWARE_OPCODE, DEFAULT_PACKET_NO);
                Array.Copy(fileSize, 0, hidOutBuffer, DATA_WITH_PACKET_NO_OFFSET, 4);
                Array.Copy(fileCRC, 0, hidOutBuffer, DATA_WITH_PACKET_NO_OFFSET + 4, 4);
                hidOutBuffer[DATA_WITH_PACKET_NO_OFFSET + 8] = calcCRC(hidOutBuffer, DATA_OFFSET, PACKET_NO_SIZE + 8);
                SendHIDData(hidOutBuffer, 0, HEADER_SIZE + PACKET_NO_SIZE + 8 + CRC_SIZE);
                retrieveLogPrgBar.Maximum = (int)(firmwareReader.BaseStream.Length / 100);
                retrieveLogStatusLbl.Text = DOWNLOADING_FIRMWARE_STR.Text;
                UpdateProgressBar(true);
            }
            else
            {
                if (!serialPort.IsOpen)
                {
                    return;
                }

                if (firmwareReader != null)
                {
                    try
                    {
                        firmwareReader.Close();
                    }
                    catch (Exception)
                    { }
                }

                fileCRC = BitConverter.GetBytes(CalcFileCRC(firmwarePathTxtBox.Text));
                try
                {
                    firmwareReader = new BinaryReader(new FileStream(firmwarePathTxtBox.Text, FileMode.Open));
                }
                catch (Exception)
                {
                    MessageBox.Show(CANNOT_OPEN_FILE_STR.Text + fileName, APP_CAPTION_STR.Text);
                    return;
                }

                downloadingFirmware = true;
                EnableDeviceCmd(false);
                fileSize = BitConverter.GetBytes(firmwareReader.BaseStream.Length);
                CreateCOMBuffer(PACKET_NO_SIZE + 8, DOWNLOAD_FIRMWARE_OPCODE, DEFAULT_PACKET_NO);
                Array.Copy(fileSize, 0, comOutBuffer, DATA_WITH_PACKET_NO_OFFSET, 4);
                Array.Copy(fileCRC, 0, comOutBuffer, DATA_WITH_PACKET_NO_OFFSET + 4, 4);
                comOutBuffer[DATA_WITH_PACKET_NO_OFFSET + 8] = calcCRC(comOutBuffer, DATA_OFFSET, PACKET_NO_SIZE + 8);
                SendCOMData(comOutBuffer, 0, HEADER_SIZE + PACKET_NO_SIZE + 8 + CRC_SIZE);
                retrieveLogPrgBar.Maximum = (int)(firmwareReader.BaseStream.Length/ 100);
                retrieveLogStatusLbl.Text = DOWNLOADING_FIRMWARE_STR.Text;
                UpdateProgressBar(true);
            }
        }

        private void clearLogBtn_Click(object sender, EventArgs e)
        {
            lock (logTxtBox)
            {
                logTxtBox.Clear();
            }
        }

        private void cameraParameter_Changed(object sender, EventArgs e)
        {
            if (configUpdateDisabled)
            {
                return;
            }

            tempCfg.numCameras = (byte)cameraChannelUDBox.Value;
            byte.TryParse(compressionRateTxt.Text, out tempCfg.cameraCompression);
            byte.TryParse(cameraWorkingStartTxt.Text, out tempCfg.cameraWorkingStartTime);
            byte.TryParse(cameraWorkingStopTxt.Text, out tempCfg.cameraWorkingStopTime);
            
            ushort.TryParse(cameraIntervalTxt.Text, out tempCfg.cameraInterval);            
        }

        private void setCameraIdBtn_Click(object sender, EventArgs e)
        {
            if ((communicationType == USB_COMMUNICATION) && (!hidDevice.connected))
            {
                return;
            }
            if ((communicationType == COM_COMMUNICATION) && ((serialPort == null) || (!serialPort.IsOpen)))
            {
                return;
            }
            
            if (communicationType == USB_COMMUNICATION)
            {
                CreateHIDBuffer(PACKET_NO_SIZE + 1, SET_CAMERA_ID_OPCODE, DEFAULT_PACKET_NO);
                hidOutBuffer[DATA_WITH_PACKET_NO_OFFSET] = (byte)cameraIdUDBox.Value;
                hidOutBuffer[DATA_WITH_PACKET_NO_OFFSET + 1] = calcCRC(hidOutBuffer, PACKET_NO_OFFSET, PACKET_NO_SIZE + 1);
                SendHIDData(hidOutBuffer, 0, SIMPLE_PACKET_SIZE + 1);
            }
            else
            {
                CreateCOMBuffer(PACKET_NO_SIZE + 1, SET_CAMERA_ID_OPCODE, DEFAULT_PACKET_NO);
                comOutBuffer[DATA_WITH_PACKET_NO_OFFSET] = (byte)cameraIdUDBox.Value;
                comOutBuffer[DATA_WITH_PACKET_NO_OFFSET + 1] = calcCRC(comOutBuffer, PACKET_NO_OFFSET, PACKET_NO_SIZE + 1);
                SendCOMData(comOutBuffer, 0, SIMPLE_PACKET_SIZE + 1);
            }
        }

        private void triggerSourceChkList_ItemCheck(object sender, ItemCheckEventArgs e)
        {
            if (configUpdateDisabled)
            {
                return;
            }
            if (e.NewValue == CheckState.Checked)
            {
                switch (e.Index)
                {
                    case CAMERA_TRIGGER_INTERVAL:
                        tempCfg.cameraEvents |= INTERVAL_TRIGGER_MASK;
                        break;
                    case CAMERA_TRIGGER_DI1:
                        tempCfg.cameraEvents |= DI1_TRIGGER_MASK;
                        break;
                    case CAMERA_TRIGGER_DI2:
                        tempCfg.cameraEvents |= DI2_TRIGGER_MASK;
                        break;
                    case CAMERA_TRIGGER_DI3:
                        tempCfg.cameraEvents |= DI3_TRIGGER_MASK;
                        break;
                    case CAMERA_TRIGGER_DI4:
                        tempCfg.cameraEvents |= DI4_TRIGGER_MASK;
                        break;
                    default:
                        break;
                }
            }
            else
            {
                switch (e.Index)
                {
                    case CAMERA_TRIGGER_INTERVAL:
                        tempCfg.cameraEvents &= 0xFFFF - INTERVAL_TRIGGER_MASK;
                        break;
                    case CAMERA_TRIGGER_DI1:
                        tempCfg.cameraEvents &= 0xFFFF - DI1_TRIGGER_MASK;
                        break;
                    case CAMERA_TRIGGER_DI2:
                        tempCfg.cameraEvents &= 0xFFFF - DI2_TRIGGER_MASK;
                        break;
                    case CAMERA_TRIGGER_DI3:
                        tempCfg.cameraEvents &= 0xFFFF - DI3_TRIGGER_MASK;
                        break;
                    case CAMERA_TRIGGER_DI4:
                        tempCfg.cameraEvents &= 0xFFFF - DI4_TRIGGER_MASK;
                        break;
                    default:
                        break;
                }
            }
        }

        private void sendSMSBtn_Click(object sender, EventArgs e)
        {
            SendSMS();
        }

        private void tabMain_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (bootMode)
            {
                tabMain.SelectedTab = settingTabPage;
            }
        }

        private void jumpToAppBtn_Click(object sender, EventArgs e)
        {
            if (communicationType == USB_COMMUNICATION)
            {
                if (!hidDevice.connected)
                {
                    return;
                }

                CreateHIDBuffer(PACKET_NO_SIZE, JUMP_TO_APP_OPCODE, DEFAULT_PACKET_NO);
                hidOutBuffer[SIMPLE_CRC_WITH_PACKET_NO_OFFSET] = calcCRC(hidOutBuffer, PACKET_NO_OFFSET, PACKET_NO_SIZE);
                SendHIDData(hidOutBuffer, 0, SIMPLE_PACKET_SIZE);
            }
        }

        private uint CalcFileCRC(string fileName)
        {
            List<byte> temp;
            uint crc;

            temp = ReadAllByteInFile(fileName);
            crc = calcCRC32(temp.ToArray(), 0, temp.Count);
            temp.Clear();
            temp = null;

            return crc;
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

		private void featureSet_Changed(object sender, EventArgs e)
		{
            lowBatteryWarningLevelTxt.Enabled = lowBatteryWarningPeriodTxt.Enabled = lowBatteryWarningChkBox.Checked;
            lowPowerWarningLevelTxt.Enabled = lowPowerWarningPeriodTxt.Enabled = lowPowerWarningChkBox.Checked;
			if (configUpdateDisabled)
			{
				return;
			}

            tempCfg.featureSet = 0;
            tempCfg.featureSet |= (ushort)((lowBatteryWarningChkBox.Checked ? 1 : 0) << 4);
            tempCfg.featureSet |= (ushort)((lowPowerWarningChkBox.Checked ? 1 : 0) << 5);
            tempCfg.featureSet |= (ushort)((accWarningChkBox.Checked ? 1 : 0) << 6);
            tempCfg.featureSet |= (ushort)((tapWarningChkBox.Checked ? 1 : 0) << 7);
            tempCfg.featureSet |= (ushort)((removePowerWarningChkBox.Checked ? 1 : 0) << 8);
            tempCfg.featureSet |= (ushort)((autoSecurityOnChkBox.Checked ? 1 : 0) << 9);
        }

        private void otherSettings_Changed(object sender, EventArgs e)
        {
            if (manualSecurityOnChkBox.Checked)
            {
                sendSMSActionChkBox.Enabled = true;
                callActionChkBox.Enabled = true;
            }
            else if (!autoSecurityOnChkBox.Checked)
            {
                sendSMSActionChkBox.Enabled = false;
                callActionChkBox.Enabled = false;
            }

            if (configUpdateDisabled)
            {
                return;
            }

            tempCfg.securityOn = (byte)(manualSecurityOnChkBox.Checked ? 1 : 0);
            tempCfg.securityAction = 0;
            tempCfg.securityAction |= (byte)(sendSMSActionChkBox.Checked ? 1 : 0);
            tempCfg.securityAction |= (byte)((callActionChkBox.Checked ? 1 : 0) << 1);
            byte.TryParse(sosKeyPressTimeTxt.Text, out tempCfg.sosKeyPressTimes);
            byte.TryParse(sosKeyPressPeriodTxt.Text, out tempCfg.sosKeyPressPeriod);
            UInt32.TryParse(lowPowerWarningPeriodTxt.Text, out tempCfg.powerLowWarningPeriod);
            UInt32.TryParse(lowBatteryWarningPeriodTxt.Text, out tempCfg.batteryLowWarningPeriod);
            UInt32.TryParse(speedEngineOffTxt.Text, out tempCfg.speedEngineOff);
            UInt32.TryParse(powerOffDelayTimeTxt.Text, out tempCfg.powerOffDelayTime);
            byte.TryParse(lowPowerWarningLevelTxt.Text, out tempCfg.powerLowWarningLevel);
            byte.TryParse(lowBatteryWarningLevelTxt.Text, out tempCfg.batteryLowWarningLevel);
            UInt32.TryParse(accountAlarmCheckTxt.Text, out tempCfg.autoSecurityTime);
            byte accountAlarmCheck = 0;
            byte.TryParse(accountAlarmCheckTxt.Text, out accountAlarmCheck);
            if ((accountAlarmCheck >= 0) && (accountAlarmCheck <= 23))
            {
                tempCfg.accountAlarmCheck = accountAlarmCheck;
            }
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
