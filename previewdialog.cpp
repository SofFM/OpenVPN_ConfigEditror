#include "previewdialog.h"
#include "ui_previewdialog.h"

PreviewDialog::PreviewDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PreviewDialog)
{
    ui->setupUi(this);

    this->setWindowTitle("Preview"); //устанавливаем заголовок
    this->setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint); //убираем знак вопроса
}

PreviewDialog::~PreviewDialog()
{
    delete ui;
}

void PreviewDialog::setPreviewText(QString text)
{
    ui->previewPlainTextEdit->setPlainText(text);
}

void PreviewDialog::on_okPushButton_released()
{
    done(0);
}
