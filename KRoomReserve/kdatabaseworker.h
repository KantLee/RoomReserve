// -----------------------------------------------------------
// kdatabaseworker.h
// 创建者： 李康
// 创建时间： 2024/5/11
// 功能描述： 会议室预约系统——异步执行数据库查询操作
// -----------------------------------------------------------

#ifndef _KROOMRESERVE_DATABASEWORKER_H_
#define _KROOMRESERVE_DATABASEWORKER_H_

#include <QObject>
#include "kdatabase.h"

class KDatabaseWorker : public QObject 
{
    Q_OBJECT

public:
    friend class KRoomReserveUnitTest;
    explicit KDatabaseWorker(QObject* parent = nullptr);
    ~KDatabaseWorker();

public slots:
    void performRoomQuery();
    void performCoderQuery();
    void performAdminQuery();
    void performApprovalReservationQuery();
    void performSearchRooms(const QString week, const QString day);
    void performPersonalReserveQuery(const QString userName, const bool concealReserveEnabled);

signals:
    void roomsQueryFinished(const QString results);
    void coderQueryFinished(const QString results);
    void adminsQueryFinished(const QString results);
    void approvalReservationQueryFinished(const QVector<KWaitApproval> results);
    void searchRoomsFinished(const QString results, const QString week, const QString day);
    void personalReserveQueryFinished(const QVector<KPersonalReserve> results);
    void personalReserveQueryFinishedNeedConceal(const QVector<KPersonalReserve> results);
};

#endif // _KROOMRESERVE_DATABASEWORKER_H_
