using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Diagnostics;

using System.Windows.Forms;

using System.IO;
using System.IO.Ports;

namespace TestCOM {
	class Program {

		static WAVReader	WAV = null;

//		const double		F = 32.0;	// Channel restitution frequency (KHz)

		static void Main( string[] args ) {

			WAV = new WAVReader( new FileInfo( "../../Test.wav" ) );

			SerialPort	port = null;
			try {
				string[]	portNames = SerialPort.GetPortNames();

//				string		portName = portNames[0];
				string		portName = "COM3";

				port = new SerialPort( portName, 115200, Parity.None, 8, StopBits.One );
				port.Open();

if ( port.BytesToRead > 0 ) {
	Console.WriteLine( "Existing data in port at startup:" );
	Console.WriteLine( port.ReadExisting() );
	port.DiscardInBuffer();
}

				#if true
					//////////////////////////////////////////////////////////////////////////
					// Mono 8-Bits @ 8KHz
					//
					// We need to send regular audio packets.
					// We decided an audio packet is:
					//	1 byte ID = 0xAB
					//	3 bytes timestamp (time unit is a sample)
					//	32 bytes audio payload
					//	-------------------------
					//	36 bytes total
					//
					// We target a frequency of 8KHz, we need to send 32 samples of 8-bits mono audio data
					//	so the actual frequency is 8000 / 32 = 250 packets per second or 250 * 36 = 9000 Bps or 72000 bps
					//
					// In any case, we need to get ready to transfer packets 250 times per second
					//
//const double	PACKETS_PER_SECOND = 250.0;
const double	PACKETS_PER_SECOND = 1.0;

					// Pre-read the entire WAV file
					WAV.PreReadAll_Mono8Bits( 8000 );

					Stopwatch	watch = new Stopwatch();
					double		frequency = 1.0 / Stopwatch.Frequency;
					long		nextUpload_ticks = 0;
					double		seconds = 0;
					long		ticks = 0;

					uint		sampleIndex = 0;
					uint		timeStamp = 0;
					byte[]		audioPacket = new byte[36];
								audioPacket[0] = 0xAB;	// Packet ID

uint[]		DEBUG_samplesIndices = new uint[256];
uint[]		DEBUG_timeStamps = new uint[256];
byte		DEBUG_counter = 0;

					byte[]		responseBuffer = new byte[256];
					uint		nextDebugSecond = 0;



//port.Write( responseBuffer, 0, 4 );



					watch.Start();
					while ( true ) {
// 						if ( Control.ModifierKeys != Keys.None )
// 							break;


						ticks = watch.ElapsedTicks;
						seconds = ticks * frequency;


//////////////////////////////////////////////////////////////////////////
// Always check for packets to read
int		count = port.BytesToRead;
//if ( count >= 36 ) {
if ( count > 0 && (count % 36) == 0 ) {
	if ( count > 36 ) {
		// ???
		Console.WriteLine( "Long packet! {0} instead of expected 36 bytes!", count );
//	port.DiscardInBuffer();
	}

	// Read debug values sent from Arduino
	port.Read( responseBuffer, 0, count );
//	if ( seconds > nextDebugSecond ) {
	{
		// Allow one debug a second to avoid losing time formatting and outputting strings
		string	A = "0x";
		for ( int i=0; i < count; i++ )
			A += responseBuffer[i].ToString( "X2" );

		Console.WriteLine( A );
		nextDebugSecond++;
	}
} else if ( count > 0 ) {
	// Data to read but less than 36 bytes... Let's wait until the packet is complete!
}
//////////////////////////////////////////////////////////////////////////


						if ( ticks < nextUpload_ticks )
							continue;	// Wait for next upload

// 						// Use the current time to estimate sample index
// 						sampleIndex = (uint) (seconds * 8000);
// DEBUG_samplesIndices[DEBUG_counter] = sampleIndex;
// 
// 						// Update timestamp
// 						timeStamp = sampleIndex >> 5;	// Time stamp is the amount of packets, not the amount of samples
// DEBUG_timeStamps[DEBUG_counter] = timeStamp;
// 
// 						audioPacket[3] = (byte) timeStamp;	timeStamp >>= 8;
// 						audioPacket[2] = (byte) timeStamp;	timeStamp >>= 8;
// 						audioPacket[1] = (byte) timeStamp;
// 
// DEBUG_counter++;
// 
// 						// Fetch payload
// 						WAV.FetchData( sampleIndex, audioPacket, 3, 32 );


for ( int i=0; i < 36; i++ )
	audioPacket[i] = (byte) i;

						// Write packet
						port.Write( audioPacket, 0, 36 );

						// Compute time for next upload
						nextUpload_ticks = (long) Math.Floor( (seconds + 1.0 / PACKETS_PER_SECOND) / frequency );	// We need a specific packets/second frequency to reach our target frquency
					}


				#elif true
					//////////////////////////////////////////////////////////////////////////
					// Stereo 8-Bits at ??
					//
					// Pre-read the entire WAV file
//					WAV.PreReadAll_Stereo8Bits();
TODO! Laissé en plan!
					byte[]	sineWaveL = new byte[256];
					byte[]	sineWaveR = new byte[256];

					double	f = 1.0;	// 1KHz
					double	v = 2.0f * Math.PI * f / F;
					for ( int i=0; i < 256; i++ ) {
						sineWaveL[i] = (byte) (127 + 127 * Math.Sin( v * i ));
						sineWaveR[i] = (byte) (127 + 127 * Math.Sin( v * i ));
	//					sineWave[i] = 64;
					}

					Stopwatch	watch = new Stopwatch();
					double		frequency = 1.0 / Stopwatch.Frequency;
// 					long		lastUpload = 0;
					long		nextUpload_ticks = 0;
					double		seconds = 0;
					long		ticks = 0;

					watch.Start();
					while ( true ) {
						if ( Control.ModifierKeys != Keys.None )
							break;

						ticks = watch.ElapsedTicks;
						if ( ticks < nextUpload_ticks )
							continue;	// Wait for next upload

						nextUpload_ticks += 10;
						seconds = ticks * frequency;

						WAV.FetchData( seconds, sineWaveL, sineWaveR, 256 );

						port.Write( sineWaveL, 0, 256 );
						port.Write( sineWaveR, 0, 256 );
					}
				#else

				byte		counter = 0;
				DateTime	startTime = DateTime.Now;

				while ( true ) {
//						port.Write( sineWave, counter, 1 );
// 						counter++;

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

//						System.Threading.Thread.Sleep( 1 );

						if ( Control.ModifierKeys != Keys.None )
							break;
					}
				#endif

			} catch ( Exception _e ) {
				MessageBox.Show( "An error occurred:\r\n" + _e.Message, "Error!", MessageBoxButtons.OK, MessageBoxIcon.Error );
			} finally {
				port.Close(); 
			}
		}
	}
}
