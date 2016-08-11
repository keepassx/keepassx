#include "qqrencode.h"
#include "qqrencode_p.h"

#include <QDateTime>


#define INCHES_PER_METER (100.0/2.54)

QQREncodePrivate::~QQREncodePrivate()
{
    QRcode_free(m_code);
}

void QQREncodePrivate::paint(QPainter &painter)
{
    unsigned char *row, *p;
    int x, y;

    int symwidth = m_code->width + m_margin * 2;
    painter.setClipRect(QRect(0,0,symwidth, symwidth));
    painter.setPen(m_pen);
    painter.setBrush(m_fg);

    /* Make solid background */
    painter.fillRect(QRect(0, 0, symwidth, symwidth), m_bg);

    /* Write data */
    p = m_code->data;
    for(y=0; y<m_code->width; y++) {
        row = (p+(y*m_code->width));
        /* no RLE */
        for(x=0; x<m_code->width; x++) {
            if(*(row+x)&0x1) {
                painter.drawRect(m_margin + x, m_margin + y, 1, 1);
            }
        }

    }
}

QQREncode::QQREncode()
    : d_ptr(new QQREncodePrivate(this))
{
}

QQREncode::~QQREncode()
{
}

void QQREncode::setLevel(QQREncode::ErrorCorrectionLevel value)
{
    Q_D(QQREncode);
    switch (value) {
    case LOW:
        d->m_level = QR_ECLEVEL_L;
        break;
    case MEDIUM:
        d->m_level = QR_ECLEVEL_M;
        break;
    case QUARTILE:
        d->m_level = QR_ECLEVEL_Q;
        break;
    case HIGH:
        d->m_level = QR_ECLEVEL_H;
        break;
    }
}

QQREncode::ErrorCorrectionLevel QQREncode::getLevel() const
{
    Q_D(const QQREncode);
    switch (d->m_level) {
    case QR_ECLEVEL_L:
        return LOW;
    case QR_ECLEVEL_M:
        return MEDIUM;
    case QR_ECLEVEL_Q:
        return QUARTILE;
    case QR_ECLEVEL_H:
        return HIGH;
    }
    return LOW;
}

//bool QQREncode::encode(QString code, QString output)
//{
//    if (code.isEmpty() || output.isEmpty()) {
//        return false;
//    }
//    QString program = "./qrencode.exe";
//    QStringList arguments;
//    arguments << "-t" << "PNG";
//    arguments << "-o" << output;
//    arguments << code;

//    QProcess *myProcess = new QProcess(this);
//    myProcess->start(program, arguments);
//    return myProcess->waitForFinished();
//}


void QQREncode::setVersion(int version)
{
    Q_D(QQREncode);
    // 1 - 40
    if (version > 0 && version <= 40)
        d->m_version = version;
}

int QQREncode::version() const
{
    Q_D(const QQREncode);
    return d->m_version;
}

void QQREncode::setMargin(int value)
{
    Q_D(QQREncode);
    if (value > -1)
        d->m_margin = value;
}

int QQREncode::margin() const
{
    Q_D(const QQREncode);
    return d->m_margin;
}

void QQREncode::setMicro(bool value)
{
    Q_D(QQREncode);
    d->m_micro = (value) ? 1 : 0;
}

bool QQREncode::isMicro() const
{
    Q_D(const QQREncode);
    return (d->m_micro == 1) ? true : false;
}

void QQREncode::setBackground(QColor color)
{
    Q_D(QQREncode);
    d->m_bg.setColor(color);
}

void QQREncode::setForeground(QColor color)
{
    Q_D(QQREncode);
    d->m_fg.setColor(color);
    d->m_pen.setColor(color);
}

bool QQREncode::encode(QByteArray input)
{
    Q_D(QQREncode);
    QRcode *c = NULL;
    if (input.isEmpty()) return false;
    if (d->m_micro) {
        c = QRcode_encodeDataMQR(input.size(), (const unsigned char*)input.constData(),
                                   d->m_version, d->m_level);
    } else {
        c = QRcode_encodeData(input.size(), (const unsigned char*)input.constData(),
                              d->m_version, d->m_level);
    }
    if (c == NULL) {
        return false;
    }
    if (d->m_code) QRcode_free(d->m_code);
    d->m_code = c;
    return true;
}

bool QQREncode::encode(QString input, bool caseSensitive)
{
    Q_D(QQREncode);
    if (input.isEmpty()) return false;
    QRcode *c = NULL;
    if (d->m_micro) {
        c = QRcode_encodeStringMQR(input.toStdString().c_str(),
                                   d->m_version,
                                   d->m_level,
                                   QR_MODE_8,
                                   (caseSensitive) ? 1 : 0);
    } else {
        c = QRcode_encodeString(input.toStdString().c_str(),
                                d->m_version,
                                d->m_level,
                                QR_MODE_8,
                                (caseSensitive) ? 1 : 0);
    }
    if (c == NULL) {
        return false;
    }
    if (d->m_code) QRcode_free(d->m_code);
    d->m_code = c;
    return true;
}

bool QQREncode::encodeKanji(QByteArray input, bool caseSensitive)
{
    Q_D(QQREncode);
    if (input.isEmpty()) return false;
    QRcode *c = NULL;
    if (d->m_micro) {
        c = QRcode_encodeStringMQR(input.constData(),
                                   d->m_version,
                                   d->m_level,
                                   QR_MODE_KANJI,
                                   (caseSensitive) ? 1 : 0);
    } else {
        c = QRcode_encodeString(input.constData(),
                                d->m_version,
                                d->m_level,
                                QR_MODE_KANJI,
                                (caseSensitive) ? 1 : 0);
    }
    if (c == NULL) {
        return false;
    }
    if (d->m_code) QRcode_free(d->m_code);
    d->m_code = c;
    return true;
}


QImage QQREncode::toQImage(int size)
{
    Q_D(QQREncode);
    if (size < 0) {
        return QImage();
    }

    if (d->m_code == NULL) {
        std::logic_error("No qr code to convert");
    }

    int symwidth = d->m_code->width + d->m_margin * 2;
    QImage result(QSize(symwidth, symwidth), QImage::Format_Mono);
    result.fill(Qt::white);

    QPainter painter;
    painter.begin(&result);
    d->paint(painter);
    painter.end();

    if (size > 0)
        return result.scaled(size, size);
    return result;
}
