/****************************************************************************
**
** Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of a Qt Solutions component.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Solutions Commercial License Agreement provided
** with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and Nokia.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain
** additional rights. These rights are described in the Nokia Qt LGPL
** Exception version 1.1, included in the file LICENSE.NOKIA-LGPL-EXCEPTION
** in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL-3 included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
** Please note Third Party Software included with Qt Solutions may impose
** additional restrictions and it is the user's responsibility to ensure
** that they have met the licensing requirements of the GPL, LGPL, or Qt
** Solutions Commercial license and the relevant license of the Third
** Party Software they are using.
**
** If you are unsure which license is appropriate for your use, please
** contact Nokia at qt-info@nokia.com.
**
****************************************************************************/

#include "qtiocompressor.h"
#include <zlib.h>

typedef Bytef ZlibByte;
typedef uInt ZlibSize;

class QtIOCompressorPrivate {
    QtIOCompressor *q_ptr;
    Q_DECLARE_PUBLIC(QtIOCompressor)
public:
    enum State {
        // Read state
        NotReadFirstByte,
        InStream,
        EndOfStream,
        // Write state
        NoBytesWritten,
        BytesWritten,
        // Common
        Closed,
        Error
    };

    QtIOCompressorPrivate(QtIOCompressor *q_ptr, QIODevice *device, int compressionLevel, int bufferSize);
    ~QtIOCompressorPrivate();
    void flushZlib(int flushMode);
    bool writeBytes(ZlibByte *buffer, ZlibSize outputSize);
    void setZlibError(const QString &erroMessage, int zlibErrorCode);

    QIODevice *device;
    bool manageDevice;
    z_stream zlibStream;
    const int compressionLevel;
    const ZlibSize bufferSize;
    ZlibByte *buffer;
    State state;
    QtIOCompressor::StreamFormat streamFormat;
};

/*!
    \internal
*/
QtIOCompressorPrivate::QtIOCompressorPrivate(QtIOCompressor *q_ptr, QIODevice *device, int compressionLevel, int bufferSize)
:q_ptr(q_ptr)
,device(device)
,compressionLevel(compressionLevel)
,bufferSize(bufferSize)
,buffer(new ZlibByte[bufferSize])
,state(Closed)
,streamFormat(QtIOCompressor::ZlibFormat)
{
    // Use default zlib memory management.
    zlibStream.zalloc = Z_NULL;
    zlibStream.zfree = Z_NULL;
    zlibStream.opaque = Z_NULL;
}

/*!
    \internal
*/
QtIOCompressorPrivate::~QtIOCompressorPrivate()
{
    delete[] buffer;
}

/*!
    \internal
    Flushes the zlib stream.
*/
void QtIOCompressorPrivate::flushZlib(int flushMode)
{
    // No input.
    zlibStream.next_in = 0;
    zlibStream.avail_in = 0;
    int status;
    do {
        zlibStream.next_out = buffer;
        zlibStream.avail_out = bufferSize;
        status = deflate(&zlibStream, flushMode);
        if (status != Z_OK && status != Z_STREAM_END) {
            state = QtIOCompressorPrivate::Error;
            setZlibError(QT_TRANSLATE_NOOP("QtIOCompressor", "Internal zlib error when compressing: "), status);
            return;
        }

        ZlibSize outputSize = bufferSize - zlibStream.avail_out;

        // Try to write data from the buffer to to the underlying device, return on failure.
        if (!writeBytes(buffer, outputSize))
            return;

    // If the mode is Z_FNISH we must loop until we get Z_STREAM_END,
    // else we loop as long as zlib is able to fill the output buffer.
    } while ((flushMode == Z_FINISH && status != Z_STREAM_END) || (flushMode != Z_FINISH && zlibStream.avail_out == 0));

    if (flushMode == Z_FINISH)
        Q_ASSERT(status == Z_STREAM_END);
    else
        Q_ASSERT(status == Z_OK);
}

