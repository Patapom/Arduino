using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using System.Windows.Forms;

using System.IO.Ports;

namespace TestCOM {
	class Program {

		const double	F = 32.0;	// Channel restitution frequency (KHz)

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

				byte[]	sineWaveL = new byte[256];
				byte[]	sineWaveR = new byte[256];

				double	f = 1.0;	// 1KHz
				double	v = 2.0f * Math.PI * f / F;
				for ( int i=0; i < 256; i++ ) {
					sineWaveL[i] = (byte) (127 + 127 * Math.Sin( v * i ));
					sineWaveR[i] = (byte) (127 + 127 * Math.Sin( v * i ));
//					sineWave[i] = 64;
				}

				byte		counter = 0;
				DateTime	startTime = DateTime.Now;
				while ( true ) {

//					port.Write( sineWave, counter, 1 );
// 					counter++;

					double	seconds = (DateTime.Now - startTime).TotalSeconds;
					double	t = 0.5 * (1.0 + Math.Sin( 2 * Math.PI * seconds / 10.0 ));

					double	fl = 0.1f + 1.9f * t;		// Interpolate between 100Hz and 2KHz
					double	fr = 0.1f + 1.9f * (1.0-t);	// Interpolate between 100Hz and 2KHz
					double	vl = 2.0f * Math.PI * fl / F;
					double	vr = 2.0f * Math.PI * fr / F;

					for ( int i=0; i < 256; i++ ) {
						sineWaveL[i] = (byte) (127 + 127 * Math.Sin( vl * i ));
						sineWaveR[i] = (byte) (127 + 127 * Math.Sin( vr * i ));
					}
					port.Write( sineWaveL, counter, 256 );
					port.Write( sineWaveR, counter, 256 );

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
