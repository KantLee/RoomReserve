// -----------------------------------------------------------
// kadminwidget.h
// 创建者： 李康
// 创建时间： 2024/4/20
// 功能描述： 会议室预约系统——系统管理员操作界面
// -----------------------------------------------------------

#ifndef _KROOMRESERVE_KADMINWIDGET_H_
#define _KROOMRESERVE_KADMINWIDGET_H_

#include <QWidget>
#include <QMessageBox>
#include <QThread>

#include "ui_kadminwidget.h"
#include "kdatabase.h"
#include "kdatabaseworker.h"

namespace Ui { class KAdminWidget; };

class KAdminWidget : public QWidget
{
	Q_OBJECT

public:
	friend class KRoomReserveUnitTest;
	explicit KAdminWidget(QWidget *parent = Q_NULLPTR);
	~KAdminWidget();

public slots:
	void on_logoutButton_clicked();
	void on_addPeople_clicked();
	void on_getCoders_clicked();
	void on_getAdmins_clicked();
	void on_showRooms_clicked();
	void on_clearAllReserve_clicked();
	void handleRoomQueryResults(const QString results);
	void handleCoderQueryResults(const QString results);
	void handleAdminsQueryResults(const QString results);

signals:
	void logout();
	void startRoomQuery();
	void startCoderQuery();
	void startAdminsQuery();

private:
	// 私有拷贝构造函数
	KAdminWidget(const KAdminWidget &other) = delete;
	// 私有赋值运算符
	KAdminWidget &operator=(const KAdminWidget &other) = delete;

	Ui::KAdminWidget *ui;
	KDataBase &m_database;
	QThread m_queryThread;
	KDatabaseWorker *m_worker;
};

#endif // _KROOMRESERVE_KADMINWIDGET_H_