/*!
    \internal
    Writes outputSize bytes from buffer to the underlying device.
*/
bool QtIOCompressorPrivate::writeBytes(ZlibByte *buffer, ZlibSize outputSize)
{
    Q_Q(QtIOCompressor);
    ZlibSize totalBytesWritten = 0;
    // Loop until all bytes are written to the underlying device.
    do {
        const qint64 bytesWritten = device->write(reinterpret_cast<char *>(buffer), outputSize);
        if (bytesWritten == -1) {
            q->setErrorString(QT_TRANSLATE_NOOP("QtIOCompressor", "Error writing to underlying device: ") + device->errorString());
            return false;
        }
        totalBytesWritten += bytesWritten;
    } while (totalBytesWritten != outputSize);

    // put up a flag so that the device will be flushed on close.
    state = BytesWritten;
    return true;
}

/*!
    \internal
    Sets the error string to errorMessage + zlib error string for zlibErrorCode
*/
void QtIOCompressorPrivate::setZlibError(const QString &errorMessage, int zlibErrorCode)
{
    Q_Q(QtIOCompressor);
    // Watch out, zlibErrorString may be null.
    const char * const zlibErrorString = zError(zlibErrorCode);
    QString errorString;
    if (zlibErrorString)
        errorString = errorMessage + zlibErrorString;
    else
        errorString = errorMessage  + " Unknown error, code " + QString::number(zlibErrorCode);

    q->setErrorString(errorString);
}

/*! \class QtIOCompressor
    \brief The QtIOCompressor class is a QIODevice that compresses data streams.

    A QtIOCompressor object is constructed with a pointer to an
    underlying QIODevice.  Data written to the QtIOCompressor object
    will be compressed before it is written to the underlying
    QIODevice. Similarly, if you read from the QtIOCompressor object,
    the data will be read from the underlying device and then
    decompressed.

    QtIOCompressor is a sequential device, which means that it does
    not support seeks or random access. Internally, QtIOCompressor
    uses the zlib library to compress and uncompress data.

    Usage examples:
    Writing compressed data to a file:
    \code
        QFile file("foo");
        QtIOCompressor compressor(&file);
        compressor.open(QIODevice::WriteOnly);
        compressor.write(QByteArray() << "The quick brown fox");
        compressor.close();
    \endcode

    Reading compressed data from a file:
    \code
        QFile file("foo");
        QtIOCompressor compressor(&file);
        compressor.open(QIODevice::ReadOnly);
        const QByteArray text = compressor.readAll();
        compressor.close();
    \endcode

    QtIOCompressor can also read and write compressed data in
    different compressed formats, ref. StreamFormat. Use
    setStreamFormat() before open() to select format.
*/

/*!
    \enum QtIOCompressor::StreamFormat
    This enum specifies which stream format to use.

    \value ZlibFormat: This is the default and has the smallest overhead.

    \value GzipFormat: This format is compatible with the gzip file
    format, but has more overhead than ZlibFormat. Note: requires zlib
    version 1.2.x or higher at runtime.

    \value RawZipFormat: This is compatible with the most common
    compression method of the data blocks contained in ZIP
    archives. Note: ZIP file headers are not read or generated, so
    setting this format, by itself, does not let QtIOCompressor read
    or write ZIP files. Ref. the ziplist example program.

    \sa setStreamFormat()
*/

/*!
    Constructs a QtIOCompressor using the given \a device as the underlying device.

    The allowed value range for \a compressionLevel is 0 to 9, where 0 means no compression
    and 9 means maximum compression. The default value is 6.

    \a bufferSize specifies the size of the internal buffer used when reading from and writing to the
    underlying device. The default value is 65KB. Using a larger value allows for faster compression and
    decompression at the expense of memory usage.
*/
QtIOCompressor::QtIOCompressor(QIODevice *device, int compressionLevel, int bufferSize)
:d_ptr(new QtIOCompressorPrivate(this, device, compressionLevel, bufferSize))
{}

