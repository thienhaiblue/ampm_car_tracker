namespace Gen_Product_Key
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
            this.generateBtn = new System.Windows.Forms.Button();
            this.productKeyTextBox = new System.Windows.Forms.TextBox();
            this.label2 = new System.Windows.Forms.Label();
            this.activationCodeTxtBox = new System.Windows.Forms.TextBox();
            this.label1 = new System.Windows.Forms.Label();
            this.SuspendLayout();
            // 
            // generateBtn
            // 
            this.generateBtn.Font = new System.Drawing.Font("Microsoft Sans Serif", 10F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.generateBtn.ForeColor = System.Drawing.Color.Red;
            this.generateBtn.Location = new System.Drawing.Point(194, 100);
            this.generateBtn.Name = "generateBtn";
            this.generateBtn.Size = new System.Drawing.Size(179, 35);
            this.generateBtn.TabIndex = 10;
            this.generateBtn.Text = "GENERATE";
            this.generateBtn.UseVisualStyleBackColor = true;
            this.generateBtn.Click += new System.EventHandler(this.generateBtn_Click);
            // 
            // productKeyTextBox
            // 
            this.productKeyTextBox.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.productKeyTextBox.Location = new System.Drawing.Point(117, 64);
            this.productKeyTextBox.Name = "productKeyTextBox";
            this.productKeyTextBox.Size = new System.Drawing.Size(172, 20);
            this.productKeyTextBox.TabIndex = 9;
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(15, 67);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(65, 13);
            this.label2.TabIndex = 8;
            this.label2.Text = "Product Key";
            // 
            // activationCodeTxtBox
            // 
            this.activationCodeTxtBox.BackColor = System.Drawing.SystemColors.Window;
            this.activationCodeTxtBox.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.activationCodeTxtBox.Location = new System.Drawing.Point(117, 26);
            this.activationCodeTxtBox.Name = "activationCodeTxtBox";
            this.activationCodeTxtBox.Size = new System.Drawing.Size(172, 20);
            this.activationCodeTxtBox.TabIndex = 7;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(15, 29);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(82, 13);
            this.label1.TabIndex = 6;
            this.label1.Text = "Activation Code";
            // 
            // MainForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(395, 147);
            this.Controls.Add(this.generateBtn);
            this.Controls.Add(this.productKeyTextBox);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.activationCodeTxtBox);
            this.Controls.Add(this.label1);
            this.Name = "MainForm";
            this.Text = "AMBO";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Button generateBtn;
        private System.Windows.Forms.TextBox productKeyTextBox;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.TextBox activationCodeTxtBox;
        private System.Windows.Forms.Label label1;
    }
}

