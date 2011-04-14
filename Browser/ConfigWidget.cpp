/*
 * Copyright (c) 2011, Torch Mobile (Beijing) Co. Ltd.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms,
 * with or without modification, are permitted provided that
 * the following conditions are met:
 *
 *  -- Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *  -- Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *  -- Neither the name of the Torch Mobile (Beijing) Co. Ltd. nor the
 * names of its contributors may be used to endorse or promote products
 * derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "ConfigWidget.h"
#include "ui_ConfigWidget.h"

#include "Constant.h"
#include <QDir>
#include <QMessageBox>
#include <QSettings>
#include <WebSettings.h>

namespace Olympia {
namespace Browser {

void createPathIfNeeded(const QString& path)
{
    QDir dir(path);
    if (!dir.exists())
        dir.mkpath(path);
}

// All logic should implemented in this class,using different ui files (*.ui, ui_*.h)
// to get different look for different platform.

ConfigWidget::ConfigWidget(QWidget* parent)
    : QWidget(parent),
    ui(new ::Ui::ConfigWidget)
{
    ui->setupUi(this);

    readToWebSettings();
    readToUI();

    connect(ui->comboBoxStartUpPage, SIGNAL(activated(const QString&)),
        this, SLOT(slotStartupPageChanged(const QString&)));
    connect(ui->pushButtonUseCurrent, SIGNAL(clicked()), SIGNAL(sigReqCurrentPageUrl()));
    connect(ui->pushButtonResetDefault, SIGNAL(clicked()), this, SLOT(restoreToDefault()));
    connect(ui->pushButtonSave, SIGNAL(clicked()), this, SLOT(saveChanges()));
    connect(ui->pushButtonCancel, SIGNAL(clicked()), this, SLOT(cancelChanges()));
    connect(ui->pushButtonClearNow, SIGNAL(clicked()), this, SLOT(clearBrowsingDataStarted()));
}

ConfigWidget::~ConfigWidget()
{
    delete ui;
}

void ConfigWidget::saveSettings()
{
    createPathIfNeeded(BASEPATH);

    // save settings UI value both into WebSettings and QSettings, and save them into .ini file in one action.
    QSettings settings(CONF_FILEPATH, QSettings::IniFormat);

    settings.beginGroup("generalConf");
    settings.setValue("startupPage", ui->comboBoxStartUpPage->currentText());
    settings.setValue("homePage", ui->lineEditHomePage->text());
    settings.setValue("enableShortcuts", ui->checkBoxEnableShortcuts->isChecked());
    settings.setValue("promptCloseBroser", ui->checkBoxPromptCloseBrowser->isChecked());
    settings.setValue("promptCloseTabs", ui->checkBoxPromptCloseTabs->isChecked());
    settings.setValue("promptStreamMedia", ui->checkBoxPromptStreamMedia->isChecked());
    settings.endGroup();

    settings.beginGroup("webContent");
    WebKit::WebSettings::globalSettings()->setLoadsImagesAutomatically(ui->checkBoxLoadImages->isChecked());
    settings.setValue("loadImages", ui->checkBoxLoadImages->isChecked());
    WebKit::WebSettings::globalSettings()->setJavaScriptEnabled(ui->checkBoxEnableJS->isChecked());
    settings.setValue("enableJS", ui->checkBoxEnableJS->isChecked());
    WebKit::WebSettings::globalSettings()->setJavaScriptOpenWindowsAutomatically(!ui->checkBoxBlockPopups->isChecked());
    settings.setValue("blockPopups", ui->checkBoxBlockPopups->isChecked());
    WebKit::WebSettings::globalSettings()->setPluginsEnabled(ui->checkBoxEnableMedia->isChecked());
    settings.setValue("enableMedia", ui->checkBoxEnableMedia->isChecked());
    WebKit::WebSettings::globalSettings()->setDefaultFontSize(ui->comboBoxFontSize->currentText().toInt());
    WebKit::WebSettings::globalSettings()->setDefaultFixedFontSize(ui->comboBoxFontSize->currentText().toInt());
    settings.setValue("fontSize", ui->comboBoxFontSize->currentText());
    WebKit::WebSettings::globalSettings()->setDefaultTextEncodingName(ui->comboBoxTextEncoding->currentText().toLatin1());
    settings.setValue("textEncoding", ui->comboBoxTextEncoding->currentText());
    settings.setValue("autoTextEncoding", ui->checkBoxAutoTextEncoding->isChecked());
    settings.endGroup();

    settings.beginGroup("privacyAndSecurity");
    settings.setValue("acceptCookie", ui->checkBoxAcceptCookie->isChecked());
    WebKit::WebSettings::globalSettings()->setGeolocationEnabled(ui->checkBoxEnableGeo->isChecked());
    settings.setValue("enableGeo", ui->checkBoxEnableGeo->isChecked());
    settings.endGroup();

    settings.sync();
}

void ConfigWidget::readToWebSettings()
{
    createPathIfNeeded(BASEPATH);

    // load the setting fields related with WebSettings from .ini file into WebSettings::globalSettings(),
    // others stay in QSetting object, and used only by browser application.
    QSettings settings(CONF_FILEPATH, QSettings::IniFormat);

    settings.beginGroup("webContent");
    WebKit::WebSettings::globalSettings()->setLoadsImagesAutomatically(settings.value("loadImages", true).toBool());
    WebKit::WebSettings::globalSettings()->setJavaScriptEnabled(settings.value("enableJS", true).toBool());
    WebKit::WebSettings::globalSettings()->setJavaScriptOpenWindowsAutomatically(!settings.value("blockPopups", true).toBool());
    WebKit::WebSettings::globalSettings()->setPluginsEnabled(settings.value("enableMedia", false).toBool());
    WebKit::WebSettings::globalSettings()->setDefaultFontSize(settings.value("fontSize", "16").toInt());
    WebKit::WebSettings::globalSettings()->setDefaultFixedFontSize(settings.value("fontSize", "16").toInt());
    WebKit::WebSettings::globalSettings()->setDefaultTextEncodingName(settings.value("textEncoding", "ISO-8859-1").toString().toLatin1());
    settings.endGroup();

    settings.beginGroup("privacyAndSecurity");
    // seems we don't have AcceptCookie related configuration field in WebSettings
    WebKit::WebSettings::globalSettings()->setGeolocationEnabled(settings.value("enableGeo", true).toBool());
    settings.endGroup();

    // we don't need to save the clear state according with the BB browser's behavior. so that's all.
}

void ConfigWidget::readToUI()
{
    createPathIfNeeded(BASEPATH);

    // read setting fields related with WebSettings from WebSettings::globalSettings(),
    // and load other settings from QSettings object.
    // because we suppose that the from WebSettings::globalSettings() could be modified outside the ConfigWidget.
    QSettings settings(CONF_FILEPATH, QSettings::IniFormat);

    settings.beginGroup("generalConf");
    int index = ui->comboBoxStartUpPage->findText(settings.value("startupPage", "Start Page").toString());
    if (index != -1)
        ui->comboBoxStartUpPage->setCurrentIndex(index);
    slotStartupPageChanged(ui->comboBoxStartUpPage->currentText());
    ui->lineEditHomePage->setText(settings.value("homePage", QString("about:blank")).toString());
    ui->checkBoxEnableShortcuts->setChecked(settings.value("enableShortcuts", false).toBool());
    ui->checkBoxPromptCloseBrowser->setChecked(settings.value("promptCloseBroser", false).toBool());
    ui->checkBoxPromptCloseTabs->setChecked(settings.value("promptCloseTabs", true).toBool());
    ui->checkBoxPromptStreamMedia->setChecked(settings.value("promptStreamMedia", true).toBool());
    settings.endGroup();

    settings.beginGroup("webContent");
    ui->checkBoxLoadImages->setChecked(WebKit::WebSettings::globalSettings()->loadsImagesAutomatically());
    ui->checkBoxEnableJS->setChecked(WebKit::WebSettings::globalSettings()->isJavaScriptEnabled());
    ui->checkBoxBlockPopups->setChecked(!WebKit::WebSettings::globalSettings()->canJavaScriptOpenWindowsAutomatically());
    ui->checkBoxEnableMedia->setChecked(WebKit::WebSettings::globalSettings()->arePluginsEnabled());
    index = ui->comboBoxFontSize->findText(QString::number(WebKit::WebSettings::globalSettings()->defaultFontSize()));
    if (index != -1)
        ui->comboBoxFontSize->setCurrentIndex(index);
    // WebSettings only have set but not get function of textEncoding.
    index = ui->comboBoxTextEncoding->findText(settings.value("textEncoding", "ISO-8859-1").toString());
    if (index != -1)
        ui->comboBoxTextEncoding->setCurrentIndex(index);
    ui->checkBoxAutoTextEncoding->setChecked(settings.value("autoTextEncoding", true).toBool());
    settings.endGroup();

    settings.beginGroup("privacyAndSecurity");
    // seems we don't have AcceptCookie related configuration field in WebSettings
    ui->checkBoxAcceptCookie->setChecked(settings.value("acceptCookie", true).toBool());
    ui->checkBoxEnableGeo->setChecked(WebKit::WebSettings::globalSettings()->isGeolocationEnabled());
    settings.endGroup();

    ui->checkBoxClearPasswords->setChecked(true);
    ui->checkBoxClearHistory->setChecked(true);
    ui->checkBoxClearCookies->setChecked(false);
    ui->checkBoxClearCache->setChecked(true);
    ui->checkBoxClearPushContent->setChecked(false);
}

void ConfigWidget::restoreToDefault()
{
    QMessageBox::StandardButton ret = QMessageBox::question(this, "Restore Default Configurations"
        , "Do you really want to restore\nthe default configurations?"
        , QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Ok);

    if (ret == QMessageBox::Cancel)
        return;

    // generalConf
    ui->comboBoxStartUpPage->setCurrentIndex(0);
    ui->lineEditHomePage->setText("about:blank");
    ui->checkBoxEnableShortcuts->setChecked(false);
    ui->checkBoxPromptCloseBrowser->setChecked(false);
    ui->checkBoxPromptCloseTabs->setChecked(true);
    ui->checkBoxPromptStreamMedia->setChecked(true);

    // webContent
    ui->checkBoxLoadImages->setChecked(true);
    ui->checkBoxEnableJS->setChecked(true);
    ui->checkBoxBlockPopups->setChecked(true);
    ui->checkBoxEnableMedia->setChecked(false);
    ui->comboBoxFontSize->setCurrentIndex(15); // default font size set to 16.
    ui->comboBoxTextEncoding->setCurrentIndex(0);
    ui->checkBoxAutoTextEncoding->setChecked(true);

    // privacyAndSecurity
    ui->checkBoxAcceptCookie->setChecked(true);
    ui->checkBoxEnableGeo->setChecked(true);

    // clear state.
    ui->checkBoxClearPasswords->setChecked(true);
    ui->checkBoxClearHistory->setChecked(true);
    ui->checkBoxClearCookies->setChecked(false);
    ui->checkBoxClearCache->setChecked(true);
    ui->checkBoxClearPushContent->setChecked(false);
}

void ConfigWidget::slotSetHomePageUrl(const QString& url)
{
    if (url.isEmpty())
        ui->lineEditHomePage->setText("about:blank");
    else
        ui->lineEditHomePage->setText(url);
}

void ConfigWidget::clearBrowsingDataStarted()
{
    emit sigClearBrowsingData(
        ui->checkBoxClearPasswords->isChecked(),
        ui->checkBoxClearHistory->isChecked(),
        ui->checkBoxClearCookies->isChecked(),
        ui->checkBoxClearCache->isChecked(),
        ui->checkBoxClearPushContent->isChecked()
        );
}

void ConfigWidget::slotClearBrowsingDataFinished()
{
    //clear done.
    QMessageBox::information(this, "Clear Memory", "Memory Cleared.");
}

void ConfigWidget::saveChanges()
{
    saveSettings();

    QMessageBox::information(this, "Saving Configurations", "Configurations Saved.");
    close();
}

void ConfigWidget::cancelChanges()
{
    readToUI();
    close();
}

void ConfigWidget::slotStartupPageChanged(const QString& text)
{
    bool enabled(false);
    if (text == QString("Home Page"))
        enabled = true;
    ui->lineEditHomePage->setEnabled(enabled);
    ui->pushButtonUseCurrent->setEnabled(enabled);
}

void ConfigWidget::changeEvent(QEvent* e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

} // namespace Browser
} // namespace Olympia
