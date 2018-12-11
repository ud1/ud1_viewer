#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include "settings.h"

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = 0);
    ~SettingsDialog();

signals:
    void settingsChanged();

private slots:
    void on_saveBtn_clicked();

    void on_cancelBtn_clicked();

    void on_toolButton_clicked();

private:
    Ui::SettingsDialog *ui;
    Settings settings;
};

#endif // SETTINGSDIALOG_H
