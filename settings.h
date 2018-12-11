#ifndef SETTINGS_H
#define SETTINGS_H

#include <QString>

class Settings
{
public:
    Settings();

    int port;
    bool saveToFile;
    QString outputDir;

    double fov;

    void load();
    void save();
};

#endif // SETTINGS_H