/*!
    Destroys the QtIOCompressor, closing it if necessary.
*/
QtIOCompressor::~QtIOCompressor()
{
    Q_D(QtIOCompressor);
    close();
    delete d;
}

/*!
    Sets the format on the compressed stream to \a format.

    \sa QtIOCompressor::StreamFormat
*/
void QtIOCompressor::setStreamFormat(StreamFormat format)
{
    Q_D(QtIOCompressor);

    // Print a waning if the compile-time version of zlib does not support gzip.
    if (format == GzipFormat && checkGzipSupport(ZLIB_VERSION) == false)
        qWarning("QtIOCompressor::setStreamFormat: zlib 1.2.x or higher is "
                 "required to use the gzip format. Current version is: %s",
                 ZLIB_VERSION);

    d->streamFormat = format;
}

/*!
    Returns the format set on the compressed stream.
    \sa QtIOCompressor::StreamFormat
*/
QtIOCompressor::StreamFormat QtIOCompressor::streamFormat() const
{
    Q_D(const QtIOCompressor);
    return d->streamFormat;
}

/*!
    Returns true if the zlib library in use supports the gzip format, false otherwise.
*/
bool QtIOCompressor::isGzipSupported()
{
    return checkGzipSupport(zlibVersion());
}

/*!
    \reimp
*/
bool QtIOCompressor::isSequential() const
{
    return true;
}

/*!
    Opens the QtIOCompressor in \a mode. Only ReadOnly and WriteOnly is supported.
    This function will return false if you try to open in other modes.

    If the underlying device is not opened, this function will open it in a suitable mode. If this happens
    the device will also be closed when close() is called.

    If the underlying device is already opened, its openmode must be compatible with \a mode.

    Returns true on success, false on error.

    \sa close()
*/
bool QtIOCompressor::open(OpenMode mode)
{
    Q_D(QtIOCompressor);
    if (isOpen()) {
        qWarning("QtIOCompressor::open: device already open");
        return false;
    }

    // Check for correct mode: ReadOnly xor WriteOnly
    const bool read = (mode & ReadOnly);
    const bool write = (mode & WriteOnly);
    const bool both = (read && write);
    const bool neither = !(read || write);
    if (both || neither) {
        qWarning("QtIOCompressor::open: QtIOCompressor can only be opened in the ReadOnly or WriteOnly modes");
        return false;
    }

    // If the underlying device is open, check that is it opened in a compatible mode.
    if (d->device->isOpen()) {
        d->manageDevice = false;
        const OpenMode deviceMode = d->device->openMode();
        if (read && !(deviceMode & ReadOnly)) {
            qWarning("QtIOCompressor::open: underlying device must be open in one of the ReadOnly or WriteOnly modes");
            return false;
        } else if (write && !(deviceMode & WriteOnly)) {
            qWarning("QtIOCompressor::open: underlying device must be open in one of the ReadOnly or WriteOnly modes");
            return false;
        }

    // If the underlying device is closed, open it.
    } else {
        d->manageDevice = true;
        if (d->device->open(mode) == false) {
            setErrorString(QT_TRANSLATE_NOOP("QtIOCompressor", "Error opening underlying device: ") + d->device->errorString());
            return false;
        }
    }

    // Initialize zlib for deflating or inflating.

    // The second argument to inflate/deflateInit2 is the windowBits parameter,
    // which also controls what kind of compression stream headers to use.
    // The default value for this is 15. Passing a value greater than 15
    // enables gzip headers and then subtracts 16 form the windowBits value.
    // (So passing 31 gives gzip headers and 15 windowBits). Passing a negative
    // value selects no headers hand then negates the windowBits argument.
    int windowBits;
    switch (d->streamFormat) {
    case QtIOCompressor::GzipFormat:
        windowBits = 31;
        break;
    case QtIOCompressor::RawZipFormat:
        windowBits = -15;
        break;
    default:
        windowBits = 15;
    }

    int status;
    if (read) {
        d->state = QtIOCompressorPrivate::NotReadFirstByte;
        d->zlibStream.avail_in = 0;
        d->zlibStream.next_in = 0;
        if (d->streamFormat == QtIOCompressor::ZlibFormat) {
            status = inflateInit(&d->zlibStream);
        } else {
            if (checkGzipSupport(zlibVersion()) == false) {
                setErrorString(QT_TRANSLATE_NOOP("QtIOCompressor::open", "The gzip format not supported in this version of zlib."));
                return false;
            }

            status = inflateInit2(&d->zlibStream, windowBits);
        }
    } else {
        d->state = QtIOCompressorPrivate::NoBytesWritten;
        if (d->streamFormat == QtIOCompressor::ZlibFormat)
            status = deflateInit(&d->zlibStream, d->compressionLevel);
        else
            status = deflateInit2(&d->zlibStream, d->compressionLevel, Z_DEFLATED, windowBits, 8, Z_DEFAULT_STRATEGY);
    }

    // Handle error.
    if (status != Z_OK) {
        d->setZlibError(QT_TRANSLATE_NOOP("QtIOCompressor::open", "Internal zlib error: "), status);
        return false;
    }
    return QIODevice::open(mode);
}

