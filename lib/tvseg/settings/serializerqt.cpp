#include "serializerqt.h"

#include <QSettings>
#include <QString>

namespace tvseg {
namespace settings {

SerializerQt::SerializerQt(BackendPtr backend, std::string location, bool autoLoad):
    SerializerBase(backend, location)
{
    if (autoLoad) {
        load();
    }
}

void SerializerQt::saveImpl(std::string location)
{
    QString fileName = QString::fromStdString(location);
//    LDEBUG << qPrintable(QString("Writing Settings to file '%1'").arg(fileName));

    QSettings settings(fileName, QSettings::IniFormat);

    for (Backend::iterator it = begin(); it != end(); ++it)
    {
        it->second.updateRaw();
        settings.setValue(QString::fromStdString(it->first), QString::fromStdString(it->second.raw_value));
    }
}


void SerializerQt::loadImpl(std::string location)
{
    QString fileName = QString::fromStdString(location);
//    LDEBUG << qPrintable(QString("Loading Settings from file '%1'").arg(fileName));

    QSettings settings(fileName, QSettings::IniFormat);

    QStringList keys = settings.allKeys();
    foreach (const QString &s, keys) {
        Backend::blob_type& entry = backend()->at(s.toStdString());
        entry.raw_value = settings.value(s).toString().toStdString();
        entry.updateFromRaw();
    }
}


} // namespace settings
} // namespace tvseg
