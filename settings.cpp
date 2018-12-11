#include "settings.h"
#include <QSettings>

Settings::Settings()
{
    load();
}

void Settings::load()
{
    QSettings qsettings;

    port = qsettings.value("port", 8400).toInt();
    saveToFile = qsettings.value("saveToFile", false).toBool();
    outputDir = qsettings.value("outputDir").toString();
    fov = qsettings.value("fov", 90.0).toDouble();
}

void Settings::save()
{
    QSettings qsettings;
    qsettings.setValue("port", port);
    qsettings.setValue("saveToFile", saveToFile);
    qsettings.setValue("outputDir", outputDir);
    qsettings.setValue("fov", fov);
}
