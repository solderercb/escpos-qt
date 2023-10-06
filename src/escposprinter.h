#ifndef ESCPOSPRINTER_H
#define ESCPOSPRINTER_H

#include <QObject>
#include <escposexports.h>
#ifdef Q_OS_WINDOWS
#include <Windows.h>
#endif

#define LINE_WIDTH 32 // TODO: get print boundaries from device

class QIODevice;
class QTextCodec;

namespace EscPosQt {

class ESC_POS_QT_EXPORT EscPosPrinter : public QObject
{
    Q_OBJECT
private:
    struct _feed { int _lines; };
    struct _lineSpacing { int mils; };
    struct _2CTable {const QString &lColumn; const QString &rColumn;};
public:
    explicit EscPosPrinter(const QString &printerName, const QByteArray &codecName = QByteArray(), QObject *parent = nullptr);
    ~EscPosPrinter();
    bool open();
    void close();
    bool startSpool(const QString &jobName = QString());
    void endSpool();
    bool startPage();
    void endPage();

    struct QRCode {
        enum Model {
            Model1 = 49,
            Model2,
            MicroQRCode,
        };
        enum ErrorCorrection {
            L = 48, // 7%
            M, // 15%
            Q, // 25%
            H, // 30%
        };
        QRCode(EscPosPrinter::QRCode::Model model, int moduleSize, EscPosPrinter::QRCode::ErrorCorrection erroCorrection, const QByteArray &_data);

        QByteArray data;
    };

    enum PrintMode {
        PrintModeNone = 0x00, // 32char on mini, 48 on 80mm
        PrintModeFont2 = 0x01,
        PrintModeEmphasized = 0x08,
        PrintModeDoubleHeight = 0x10,
        PrintModeDoubleWidth = 0x20, // 16char on mini, 24 on 80mm
        PrintModeUnderline = 0x80,
    };
    Q_ENUM(PrintMode)
    Q_DECLARE_FLAGS(PrintModes, PrintMode)

    enum Justification {
        JustificationLeft = 0x30,
        JustificationCenter = 0x31,
        JustificationRight = 0x32,
    };
    Q_ENUM(Justification)

    enum HriPosition {
        HriNotPrinted = 0x00,
        HriNotAbove = 0x01,
        HriNotBelow = 0x02,
        HriNotAboveAndBelow = 0x03,
    };
    Q_ENUM(HriPosition)

    enum Encoding {
        EncodingPC437 = 0,
        EncodingKatakana = 1,
        EncodingPC850 = 2,// Qt supported
        EncodingPC860 = 3,
        EncodingPC866 = 17,// Qt supported
        EncodingPC852 = 18,
        EncodingISO8859_2 = 39,// Qt supported
        EncodingISO8859_15 = 40,// Qt supported
    };
    Q_ENUM(Encoding)

    inline static _feed feed(int __lines) { return { __lines }; }
    inline static _lineSpacing lineSpacing(int mils) { return { mils }; }
    inline static _2CTable formatAs2CTable(const QString &lColumn, const QString &rColumn) { return {lColumn, rColumn}; }

    EscPosPrinter &operator<<(PrintModes i);
    EscPosPrinter &operator<<(Justification i);
    EscPosPrinter &operator<<(Encoding i);
    EscPosPrinter &operator<<(_feed lines);
    EscPosPrinter &operator<<(_lineSpacing mils);
    EscPosPrinter &operator<<(_2CTable table);
    EscPosPrinter &operator<<(const char *s);
    EscPosPrinter &operator<<(const QByteArray &s);
    EscPosPrinter &operator<<(const QRCode &qr);
    EscPosPrinter &operator<<(const QString &text);
    EscPosPrinter &operator<<(QStringView text);
    EscPosPrinter &operator<<(void (*pf) ());

    static void init() {}
    static void eol() {}
    static void standardMode() {}
    static void pageMode() {}

public Q_SLOTS:
    void getStatus();

private:
    QTextCodec *m_codec = nullptr;
    QIODevice *m_device;
#ifdef Q_OS_WINDOWS
    LPWSTR m_printerName;
    DWORD  m_dwBytesWritten = 0L;
    HANDLE m_hPrinter = NULL;
    BOOL   m_bStatus = FALSE;
    DWORD  m_dwJob = 0L;
#endif
    QByteArray decode(const QString &text);
    void write(const QByteArray &data);
    void write(const char *data, int size);
    EscPosPrinter &initialize();
    EscPosPrinter &encode(Encoding codec);
    EscPosPrinter &mode(PrintModes pm);
    EscPosPrinter &modeStandard();
    EscPosPrinter &modePage();
    EscPosPrinter &partialCut();
    EscPosPrinter &printAndFeedPaper(quint8 n = 1);
    EscPosPrinter &align(Justification i);
    EscPosPrinter &paperFeed(int lines = 1);
    EscPosPrinter &setLineSpacing(int mils);
    EscPosPrinter &data2CTable(const QString &lColumn, const QString &rColumn);
};

}

Q_DECLARE_OPERATORS_FOR_FLAGS(EscPosQt::EscPosPrinter::PrintModes)

#endif // ESCPOSPRINTER_H
