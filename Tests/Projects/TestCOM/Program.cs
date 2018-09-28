using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using System.Windows.Forms;

using System.IO.Ports;

namespace TestCOM {
	class Program {
		static void Main( string[] args ) {

			SerialPort	port = null;
			try {
				string[]	portNames = SerialPort.GetPortNames();

//				string		portName = portNames[0];
				string		portName = "COM3";

				port = new SerialPort( portName, 115200 );
				port.Open();

// 				byte	b = (byte)myserialPort.ReadByte(); ///read a byte 
// 				char	c = (char)myserialPort.ReadChar(); // read a char 
// 				string	line = (string)myserialPort.ReadLine(); //read a whole line 
// 				string	all = (string)myserialPort.ReadExisting(); //read everythin in the buffer 

				byte[]	sineWave = new byte[256];
				byte	counter = 0;
				for ( int i=0; i < 256; i++ )
					sineWave[i] = (byte) (127 + 127 * Math.Sin( 2 * Math.PI * i / 255 ));
//					sineWave[i] = 64;

				DateTime	startTime = DateTime.Now;
				while ( true ) {

//					port.Write( sineWave, counter, 1 );
// 					counter++;

					double	seconds = (DateTime.Now - startTime).TotalSeconds;
					double	F = 0.2 + 8.0 * (1.0 + Math.Cos( seconds ));

					for ( int i=0; i < 256; i++ )
						sineWave[i] = (byte) (127 + 127 * Math.Sin( 2 * Math.PI * F * i / 255 ));
					port.Write( sineWave, counter, 256 );

//					System.Threading.Thread.Sleep( 1 );

					if ( Control.ModifierKeys != Keys.None )
						break;
				}

			} catch ( Exception _e ) {
				MessageBox.Show( "An error occurred:\r\n" + _e.Message, "Error!", MessageBoxButtons.OK, MessageBoxIcon.Error );
			} finally {
				port.Close(); 
			}
		}
	}
}
