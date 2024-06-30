// ----------------------------------------------------------
// kapprovalwidget.h
// 创建者： 李康
// 创建时间： 2024/4/20
// 功能描述： 会议室预约系统——行政人员操作界面
// ----------------------------------------------------------

#ifndef _KROOMRESERVE_KAPPROVALWIDGET_H_
#define _KROOMRESERVE_KAPPROVALWIDGET_H_

#include <QWidget>
#include <QMessageBox>
#include <QListWidgetItem>
#include <QThread>

#include "ui_kapprovalwidget.h"
#include "kdatabase.h"
#include "kdatabaseworker.h"

namespace Ui { class KApprovalWidget; };

class KApprovalWidget : public QWidget
{
	Q_OBJECT

public:
	friend class KRoomReserveUnitTest;
	explicit KApprovalWidget(QWidget *parent = Q_NULLPTR);
	~KApprovalWidget();
	void approvalReservation(int reservationID, bool isPassed);

public slots:
	void on_logout_clicked();
	void on_showAllReservation_clicked();
	void on_showApprovalReserve_clicked();
	void handleAllReservationQueryResults(const QString results);
	void handleApprovalReservationQueryResults(const QVector<KWaitApproval> results);

signals:
	void logout();
	void startAllReservationQuery();
	void startApprovalReservationQuery();

private:
	KApprovalWidget(const KApprovalWidget &other) = delete;
	KApprovalWidget &operator=(const KApprovalWidget &other) = delete;
	Ui::KApprovalWidget *ui;
	KDataBase &m_database;
	QThread m_queryThread;
	KDatabaseWorker *m_worker;
};

#endif // _KROOMRESERVE_KAPPROVALWIDGET_H_
