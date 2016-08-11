#ifndef QQRENCODE_P_H
#define QQRENCODE_P_H

#include "qqrencode.h"

#include "qrencode.h"
#include <QPainter>

class QQREncodePrivate
{
    Q_DECLARE_PUBLIC(QQREncode)

public:
    QQREncodePrivate(QQREncode *qqrencode)
        : q_ptr(qqrencode), m_code(NULL), m_level(QR_ECLEVEL_L),m_version(1), m_margin(4), m_micro(0),
          m_pen(Qt::black, 0.1, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin), m_bg(Qt::white),
          m_fg(Qt::black)
    {
    }

    ~QQREncodePrivate();

    void paint(QPainter &painter);

    QQREncode *q_ptr;
    QRcode *m_code;
    QRecLevel m_level;
    int m_version;
    int m_margin;
    int m_micro;
    QPen m_pen;
    QBrush m_bg;
    QBrush m_fg;

    // ToDo: Corrupted SVG when setting the resolution
//    int m_dpi = 72;
    // ToDo: structured and rle features
//    int m_structured = 0;
//    int m_rle = 0;
};

#endif // QQRENCODE_P_H
