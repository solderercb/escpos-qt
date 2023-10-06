#include "escposprinter.h"

#include <QLoggingCategory>
#include <QTextCodec>

Q_LOGGING_CATEGORY(EPP, "esc_pos")

static const char ESC = 0x1B;
static const char GS = 0x1D;

using namespace EscPosQt;

// code example from https://stackoverflow.com/a/31507248/22660723
EscPosPrinter::EscPosPrinter(const QString &printerName, const QByteArray &codecName, QObject *parent) :
    QObject(parent)
{
    if(codecName.isEmpty())
        m_codec = QTextCodec::codecForLocale();
    else
        m_codec = QTextCodec::codecForName(codecName);

#ifdef Q_OS_WINDOWS
    m_printerName = (LPWSTR)printerName.data();
#endif
}

EscPosPrinter::~EscPosPrinter()
{
#ifdef Q_OS_WINDOWS
#endif
}

/* Opens printing device
*/
bool EscPosPrinter::open()
{
    bool ret = 1;
#ifdef Q_OS_WINDOWS
    OpenPrinter(m_printerName, &m_hPrinter, NULL);
    if(!m_hPrinter)
    {
        ret = 0;
        qDebug() << "Printer opening Failed";
    }
#endif

    return ret;
}

/* Closes printing device
 * Optionaly ends page and spool if its not been ended
*/
void EscPosQt::EscPosPrinter::close()
{
#ifdef Q_OS_WINDOWS
    endPage();
    endSpool();
    ClosePrinter(m_hPrinter);
#endif
}

/* Starts new spool
*/
bool EscPosPrinter::startSpool(const QString &jobName)
{
#ifdef Q_OS_WINDOWS
    QString strDocName(jobName);
    QString strDataType = "RAW";
    if(strDocName.isEmpty())
    {
        strDocName = "ESC/POS reports";
    }

    if(m_hPrinter && !m_dwJob)
    {
        DOC_INFO_1 DocInfo;

        DocInfo.pDocName = (LPWSTR)strDocName.data();
        DocInfo.pOutputFile = NULL;
        DocInfo.pDatatype = (LPWSTR)strDataType.data();

        m_dwJob = StartDocPrinter(m_hPrinter, 1, (LPBYTE)&DocInfo);
        if (!m_dwJob)
        {
            qDebug() << "Couldn't start job";
            return 0;
        }
    }
#endif

    return 1;
}

/* Closes spool of task and printing all data from it
 * (physical output will be started on spool close)
*/
void EscPosPrinter::endSpool()
{
    if(m_dwJob)
    {
        m_dwJob = 0L;
        EndDocPrinter(m_hPrinter);
    }
}

/* Starts new page
 * This is not necessary
*/
bool EscPosPrinter::startPage()
{
    if(m_dwJob && !m_bStatus)
        m_bStatus = StartPagePrinter(m_hPrinter);

    return m_bStatus;
}

/* Ends page
*/
void EscPosPrinter::endPage()
{
    if(m_bStatus)
    {
        m_bStatus = FALSE;
        EndPagePrinter(m_hPrinter);
    }

}

/* Sends data to spool
*/
void EscPosPrinter::write(const QByteArray &data)
{
#ifdef Q_OS_WINDOWS
    if (m_bStatus)
    {
        DWORD dwBytesWritten = 0L;

        WritePrinter(m_hPrinter, (LPVOID)data.data(), data.length(), &dwBytesWritten);
    }
    else
    {
        qDebug() << "Can't print, page printer not started";
    }
#endif
}

/* Overloaded method
*/
void EscPosPrinter::write(const char *data, int size)
{
#ifdef Q_OS_WINDOWS
    if (m_bStatus)
    {
        DWORD dwBytesWritten = 0L;

        WritePrinter(m_hPrinter, (LPVOID)data, size, &dwBytesWritten);
    }
    else
    {
        qDebug() << "Can't print, page printer not started";
    }
#endif
}

EscPosPrinter &EscPosPrinter::operator<<(PrintModes i)
{
    return mode(i);
}

/* The UTF-8 string will be encoded with QTextCodec
*  if one of the Qt supported encodings is selected.
*/
QByteArray EscPosPrinter::decode(const QString &text)
{
    if (m_codec) {
        return m_codec->fromUnicode(text);
    } else {
        return text.toLatin1();
    }
}

EscPosPrinter &EscPosPrinter::operator<<(EscPosPrinter::Justification i)
{
    return align(i);
}

EscPosPrinter &EscPosPrinter::operator<<(EscPosPrinter::Encoding i)
{
    return encode(i);
}

EscPosPrinter &EscPosPrinter::operator<<(EscPosPrinter::_feed lines)
{
    return paperFeed(lines._lines);
}

EscPosPrinter &EscPosPrinter::operator<<(EscPosPrinter::_lineSpacing mils)
{
    return setLineSpacing(mils.mils);
}

EscPosPrinter &EscPosPrinter::operator<<(EscPosPrinter::_2CTable table)
{
    return data2CTable(table.lColumn, table.rColumn);
}

EscPosPrinter &EscPosPrinter::operator<<(const char *s)
{
    write(s, int(strlen(s)));
    return *this;
}

EscPosPrinter &EscPosPrinter::operator<<(const QByteArray &s)
{
    write(s);
    return *this;
}

EscPosPrinter &EscPosPrinter::operator<<(const EscPosPrinter::QRCode &qr)
{
    write(qr.data);
    return *this;
}

