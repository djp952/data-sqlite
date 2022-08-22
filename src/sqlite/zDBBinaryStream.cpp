//---------------------------------------------------------------------------
// Copyright (c) 2008-2022 Michael G. Brehm
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//---------------------------------------------------------------------------

#include "stdafx.h"					// Include project pre-compiled headers
#include "zDBBinaryStream.h"		// Include zDBBinaryStream declarations

#pragma warning(push, 4)			// Enable maximum compiler warnings

namespace zuki::data::sqlite {

//---------------------------------------------------------------------------
// zdb_binstream_release
//
// Unmanaged callback passed into SQLite when setting a zDBBinaryStream as
// the result of a function or virtual table column query
//
// Arguments:
//
//	pv			- Pointer to the result buffer passed into SQLite

static void zdb_binstream_release(void* pv)
{
	unsigned char*		puBase;					// Pointer to real base address
	GCHandle			gchBinaryStream;		// (-8) Strong GCHandle
	GCHandle			gchStreamBuffer;		// (-4) Pinning GCHandle
	zDBBinaryStream^	binaryStream;			// Actual object reference

	puBase = reinterpret_cast<unsigned char*>(pv) - zDBBinaryStream::RESERVED_SPACE;
	Debug::Assert(!IsBadReadPtr(puBase, zDBBinaryStream::RESERVED_SPACE));

	// Convert the serialized data from the head of the buffer back into a pair
	// of GCHandles.  The first entry is a strong handle to the zDBBinaryStream, 
	// and the second handle is the pinning GCHandle to the byte array.  Yeah,
	// those IntPtr declarations down there are pretty gnarly looking beasts.

	gchBinaryStream = GCHandle::FromIntPtr(IntPtr(*reinterpret_cast<void**>(puBase)));
	gchStreamBuffer = GCHandle::FromIntPtr(IntPtr(*reinterpret_cast<void**>(puBase + sizeof(void*))));

	// Before we release the GCHandle against the zDBBinaryStream, we want to 
	// dispose of it first to ensure the internal MemoryStream gets disposed of

	binaryStream = safe_cast<zDBBinaryStream^>(gchBinaryStream.Target);
	delete binaryStream;
	
	gchBinaryStream.Free();						// Release the strong GCHandle
	gchStreamBuffer.Free();						// Release the pinning GCHandle
}

//---------------------------------------------------------------------------
// zDBBinaryStream Constructor
//
// Arguments:
//
//	NONE

zDBBinaryStream::zDBBinaryStream()
{
	// Create the underlying memory stream object and immediately
	// add in the necessary space for the prepended void pointers

	m_stream = gcnew MemoryStream();
	m_stream->Write(gcnew array<System::Byte>(RESERVED_SPACE), 0, RESERVED_SPACE);
}

//---------------------------------------------------------------------------
// zDBBinaryStream Constructor
//
// Arguments:
//
//	capacity	- Sets the initial capacity of the stream

zDBBinaryStream::zDBBinaryStream(int capacity)
{
	if(capacity < 0) throw gcnew ArgumentOutOfRangeException();

	// Create the underlying memory stream object and immediately
	// add in the necessary space for the prepended pointers

	m_stream = gcnew MemoryStream(capacity + RESERVED_SPACE);
	m_stream->Write(gcnew array<System::Byte>(RESERVED_SPACE), 0, RESERVED_SPACE);
}

//---------------------------------------------------------------------------
// zDBBinaryStream::CanRead::get
//
// Determines if the state of the stream allows READ operations from it

bool zDBBinaryStream::CanRead::get(void)
{
	return !m_disposed;			// As long as we're not disposed, yes
}

//---------------------------------------------------------------------------
// zDBBinaryStream::CanSeek::get
//
// Determines if the position of the stream pointer can be changed

bool zDBBinaryStream::CanSeek::get(void)
{
	return !m_disposed;			// As long as we're not disposed, yes
}

//---------------------------------------------------------------------------
// zDBBinaryStream::CanWrite::get
//
// Determines if the position of the stream pointer can be changed

bool zDBBinaryStream::CanWrite::get(void)
{
	return ((!m_disposed) && (!m_locked));	// Locked or disposed .. no
}

//---------------------------------------------------------------------------
// zDBBinaryStream::Flush
//
// Ensures that any changes to the stream are written to the backing store
// from cache (not really relevant for this stream, but whatever)
//
// Arguments:
//
//	NONE

void zDBBinaryStream::Flush(void)
{
	CHECK_DISPOSED(m_disposed);
	m_stream->Flush();
}

//---------------------------------------------------------------------------
// zDBBinaryStream::Length::get
//
// Exposes the current overall length of the stream.  Does not necessarily
// indicate the size of the backing store.

__int64 zDBBinaryStream::Length::get(void)
{
	CHECK_DISPOSED(m_disposed);
	return (m_stream->Length - RESERVED_SPACE);
}

//---------------------------------------------------------------------------
// zDBBinaryStream::Lock (internal)
//
// Locks down the contents of the MemoryStream, initializes the embedded
// GCHandles needed to keep things pinned and/or alive in unmanaged code,
// and returns a pointer to the DATA in the stream that can be handed off
// to the SQLite engine as-is.  zdb_binstream_release() undoes all of this
//
// Arguments:
//
//	ppv		- Pointer to receieve the base buffer pointer value

int zDBBinaryStream::Lock(void** ppv)
{
	int						cbData;				// Length of the data stream, in bytes
	array<System::Byte>^	rgBuffer;			// Buffer from the MemoryStream
	PinnedBytePtr			pinBuffer;			// Pinned buffer pointer
	GCHandle				gchandle;			// GCHandle structure

	CHECK_DISPOSED(m_disposed);

	*ppv = NULL;									// Initialize [out] pointer

	// Determine the length of the DATA in the memory stream, which does not
	// include the space we reserved at the beginning of it for pointers

	cbData = static_cast<int>(m_stream->Length - RESERVED_SPACE);

	m_stream->Close();								// Close out the MemoryStream
	rgBuffer = m_stream->GetBuffer();				// Get the backing buffer array
	pinBuffer = &rgBuffer[0];						// Pin it locally for direct access

	// The head of the buffer will contain a strong GCHandle against ourselves
	// so the release function can access this instance when it's called

	gchandle = GCHandle::Alloc(this);
	*reinterpret_cast<void**>(pinBuffer) = GCHandle::ToIntPtr(gchandle).ToPointer();

	// Immediately following this class' GCHandle, we throw in an another GCHandle
	// that keeps the managed byte array pinned in place until zdb_binstream_release

	gchandle = GCHandle::Alloc(rgBuffer, GCHandleType::Pinned);
	*reinterpret_cast<void**>(pinBuffer + sizeof(void*)) = GCHandle::ToIntPtr(gchandle).ToPointer();

	*ppv = (pinBuffer + RESERVED_SPACE);			// Copy back pointer to the DATA
	return cbData;									// Return length of the DATA
}

//---------------------------------------------------------------------------
// zDBBinaryStream::OnRelease::get (static)
//
// Exposes the local zdb_binstream_release function pointer

zDBBinaryStream::RELEASEFUNC zDBBinaryStream::OnRelease::get(void)
{
	return zdb_binstream_release;		// Return address of local function
}

//---------------------------------------------------------------------------
// zDBBinaryStream::Position::get
//
// Gets the current absolute position of the stream pointer

__int64 zDBBinaryStream::Position::get(void)
{
	CHECK_DISPOSED(m_disposed);
	return (m_stream->Position - RESERVED_SPACE);
}

//---------------------------------------------------------------------------
// zDBBinaryStream::Position::set
//
// Sets a new absolute position for the stream pointer

void zDBBinaryStream::Position::set(__int64 value)
{
	CHECK_DISPOSED(m_disposed);
	if(m_locked) throw gcnew InvalidOperationException();

	// Make sure they're not trying to do something bad, and then just
	// append the size of the void pointers to the requested position

	if(value < 0) throw gcnew ArgumentOutOfRangeException();
	m_stream->Position = (value + RESERVED_SPACE); 
}

//---------------------------------------------------------------------------
// zDBBinaryStream::Read
//
// Reads a specific number of bytes from the stream into a provided buffer
//
// Arguments:
//
//	buffer		- Buffer to read data into
//	offset		- Offset into buffer to begin writing data
//	count		- Number of bytes of data to be read into buffer

int	zDBBinaryStream::Read(array<System::Byte>^ buffer, int offset, int count)
{
	CHECK_DISPOSED(m_disposed);
	return m_stream->Read(buffer, offset, count);
}

//---------------------------------------------------------------------------
// zDBBinaryStream::Seek
//
// Moves the internal stream pointer to a new position
//
// Arguments:
//
//	offset		- Offset from current location to move
//	origin		- Origin within the stream to move from

__int64	zDBBinaryStream::Seek(__int64 offset, SeekOrigin origin)
{
	CHECK_DISPOSED(m_disposed);

	// Depending on the SeekOrigin, we need to adjust the offset value
	// so that the pointers remain hidden.  For forward seeks, just add
	// the size of the pointers to the offset.  For a reverse seek, make
	// sure that they cannot seek back into the pointer values

	if(origin == SeekOrigin::Begin) offset += RESERVED_SPACE;
	else if(origin == SeekOrigin::Current) offset += RESERVED_SPACE;
	else { if(offset > Length) offset = Length; }

	m_stream->Seek(offset, origin);		// Seek the underlying stream
	return Position;					// Return the new position
}

//---------------------------------------------------------------------------
// zDBBinaryStream:SetLength
//
// Extends or truncates the stream to the specified length
//
// Arguments:
//
//	value		- The new overall length of the stream

void zDBBinaryStream::SetLength(__int64 value)
{
	CHECK_DISPOSED(m_disposed);
	if(m_locked) throw gcnew InvalidOperationException();

	// Make sure they're not trying to do something bad, and then just
	// append the size of the pointers to the requested size

	if(value < 0) throw gcnew ArgumentOutOfRangeException();
	m_stream->SetLength(value + RESERVED_SPACE); 
}

//---------------------------------------------------------------------------
// zDBBinaryStream::Write
//
// Writes a specific number of bytes into the stream from a source array
//
// Arguments:
//
//	buffer		- Buffer containing the data to be written
//	offset		- Offset into the buffer to begin reading from
//	count		- Number of bytes to write into the stream

void zDBBinaryStream::Write(array<System::Byte>^ buffer, int offset, int count)
{
	CHECK_DISPOSED(m_disposed);
	if(m_locked) throw gcnew InvalidOperationException();

	m_stream->Write(buffer, offset, count);
}

//---------------------------------------------------------------------------

} // zuki::data::sqlite

#pragma warning(pop)