/*!
     Closes the QtIOCompressor, and also the underlying device if it was opened by QtIOCompressor.
    \sa open()
*/
void QtIOCompressor::close()
{
    Q_D(QtIOCompressor);
    if (isOpen() == false)
        return;

    // Flush and close the zlib stream.
    if (openMode() & ReadOnly) {
        d->state = QtIOCompressorPrivate::NotReadFirstByte;
        inflateEnd(&d->zlibStream);
    } else {
        if (d->state == QtIOCompressorPrivate::BytesWritten) { // Only flush if we have written anything.
            d->state = QtIOCompressorPrivate::NoBytesWritten;
            d->flushZlib(Z_FINISH);
        }
        deflateEnd(&d->zlibStream);
    }

    // Close the underlying device if we are managing it.
    if (d->manageDevice)
        d->device->close();

    QIODevice::close();
}

/*!
    Flushes the internal buffer.

    Each time you call flush, all data written to the QtIOCompressor is compressed and written to the
    underlying device. Calling this function can reduce the compression ratio. The underlying device
    is not flushed.

    Calling this function when QtIOCompressor is in ReadOnly mode has no effect.
*/
void QtIOCompressor::flush()
{
    Q_D(QtIOCompressor);
    if (isOpen() == false || openMode() & ReadOnly)
        return;

    d->flushZlib(Z_SYNC_FLUSH);
}

/*!
    Returns 1 if there might be data available for reading, or 0 if there is no data available.

    There is unfortunately no way of knowing how much data there is available when dealing with compressed streams.

    Also, since the remaining compressed data might be a part of the meta-data that ends the compressed stream (and
    therefore will yield no uncompressed data), you cannot assume that a read after getting a 1 from this function will return data.
*/
qint64 QtIOCompressor::bytesAvailable() const
{
    Q_D(const QtIOCompressor);
    if ((openMode() & ReadOnly) == false)
        return 0;

    int numBytes = 0;

    switch (d->state) {
        case QtIOCompressorPrivate::NotReadFirstByte:
            numBytes = d->device->bytesAvailable();
        break;
        case QtIOCompressorPrivate::InStream:
            numBytes = 1;
        break;
        case QtIOCompressorPrivate::EndOfStream:
        case QtIOCompressorPrivate::Error:
        default:
            numBytes = 0;
        break;
    };

    numBytes += QIODevice::bytesAvailable();

    if (numBytes > 0)
        return 1;
    else
        return 0;
}

