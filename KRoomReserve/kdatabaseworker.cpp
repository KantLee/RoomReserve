#include "kdatabaseworker.h"

KDatabaseWorker::KDatabaseWorker(QObject *parent)
	: QObject(parent)
{
}

KDatabaseWorker::~KDatabaseWorker()
{
}

// 异步查询会议室信息
void KDatabaseWorker::performRoomQuery()
{
	QString results = KDataBase::getInstance().getAllRooms();
	emit roomsQueryFinished(results);
}

// 异步查询研发人员信息
void KDatabaseWorker::performCoderQuery()
{
	QString results = KDataBase::getInstance().getAllUsers("研发人员");
	emit coderQueryFinished(results);
}

// 异步查询行政人员信息
void KDatabaseWorker::performAdminQuery()
{
	QString results = KDataBase::getInstance().getAllUsers("行政人员");
	emit coderQueryFinished(results);
}

// 异步查询待审批的预约信息
void KDatabaseWorker::performApprovalReservationQuery()
{
	QVector<KWaitApproval> results = KDataBase::getInstance().getWaitApprovalReserve();
	emit approvalReservationQueryFinished(results);
}

// 异步查询指定时间段的会议室状态
void KDatabaseWorker::performSearchRooms(const QString week, const QString day)
{
	QString results = KDataBase::getInstance().getTheRooms(week, day);
	emit searchRoomsFinished(results, week, day);
}

// 异步查询我的预约信息
void KDatabaseWorker::performPersonalReserveQuery(const QString userName, const bool concealReserveEnabled)
{
	QVector<KPersonalReserve> results = KDataBase::getInstance().getPersonalReserve(userName);
	if (concealReserveEnabled)
		emit personalReserveQueryFinishedNeedConceal(results);
	else
		emit personalReserveQueryFinished(results);
}