EscPosPrinter &EscPosPrinter::operator<<(const QString &text)
{
    write(decode(text));
    return *this;
}

EscPosPrinter &EscPosPrinter::operator<<(QStringView text)
{
    if (m_codec) {
        write(m_codec->fromUnicode(text));
    } else {
        write(text.toLatin1());
    }
    return *this;
}

EscPosPrinter &EscPosPrinter::operator<<(void (*pf)())
{
    if (pf == EscPosPrinter::eol) {
        write("\n", 1);
    } else if (pf == EscPosPrinter::init) {
        return initialize();
    } else if (pf == EscPosPrinter::standardMode) {
        return modeStandard();
    } else if (pf == EscPosPrinter::pageMode) {
        return modePage();
    }
    return *this;
}

EscPosPrinter &EscPosPrinter::initialize()
{
    const char str[] = { ESC, '@'};
    write(str, sizeof(str));
    return *this;
}

EscPosPrinter &EscPosPrinter::encode(EscPosPrinter::Encoding codec)
{
    switch (codec) {
    case EncodingPC850:
        m_codec = QTextCodec::codecForName("IBM 850");
        break;
    case EncodingPC866:
        m_codec = QTextCodec::codecForName("IBM 866");
        break;
    case EncodingISO8859_2:
        m_codec = QTextCodec::codecForName("ISO8859-2");
        break;
    case EncodingISO8859_15:
        m_codec = QTextCodec::codecForName("ISO8859-15");
        break;
    default:
        m_codec = nullptr;
    }

    if (!m_codec) {
        qCWarning(EPP) << "Could not find a Qt Codec for" << codec;
    }

    const char str[] = { ESC, 't', char(codec)};
    write(str, sizeof(str));
    return *this;
}

EscPosPrinter &EscPosPrinter::mode(EscPosPrinter::PrintModes pm)
{
    const char str[] = { ESC, '!', char(pm)};
    write(str, sizeof(str));
    return *this;
}

EscPosPrinter &EscPosPrinter::modeStandard()
{
    const char str[] = { ESC, 'L'};
    write(str, sizeof(str));
    return *this;
}

EscPosPrinter &EscPosPrinter::modePage()
{
    const char str[] = { ESC, 'S'};
    write(str, sizeof(str));
    return *this;
}

EscPosPrinter &EscPosPrinter::partialCut()
{
    const char str[] = { ESC, 'm'};
    write(str, sizeof(str));
    return *this;
}

EscPosPrinter &EscPosPrinter::printAndFeedPaper(quint8 n)
{
    const char str[] = { ESC, 'J', char(n)};
    write(str, sizeof(str));
    return *this;
}

EscPosPrinter &EscPosPrinter::align(EscPosPrinter::Justification i)
{
    const char str[] = { ESC, 'a', char(i)};
    write(str, 3);// TODO doesn't work on DR700
    return *this;
}

EscPosPrinter &EscPosPrinter::paperFeed(int lines)
{
    const char str[] = { ESC, 'd', char(lines)};
    write(str, 3);
    return *this;
}

EscPosPrinter &EscPosPrinter::setLineSpacing(int mils)
{
    const char str[] = { ESC, '3', char(mils)};
    write(str, 3);
    return *this;
}

/* Formats input values as table with two columns
 * lColumn will be justified left
 * rColumn will be justified right
 * space between values will be filled with 0x20 character
*/
EscPosPrinter &EscPosPrinter::data2CTable(const QString &lColumn, const QString &rColumn)
{
    QByteArray buf;
    int padding = LINE_WIDTH - lColumn.length();

    buf = decode(lColumn);
    if( rColumn.length() + 1 > padding )
    {
        qDebug() << "data2CTable: resulting string exceeds print boundaries";
        buf += "  " + decode(rColumn.left(padding - 2));
    }
    else
    {
        buf += decode(rColumn.rightJustified(padding));
    }

    buf.append('\n');

    write(buf);
    return *this;
}

void EscPosPrinter::getStatus()
{

}

EscPosPrinter::QRCode::QRCode(EscPosPrinter::QRCode::Model model, int moduleSize, EscPosPrinter::QRCode::ErrorCorrection erroCorrection, const QByteArray &_data)
{

    // Model f165
    // 49 - model1
    // 50 - model2
    // 51 - micro qr code
    const char mT[] = {
        GS, '(', 'k', 0x04, 0x00, 0x31, 0x41, char(model), 0x00};
    data.append(mT, sizeof(mT));


    // Module Size f167
    const char mS[] = {
        GS, '(', 'k', 0x03, 0x00, 0x31, 0x43, char(moduleSize)};
    data.append(mS, sizeof(mS));

    // Error Level f169
    // L = 0, M = 1, Q = 2, H = 3
    const char eL[] = {
        GS, '(', 'k', 0x03, 0x00, 0x31, 0x45, char(erroCorrection)};
    data.append(eL, sizeof(eL));

    // truncate data f180
    int len = _data.length() + 3;// 3 header bytes
    if (len > 7092) {
        len = 7092;
    }

    // data header
    const char dH[] = {
        GS, '(', 'k', char(len), char(len >> 8), 0x31, 0x50, 0x30};
    data.append(dH, sizeof(dH));

    data.append(_data.mid(0, 7092));

    // Print f181
    const char pT[] = {
        GS, '(', 'k', 0x03, 0x00, 0x31, 0x51, 0x30};
    data.append(pT, sizeof(pT));
}