/*!
    \internal
    Reads and decompresses data from the underlying device.
*/
qint64 QtIOCompressor::readData(char *data, qint64 maxSize)
{
    Q_D(QtIOCompressor);

    if (d->state == QtIOCompressorPrivate::EndOfStream)
        return 0;

    if (d->state == QtIOCompressorPrivate::Error)
        return -1;

    // We are going to try to fill the data buffer
    d->zlibStream.next_out = reinterpret_cast<ZlibByte *>(data);
    d->zlibStream.avail_out = maxSize;

    int status;
    do {
        // Read data if if the input buffer is empty. There could be data in the buffer
        // from a previous readData call.
        if (d->zlibStream.avail_in == 0) {
            qint64 bytesAvalible = d->device->read(reinterpret_cast<char *>(d->buffer), d->bufferSize);
            d->zlibStream.next_in = d->buffer;
            d->zlibStream.avail_in = bytesAvalible;

            if (bytesAvalible == -1) {
                d->state = QtIOCompressorPrivate::Error;
                setErrorString(QT_TRANSLATE_NOOP("QtIOCompressor", "Error reading data from underlying device: ") + d->device->errorString());
                return -1;
            }

            if (d->state != QtIOCompressorPrivate::InStream) {
                // If we are not in a stream and get 0 bytes, we are probably trying to read from an empty device.
                if(bytesAvalible == 0)
                    return 0;
                else if (bytesAvalible > 0)
                    d->state = QtIOCompressorPrivate::InStream;
            }
        }

        // Decompress.
        status = inflate(&d->zlibStream, Z_SYNC_FLUSH);
        switch (status) {
            case Z_NEED_DICT:
            case Z_DATA_ERROR:
            case Z_MEM_ERROR:
                d->state = QtIOCompressorPrivate::Error;
                d->setZlibError(QT_TRANSLATE_NOOP("QtIOCompressor", "Internal zlib error when decompressing: "), status);
                return -1;
            case Z_BUF_ERROR: // No more input and zlib can not provide more output - Not an error, we can try to read again when we have more input.
                return 0;
        }
    // Loop util data buffer is full or we reach the end of the input stream.
    } while (d->zlibStream.avail_out != 0 && status != Z_STREAM_END);

    if (status == Z_STREAM_END) {
        d->state = QtIOCompressorPrivate::EndOfStream;

        // Unget any data left in the read buffer.
        for (int i = d->zlibStream.avail_in;  i >= 0; --i)
            d->device->ungetChar(*reinterpret_cast<char *>(d->zlibStream.next_in + i));
    }

    const ZlibSize outputSize = maxSize - d->zlibStream.avail_out;
    return outputSize;
}


/*!
    \internal
    Compresses and writes data to the underlying device.
*/
qint64 QtIOCompressor::writeData(const char *data, qint64 maxSize)
{
    if (maxSize < 1)
        return 0;
    Q_D(QtIOCompressor);
    d->zlibStream.next_in = reinterpret_cast<ZlibByte *>(const_cast<char *>(data));
    d->zlibStream.avail_in = maxSize;

    if (d->state == QtIOCompressorPrivate::Error)
        return -1;

    do {
        d->zlibStream.next_out = d->buffer;
        d->zlibStream.avail_out = d->bufferSize;
        const int status = deflate(&d->zlibStream, Z_NO_FLUSH);
        if (status != Z_OK) {
            d->state = QtIOCompressorPrivate::Error;
            d->setZlibError(QT_TRANSLATE_NOOP("QtIOCompressor", "Internal zlib error when compressing: "), status);
            return -1;
        }

        ZlibSize outputSize = d->bufferSize - d->zlibStream.avail_out;

        // Try to write data from the buffer to to the underlying device, return -1 on failure.
        if (d->writeBytes(d->buffer, outputSize) == false)
            return -1;

    } while (d->zlibStream.avail_out == 0); // run until output is not full.
    Q_ASSERT(d->zlibStream.avail_in == 0);

    return maxSize;
}

/*
    \internal
    Checks if the run-time zlib version is 1.2.x or higher.
*/
bool QtIOCompressor::checkGzipSupport(const char * const versionString)
{
    if (strlen(versionString) < 3)
        return false;

    if (versionString[0] == '0' || (versionString[0] == '1' && (versionString[2] == '0' || versionString[2]  == '1' )))
        return false;

    return true;
}
