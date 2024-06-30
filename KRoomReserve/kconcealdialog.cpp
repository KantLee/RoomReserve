#include "kconcealdialog.h"

KConcealDialog::KConcealDialog(const QVector<KPersonalReserve> &reservations, QWidget *parent)
	: QDialog(parent)
{
	ui = new Ui::KConcealDialog();
	ui->setupUi(this);
    for (const auto &reservation : reservations)
    {
        QString displayText = QString("预约ID: %1, 会议室: %2, 预约时间: %3 %4, 状态: %5")
            .arg(reservation.reserveID)
            .arg(reservation.roomName)
            .arg(reservation.week)
            .arg(reservation.day)
            .arg(reservation.status);
        QListWidgetItem *item = new QListWidgetItem(displayText, ui->listWidget);
        item->setData(Qt::UserRole, reservation.reserveID); // 存储预约ID
        item->setData(Qt::UserRole + 1, reservation.status); // 存储预约状态
    }
    connect(ui->confirmButton, &QPushButton::clicked, [this]() {
        // 在此匿名函数中，现在可以使用 this->ui->listWidget 来访问listWidget
        if (ui->listWidget->selectedItems().isEmpty())
        {
            QMessageBox::warning(this, "取消预约", "请先选择一个预约以进行取消");
        }
        else
        {
            for (QListWidgetItem *item : ui->listWidget->selectedItems())
            {
                QString status = item->data(Qt::UserRole + 1).toString();
                if (status == "预约取消")
                {
                    QMessageBox::warning(this, "取消预约", "选中的预约已经是取消状态，无法再次取消");
                }
                else
                {
                    emit confirmed(item->data(Qt::UserRole).toInt());
                    accept();  // 关闭对话框
                    break;
                }
            }
        }
    });
    connect(ui->cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

KConcealDialog::~KConcealDialog()
{
	delete ui;
}
