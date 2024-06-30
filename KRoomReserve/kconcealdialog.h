// ------------------------------------------------------------------
// kconcealdialog.h
// 创建者： 李康
// 创建时间： 2024/4/20
// 功能描述： 会议室预约系统——研发人员取消预约的对话框
// ------------------------------------------------------------------

#ifndef _KROOMRESERVE_KCONCEALDIALOG_H_
#define _KROOMRESERVE_KCONCEALDIALOG_H_

#include <QDialog>
#include <QVector>
#include <QListWidgetItem>
#include <QMessageBox>

#include "ui_kconcealdialog.h"
#include "kdatabase.h"

namespace Ui { class KConcealDialog; };

class KConcealDialog : public QDialog
{
	Q_OBJECT

public:
	friend class KRoomReserveUnitTest;
	KConcealDialog(const QVector<KPersonalReserve> &reservations, QWidget *parent = Q_NULLPTR);
	~KConcealDialog();

signals:
	void confirmed(int reservationID);

private:
	KConcealDialog(const KConcealDialog &other) = delete;
	KConcealDialog &operator=(const KConcealDialog &other) = delete;

	Ui::KConcealDialog *ui;
};

#endif // _KROOMRESERVE_KCONCEALDIALOG_H_
