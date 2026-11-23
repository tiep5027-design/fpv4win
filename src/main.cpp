// Replace fpv4win/src/main.cpp with this file for a headless build.
// Reuses the project's existing libusb/RTL8812AU/WFB-ng code.
// This does not modify flight controls or any radio parameters beyond
// selecting the same receive channel/width as the working fpv4win setup.
#include "src/QmlNativeAPI.h"
#include <QCoreApplication>
#include <QCommandLineParser>
#include <QFileInfo>
#include <QObject>
#include <QString>
#include <QTimer>
#include <iostream>

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    app.setApplicationName("openipc-headless-receiver");

    QCommandLineParser parser;
    parser.setApplicationDescription("OpenIPC fpv4win WFB-ng receiver without video GUI; RTP sent to localhost");
    parser.addHelpOption();
    QCommandLineOption vidOpt({"d", "vidpid"}, "RTL8812AU VID:PID", "vidpid", "0bda:881a");
    QCommandLineOption channelOpt({"c", "channel"}, "Wi-Fi receive channel", "channel", "161");
    // fpv4win UI passes ComboBox index, not literal MHz: index 0 == 20 MHz.
    QCommandLineOption widthOpt({"w", "width-index"}, "Channel width index: 0=20 MHz, 1=40 MHz, 2=80 MHz", "index", "0");
    QCommandLineOption keyOpt({"k", "key"}, "Existing OpenIPC ground-station key file", "path", "gs.key");
    QCommandLineOption codecOpt({"x", "codec"}, "H264, H265, or AUTO", "codec", "H265");
    parser.addOptions({vidOpt, channelOpt, widthOpt, keyOpt, codecOpt});
    parser.process(app);

    bool channelOk = false;
    bool widthOk = false;
    int channel = parser.value(channelOpt).toInt(&channelOk);
    int widthIndex = parser.value(widthOpt).toInt(&widthOk);
    const QString vidpid = parser.value(vidOpt).trimmed();
    const QString codec = parser.value(codecOpt).trimmed().toUpper();
    const QString keyFile = QFileInfo(parser.value(keyOpt)).absoluteFilePath();
    if (!channelOk || channel < 1 || channel > 165 || !widthOk || widthIndex < 0 || widthIndex > 7) {
        std::cerr << "Invalid receive channel or width index.\n";
        return 2;
    }
    if (vidpid.size() != 9 || vidpid[4] != ':') {
        std::cerr << "Invalid VID:PID format (expected 0bda:881a).\n";
        return 2;
    }
    if (codec != "H264" && codec != "H265" && codec != "AUTO") {
        std::cerr << "Invalid codec; use H264, H265 or AUTO.\n";
        return 2;
    }
    if (!QFileInfo(keyFile).isFile()) {
        std::cerr << "Missing key file: " << keyFile.toStdString() << "\n";
        return 2;
    }

    auto &api = QmlNativeAPI::Instance();
    QObject::connect(&api, &QmlNativeAPI::onLog, &app,
        [](const QString &level, const QString &msg) {
            std::cout << "[" << level.toStdString() << "] " << msg.toStdString() << std::endl;
        }, Qt::QueuedConnection);
    QObject::connect(&api, &QmlNativeAPI::onRtpStream, &app,
        [](const QString &sdp) {
            std::cout << "SDP_READY: " << QFileInfo(sdp).absoluteFilePath().toStdString() << std::endl;
        }, Qt::QueuedConnection);
    QObject::connect(&api, &QmlNativeAPI::onWifiStop, &app,
        []() { std::cerr << "Receiver stopped.\n"; QCoreApplication::quit(); }, Qt::QueuedConnection);

    std::cout << "Starting RTL8812AU receiver, channel=" << channel
              << ", width index=" << widthIndex << ", codec=" << codec.toStdString() << std::endl;
    std::cout << "Video RTP output: localhost:52356 (SDP generated on first RTP packet)\n";
    const bool started = QmlNativeAPI::Start(
        vidpid, channel, widthIndex, keyFile, codec);
    if (!started) {
        std::cerr << "Could not start receiver: check adapter, USB driver and other software using it.\n";
        return 3;
    }
    return app.exec();
}
