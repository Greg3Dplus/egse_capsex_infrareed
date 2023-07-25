#ifndef SENDCMDDIALOG_H
#define SENDCMDDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QDebug>
#include <QPushButton>
#include <QSpinBox>
#include <QLineEdit>
#include <QValidator>
#include <QEvent>
#include <QGridLayout>
#include <QPlainTextEdit>
#include <QSpacerItem>
#include <QGroupBox>
#include <QFrame>
#include <QMessageBox>


typedef struct TMTC_params {
    quint8 nb_data;
    quint32 addr;
    QString data_pkts;
} TMTC_params;

// Reset cursor to position zero for transmit packet QLineEdit during focus-in
class ReturnOnFocus : public QObject {
   Q_OBJECT
   /// Catches FocusIn events on the target line edit, and appends a call
   /// to resetCursor at the end of the event queue.
   bool eventFilter(QObject * obj, QEvent * ev) {
      QLineEdit * w = qobject_cast<QLineEdit*>(obj);
      // w is nullptr if the object isn't a QLineEdit
      if (w && (ev->type() == QEvent::FocusIn || ev->type() == QEvent::MouseButtonRelease)) {
         QMetaObject::invokeMethod(this, "resetCursor",
                                   Qt::QueuedConnection, Q_ARG(QWidget*, w));
      }
      // A base QObject is free to be an event filter itself
      return QObject::eventFilter(obj, ev);
   }
   // Q_INVOKABLE is invokable, but is not a slot
   /// Resets the cursor position of a given widget.
   /// The widget must be a line edit.
   Q_INVOKABLE void resetCursor(QWidget * w) {
      static_cast<QLineEdit*>(w)->setCursorPosition(0);
   }
public:
   ReturnOnFocus(QObject * parent = nullptr) : QObject(parent) {}
   /// Installs the reset functionality on a given line edit
   void installOn(QLineEdit * ed) { ed->installEventFilter(this); }
};


class sendcmddialog : public QDialog
{
    Q_OBJECT

public:
    explicit sendcmddialog();
    ~sendcmddialog();

    TMTC_params get_TM_params() const;
    TMTC_params get_TC_params() const;
    void set_TM_params(const QString &received_pkt, bool reset);
    void set_TC_params(const QString &TC_txt, bool reset);


private slots:
    void on_TM_pushButton_clicked();
    void on_TC_pushButton_clicked();
    void on_sendcmddialog_finished(void);

    void on_TC_addr_valueChanged(int value);

    void on_TM_nbdata_valueChanged(int value);

    void on_TM_addr_valueChanged(int value);

    void on_transmitpkts_textChanged();

signals:
    void TM_clicked();
    void TC_clicked();
    void TM_closed();

private:
    TMTC_params TM_params;
    TMTC_params TC_params;

    bool send_TM_clicked;
    bool send_TC_clicked;

    ReturnOnFocus m_return;

    QGridLayout *mainLayout;
    QGridLayout *readLayout;
    QGridLayout *writeLayout;
    QGridLayout *TMLayout;
    QGridLayout *TCLayout;

    QGroupBox *TMbox;
    QGroupBox *TCbox;

    QPlainTextEdit *receivedpkts;

    QSpinBox *TM_addr;
    QSpinBox *TM_nbdata;
    QSpinBox *TC_addr;

    QPushButton *TM_pushButton;
    QPushButton *TC_pushButton;

    QLineEdit *transmitpkts;

    QLabel *TC_status;

};


#endif // SENDCMDDIALOG_H
