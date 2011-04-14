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

#ifndef AddressBar_h
#define AddressBar_h

#include <QAbstractButton>
#include <QLineEdit>

class QLabel;
class QMovie;
class QPushButton;
class QSqlQueryModel;
class QStyleOptionFrameV2;

namespace Olympia {
namespace Browser {

class MatchedHistView;

//////////////////////////////////////////////////////////////////
class ClearButton : public QAbstractButton
{
    Q_OBJECT

public:
    ClearButton(QWidget* parent = 0);
    void paintEvent(QPaintEvent* event);

public slots:
    void textChanged(const QString& text);
};

//////////////////////////////////////////////////////////////////
class ExLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    ExLineEdit(QWidget* parent = 0) : QLineEdit(parent) { }

signals:
    void sigFocusOut();

private:
    void focusOutEvent(QFocusEvent* event);
};

//////////////////////////////////////////////////////////////////
class AddressBar : public QFrame
{
    Q_OBJECT

public:
    AddressBar(QWidget* parent = 0);
    virtual ~AddressBar();
    void updateSize(const QSize& size);
    virtual void setUrlText(const QString& url);

    void setMatchedHistModel(QSqlQueryModel* model);

public slots:
    void slotReturnPressedInAddrBar(void);
    void slotUpdateMachedHistItems(void);
    void slotLaunchURL(const QString& url);
    void slotSetUrlIcon(const QString&, const QString&, const QIcon& icon);
    void slotLoadFinished();
    void slotUseDefaultUrlIcon();

signals:
    void sigGo(const QString& url);
    void sigMatchInput(const QString& word);

private slots:
    void slotTextEdited(const QString&);
    void slotClearContents();

private:
    void keyPressEvent(QKeyEvent* event);
    void resizeEvent(QResizeEvent* event);
    void paintEvent(QPaintEvent* event);

    void initStyleOption(QStyleOptionFrameV2& option);

    QLabel* m_urlIconLabel;
    QMovie* m_loadingMovie;
    ExLineEdit* m_lineEdit;
    ClearButton* m_clearBtn;
    MatchedHistView* m_matchedHistView;
};

} // Browser
} // Olympia
#endif // AddressBar_h

