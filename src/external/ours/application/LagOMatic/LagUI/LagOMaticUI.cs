using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;
using System.Data;
using System.Net.Sockets;
using System.Net;
using System.Text;

namespace LagUI
{
	/// <summary>
	/// Summary description for LagOMaticUI.
	/// </summary>
	public class LagOMaticUI : System.Windows.Forms.Form
	{
		private System.Windows.Forms.Label recvLabel;
		private System.Windows.Forms.Label sendLabel;
		private System.Windows.Forms.Label packetLossLabel;
		private System.Windows.Forms.TrackBar recvTrackBar;
		private System.Windows.Forms.TrackBar sendTrackBar;
		private System.Windows.Forms.TrackBar packetLossTrackBar;
		private System.Windows.Forms.Label totalLatencyLabel;
		private System.ComponentModel.IContainer components;

		private Socket    m_socket;
		private EndPoint  m_destAddress;
		private System.Windows.Forms.Timer updateTimer;
		private const int cm_cpListenPort = 2373;

		public LagOMaticUI()
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

			//
			// TODO: Add any constructor code after InitializeComponent call
			//
			m_socket = new Socket(
				AddressFamily.InterNetwork,
				SocketType.Dgram,
				ProtocolType.Udp );

			m_destAddress = new IPEndPoint( IPAddress.Loopback, cm_cpListenPort );

			sendUpdate();

		}

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		protected override void Dispose( bool disposing )
		{
			if( disposing )
			{
				if (components != null) 
				{
					components.Dispose();
				}
			}
			base.Dispose( disposing );
		}

