#ifndef QQRENCODE_H
#define QQRENCODE_H

#include <QtCore>

#include "libqtqrencode_global.h"

class QQREncodePrivate;

class LIBQTQRENCODESHARED_EXPORT QQREncode
{
    Q_GADGET
    Q_ENUMS(ErrorCorrectionLevel)

public:
    enum ErrorCorrectionLevel {
        LOW,
        MEDIUM,
        QUARTILE,
        HIGH
    };

    QQREncode();
    ~QQREncode();

    void setLevel(ErrorCorrectionLevel value);
    ErrorCorrectionLevel getLevel() const;
    void setVersion(int version);
    int version() const;
    void setMargin(int value);
    int margin() const;
    void setMicro(bool value);
    bool isMicro() const;
    void setBackground(QColor color);
    void setForeground(QColor color);

    bool encode(QByteArray input);
    bool encode(QString input, bool caseSensitive=true);
    bool encodeKanji(QByteArray input, bool caseSensitive=true);

    QImage toQImage(int size=0);

    // ToDo: encode structured, rle

private:
    Q_DISABLE_COPY(QQREncode)
    QScopedPointer<QQREncodePrivate> d_ptr;
    Q_DECLARE_PRIVATE(QQREncode)
};

#endif // QQRENCODE_H
