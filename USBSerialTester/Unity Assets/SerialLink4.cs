using System.Collections;
using System.Collections.Generic;
using UnityEngine;
/*
 * NOTE: TO use Windows 10 USB COM port for serial
 * Edit > ProjectSettings > Player > Settings for PC,MAC & Linux Standalone > 
 *   Other Settings > Configuration > ApiCompatibilityLevel .. use .NET 2.0 
 */
using System.IO.Ports;

using System;
using System.Threading;

public class SerialLink4 : MonoBehaviour {
	public MsgQueue testQueue;
	public string msg;

	private static SerialPort sp;

	private Thread readThread;
	private bool runThread;

	float timeWait = 0;

	// Use this for initialization
	void Start () {
		readThread = new Thread (readProc);

		sp = new SerialPort ("COM8", 9600, Parity.None, 8, StopBits.One);
		sp.NewLine = Environment.NewLine;		// CRLF
		sp.ReadTimeout = 5;
		sp.WriteTimeout = 5;
		sp.Open ();
		if (sp.IsOpen) {
			Debug.Log ("COM8 open");
		} else {
			Debug.Log ("COM8 failed");
		}
		runThread = true;
		readThread.Start ();
	}

	// Update is called once per frame
	void Update () {
		if(timeWait < Time.time) {			// time in seconds since start of game.
			++timeWait;
			try {
				switch((int)timeWait % 4) {
				case 0:
					sp.Write("C");
					break;
				case 1:
					sp.Write("B");
					break;
				case 2:
					sp.Write("R");
					break;
				case 3:
					sp.Write("G");
					break;
				}
			}
			catch (Exception ew) { }
		}
	}

	void OnDestroy() {
		sp.Close ();
		runThread = false;
	}

	void readProc() {
		while (runThread) {
			try {
				// For ReadLine() to work, the last two bytes must be CR(13) LF(10)
				// Drops first character. So use @. Check and remove when parsing.
				msg = "IN: " + sp.ReadLine();
				Debug.Log(msg);
				testQueue.comMsgQueue.Enqueue(msg);		// simple as this?
			}
			catch (Exception ex) {	}

			Thread.Sleep (100);       // for lower values, ReadLine() starts
			// dropping the first character of the line received.
		}
	}

}
