// ---------------------------------------------------------
// kcoderwidget.h
// 创建者： 李康
// 创建时间： 2024/4/20
// 功能描述： 会议室预约系统——研发人员操作界面
// ---------------------------------------------------------

#ifndef _KROOMRESERVE_KCODERWIDGET_H_
#define _KROOMRESERVE_KCODERWIDGET_H_

#include <QWidget>
#include <QMessageBox>
#include <QThread>

#include "ui_kcoderwidget.h"
#include "kdatabase.h"
#include "kconcealdialog.h"
#include "kdatabaseworker.h"

namespace Ui { class KCoderWidget; };

class KCoderWidget : public QWidget
{
	Q_OBJECT

public:
	friend class KRoomReserveUnitTest;
	KCoderWidget(QString userName, QWidget *parent = Q_NULLPTR);
	~KCoderWidget();

public slots:
	void on_logout_clicked();
	void on_searchRooms_clicked();
	void on_reserveRoom_clicked();
	void on_showPersonalReserve_clicked();
	void on_concealReserve_clicked();
	void on_showAllReserve_clicked();
	void concealReservation(int reservationID);
	void handleSearchRooms(const QString results, const QString week, const QString day);
	void handleShowPersonalReserve(const QVector<KPersonalReserve> results);
	void handleConcealReserve(const QVector<KPersonalReserve> results);
	void handleShowAllReserve(const QString results);

signals:
	void logout();
	void startSearchRooms(const QString week, const QString day);
	void startShowPersonalReserve(const QString userName, const bool concealReserveEnabled);
	void startConcealReserve(const QString userName, const bool concealReserveEnabled);
	void startShowAllReserve();

private:
	KCoderWidget(const KCoderWidget &other) = delete;
	KCoderWidget &operator=(const KCoderWidget &other) = delete;

	Ui::KCoderWidget *ui;
	KDataBase &m_database;
	QString m_userName;
	QThread m_queryThread;
	KDatabaseWorker *m_worker;
};

#endif // _KROOMRESERVE_KCODERWIDGET_H_
