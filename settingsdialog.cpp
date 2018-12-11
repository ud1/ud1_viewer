#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include <QFileDialog>

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);

    ui->port->setValidator(new QIntValidator(0, 9999, this));
    ui->port->setText(QString::number(settings.port));
    ui->saveToFile->setChecked(settings.saveToFile);
    ui->outputDir->setText(settings.outputDir);
    ui->fov->setValidator(new QDoubleValidator(50.0, 120.0, 0));
    ui->fov->setText(QString::number(settings.fov));
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::on_saveBtn_clicked()
{
    settings.port = ui->port->text().toInt();
    settings.saveToFile = ui->saveToFile->checkState() == Qt::Checked;
    settings.outputDir = ui->outputDir->text();
    settings.fov = ui->fov->text().toDouble();
    settings.save();
    close();

    emit settingsChanged();
}

void SettingsDialog::on_cancelBtn_clicked()
{
    close();
}

void SettingsDialog::on_toolButton_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Open Directory",
                                                 ui->outputDir->text(),
                                                 QFileDialog::ShowDirsOnly
                                                 | QFileDialog::DontResolveSymlinks);

    ui->outputDir->setText(dir);
}