		#region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			this.components = new System.ComponentModel.Container();
			this.recvLabel = new System.Windows.Forms.Label();
			this.sendLabel = new System.Windows.Forms.Label();
			this.packetLossLabel = new System.Windows.Forms.Label();
			this.recvTrackBar = new System.Windows.Forms.TrackBar();
			this.sendTrackBar = new System.Windows.Forms.TrackBar();
			this.packetLossTrackBar = new System.Windows.Forms.TrackBar();
			this.totalLatencyLabel = new System.Windows.Forms.Label();
			this.updateTimer = new System.Windows.Forms.Timer(this.components);
			((System.ComponentModel.ISupportInitialize)(this.recvTrackBar)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.sendTrackBar)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.packetLossTrackBar)).BeginInit();
			this.SuspendLayout();
			// 
			// recvLabel
			// 
			this.recvLabel.Location = new System.Drawing.Point(8, 8);
			this.recvLabel.Name = "recvLabel";
			this.recvLabel.Size = new System.Drawing.Size(152, 16);
			this.recvLabel.TabIndex = 0;
			this.recvLabel.Text = "Receive Latency";
			// 
			// sendLabel
			// 
			this.sendLabel.Location = new System.Drawing.Point(8, 72);
			this.sendLabel.Name = "sendLabel";
			this.sendLabel.Size = new System.Drawing.Size(152, 16);
			this.sendLabel.TabIndex = 1;
			this.sendLabel.Text = "Send Latency";
			// 
			// packetLossLabel
			// 
			this.packetLossLabel.Location = new System.Drawing.Point(8, 136);
			this.packetLossLabel.Name = "packetLossLabel";
			this.packetLossLabel.Size = new System.Drawing.Size(168, 16);
			this.packetLossLabel.TabIndex = 2;
			this.packetLossLabel.Text = "Packet Loss";
			// 
			// recvTrackBar
			// 
			this.recvTrackBar.LargeChange = 50;
			this.recvTrackBar.Location = new System.Drawing.Point(8, 24);
			this.recvTrackBar.Maximum = 1000;
			this.recvTrackBar.Name = "recvTrackBar";
			this.recvTrackBar.Size = new System.Drawing.Size(280, 45);
			this.recvTrackBar.TabIndex = 3;
			this.recvTrackBar.TickFrequency = 100;
			this.recvTrackBar.ValueChanged += new System.EventHandler(this.recvTrackBar_ValueChanged);
			// 
			// sendTrackBar
			// 
			this.sendTrackBar.LargeChange = 50;
			this.sendTrackBar.Location = new System.Drawing.Point(8, 88);
			this.sendTrackBar.Maximum = 1000;
			this.sendTrackBar.Name = "sendTrackBar";
			this.sendTrackBar.Size = new System.Drawing.Size(280, 45);
			this.sendTrackBar.TabIndex = 4;
			this.sendTrackBar.TickFrequency = 100;
			this.sendTrackBar.ValueChanged += new System.EventHandler(this.sendTrackBar_ValueChanged);
			// 
			// packetLossTrackBar
			// 
			this.packetLossTrackBar.Location = new System.Drawing.Point(8, 152);
			this.packetLossTrackBar.Maximum = 100;
			this.packetLossTrackBar.Name = "packetLossTrackBar";
			this.packetLossTrackBar.Size = new System.Drawing.Size(280, 45);
			this.packetLossTrackBar.TabIndex = 5;
			this.packetLossTrackBar.TickFrequency = 10;
			this.packetLossTrackBar.ValueChanged += new System.EventHandler(this.packetLossTrackBar_ValueChanged);
			// 
			// totalLatencyLabel
			// 
			this.totalLatencyLabel.Location = new System.Drawing.Point(168, 8);
			this.totalLatencyLabel.Name = "totalLatencyLabel";
			this.totalLatencyLabel.Size = new System.Drawing.Size(112, 16);
			this.totalLatencyLabel.TabIndex = 6;
			this.totalLatencyLabel.Text = "--";
			// 
			// updateTimer
			// 
			this.updateTimer.Interval = 1000;
			this.updateTimer.Tick += new System.EventHandler(this.updateTimer_Tick);
			// 
			// LagOMaticUI
			// 
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.ClientSize = new System.Drawing.Size(296, 206);
			this.Controls.Add(this.totalLatencyLabel);
			this.Controls.Add(this.packetLossTrackBar);
			this.Controls.Add(this.sendTrackBar);
			this.Controls.Add(this.recvTrackBar);
			this.Controls.Add(this.packetLossLabel);
			this.Controls.Add(this.sendLabel);
			this.Controls.Add(this.recvLabel);
			this.Name = "LagOMaticUI";
			this.Text = "Lag-O-Matic UI";
			((System.ComponentModel.ISupportInitialize)(this.recvTrackBar)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.sendTrackBar)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.packetLossTrackBar)).EndInit();
			this.ResumeLayout(false);

		}
		#endregion

		/// <summary>
		/// The main entry point for the application.
		/// </summary>
		[STAThread]
		static void Main() 
		{
			Application.Run(new LagOMaticUI());
		}

		private void recvTrackBar_ValueChanged(object sender, System.EventArgs e)
		{
			sendUpdate();
		}

		private void sendTrackBar_ValueChanged(object sender, System.EventArgs e)
		{
			sendUpdate();
		}

		private void packetLossTrackBar_ValueChanged(object sender, System.EventArgs e)
		{
			sendUpdate();
		}

		private void sendUpdate()
		{
			recvLabel.Text = String.Format( "Receive Latency ({0}ms)", recvTrackBar.Value );
			sendLabel.Text = String.Format( "Send Latency ({0}ms)", sendTrackBar.Value );
			packetLossLabel.Text = String.Format( "Packet Loss ({0}%)", packetLossTrackBar.Value );
			totalLatencyLabel.Text = String.Format( "total: {0}ms", recvTrackBar.Value + sendTrackBar.Value );

			String msgText = String.Format( "{0} {1} {2}", recvTrackBar.Value, sendTrackBar.Value, packetLossTrackBar.Value );
			Byte [] msgByteArray = ASCIIEncoding.ASCII.GetBytes( msgText );
			m_socket.SendTo( msgByteArray, m_destAddress );
		}

		private void updateTimer_Tick(object sender, System.EventArgs e)
		{
			sendUpdate();
		}
	}
}
