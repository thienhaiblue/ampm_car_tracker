namespace GT400VNReportReader
{
    partial class MainForm
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(MainForm));
            this.tabMain = new System.Windows.Forms.TabControl();
            this.summaryReportTabPage = new System.Windows.Forms.TabPage();
            this.summaryReportGridView = new System.Windows.Forms.DataGridView();
            this.noCol = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.plateCol = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.driverCol = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.licenseCol = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.startTimeCol = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.endTimeCol = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.exSpeedCountCol = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.doorOpenCountCol = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.doorCloseCountCol = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.engineOnCountCol = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.engineOffCountCol = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.parkCountCol = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.contDriveTimeCol = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.overDrivingTimeCountCol = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.totalDriveTimeCol = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.overTotalDrivingTimeCountCol = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.totalLengthCol = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.reportGrpBox = new System.Windows.Forms.GroupBox();
            this.readReportFromDeviceBtn = new System.Windows.Forms.Button();
            this.logIntervalCmbBox = new System.Windows.Forms.ComboBox();
            this.label18 = new System.Windows.Forms.Label();
            this.clearReportBtn = new System.Windows.Forms.Button();
            this.readReportFromHardDiskBtn = new System.Windows.Forms.Button();
            this.outputReportToKMLBtn = new System.Windows.Forms.Button();
            this.reportBeginDatePicker = new System.Windows.Forms.DateTimePicker();
            this.label12 = new System.Windows.Forms.Label();
            this.label13 = new System.Windows.Forms.Label();
            this.reportEndDatePicker = new System.Windows.Forms.DateTimePicker();
            this.detailReportTabPage = new System.Windows.Forms.TabPage();
            this.groupBox3 = new System.Windows.Forms.GroupBox();
            this.reportDriverNameCmbBox = new System.Windows.Forms.ComboBox();
            this.label5 = new System.Windows.Forms.Label();
            this.detailReportGridView = new System.Windows.Forms.DataGridView();
            this.dataGridViewTextBoxColumn12 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.gridDetailTime = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.gridDetailLng = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.gridDetailLat = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.gridDetailSpeed = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.gridDetailDoor = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.gridDetailMileaage = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.googleLinkCol = new System.Windows.Forms.DataGridViewLinkColumn();
            this.menuStrip1 = new System.Windows.Forms.MenuStrip();
            this.fileToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.exitToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.languageToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.englishToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.vietnameseToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.helpToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.registerToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.aboutToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.saveFileDialog1 = new System.Windows.Forms.SaveFileDialog();
            this.openFileDialog1 = new System.Windows.Forms.OpenFileDialog();
            this.selectFolderDialog = new System.Windows.Forms.FolderBrowserDialog();
            this.imeiLbl = new System.Windows.Forms.Label();
            this.statusGrpBox = new System.Windows.Forms.GroupBox();
            this.connectionStatusLbl = new System.Windows.Forms.Label();
            this.retrieveLogPercentLbl = new System.Windows.Forms.Label();
            this.retrieveLogStatusLbl = new System.Windows.Forms.Label();
            this.commuTypeLbl = new System.Windows.Forms.Label();
            this.retrieveLogPrgBar = new System.Windows.Forms.ProgressBar();
            this.btnPrint = new System.Windows.Forms.Button();
            this.btnResetDevice = new System.Windows.Forms.Button();
            this.btnResetToDefault = new System.Windows.Forms.Button();
            this.btnReadConfig = new System.Windows.Forms.Button();
            this.btnWriteConfig = new System.Windows.Forms.Button();
            this.Column9 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.Column8 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.Column7 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.Column6 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.Column5 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.Column4 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.Column3 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.dataGridViewTextBoxColumn5 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.dataGridViewTextBoxColumn4 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.dataGridViewTextBoxColumn3 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.dataGridViewTextBoxColumn2 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.dataGridViewTextBoxColumn1 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.tabMain.SuspendLayout();
            this.summaryReportTabPage.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.summaryReportGridView)).BeginInit();
            this.reportGrpBox.SuspendLayout();
            this.detailReportTabPage.SuspendLayout();
            this.groupBox3.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.detailReportGridView)).BeginInit();
            this.menuStrip1.SuspendLayout();
            this.statusGrpBox.SuspendLayout();
            this.SuspendLayout();
            // 
            // tabMain
            // 
            resources.ApplyResources(this.tabMain, "tabMain");
            this.tabMain.Controls.Add(this.summaryReportTabPage);
            this.tabMain.Controls.Add(this.detailReportTabPage);
            this.tabMain.Name = "tabMain";
            this.tabMain.SelectedIndex = 0;
            // 
            // summaryReportTabPage
            // 
            this.summaryReportTabPage.Controls.Add(this.summaryReportGridView);
            this.summaryReportTabPage.Controls.Add(this.reportGrpBox);
            resources.ApplyResources(this.summaryReportTabPage, "summaryReportTabPage");
            this.summaryReportTabPage.Name = "summaryReportTabPage";
            this.summaryReportTabPage.UseVisualStyleBackColor = true;
            // 
            // summaryReportGridView
            // 
            this.summaryReportGridView.AllowUserToAddRows = false;
            this.summaryReportGridView.AllowUserToDeleteRows = false;
            this.summaryReportGridView.AllowUserToResizeRows = false;
            resources.ApplyResources(this.summaryReportGridView, "summaryReportGridView");
            this.summaryReportGridView.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            this.summaryReportGridView.Columns.AddRange(new System.Windows.Forms.DataGridViewColumn[] {
            this.noCol,
            this.plateCol,
            this.driverCol,
            this.licenseCol,
            this.startTimeCol,
            this.endTimeCol,
            this.exSpeedCountCol,
            this.doorOpenCountCol,
            this.doorCloseCountCol,
            this.engineOnCountCol,
            this.engineOffCountCol,
            this.parkCountCol,
            this.contDriveTimeCol,
            this.overDrivingTimeCountCol,
            this.totalDriveTimeCol,
            this.overTotalDrivingTimeCountCol,
            this.totalLengthCol});
            this.summaryReportGridView.Name = "summaryReportGridView";
            this.summaryReportGridView.ReadOnly = true;
            this.summaryReportGridView.RowHeadersVisible = false;
            this.summaryReportGridView.RowHeadersWidthSizeMode = System.Windows.Forms.DataGridViewRowHeadersWidthSizeMode.DisableResizing;
            // 
            // noCol
            // 
            resources.ApplyResources(this.noCol, "noCol");
            this.noCol.Name = "noCol";
            this.noCol.ReadOnly = true;
            // 
            // plateCol
            // 
            resources.ApplyResources(this.plateCol, "plateCol");
            this.plateCol.Name = "plateCol";
            this.plateCol.ReadOnly = true;
            // 
            // driverCol
            // 
            resources.ApplyResources(this.driverCol, "driverCol");
            this.driverCol.Name = "driverCol";
            this.driverCol.ReadOnly = true;
            // 
            // licenseCol
            // 
            resources.ApplyResources(this.licenseCol, "licenseCol");
            this.licenseCol.Name = "licenseCol";
            this.licenseCol.ReadOnly = true;
            // 
            // startTimeCol
            // 
            resources.ApplyResources(this.startTimeCol, "startTimeCol");
            this.startTimeCol.Name = "startTimeCol";
            this.startTimeCol.ReadOnly = true;
            // 
            // endTimeCol
            // 
            resources.ApplyResources(this.endTimeCol, "endTimeCol");
            this.endTimeCol.Name = "endTimeCol";
            this.endTimeCol.ReadOnly = true;
            // 
            // exSpeedCountCol
            // 
            resources.ApplyResources(this.exSpeedCountCol, "exSpeedCountCol");
            this.exSpeedCountCol.Name = "exSpeedCountCol";
            this.exSpeedCountCol.ReadOnly = true;
            // 
            // doorOpenCountCol
            // 
            resources.ApplyResources(this.doorOpenCountCol, "doorOpenCountCol");
            this.doorOpenCountCol.Name = "doorOpenCountCol";
            this.doorOpenCountCol.ReadOnly = true;
            // 
            // doorCloseCountCol
            // 
            resources.ApplyResources(this.doorCloseCountCol, "doorCloseCountCol");
            this.doorCloseCountCol.Name = "doorCloseCountCol";
            this.doorCloseCountCol.ReadOnly = true;
            // 
            // engineOnCountCol
            // 
            resources.ApplyResources(this.engineOnCountCol, "engineOnCountCol");
            this.engineOnCountCol.Name = "engineOnCountCol";
            this.engineOnCountCol.ReadOnly = true;
            // 
            // engineOffCountCol
            // 
            resources.ApplyResources(this.engineOffCountCol, "engineOffCountCol");
            this.engineOffCountCol.Name = "engineOffCountCol";
            this.engineOffCountCol.ReadOnly = true;
            // 
            // parkCountCol
            // 
            resources.ApplyResources(this.parkCountCol, "parkCountCol");
            this.parkCountCol.Name = "parkCountCol";
            this.parkCountCol.ReadOnly = true;
            // 
            // contDriveTimeCol
            // 
            this.contDriveTimeCol.AutoSizeMode = System.Windows.Forms.DataGridViewAutoSizeColumnMode.Fill;
            resources.ApplyResources(this.contDriveTimeCol, "contDriveTimeCol");
            this.contDriveTimeCol.Name = "contDriveTimeCol";
            this.contDriveTimeCol.ReadOnly = true;
            // 
            // overDrivingTimeCountCol
            // 
            resources.ApplyResources(this.overDrivingTimeCountCol, "overDrivingTimeCountCol");
            this.overDrivingTimeCountCol.Name = "overDrivingTimeCountCol";
            this.overDrivingTimeCountCol.ReadOnly = true;
            // 
            // totalDriveTimeCol
            // 
            this.totalDriveTimeCol.AutoSizeMode = System.Windows.Forms.DataGridViewAutoSizeColumnMode.Fill;
            resources.ApplyResources(this.totalDriveTimeCol, "totalDriveTimeCol");
            this.totalDriveTimeCol.Name = "totalDriveTimeCol";
            this.totalDriveTimeCol.ReadOnly = true;
            // 
            // overTotalDrivingTimeCountCol
            // 
            resources.ApplyResources(this.overTotalDrivingTimeCountCol, "overTotalDrivingTimeCountCol");
            this.overTotalDrivingTimeCountCol.Name = "overTotalDrivingTimeCountCol";
            this.overTotalDrivingTimeCountCol.ReadOnly = true;
            // 
            // totalLengthCol
            // 
            this.totalLengthCol.AutoSizeMode = System.Windows.Forms.DataGridViewAutoSizeColumnMode.Fill;
            resources.ApplyResources(this.totalLengthCol, "totalLengthCol");
            this.totalLengthCol.Name = "totalLengthCol";
            this.totalLengthCol.ReadOnly = true;
            // 
            // reportGrpBox
            // 
            this.reportGrpBox.Controls.Add(this.readReportFromDeviceBtn);
            this.reportGrpBox.Controls.Add(this.logIntervalCmbBox);
            this.reportGrpBox.Controls.Add(this.label18);
            this.reportGrpBox.Controls.Add(this.clearReportBtn);
            this.reportGrpBox.Controls.Add(this.readReportFromHardDiskBtn);
            this.reportGrpBox.Controls.Add(this.outputReportToKMLBtn);
            this.reportGrpBox.Controls.Add(this.reportBeginDatePicker);
            this.reportGrpBox.Controls.Add(this.label12);
            this.reportGrpBox.Controls.Add(this.label13);
            this.reportGrpBox.Controls.Add(this.reportEndDatePicker);
            resources.ApplyResources(this.reportGrpBox, "reportGrpBox");
            this.reportGrpBox.Name = "reportGrpBox";
            this.reportGrpBox.TabStop = false;
            // 
            // readReportFromDeviceBtn
            // 
            resources.ApplyResources(this.readReportFromDeviceBtn, "readReportFromDeviceBtn");
            this.readReportFromDeviceBtn.Name = "readReportFromDeviceBtn";
            this.readReportFromDeviceBtn.UseVisualStyleBackColor = true;
            this.readReportFromDeviceBtn.Click += new System.EventHandler(this.readReportFromDeviceBtn_Click);
            // 
            // logIntervalCmbBox
            // 
            this.logIntervalCmbBox.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.logIntervalCmbBox.FormattingEnabled = true;
            this.logIntervalCmbBox.Items.AddRange(new object[] {
            resources.GetString("logIntervalCmbBox.Items"),
            resources.GetString("logIntervalCmbBox.Items1"),
            resources.GetString("logIntervalCmbBox.Items2"),
            resources.GetString("logIntervalCmbBox.Items3"),
            resources.GetString("logIntervalCmbBox.Items4"),
            resources.GetString("logIntervalCmbBox.Items5"),
            resources.GetString("logIntervalCmbBox.Items6"),
            resources.GetString("logIntervalCmbBox.Items7")});
            resources.ApplyResources(this.logIntervalCmbBox, "logIntervalCmbBox");
            this.logIntervalCmbBox.Name = "logIntervalCmbBox";
            // 
            // label18
            // 
            resources.ApplyResources(this.label18, "label18");
            this.label18.Name = "label18";
            // 
            // clearReportBtn
            // 
            resources.ApplyResources(this.clearReportBtn, "clearReportBtn");
            this.clearReportBtn.Name = "clearReportBtn";
            this.clearReportBtn.UseVisualStyleBackColor = true;
            this.clearReportBtn.Click += new System.EventHandler(this.clearReportBtn_Click);
            // 
            // readReportFromHardDiskBtn
            // 
            resources.ApplyResources(this.readReportFromHardDiskBtn, "readReportFromHardDiskBtn");
            this.readReportFromHardDiskBtn.Name = "readReportFromHardDiskBtn";
            this.readReportFromHardDiskBtn.UseVisualStyleBackColor = true;
            this.readReportFromHardDiskBtn.Click += new System.EventHandler(this.readReportFromHardDiskBtn_Click);
            // 
            // outputReportToKMLBtn
            // 
            resources.ApplyResources(this.outputReportToKMLBtn, "outputReportToKMLBtn");
            this.outputReportToKMLBtn.Name = "outputReportToKMLBtn";
            this.outputReportToKMLBtn.UseVisualStyleBackColor = true;
            this.outputReportToKMLBtn.Click += new System.EventHandler(this.outputReportToKMLBtn_Click);
            // 
            // reportBeginDatePicker
            // 
            resources.ApplyResources(this.reportBeginDatePicker, "reportBeginDatePicker");
            this.reportBeginDatePicker.Format = System.Windows.Forms.DateTimePickerFormat.Custom;
            this.reportBeginDatePicker.Name = "reportBeginDatePicker";
            this.reportBeginDatePicker.CloseUp += new System.EventHandler(this.dateTimePickerBegin_CloseUp);
            // 
            // label12
            // 
            resources.ApplyResources(this.label12, "label12");
            this.label12.Name = "label12";
            // 
            // label13
            // 
            resources.ApplyResources(this.label13, "label13");
            this.label13.Name = "label13";
            // 
            // reportEndDatePicker
            // 
            resources.ApplyResources(this.reportEndDatePicker, "reportEndDatePicker");
            this.reportEndDatePicker.Format = System.Windows.Forms.DateTimePickerFormat.Custom;
            this.reportEndDatePicker.Name = "reportEndDatePicker";
            this.reportEndDatePicker.CloseUp += new System.EventHandler(this.dateTimePickerEnd_CloseUp);
            // 
            // detailReportTabPage
            // 
            this.detailReportTabPage.Controls.Add(this.groupBox3);
            this.detailReportTabPage.Controls.Add(this.detailReportGridView);
            resources.ApplyResources(this.detailReportTabPage, "detailReportTabPage");
            this.detailReportTabPage.Name = "detailReportTabPage";
            this.detailReportTabPage.UseVisualStyleBackColor = true;
            // 
            // groupBox3
            // 
            this.groupBox3.Controls.Add(this.reportDriverNameCmbBox);
            this.groupBox3.Controls.Add(this.label5);
            resources.ApplyResources(this.groupBox3, "groupBox3");
            this.groupBox3.Name = "groupBox3";
            this.groupBox3.TabStop = false;
            // 
            // reportDriverNameCmbBox
            // 
            this.reportDriverNameCmbBox.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.reportDriverNameCmbBox.FormattingEnabled = true;
            resources.ApplyResources(this.reportDriverNameCmbBox, "reportDriverNameCmbBox");
            this.reportDriverNameCmbBox.Name = "reportDriverNameCmbBox";
            this.reportDriverNameCmbBox.SelectedIndexChanged += new System.EventHandler(this.reportDriverNameCmbBox_SelectedIndexChanged);
            // 
            // label5
            // 
            resources.ApplyResources(this.label5, "label5");
            this.label5.Name = "label5";
            // 
            // detailReportGridView
            // 
            this.detailReportGridView.AllowUserToAddRows = false;
            this.detailReportGridView.AllowUserToDeleteRows = false;
            this.detailReportGridView.AllowUserToResizeRows = false;
            resources.ApplyResources(this.detailReportGridView, "detailReportGridView");
            this.detailReportGridView.AutoSizeColumnsMode = System.Windows.Forms.DataGridViewAutoSizeColumnsMode.Fill;
            this.detailReportGridView.ColumnHeadersBorderStyle = System.Windows.Forms.DataGridViewHeaderBorderStyle.Single;
            this.detailReportGridView.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            this.detailReportGridView.Columns.AddRange(new System.Windows.Forms.DataGridViewColumn[] {
            this.dataGridViewTextBoxColumn12,
            this.gridDetailTime,
            this.gridDetailLng,
            this.gridDetailLat,
            this.gridDetailSpeed,
            this.gridDetailDoor,
            this.gridDetailMileaage,
            this.googleLinkCol});
            this.detailReportGridView.Name = "detailReportGridView";
            this.detailReportGridView.ReadOnly = true;
            this.detailReportGridView.RowHeadersBorderStyle = System.Windows.Forms.DataGridViewHeaderBorderStyle.Single;
            this.detailReportGridView.RowHeadersVisible = false;
            this.detailReportGridView.RowHeadersWidthSizeMode = System.Windows.Forms.DataGridViewRowHeadersWidthSizeMode.DisableResizing;
            this.detailReportGridView.CellContentClick += new System.Windows.Forms.DataGridViewCellEventHandler(this.detailReportGridView_CellContentClick);
            // 
            // dataGridViewTextBoxColumn12
            // 
            resources.ApplyResources(this.dataGridViewTextBoxColumn12, "dataGridViewTextBoxColumn12");
            this.dataGridViewTextBoxColumn12.Name = "dataGridViewTextBoxColumn12";
            this.dataGridViewTextBoxColumn12.ReadOnly = true;
            // 
            // gridDetailTime
            // 
            resources.ApplyResources(this.gridDetailTime, "gridDetailTime");
            this.gridDetailTime.Name = "gridDetailTime";
            this.gridDetailTime.ReadOnly = true;
            // 
            // gridDetailLng
            // 
            resources.ApplyResources(this.gridDetailLng, "gridDetailLng");
            this.gridDetailLng.Name = "gridDetailLng";
            this.gridDetailLng.ReadOnly = true;
            // 
            // gridDetailLat
            // 
            resources.ApplyResources(this.gridDetailLat, "gridDetailLat");
            this.gridDetailLat.Name = "gridDetailLat";
            this.gridDetailLat.ReadOnly = true;
            // 
            // gridDetailSpeed
            // 
            resources.ApplyResources(this.gridDetailSpeed, "gridDetailSpeed");
            this.gridDetailSpeed.Name = "gridDetailSpeed";
            this.gridDetailSpeed.ReadOnly = true;
            // 
            // gridDetailDoor
            // 
            resources.ApplyResources(this.gridDetailDoor, "gridDetailDoor");
            this.gridDetailDoor.Name = "gridDetailDoor";
            this.gridDetailDoor.ReadOnly = true;
            // 
            // gridDetailMileaage
            // 
            resources.ApplyResources(this.gridDetailMileaage, "gridDetailMileaage");
            this.gridDetailMileaage.Name = "gridDetailMileaage";
            this.gridDetailMileaage.ReadOnly = true;
            // 
            // googleLinkCol
            // 
            resources.ApplyResources(this.googleLinkCol, "googleLinkCol");
            this.googleLinkCol.Name = "googleLinkCol";
            this.googleLinkCol.ReadOnly = true;
            // 
            // menuStrip1
            // 
            this.menuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.fileToolStripMenuItem,
            this.languageToolStripMenuItem,
            this.helpToolStripMenuItem});
            resources.ApplyResources(this.menuStrip1, "menuStrip1");
            this.menuStrip1.Name = "menuStrip1";
            // 
            // fileToolStripMenuItem
            // 
            this.fileToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.exitToolStripMenuItem});
            this.fileToolStripMenuItem.Name = "fileToolStripMenuItem";
            resources.ApplyResources(this.fileToolStripMenuItem, "fileToolStripMenuItem");
            // 
            // exitToolStripMenuItem
            // 
            this.exitToolStripMenuItem.Name = "exitToolStripMenuItem";
            resources.ApplyResources(this.exitToolStripMenuItem, "exitToolStripMenuItem");
            this.exitToolStripMenuItem.Click += new System.EventHandler(this.exitToolStripMenuItem_Click);
            // 
            // languageToolStripMenuItem
            // 
            this.languageToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.englishToolStripMenuItem,
            this.vietnameseToolStripMenuItem});
            this.languageToolStripMenuItem.Name = "languageToolStripMenuItem";
            resources.ApplyResources(this.languageToolStripMenuItem, "languageToolStripMenuItem");
            // 
            // englishToolStripMenuItem
            // 
            this.englishToolStripMenuItem.Name = "englishToolStripMenuItem";
            resources.ApplyResources(this.englishToolStripMenuItem, "englishToolStripMenuItem");
            this.englishToolStripMenuItem.Click += new System.EventHandler(this.englishToolStripMenuItem_Click);
            // 
            // vietnameseToolStripMenuItem
            // 
            this.vietnameseToolStripMenuItem.Name = "vietnameseToolStripMenuItem";
            resources.ApplyResources(this.vietnameseToolStripMenuItem, "vietnameseToolStripMenuItem");
            this.vietnameseToolStripMenuItem.Click += new System.EventHandler(this.vietnameseToolStripMenuItem_Click);
            // 
            // helpToolStripMenuItem
            // 
            this.helpToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.registerToolStripMenuItem,
            this.aboutToolStripMenuItem});
            this.helpToolStripMenuItem.Name = "helpToolStripMenuItem";
            resources.ApplyResources(this.helpToolStripMenuItem, "helpToolStripMenuItem");
            // 
            // registerToolStripMenuItem
            // 
            this.registerToolStripMenuItem.Name = "registerToolStripMenuItem";
            resources.ApplyResources(this.registerToolStripMenuItem, "registerToolStripMenuItem");
            this.registerToolStripMenuItem.Click += new System.EventHandler(this.registerToolStripMenuItem_Click);
            // 
            // aboutToolStripMenuItem
            // 
            this.aboutToolStripMenuItem.Name = "aboutToolStripMenuItem";
            resources.ApplyResources(this.aboutToolStripMenuItem, "aboutToolStripMenuItem");
            // 
            // saveFileDialog1
            // 
            resources.ApplyResources(this.saveFileDialog1, "saveFileDialog1");
            // 
            // openFileDialog1
            // 
            resources.ApplyResources(this.openFileDialog1, "openFileDialog1");
            // 
            // selectFolderDialog
            // 
            resources.ApplyResources(this.selectFolderDialog, "selectFolderDialog");
            this.selectFolderDialog.ShowNewFolderButton = false;
            // 
            // imeiLbl
            // 
            resources.ApplyResources(this.imeiLbl, "imeiLbl");
            this.imeiLbl.Name = "imeiLbl";
            // 
            // statusGrpBox
            // 
            this.statusGrpBox.Controls.Add(this.connectionStatusLbl);
            this.statusGrpBox.Controls.Add(this.retrieveLogPercentLbl);
            this.statusGrpBox.Controls.Add(this.retrieveLogStatusLbl);
            this.statusGrpBox.Controls.Add(this.commuTypeLbl);
            this.statusGrpBox.Controls.Add(this.imeiLbl);
            this.statusGrpBox.Controls.Add(this.retrieveLogPrgBar);
            resources.ApplyResources(this.statusGrpBox, "statusGrpBox");
            this.statusGrpBox.Name = "statusGrpBox";
            this.statusGrpBox.TabStop = false;
            // 
            // connectionStatusLbl
            // 
            resources.ApplyResources(this.connectionStatusLbl, "connectionStatusLbl");
            this.connectionStatusLbl.ForeColor = System.Drawing.Color.Red;
            this.connectionStatusLbl.Name = "connectionStatusLbl";
            // 
            // retrieveLogPercentLbl
            // 
            resources.ApplyResources(this.retrieveLogPercentLbl, "retrieveLogPercentLbl");
            this.retrieveLogPercentLbl.Name = "retrieveLogPercentLbl";
            // 
            // retrieveLogStatusLbl
            // 
            resources.ApplyResources(this.retrieveLogStatusLbl, "retrieveLogStatusLbl");
            this.retrieveLogStatusLbl.Name = "retrieveLogStatusLbl";
            // 
            // commuTypeLbl
            // 
            resources.ApplyResources(this.commuTypeLbl, "commuTypeLbl");
            this.commuTypeLbl.ForeColor = System.Drawing.Color.Red;
            this.commuTypeLbl.Name = "commuTypeLbl";
            // 
            // retrieveLogPrgBar
            // 
            resources.ApplyResources(this.retrieveLogPrgBar, "retrieveLogPrgBar");
            this.retrieveLogPrgBar.Name = "retrieveLogPrgBar";
            // 
            // btnPrint
            // 
            resources.ApplyResources(this.btnPrint, "btnPrint");
            this.btnPrint.Name = "btnPrint";
            // 
            // btnResetDevice
            // 
            resources.ApplyResources(this.btnResetDevice, "btnResetDevice");
            this.btnResetDevice.Name = "btnResetDevice";
            // 
            // btnResetToDefault
            // 
            resources.ApplyResources(this.btnResetToDefault, "btnResetToDefault");
            this.btnResetToDefault.Name = "btnResetToDefault";
            // 
            // btnReadConfig
            // 
            resources.ApplyResources(this.btnReadConfig, "btnReadConfig");
            this.btnReadConfig.Name = "btnReadConfig";
            // 
            // btnWriteConfig
            // 
            resources.ApplyResources(this.btnWriteConfig, "btnWriteConfig");
            this.btnWriteConfig.Name = "btnWriteConfig";
            // 
            // Column9
            // 
            resources.ApplyResources(this.Column9, "Column9");
            this.Column9.Name = "Column9";
            this.Column9.ReadOnly = true;
            // 
            // Column8
            // 
            resources.ApplyResources(this.Column8, "Column8");
            this.Column8.Name = "Column8";
            this.Column8.ReadOnly = true;
            // 
            // Column7
            // 
            resources.ApplyResources(this.Column7, "Column7");
            this.Column7.Name = "Column7";
            this.Column7.ReadOnly = true;
            // 
            // Column6
            // 
            resources.ApplyResources(this.Column6, "Column6");
            this.Column6.Name = "Column6";
            this.Column6.ReadOnly = true;
            // 
            // Column5
            // 
            resources.ApplyResources(this.Column5, "Column5");
            this.Column5.Name = "Column5";
            this.Column5.ReadOnly = true;
            // 
            // Column4
            // 
            resources.ApplyResources(this.Column4, "Column4");
            this.Column4.Name = "Column4";
            this.Column4.ReadOnly = true;
            // 
            // Column3
            // 
            resources.ApplyResources(this.Column3, "Column3");
            this.Column3.Name = "Column3";
            this.Column3.ReadOnly = true;
            // 
            // dataGridViewTextBoxColumn5
            // 
            resources.ApplyResources(this.dataGridViewTextBoxColumn5, "dataGridViewTextBoxColumn5");
            this.dataGridViewTextBoxColumn5.Name = "dataGridViewTextBoxColumn5";
            this.dataGridViewTextBoxColumn5.ReadOnly = true;
            // 
            // dataGridViewTextBoxColumn4
            // 
            resources.ApplyResources(this.dataGridViewTextBoxColumn4, "dataGridViewTextBoxColumn4");
            this.dataGridViewTextBoxColumn4.Name = "dataGridViewTextBoxColumn4";
            this.dataGridViewTextBoxColumn4.ReadOnly = true;
            // 
            // dataGridViewTextBoxColumn3
            // 
            resources.ApplyResources(this.dataGridViewTextBoxColumn3, "dataGridViewTextBoxColumn3");
            this.dataGridViewTextBoxColumn3.Name = "dataGridViewTextBoxColumn3";
            this.dataGridViewTextBoxColumn3.ReadOnly = true;
            // 
            // dataGridViewTextBoxColumn2
            // 
            resources.ApplyResources(this.dataGridViewTextBoxColumn2, "dataGridViewTextBoxColumn2");
            this.dataGridViewTextBoxColumn2.Name = "dataGridViewTextBoxColumn2";
            this.dataGridViewTextBoxColumn2.ReadOnly = true;
            // 
            // dataGridViewTextBoxColumn1
            // 
            resources.ApplyResources(this.dataGridViewTextBoxColumn1, "dataGridViewTextBoxColumn1");
            this.dataGridViewTextBoxColumn1.Name = "dataGridViewTextBoxColumn1";
            this.dataGridViewTextBoxColumn1.ReadOnly = true;
            // 
            // MainForm
            // 
            resources.ApplyResources(this, "$this");
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.tabMain);
            this.Controls.Add(this.menuStrip1);
            this.Controls.Add(this.statusGrpBox);
            this.Name = "MainForm";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.MainForm_FormClosing);
            this.Load += new System.EventHandler(this.MainForm_Load);
            this.tabMain.ResumeLayout(false);
            this.summaryReportTabPage.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.summaryReportGridView)).EndInit();
            this.reportGrpBox.ResumeLayout(false);
            this.reportGrpBox.PerformLayout();
            this.detailReportTabPage.ResumeLayout(false);
            this.groupBox3.ResumeLayout(false);
            this.groupBox3.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.detailReportGridView)).EndInit();
            this.menuStrip1.ResumeLayout(false);
            this.menuStrip1.PerformLayout();
            this.statusGrpBox.ResumeLayout(false);
            this.statusGrpBox.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TabControl tabMain;
        private System.Windows.Forms.MenuStrip menuStrip1;
        private System.Windows.Forms.Label label13;
        private System.Windows.Forms.DateTimePicker reportEndDatePicker;
        private System.Windows.Forms.DateTimePicker reportBeginDatePicker;
        private System.Windows.Forms.Label label12;
        private System.Windows.Forms.GroupBox reportGrpBox;
        private System.Windows.Forms.SaveFileDialog saveFileDialog1;
        private System.Windows.Forms.OpenFileDialog openFileDialog1;
        private System.Windows.Forms.TabPage detailReportTabPage;
        private System.Windows.Forms.DataGridView detailReportGridView;
        private System.Windows.Forms.TabPage summaryReportTabPage;
        private System.Windows.Forms.DataGridView summaryReportGridView;
        private System.Windows.Forms.FolderBrowserDialog selectFolderDialog;
        private System.Windows.Forms.Button outputReportToKMLBtn;
        private System.Windows.Forms.Button readReportFromHardDiskBtn;
        private System.Windows.Forms.Button clearReportBtn;
        private System.Windows.Forms.ToolStripMenuItem fileToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem exitToolStripMenuItem;
        private System.Windows.Forms.Label imeiLbl;
        private System.Windows.Forms.GroupBox statusGrpBox;
        private System.Windows.Forms.ProgressBar retrieveLogPrgBar;
        private System.Windows.Forms.Label retrieveLogStatusLbl;
        private System.Windows.Forms.Label retrieveLogPercentLbl;
        private System.Windows.Forms.Label commuTypeLbl;
        private System.Windows.Forms.ComboBox logIntervalCmbBox;
        private System.Windows.Forms.Label label18;
        private System.Windows.Forms.Button btnPrint;
        private System.Windows.Forms.Button btnResetDevice;
        private System.Windows.Forms.Button btnResetToDefault;
        private System.Windows.Forms.Button btnReadConfig;
        private System.Windows.Forms.Button btnWriteConfig;
        private System.Windows.Forms.DataGridViewTextBoxColumn Column9;
        private System.Windows.Forms.DataGridViewTextBoxColumn Column8;
        private System.Windows.Forms.DataGridViewTextBoxColumn Column7;
        private System.Windows.Forms.DataGridViewTextBoxColumn Column6;
        private System.Windows.Forms.DataGridViewTextBoxColumn Column5;
        private System.Windows.Forms.DataGridViewTextBoxColumn Column4;
        private System.Windows.Forms.DataGridViewTextBoxColumn Column3;
        private System.Windows.Forms.DataGridViewTextBoxColumn dataGridViewTextBoxColumn5;
        private System.Windows.Forms.DataGridViewTextBoxColumn dataGridViewTextBoxColumn4;
        private System.Windows.Forms.DataGridViewTextBoxColumn dataGridViewTextBoxColumn3;
        private System.Windows.Forms.DataGridViewTextBoxColumn dataGridViewTextBoxColumn2;
        private System.Windows.Forms.DataGridViewTextBoxColumn dataGridViewTextBoxColumn1;
        private System.Windows.Forms.Label connectionStatusLbl;
        private System.Windows.Forms.ToolStripMenuItem languageToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem englishToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem vietnameseToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem helpToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem aboutToolStripMenuItem;
        private System.Windows.Forms.GroupBox groupBox3;
        private System.Windows.Forms.ComboBox reportDriverNameCmbBox;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.DataGridViewTextBoxColumn noCol;
        private System.Windows.Forms.DataGridViewTextBoxColumn plateCol;
        private System.Windows.Forms.DataGridViewTextBoxColumn driverCol;
        private System.Windows.Forms.DataGridViewTextBoxColumn licenseCol;
        private System.Windows.Forms.DataGridViewTextBoxColumn startTimeCol;
        private System.Windows.Forms.DataGridViewTextBoxColumn endTimeCol;
        private System.Windows.Forms.DataGridViewTextBoxColumn exSpeedCountCol;
        private System.Windows.Forms.DataGridViewTextBoxColumn doorOpenCountCol;
        private System.Windows.Forms.DataGridViewTextBoxColumn doorCloseCountCol;
        private System.Windows.Forms.DataGridViewTextBoxColumn engineOnCountCol;
        private System.Windows.Forms.DataGridViewTextBoxColumn engineOffCountCol;
        private System.Windows.Forms.DataGridViewTextBoxColumn parkCountCol;
        private System.Windows.Forms.DataGridViewTextBoxColumn contDriveTimeCol;
        private System.Windows.Forms.DataGridViewTextBoxColumn overDrivingTimeCountCol;
        private System.Windows.Forms.DataGridViewTextBoxColumn totalDriveTimeCol;
        private System.Windows.Forms.DataGridViewTextBoxColumn overTotalDrivingTimeCountCol;
        private System.Windows.Forms.DataGridViewTextBoxColumn totalLengthCol;
        private System.Windows.Forms.ToolStripMenuItem registerToolStripMenuItem;
        private System.Windows.Forms.Button readReportFromDeviceBtn;
        private System.Windows.Forms.DataGridViewTextBoxColumn dataGridViewTextBoxColumn12;
        private System.Windows.Forms.DataGridViewTextBoxColumn gridDetailTime;
        private System.Windows.Forms.DataGridViewTextBoxColumn gridDetailLng;
        private System.Windows.Forms.DataGridViewTextBoxColumn gridDetailLat;
        private System.Windows.Forms.DataGridViewTextBoxColumn gridDetailSpeed;
        private System.Windows.Forms.DataGridViewTextBoxColumn gridDetailDoor;
        private System.Windows.Forms.DataGridViewTextBoxColumn gridDetailMileaage;
        private System.Windows.Forms.DataGridViewLinkColumn googleLinkCol;

    }
}

