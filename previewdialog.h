#ifndef PREVIEWDIALOG_H
#define PREVIEWDIALOG_H

#include <QDialog>

namespace Ui {
class PreviewDialog;
}

class PreviewDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PreviewDialog(QWidget *parent = nullptr);
    ~PreviewDialog();

    void setPreviewText(QString text);

private slots:
    void on_okPushButton_released();

private:
    Ui::PreviewDialog *ui;
};

#endif // PREVIEWDIALOG_H
