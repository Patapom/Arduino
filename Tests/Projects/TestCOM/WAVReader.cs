using System;
using System.Collections.Generic;
using System.IO;

namespace TestCOM {
	public class	WAVReader {

		FileStream		m_stream;
		BinaryReader	m_reader;

		public ushort		m_channelsCount;
		public uint			m_frequencyHz;
		public uint			m_bytesPerSecond;	// Frequency * Channels count * bits per sample / 8
		public ushort		m_bytesPerBlock;	// Channels count * bits per sample / 8
		public ushort		m_bitsPerSample;	// Amount of bits per sample

		public ulong		m_dataStartOffset;	// Offset where WAV data start
		public uint			m_dataSize;			// Data size

		public uint			m_position;			// Position within WAV data

		// Pre-read data
		public uint			m_samplesCount;
		public byte[]		m_data;

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

		/// <summary>
		/// Fetch data from a mono buffer
		/// </summary>
		/// <param name="_sampleIndex"></param>
		/// <param name="_buffer"></param>
		/// <param name="_count"></param>
		public void		FetchData( uint _sampleIndex, byte[] _buffer, uint _index, uint _count ) {
			for ( uint i=_index; _count > 0; _count--, i++ ) {
				_sampleIndex %= m_samplesCount;
				_buffer[i] = m_data[_sampleIndex];
				_sampleIndex++;
			}
		}

		/// <summary>
		/// Fetch data from a stereo buffer
		/// </summary>
		/// <param name="_seconds"></param>
		/// <param name="_left"></param>
		/// <param name="_right"></param>
		/// <param name="_count"></param>
		public void		FetchData( double _seconds, byte[] _left, byte[] _right, uint _count ) {
			long	sampleIndex = (long) (_seconds * m_frequencyHz);
			sampleIndex %= m_samplesCount;
			for ( uint i=0; _count > 0; _count--, i++ ) {
				_left[i] = m_data[2*sampleIndex];
				_right[i] = m_data[2*sampleIndex+1];
				sampleIndex++;
				sampleIndex %= m_samplesCount;
			}
		}

		/// <summary>
		/// Pre-reads the entire WAV file into a stereo 8-bits format
		/// </summary>
		/// <param name="_frequencyKHz">Frequency of target buffer, in KHz</param>
		public void	PreReadAll_Mono8Bits( float _frequencyKHz ) {
			uint	bytesPerSample = (uint) m_bitsPerSample >> 3;

			uint	sourceSamplesCount = m_dataSize / m_bytesPerBlock;

			m_samplesCount = (uint) Math.Ceiling( sourceSamplesCount * _frequencyKHz / m_frequencyHz );
			m_data = new byte[m_samplesCount];	// We expect 8-bits mono

			byte[]	sourceData = new byte[m_dataSize];
			m_reader.Read( sourceData, 0, (int) m_dataSize );

			if ( bytesPerSample == 1 ) {
				// 8-bits
				for ( uint targetSampleIndex=0; targetSampleIndex < m_samplesCount; targetSampleIndex++ ) {
					uint	sourceSampleIndex = sourceSamplesCount * targetSampleIndex / m_samplesCount;
					uint	sourceSampleOffset = m_bytesPerBlock * sourceSampleIndex;
					m_data[targetSampleIndex] = sourceData[sourceSampleOffset];		// Grab left channel
				}
			} else {
				// 16-bits
				long	V;
				byte	v;
				for ( uint targetSampleIndex=0; targetSampleIndex < m_samplesCount; targetSampleIndex++ ) {
					uint	sourceSampleIndex = sourceSamplesCount * targetSampleIndex / m_samplesCount;
					uint	sourceSampleOffset = m_bytesPerBlock * sourceSampleIndex;

					// 16-bit WAVs are signed
					V = (short) ((sourceData[sourceSampleOffset+1] << 8) | sourceData[sourceSampleOffset+0]);	// Grab left channel
					V += 32768;
					V >>= 8;
					v = (byte) V;

					m_data[targetSampleIndex] = v;
				}
			}
		}

		/// <summary>
		/// Pre-reads the entire WAV file into a stereo 8-bits format
		/// </summary>
		public void	PreReadAll_Stereo8Bits() {
			uint	bytesPerSample = (uint) m_bitsPerSample >> 3;

			m_samplesCount = m_dataSize / m_bytesPerBlock;

//m_samplesCount = 4096;

			m_data = new byte[2*m_samplesCount];	// We expected 8-bits stereo

			uint	dataIndex = 0;
			if ( bytesPerSample == 1 ) {
				// 8-bits
				switch ( m_channelsCount ) {
					case 1:	// MONO
						for ( uint sampleIndex=0; sampleIndex < m_samplesCount; sampleIndex++ ) {
							m_data[dataIndex++] = m_reader.ReadByte();
							m_data[dataIndex++] = m_data[dataIndex-1];	// Duplicate to right channel
						}
						break;

					case 2:	// STEREO (data are already the target size & format)
						if ( m_dataSize != m_data.Length )
							throw new Exception( "Data size is not the expected length!" );
						m_reader.Read( m_data, 0, (int) m_dataSize );
						break;
				}
			} else {
				// 16-bits
				byte	temp;
				switch ( m_channelsCount ) {
					case 1:	// MONO
						for ( uint sampleIndex=0; sampleIndex < m_samplesCount; sampleIndex++ ) {
							temp = (byte) (m_reader.ReadUInt16() >> 8);
							m_data[dataIndex++] = temp;
							m_data[dataIndex++] = temp;
						}
						break;

					case 2:	// STEREO
						for ( uint sampleIndex=0; sampleIndex < m_samplesCount; sampleIndex++ ) {
							temp = (byte) (m_reader.ReadUInt16() >> 8);
							m_data[dataIndex++] = temp;
							temp = (byte) (m_reader.ReadUInt16() >> 8);
							m_data[dataIndex++] = temp;
						}
						break;
				}
			}
		}

		void	ReadChunk_Format( BinaryReader R ) {
			uint	chunkID = R.ReadUInt32();
			if ( chunkID != 0x20746D66 )
				throw new Exception( "Wrong format chunk ID!" ) ;
			uint	chunkSize = R.ReadUInt32();

			UInt16	format = R.ReadUInt16();
			if ( format != 1 )
				throw new Exception( "Expected PCM format!" );

			m_channelsCount = R.ReadUInt16();
			if ( m_channelsCount > 2 )
				throw new Exception( "Unsupported channels count!" );

			m_frequencyHz = R.ReadUInt32();
			m_bytesPerSecond = R.ReadUInt32();
			m_bytesPerBlock = R.ReadUInt16();
			m_bitsPerSample = R.ReadUInt16();
		}
	}
}
