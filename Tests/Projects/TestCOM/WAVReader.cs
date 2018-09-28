using System;
using System.Collections.Generic;
using System.IO;

namespace TestCOM {
	public class	WAVReader {

		FileStream		m_stream;
		BinaryReader	m_reader;

		public enum		CHANNELS {
			MONO,
			STEREO
		}
		public CHANNELS		m_channels;
		public uint			m_frequencyHz;
		public uint			m_bytesPerSecond;	// Frequency * Channels count * bits per sample / 8
		public ushort		m_bytesPerBlock;	// Channels count * bits per sample / 8
		public ushort		m_bitsPerSample;	// Amount of bits per sample

		public ulong		m_dataStartOffset;	// Offset where WAV data start
		public uint			m_dataSize;			// Data size

		public uint			m_position;			// Position within WAV data

		public	WAVReader( FileInfo _fileName ) {
			m_stream = _fileName.OpenRead();
			m_reader = new BinaryReader( m_stream );

			uint	FOURCC = m_reader.ReadUInt32();
			if ( FOURCC != 0x46464952 )
				throw new Exception( "Wrong FourCC: RIFF expected" );

			uint	chunkSize = m_reader.ReadUInt32();

			// Read "WAVE" signature
			FOURCC = m_reader.ReadUInt32();
			if ( FOURCC != 0x45564157 )
				throw new Exception( "Wrong FourCC: WAVE expected" );

			// Read mandatory format chunk
			ReadChunk_Format( m_reader );

			// Skip optional chunks until WAVE chunk
			uint	chunkID = m_reader.ReadUInt32();
			while ( chunkID != 0x61746164 ) {
				chunkSize = m_reader.ReadUInt32();
				m_stream.Seek( chunkSize, SeekOrigin.Current );	// Skip chunk
			}
			m_dataSize = m_reader.ReadUInt32();
			m_dataStartOffset = (ulong) m_stream.Position;
		}

		void	ReadChunk_Format( BinaryReader R ) {
			uint	chunkID = R.ReadUInt32();
			if ( chunkID != 0x20746D66 )
				throw new Exception( "Wrong format chunk ID!" ) ;
			uint	chunkSize = R.ReadUInt32();

			UInt16	format = R.ReadUInt16();
			if ( format != 1 )
				throw new Exception( "Expected PCM format!" );

			m_channels = (CHANNELS) R.ReadUInt16();
			m_frequencyHz = R.ReadUInt32();
			m_bytesPerSecond = R.ReadUInt32();
			m_bytesPerBlock = R.ReadUInt16();
			m_bitsPerSample = R.ReadUInt16();
		}
	}
}
